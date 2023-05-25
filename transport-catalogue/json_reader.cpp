#include "json_reader.h"

using namespace std::literals;

namespace json_reader {

JsonReader::JsonReader(std::istream& input)
{
    JsonReader::ParseJSON(input);
}

void JsonReader::UpdateCatalogue(TransportCatalogue& catalogue)
{
    if (!request_queue_.stops_requests.empty())
    {
        for (const domain::StopRequest& request : request_queue_.stops_requests)
        {
            ProcessingStopRequest(catalogue, request);
        }

        for (const domain::StopRequest& request : request_queue_.stops_requests)
        {
            ProcessingDistances(catalogue, request);
        }
    }

    if (!request_queue_.buses_requests.empty())
    {
        for (const domain::BusRequest& request : request_queue_.buses_requests)
        {
            ProcessingBusRequest(catalogue, request);
        }
    }
}

void JsonReader::PrintStat(TransportCatalogue& catalogue, std::ostream& output)
{
    json::Print(json::Document{ComputeJSON(catalogue)}, output);
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
    const json::Dict& request = stop_request.AsMap();

    const std::string name = request.at("name").AsString();
    const double lat = request.at("latitude").AsDouble();
    const double lng = request.at("longitude").AsDouble();

    std::map<std::string, int> dists = {};
    if (request.count("road_distances"))
    {
        for (const auto& [stop, dist] : 
            request.at("road_distances").AsMap())
        {
            dists[stop] = dist.AsInt();
        }
    }

    request_queue_.stops_requests.push_back({name, lat, lng, dists});
}
            
void JsonReader::ParseBusRequest(const json::Node& bus_request)
{
    const json::Dict& request = bus_request.AsMap();

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
        if (request.AsMap().at("type").AsString() == "Stop")
        {
            ParseStopRequest(request);
            continue;
        }

        if (request.AsMap().at("type").AsString() == "Bus")
        {
            ParseBusRequest(request);
            continue;
        }
    }
}

void JsonReader::ParseStatRequest(const json::Node& stat_request)
{
    const json::Dict& request = stat_request.AsMap();

    const int id = request.at("id").AsInt();
    const std::string type = request.at("type").AsString();
    const std::string name = request.at("name").AsString();

    request_queue_.stats_requests.push_back({id, type, name});
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
    if (color.IsString())
    {
        return color.AsString();
    }
    
    json::Array color_arr = color.AsArray();
    if (color_arr.size() == 4)
    {
        svg::Rgba color_rgba{color_arr.at(0).AsInt(), color_arr.at(1).AsInt(),
            color_arr.at(2).AsInt(), color_arr.at(3).AsDouble()};

        return color_rgba;
    }


    svg::Rgb color_rgb{color_arr.at(0).AsInt(), color_arr.at(1).AsInt(),
        color_arr.at(2).AsInt()};
    
    return color_rgb;
}

void JsonReader::ParseRenderSettings(const json::Node& render_settings)
{
    const json::Dict& request = render_settings.AsMap();

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

void JsonReader::ParseJSON(std::istream& input)
{
    const json::Document json_data = json::Load(input);
    const json::Dict& requests = json_data.GetRoot().AsMap();

    if (requests.count("base_requests"))
    {
        ParseBaseRequests(requests.at("base_requests"));
    }

    // if (requests.count("stat_requests"))
    // {
    //     ParseStatRequests(requests.at("stat_requests"));
    // }

    if (requests.count("render_settings"))
    {
        ParseRenderSettings(requests.at("render_settings"));
    }
}

void JsonReader::ProcessingStopRequest(TransportCatalogue& catalogue,
    const domain::StopRequest& request)
{
    catalogue.AddStop(request.name, {request.lat, request.lng});
}

void JsonReader::ProcessingDistances(TransportCatalogue& catalogue,
    const domain::StopRequest& request)
{
    if (!request.dists.empty())
    {
        for (const auto& [stop_to, dist] : request.dists)
        {
            catalogue.AddDistance(request.name, stop_to, dist);
        }
    }
}

void JsonReader::ProcessingBusRequest(TransportCatalogue& catalogue,
    const domain::BusRequest& request)
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

    catalogue.AddBus(request.name, stops);
}

json::Node JsonReader::ComputeStatRequest(TransportCatalogue& catalogue,
    const domain::StatRequest& request)
{
    json::Dict result;

    if (request.type == "Stop")
    {
        try
        {
            std::set<std::string_view> buses_to_stop = 
                catalogue.GetBusesToStop(request.name);
            json::Array buses;
            
            for (std::string_view bus : buses_to_stop)
            {
                std::string bus_name(bus); 
                buses.push_back(bus_name);
            }

            result["buses"] = buses;
            result["request_id"] = request.id;
        }
        catch (const std::invalid_argument&)
        {
            result["request_id"] = request.id;
            result["error_message"] = "not found"s;
        }

        return result;
    }

    try
    {
        result["curvature"] = catalogue.ComputeCurvature(request.name);
        result["request_id"] = request.id;
        result["route_length"] = catalogue.ComputeRouteLength(request.name);
        result["stop_count"] = catalogue.ComputeStopsCount(request.name);
        result["unique_stop_count"] =
            catalogue.ComputeUniqueStopsCount(request.name);
    }
    catch (const std::invalid_argument&)
    {
        result = {};
        result["request_id"] = request.id;
        result["error_message"] = "not found"s;
    }

    return result;
}

json::Array JsonReader::ComputeJSON(TransportCatalogue& catalogue)
{
    json::Array result;

    if (!request_queue_.stats_requests.empty())
    {
        for (const domain::StatRequest& request : request_queue_.stats_requests)
        {
            result.push_back(ComputeStatRequest(catalogue, request));
        }
    }

    return result;
}

}
