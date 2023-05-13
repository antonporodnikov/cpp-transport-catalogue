#include "input_reader.h"

namespace input {

Stop::Stop(const std::string& stop_name, const geo::Coordinates& stop_coords)
    : name(stop_name), coords(stop_coords)
{
}

Bus::Bus(const std::string& bus_name, const std::vector<Stop*>& bus_stops)
    : name(bus_name), stops(bus_stops)
{
}

bool IsStop(const std::string& label)
{
    return label.find("Stop") != std::string::npos;
}

RequestQueue Processing(std::istream& input)
{
    RequestQueue result;

    std::string request;
    std::getline(input, request);
    int request_count = std::stoi(request);
    for (int i = 0; i < request_count; ++i)
    {
        std::getline(input, request);
        const auto colon = request.find(':');
        if (IsStop(request.substr(0, colon)))
        {
            result.Stops.push_back(std::move(request));
            continue;
        }

        result.Buses.push_back(std::move(request));
    }

    return result;
}

namespace parsers {

namespace details {

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

std::string CutName(const std::string& request, const std::string& label)
{
    std::string name = request.substr(request.find(label) + label.size());
    name = name.substr(0, name.find(':'));
    CutSpaces(name);

    return name;
}

geo::Coordinates CutCoords(const std::string& request)
{
    std::string coords = request.substr(request.find(':') + 1);
    std::string lat_str = coords.substr(0, coords.find(','));
    CutSpaces(lat_str);
    std::string lng_str = coords.substr(coords.find(',') + 1);
    CutSpaces(lng_str);

    return {std::stod(lat_str), std::stod(lng_str)};
}

std::vector<std::string> CutRoute(const std::string& request,
    const std::string& del)
{
    std::vector<std::string> routes;

    std::string route = request.substr(request.find(':') + 1);
    std::string stop;
    while (route.find(del) != std::string::npos)
    {
        stop = route.substr(0, route.find(del));
        CutSpaces(stop);
        routes.push_back(stop);

        route = route.substr(route.find(del) + del.size());
    }

    CutSpaces(route);
    routes.push_back(route);

    return routes;
}

}

input::Stop Stop(const std::string& request)
{
    std::string name = details::CutName(request, "Stop");
    geo::Coordinates coords = details::CutCoords(request);

    return input::Stop(name, coords);
}

std::tuple<std::string, std::vector<std::string>, bool>
    Bus(const std::string& request)
{
    bool circular;
    std::string del;

    if (request.find(" - ") != std::string::npos)
    {
        del = " - ";
        circular = false;
    }
    else
    {
        del = " > ";
        circular = true;
    }

    std::string name = details::CutName(request, "Bus");
    std::vector<std::string> route = details::CutRoute(request, del);

    return {name, route, circular};
}

std::pair<std::string, std::vector<std::string>> StopsDistance(
    const std::string& request)
{
    std::string name = details::CutName(request, "Stop");
    std::vector<std::string> distances = {};

    std::string temp = request.substr(request.find(',') + 1);
    if (temp.find(',') == std::string::npos)
    {
        return {name, distances};
    }
    std::string all_distances = temp.substr(temp.find(',') + 1);
    std::string distance; 
    while (all_distances.find(',') != std::string::npos)
    {
        distance = all_distances.substr(0, all_distances.find(',')); 
        details::CutSpaces(distance);
        distances.push_back(distance);

        all_distances = all_distances.substr(all_distances.find(',') + 1);
    }

    details::CutSpaces(all_distances);
    distances.push_back(all_distances);

    return {name, distances};
}

std::pair<std::string, int> Distance(const std::string& request)
{
    std::string name = request.substr(request.find("to") + 2);
    details::CutSpaces(name);
    std::string distance_str = request.substr(0, request.find('m'));

    return {name, std::stoi(distance_str)};
}

std::string Output(const std::string& request, const std::string& label)
{
    return details::CutName(request, label);
}

}

}
