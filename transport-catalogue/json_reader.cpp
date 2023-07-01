#include "json_reader.h"

using namespace std::literals;

namespace json_reader {

JsonReader::JsonReader(TransportCatalogue& catalogue, std::istream& input)
    : catalogue_(catalogue)
{
    JsonReader::ParseJSON(input);
}

void JsonReader::UpdateCatalogue()
{
    if (!request_queue_.stops_requests.empty())
    {
        for (const domain::StopRequest& request : request_queue_.stops_requests)
        {
            ProcessingStopRequest(request);
        }

        for (const domain::StopRequest& request : request_queue_.stops_requests)
        {
            ProcessingDistances(request);
        }
    }

    if (!request_queue_.buses_requests.empty())
    {
        for (const domain::BusRequest& request : request_queue_.buses_requests)
        {
            ProcessingBusRequest(request);
        }
    }
}

void JsonReader::PrintStat(std::ostream& output)
{
    json::Print(json::Document{ComputeJSON()}, output);
}

const domain::RequestQueue& JsonReader::GetRequestQueue() const
{
    return request_queue_;
}

map_renderer::RenderSettingsRequest JsonReader::GetRenderSettings() const
{
    return render_settings_;
}

void JsonReader::ParseStopRequest(const json::Node& stop_request)
{
    const json::Dict& request = stop_request.AsDict();

    const std::string name = request.at("name").AsString();
    const double lat = request.at("latitude").AsDouble();
    const double lng = request.at("longitude").AsDouble();

    std::map<std::string, int> dists = {};
    if (request.count("road_distances"))
    {
        for (const auto& [stop, dist] : 
            request.at("road_distances").AsDict())
        {
            dists[stop] = dist.AsInt();
        }
    }

    request_queue_.stops_requests.push_back({name, lat, lng, dists});
}
            
void JsonReader::ParseBusRequest(const json::Node& bus_request)
{
    const json::Dict& request = bus_request.AsDict();

    const std::string name = request.at("name").AsString();
    const bool is_round = request.at("is_roundtrip").AsBool();

    std::vector<std::string> stops = {};
    for (const json::Node& stop : request.at("stops").AsArray())
    {
        stops.push_back(stop.AsString());
    }

    request_queue_.buses_requests.push_back({name, stops, is_round});
}

void JsonReader::ParseBaseRequests(const json::Node& base_requests)
{
    const json::Array& requests = base_requests.AsArray();

    for (const json::Node& request : requests)
    {
        if (request.AsDict().at("type").AsString() == "Stop")
        {
            ParseStopRequest(request);
            continue;
        }

        if (request.AsDict().at("type").AsString() == "Bus")
        {
            ParseBusRequest(request);
            continue;
        }
    }
}

void JsonReader::ParseStatRequest(const json::Node& stat_request)
{
    const json::Dict& request = stat_request.AsDict();

    const int id = request.at("id").AsInt();
    const std::string type = request.at("type").AsString();

    if (request.count("name") != 0)
    {
        const std::string name = request.at("name").AsString();
        request_queue_.stats_requests.push_back({id, type, name});

        return;
    }

    request_queue_.stats_requests.push_back({id, type, ""});
}

void JsonReader::ParseStatRequests(const json::Node& stat_requests)
{
    const json::Array& requests = stat_requests.AsArray();

    for (const json::Node& request : requests)
    {
        ParseStatRequest(request);
    }
}

svg::Color JsonReader::FormatColor(const json::Node& color)
{
    try
    {
        if (color.IsString())
        {
            return color.AsString();
        }
        
        json::Array color_arr = color.AsArray();
        if (color_arr.size() == 4)
        {
            svg::Rgba color_rgba{color_arr.at(0).AsInt(),
                color_arr.at(1).AsInt(), color_arr.at(2).AsInt(),
                color_arr.at(3).AsDouble()};

            return color_rgba;
        }


        if (color_arr.size() == 3)
        {
            svg::Rgb color_rgb{color_arr.at(0).AsInt(), color_arr.at(1).AsInt(),
                color_arr.at(2).AsInt()};
            
            return color_rgb;
        }
    }
    catch (const std::invalid_argument&)
    {
        std::cout << "Unsupported color format" << std::endl;
    }

    return std::monostate{};
}

void JsonReader::ParseRenderSettings(const json::Node& render_settings)
{
    const json::Dict& request = render_settings.AsDict();

    render_settings_.width = request.at("width").AsDouble();
    render_settings_.height = request.at("height").AsDouble();
    render_settings_.padding = request.at("padding").AsDouble();
    render_settings_.line_width = request.at("line_width").AsDouble();
    render_settings_.stop_radius = request.at("stop_radius").AsDouble();
    render_settings_.bus_label_font_size =
        request.at("bus_label_font_size").AsInt();
    
    const json::Array& bus_label_offset =
        request.at("bus_label_offset").AsArray();
    render_settings_.bus_label_offset = {bus_label_offset.at(0).AsDouble(),
        bus_label_offset.at(1).AsDouble()};

    render_settings_.stop_label_font_size =
        request.at("stop_label_font_size").AsInt();
    
    const json::Array& stop_label_offset =
        request.at("stop_label_offset").AsArray();
    render_settings_.stop_label_offset = {stop_label_offset.at(0).AsDouble(),
        stop_label_offset.at(1).AsDouble()};

    render_settings_.underlayer_color =
        FormatColor(request.at("underlayer_color"));
    
    render_settings_.underlayer_width =
        request.at("underlayer_width").AsDouble();
    
    const json::Array& color_palette = request.at("color_palette").AsArray();
    for (const json::Node& color : color_palette)
    {
        render_settings_.color_palette.push_back(FormatColor(color));
    }
}

void JsonReader::ParseRoutingSettings(const json::Node& routing_settings)
{
    const json::Dict& request = routing_settings.AsDict();

    int wait_time = request.at("bus_wait_time").AsInt();
    double velocity = request.at("bus_velocity").AsDouble();

    if ((wait_time < 1 || wait_time > 1000) ||
        (velocity < 1.0 || velocity > 1000.0))
    {
        throw std::logic_error("Invalid value in router_settings");
    }

    routing_settings_.bus_wait_time = wait_time;
    routing_settings_.bus_velocity = velocity;
}

void JsonReader::ParseJSON(std::istream& input)
{
    const json::Document json_data = json::Load(input);
    const json::Dict& requests = json_data.GetRoot().AsDict();

    if (requests.count("base_requests"))
    {
        ParseBaseRequests(requests.at("base_requests"));
    }

    if (requests.count("stat_requests"))
    {
        ParseStatRequests(requests.at("stat_requests"));
    }

    if (requests.count("render_settings"))
    {
        ParseRenderSettings(requests.at("render_settings"));
    }

    if (requests.count("routing_settings"))
    {
        ParseRoutingSettings(requests.at("routing_settings"));
    }
}

void JsonReader::ProcessingStopRequest(const domain::StopRequest& request)
{
    catalogue_.AddStop(request.name, {request.lat, request.lng});
}

void JsonReader::ProcessingDistances(const domain::StopRequest& request)
{
    if (!request.dists.empty())
    {
        for (const auto& [stop_to, dist] : request.dists)
        {
            catalogue_.AddDistance(request.name, stop_to, dist);
        }
    }
}

void JsonReader::ProcessingBusRequest(const domain::BusRequest& request)
{
    std::vector<std::string> stops = request.stops;
    if (!request.is_round)
    {
        for (auto it = std::next(request.stops.rbegin(), 1);
            it != request.stops.rend(); ++it)
        {
            stops.push_back(*it);
        }
    }

    catalogue_.AddBus(request.name, stops, request.is_round);
}

void JsonReader::ComputeStatRequest(json::Builder& builder,
    const domain::StatRequest& request)
{
    if (request.type == "Stop")
    {
        try
        {
            std::set<std::string_view> buses_to_stop = 
                catalogue_.GetBusesToStop(request.name);

            builder.StartDict().Key("buses"s).StartArray();
            for (std::string_view bus : buses_to_stop)
            {
                std::string bus_name(bus); 
                builder.Value(bus_name);
            }

            builder.EndArray().Key("request_id"s).Value(request.id).EndDict();
        }
        catch (const std::invalid_argument&)
        {
            builder.StartDict().Key("request_id"s).Value(request.id)
                .Key("error_message"s).Value("not found"s).EndDict();
        }
    }
    else if (request.type == "Bus")
    {
        try
        {
            const double curvature = catalogue_.ComputeCurvature(request.name);
            const int request_id = request.id;
            const double route_length =
                catalogue_.ComputeRouteLength(request.name);
            const int stop_count = catalogue_.ComputeStopsCount(request.name);
            const int unique_stop_count =
                catalogue_.ComputeUniqueStopsCount(request.name);

            builder.StartDict().Key("curvature"s).Value(curvature)
                .Key("request_id"s).Value(request_id)
                .Key("route_length"s).Value(route_length)
                .Key("stop_count"s).Value(stop_count)
                .Key("unique_stop_count"s).Value(unique_stop_count).EndDict();
        }
        catch (const std::invalid_argument&)
        {
            builder.StartDict().Key("request_id"s).Value(request.id)
                .Key("error_message"s).Value("not found"s).EndDict();
        }
    }
    else if (request.type == "Map")
    {
        try
        {
            const map_renderer::MapRenderer renderer(render_settings_);
            const request_handler::RequestHandler handler(catalogue_, renderer);

            svg::Document map = handler.RenderMap();
            std::stringstream temp;
            map.Render(temp);

            builder.StartDict().Key("map"s).Value(temp.str())
                .Key("request_id"s).Value(request.id).EndDict();
        }
        catch (const std::invalid_argument&)
        {
            builder.StartDict().Key("request_id"s).Value(request.id)
                .Key("error_message"s).Value("not found"s).EndDict();
        }
    }
}

json::Node JsonReader::ComputeJSON()
{
    json::Builder result;
    result.StartArray();

    if (!request_queue_.stats_requests.empty())
    {
        for (const domain::StatRequest& request : request_queue_.stats_requests)
        {
            ComputeStatRequest(result, request);
        }
    }

    return result.EndArray().Build();
}

}
