#include "json_reader.h"

using namespace std::literals;

namespace json_reader {

namespace parsers {

void ParseStopRequest(const json::Node& stop_request,
    domain::RequestQueue& queue)
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

    queue.stops_requests.push_back({name, lat, lng, dists});
}
            
void ParseBusRequest(const json::Node& bus_request, domain::RequestQueue& queue)
{
    const json::Dict& request = bus_request.AsMap();

    const std::string name = request.at("name").AsString();
    const bool is_round = request.at("is_roundtrip").AsBool();

    std::vector<std::string> stops = {};
    for (const json::Node& stop : request.at("stops").AsArray())
    {
        stops.push_back(stop.AsString());
    }

    queue.buses_requests.push_back({name, stops, is_round});
}

void ParseBaseRequests(const json::Node& base_requests,
    domain::RequestQueue& queue)
{
    const json::Array& requests = base_requests.AsArray();

    for (const json::Node& request : requests)
    {
        if (request.AsMap().at("type").AsString() == "Stop")
        {
            ParseStopRequest(request, queue);
            continue;
        }

        if (request.AsMap().at("type").AsString() == "Bus")
        {
            ParseBusRequest(request, queue);
            continue;
        }
    }
}

void ParseStatRequest(const json::Node& stat_request,
    domain::RequestQueue& queue)
{
    const json::Dict& request = stat_request.AsMap();

    const int id = request.at("id").AsInt();
    const std::string type = request.at("type").AsString();
    const std::string name = request.at("name").AsString();

    queue.stats_requests.push_back({id, type, name});
}

void ParseStatRequests(const json::Node& stat_requests,
    domain::RequestQueue& queue)
{
    const json::Array& requests = stat_requests.AsArray();

    for (const json::Node& request : requests)
    {
        ParseStatRequest(request, queue);
    }
}

domain::RequestQueue ParseJSON(std::istream& input)
{
    domain::RequestQueue queue;

    const json::Document json_data = json::Load(input);
    const json::Dict& requests = json_data.GetRoot().AsMap();

    if (requests.count("base_requests"))
    {
        ParseBaseRequests(requests.at("base_requests"), queue);
    }

    if (requests.count("stat_requests"))
    {
        ParseStatRequests(requests.at("stat_requests"), queue);
    }

    return queue;
}

}

void ProcessingStopRequest(TransportCatalogue& catalogue,
    const domain::StopRequest& request)
{
    catalogue.AddStop(request.name, {request.lat, request.lng});
}

void ProcessingDistances(TransportCatalogue& catalogue,
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

void ProcessingBusRequest(TransportCatalogue& catalogue,
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

void ProcessingInput(TransportCatalogue& catalogue,
    const domain::RequestQueue& queue)
{
    if (!queue.stops_requests.empty())
    {
        for (const domain::StopRequest& request : queue.stops_requests)
        {
            ProcessingStopRequest(catalogue, request);
        }

        for (const domain::StopRequest& request : queue.stops_requests)
        {
            ProcessingDistances(catalogue, request);
        }
    }

    if (!queue.buses_requests.empty())
    {
        for (const domain::BusRequest& request : queue.buses_requests)
        {
            ProcessingBusRequest(catalogue, request);
        }
    }
}

json::Node ComputeStatRequest(TransportCatalogue& catalogue,
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

json::Array ComputeJSON(TransportCatalogue& catalogue,
    const domain::RequestQueue& queue)
{
    json::Array result;

    if (!queue.stats_requests.empty())
    {
        for (const domain::StatRequest& request : queue.stats_requests)
        {
            result.push_back(ComputeStatRequest(catalogue, request));
        }
    }

    return result;
}

void ProcessingOutput(TransportCatalogue& catalogue,
    const domain::RequestQueue& queue, std::ostream& output)
{
    json::Print(json::Document{ComputeJSON(catalogue, queue)}, output);
}

}
