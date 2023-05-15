#include "transport_catalogue.h"

namespace transport_catalogue {

namespace hashers {

std::size_t StringViewHasher::operator()(const std::string_view name) const
{
    const int SHIFT = 27;

    const int mult = name.size() + SHIFT;
    const auto letter_it = name.begin();
    std::size_t first = hasher_(*letter_it) * mult; 
    std::size_t second = hasher_(*letter_it + mult) * (mult * mult); 

    return (first + second) * (mult * mult * mult);
}

std::size_t StopPtrsHasher::operator()(const std::pair<
    structs::Stop*, structs::Stop*> stops) const
{
    const int MULT = 54;

    std::size_t first = (hasher_(stops.first)) * MULT;
    std::size_t second = (hasher_(stops.second)) * (MULT * MULT);

    return (first + second) * (MULT * MULT * MULT);
}

}

void TransportCatalogue::AddStop(const std::string&& name,
    const geo::Coordinates&& coords)
{
    stops_.push_back({std::move(name), std::move(coords)});

    const auto it = std::next(stops_.end(), -1);
    std::string_view name_strv = it->name;

    stopname_to_stop_[name_strv] = &(*it);

    stopname_to_buses_[name_strv] = {};
}

void TransportCatalogue::AddBus(const std::string&& name,
    const std::vector<std::string>& stops)
{
    std::vector<structs::Stop*> stops_ptr;
    stops_ptr.reserve(stops.size());
    for (const std::string& stop : stops)
    {
        stops_ptr.push_back(GetStop(stop));
    }

    buses_.push_back({std::move(name), stops_ptr});

    const auto it = std::next(buses_.end(), -1);
    std::string_view name_strv = it->name;

    busname_to_bus_[name_strv] = &(*it);

    for (const structs::Stop* stop_ptr : stops_ptr)
    {
        stopname_to_buses_.at(stop_ptr->name).insert(name_strv);
    }
}

void TransportCatalogue::AddDistance(const std::string& stop_from,
    const std::string& stop_to, int distance)
{
    stops_to_distance_[{GetStop(stop_from), GetStop(stop_to)}] = distance;
}
        
structs::Stop* TransportCatalogue::GetStop(const std::string& name) const
{
    if (stopname_to_stop_.count(name) == 0)
    {
        throw std::invalid_argument("Stop from route not found in catalogue");
    }

    return stopname_to_stop_.at(name);
}

std::set<std::string_view> TransportCatalogue::GetBusesToStop(
    const std::string& stop_name) const
{
    if (stopname_to_buses_.count(stop_name) == 0)
    {
        throw std::invalid_argument("Stop not found in catalogue");
    }

    return stopname_to_buses_.at(stop_name);
}

int TransportCatalogue::ComputeStopsCount(const std::string& bus_name) const
{
    return static_cast<int>(GetBus(bus_name)->stops.size());
}

int TransportCatalogue::ComputeUniqueStopsCount(
    const std::string& bus_name) const
{
    const std::set<structs::Stop*> unique_stops(GetBus(bus_name)->stops.begin(),
        GetBus(bus_name)->stops.end());

    return static_cast<int>(unique_stops.size());
}

double TransportCatalogue::ComputeRouteLength(const std::string& name) const
{
    int length = 0;
    const structs::Bus* bus_ptr = GetBus(name);

    auto stop_from = bus_ptr->stops.begin();
    auto stop_to = std::next(stop_from, 1);
    for (; stop_to != bus_ptr->stops.end(); stop_from = std::next(stop_from, 1),
        stop_to = std::next(stop_to, 1))
    {
        length += GetDistance((*stop_from)->name, (*stop_to)->name);
    }

    return static_cast<double>(length);
}

double TransportCatalogue::ComputeCurvature(const std::string& name) const
{
    const int length = ComputeRouteLength(name);
    double raw_length = 0.0;
    const structs::Bus* bus_ptr = GetBus(name);

    auto stop_from = bus_ptr->stops.begin();
    auto stop_to = std::next(stop_from, 1);
    for (; stop_to != bus_ptr->stops.end(); stop_from = std::next(stop_from, 1),
        stop_to = std::next(stop_to, 1))
    {
        raw_length += geo::ComputeDistance((*stop_from)->coords,
            (*stop_to)->coords);
    }

    return static_cast<double>(length) / raw_length;
}

structs::Bus* TransportCatalogue::GetBus(const std::string& name) const
{
    if (busname_to_bus_.count(name) == 0)
    {
        throw std::invalid_argument("Route not found in catalogue");
    }

    return busname_to_bus_.at(name);
}

int TransportCatalogue::GetDistance(const std::string& stop_from,
    const std::string& stop_to) const
{
    structs::Stop* stop_from_ptr = GetStop(stop_from);
    structs::Stop* stop_to_ptr = GetStop(stop_to);

    const bool check_from_to = stops_to_distance_.count(
        {stop_from_ptr, stop_to_ptr}) > 0;
    const bool check_to_from = stops_to_distance_.count(
        {stop_to_ptr, stop_from_ptr}) > 0;

    if (!check_from_to && !check_to_from)
    {
        throw std::invalid_argument(
            "No route between these stops in catalogue");
    }

    if (check_from_to)
    {
        return stops_to_distance_.at({stop_from_ptr, stop_to_ptr});
    }

    return stops_to_distance_.at({stop_to_ptr, stop_from_ptr});
} 

}
