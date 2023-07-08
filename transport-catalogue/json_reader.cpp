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

    bool is_route_request = false;
    for (const auto& request : request_queue_.stats_requests)
    {
        if (std::holds_alternative<domain::RouteRequest>(request))
        {
            is_route_request = true;
        }
    }

    if (is_route_request)
    {
        graph_ = std::make_unique<graph::DirectedWeightedGraph<double>>(
            catalogue_.GetAllStops().size());
        transport_router::TransportRouter tr_temp(router_settings_);
        tr_temp.FillGraph(catalogue_, *graph_);

        router_ = std::make_unique<graph::Router<double>>(*graph_);
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

void JsonReader::ParseRouteRequest(const json::Dict& route_request)
{
    const int id = route_request.at("id").AsInt();
    const std::string type = route_request.at("type").AsString();
    const std::string from = route_request.at("from").AsString();
    const std::string to = route_request.at("to").AsString();

    domain::RouteRequest temp{id, type, from, to};
    request_queue_.stats_requests.push_back(std::move(temp));
}

void JsonReader::ParseStatRequest(const json::Node& stat_request)
{
    const json::Dict& request = stat_request.AsDict();

    if (request.at("type") == "Route")
    {
        ParseRouteRequest(request);
        return;
    }

    const int id = request.at("id").AsInt();
    const std::string type = request.at("type").AsString();

    if (request.count("name") != 0)
    {
        const std::string name = request.at("name").AsString();
        domain::StatRequest temp{id, type, name};
        request_queue_.stats_requests.push_back(std::move(temp));

        return;
    }

    domain::StatRequest temp{id, type, ""};
    request_queue_.stats_requests.push_back(std::move(temp));
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

    router_settings_.bus_wait_time = static_cast<uint16_t>(wait_time);
    router_settings_.bus_velocity = velocity;
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
            const request_handler::MapRequestHandler handler(catalogue_,
                renderer);

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

void JsonReader::BuildSameStopsResponse(json::Builder& builder,
    const domain::RouteRequest& request)
{
    builder.StartDict().Key("total_time"s).Value(0)
        .Key("request_id"s).Value(request.id)
        .Key("items"s).StartArray().EndArray().EndDict();
}

void JsonReader::BuildValidRouteResponse(
    const graph::Router<double>::RouteInfo& route_data,
    json::Builder& builder, const domain::RouteRequest& request)
{
    json::Array route_items;

    for (const auto& edge_id : route_data.edges)
    {
        const auto& edge = graph_->GetEdge(edge_id);
        std::string stop_name = catalogue_.GetAllStops().at(edge.from).name;

        json::Dict wait_type = json::Builder{}.StartDict()
            .Key("time"s).Value(router_settings_.bus_wait_time)
            .Key("type"s).Value("Wait"s)
            .Key("stop_name"s).Value(stop_name).EndDict().Build().AsDict();

        json::Dict bus_type = json::Builder{}.StartDict()
            .Key("time"s).Value(edge.weight - router_settings_.bus_wait_time)
            .Key("span_count"s).Value(edge.span_count)
            .Key("bus"s).Value(edge.bus_name)
            .Key("type"s).Value("Bus"s)
            .EndDict().Build().AsDict();

        route_items.push_back(std::move(wait_type));
        route_items.push_back(std::move(bus_type));
    }

    builder.StartDict().Key("total_time"s).Value(route_data.weight)
        .Key("request_id"s).Value(request.id)
        .Key("items"s).Value(route_items).EndDict();
}

void JsonReader::BuildNonValidRouteResponse(json::Builder& builder,
    const domain::RouteRequest& request)
{
    builder.StartDict().Key("request_id"s).Value(request.id)
        .Key("error_message"s).Value("not found"s)
        .EndDict();
}

void JsonReader::ComputeRouteRequest(json::Builder& builder,
    const domain::RouteRequest& request)
{
    const domain::Stop* stop_from = catalogue_.GetStop(request.from);
    const domain::Stop* stop_to = catalogue_.GetStop(request.to);

    if (stop_from == stop_to)
    {
        BuildSameStopsResponse(builder, request);

        return;
    }
    else
    {
        request_handler::RouterRequestHandler handler(*router_);
        const auto route_data = handler.BuildRoute(stop_from->edge_id,
            stop_to->edge_id);

        if (route_data.has_value())
        {
            BuildValidRouteResponse(route_data.value(), builder, request);

            return;
        }
        else
        {
            BuildNonValidRouteResponse(builder, request);

            return;
        }
    }
}

json::Node JsonReader::ComputeJSON()
{
    json::Builder result;
    result.StartArray();

    if (!request_queue_.stats_requests.empty())
    {
        for (const auto& request : request_queue_.stats_requests)
        {
            if (std::holds_alternative<domain::StatRequest>(request))
            {
                ComputeStatRequest(result,
                    std::get<domain::StatRequest>(request));
            }
            else
            {
                ComputeRouteRequest(result,
                    std::get<domain::RouteRequest>(request));
            }
        }
    }

    return result.EndArray().Build();
}

}
