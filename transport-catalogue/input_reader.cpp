#include "input_reader.h"

namespace input_reader {

namespace details {

bool IsStop(const std::string& label_and_name)
{
    return label_and_name.find("Stop") != std::string::npos;
}

void CutSpaces(std::string& text)
{
    while (text[0] == ' ')
    {
        text.erase(0, 1);
    }

    while (*next(text.end(), -1) == ' ')
    {
        text.pop_back();
    }
}

void CutLabel(std::string& request, const std::string&& label)
{
    request = request.substr(request.find(label) + label.size());
    CutSpaces(request);
}

std::string GetName(const std::string& request)
{
    std::string name = request.substr(0, request.find(':'));
    CutSpaces(name);

    return name;
}

void CutName(std::string& request)
{
    request = request.substr(request.find(':') + 1);
    CutSpaces(request);
}

geo::Coordinates GetCoords(const std::string& request)
{
    std::string lat_str = request.substr(0, request.find(','));
    CutSpaces(lat_str);

    std::string lng_str = request.substr(request.find(',') + 1);
    lng_str = lng_str.substr(0, lng_str.find(','));
    CutSpaces(lng_str);

    return {std::stod(lat_str), std::stod(lng_str)};
}

void CutCoords(std::string& request)
{
    const int COORDS_COUNT = 2;

    if (request.find(" to ") == std::string::npos)
    {
        request = "";

        return;
    }

    for (int i = 0; i < COORDS_COUNT; ++i)
    {
        request = request.substr(request.find(',') + 1);
    }

    CutSpaces(request);
}

void FillRoute(std::vector<std::string>& route, const std::string& request,
    const std::string&& del)
{
    std::string route_buf = request;
    std::string stop;
    while (route_buf.find(del) != std::string::npos)
    {
        stop = route_buf.substr(0, route_buf.find(del));
        CutSpaces(stop);

        route.push_back(stop);

        route_buf = route_buf.substr(route_buf.find(del) + del.size());
    }

    CutSpaces(route_buf);
    route.push_back(route_buf);
}

std::vector<std::string> GetRoute(const std::string& request)
{
    std::vector<std::string> route;

    if (request.find(" > ") != std::string::npos)
    {
        FillRoute(route, request, " > ");

        return route;
    }

    FillRoute(route, request, " - ");

    std::vector<std::string> reverse_route;
    for (auto it = std::next(route.rbegin(), 1); it != route.rend();
        it = std::next(it, 1))
    {
        reverse_route.push_back(*it);
    }

    route.insert(route.end(), reverse_route.begin(), reverse_route.end());

    return route;
}

std::string GetStopTo(const std::string& dist_stop)
{
    std::string stop_to = dist_stop.substr(dist_stop.find(" to ") + 4);
    CutSpaces(stop_to);

    return stop_to;
}

int GetDistance(const std::string& dist_stop)
{
    std::string dist = dist_stop.substr(0, dist_stop.find('m'));
    CutSpaces(dist);

    return std::stoi(dist);
}

void FillDistances(std::vector<std::pair<std::string, int>>& stops_to,
    const std::string& request)
{
    std::string request_buf = request;

    if (request_buf.size() == 0)
    {
        return;
    }

    std::string dist_stop;
    std::string stop_to;
    int distance = 0;
    while (request_buf.find(',') != std::string::npos)
    {
        dist_stop = request_buf.substr(0, request_buf.find(','));

        stop_to = GetStopTo(dist_stop);
        distance = GetDistance(dist_stop);

        stops_to.push_back({stop_to, distance});

        request_buf = request_buf.substr(request_buf.find(',') + 1);
    }

    stop_to = GetStopTo(request_buf);
    distance = GetDistance(request_buf);

    stops_to.push_back({stop_to, distance});
}

}

namespace parsers {

RequestQueue ParseInput(std::istream& input)
{
    RequestQueue queue;

    std::string request;
    std::getline(input, request);
    const int request_count = std::stoi(request);
    for (int i = 0; i < request_count; ++i)
    {
        std::getline(input, request);
        const auto colon = request.find(':');
        
        if (details::IsStop(request.substr(0, colon)))
        {
            details::CutLabel(request, "Stop");
            queue.StopsQueue.push_back(std::move(request));

            continue;
        }

        details::CutLabel(request, "Bus");
        queue.BusesQueue.push_back(std::move(request));
    }

    return queue;
}

std::pair<std::string, geo::Coordinates> ParseStopRequest(std::string& request)
{
    const std::string name = details::GetName(request);
    details::CutName(request);

    const geo::Coordinates coords = details::GetCoords(request);
    details::CutCoords(request);

    request = name + ": " + request;

    return {name, coords};
}

std::pair<std::string, std::vector<std::string>> ParseBusRequest(
    TransportCatalogue& catalogue, std::string& request)
{
    const std::string name = details::GetName(request);
    details::CutName(request);

    const std::vector<std::string> route = details::GetRoute(request);

    request = name + ": " + request;

    return {name, route};
}

void ParseDistanceRequest(TransportCatalogue& catalogue, std::string& request)
{
    const std::string stop_from = details::GetName(request);
    details::CutName(request);

    std::vector<std::pair<std::string, int>> stops_dists = {};
    details::CutSpaces(request);
    details::FillDistances(stops_dists, request);

    for (const auto& stop_dist : stops_dists)
    {
        catalogue.AddDistance(stop_from, stop_dist.first, stop_dist.second);
    }

    request = stop_from + ": " + request;
}

}

void ProcessingInput(TransportCatalogue& catalogue, std::istream& input)
{
    RequestQueue queue = parsers::ParseInput(input);

    for (std::string& request : queue.StopsQueue)
    {
        const auto stop_data = parsers::ParseStopRequest(request);

        catalogue.AddStop(std::move(stop_data.first),
            std::move(stop_data.second));
    }

    for (std::string& request : queue.BusesQueue)
    {
        const auto bus_data = parsers::ParseBusRequest(catalogue, request);

        catalogue.AddBus(std::move(bus_data.first), bus_data.second);
    }

    for (std::string& request : queue.StopsQueue)
    {
        parsers::ParseDistanceRequest(catalogue, request);
    }
}

}
