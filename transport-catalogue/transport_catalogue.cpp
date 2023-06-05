#include "transport_catalogue.h"

namespace transport_catalogue {

void TransportCatalogue::AddStop(const std::string& name,
    const geo::Coordinates& coords)
{
    stops_.push_back({name, coords});

    domain::Stop& stop = stops_.back();
    std::string_view name_strv = stop.name;

    stopname_to_stop_[name_strv] = &stop;

    stopname_to_buses_[name_strv] = {};
}

void TransportCatalogue::AddBus(const std::string& name,
    const std::vector<std::string>& stops, const bool is_round)
{
    std::vector<domain::Stop*> stops_ptr;
    stops_ptr.reserve(stops.size());
    for (const std::string& stop : stops)
    {
        stops_ptr.push_back(GetStop(stop));
    }

    buses_.push_back({name, stops_ptr, is_round});

    const auto it = std::next(buses_.end(), -1);
    std::string_view name_strv = it->name;

    busname_to_bus_[name_strv] = &(*it);

    for (const domain::Stop* stop_ptr : stops_ptr)
    {
        stopname_to_buses_.at(stop_ptr->name).insert(name_strv);
    }
}

void TransportCatalogue::AddDistance(const std::string& stop_from,
    const std::string& stop_to, int distance)
{
    stops_to_distance_[{GetStop(stop_from), GetStop(stop_to)}] = distance;
}
        
domain::Stop* TransportCatalogue::GetStop(const std::string& name) const
{
    if (stopname_to_stop_.count(name) == 0)
    {
        throw std::invalid_argument("Stop from route not found in catalogue");
    }

    return stopname_to_stop_.at(name);
}

domain::Bus* TransportCatalogue::GetBus(const std::string& name) const
{
    if (busname_to_bus_.count(name) == 0)
    {
        throw std::invalid_argument("Route not found in catalogue");
    }

    return busname_to_bus_.at(name);
}

std::map<std::string, domain::Bus*> TransportCatalogue::GetRoutes() const
{
    std::map<std::string, domain::Bus*> result;

    for (const domain::Bus& bus : buses_)
    {
        result[bus.name] = busname_to_bus_.at(bus.name);
    }

    return result;
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
    const std::set<domain::Stop*> unique_stops(GetBus(bus_name)->stops.begin(),
        GetBus(bus_name)->stops.end());

    return static_cast<int>(unique_stops.size());
}

double TransportCatalogue::ComputeRouteLength(const std::string& name) const
{
    int length = 0;
    const domain::Bus* bus_ptr = GetBus(name);

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
    const domain::Bus* bus_ptr = GetBus(name);

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

int TransportCatalogue::GetDistance(const std::string& stop_from,
    const std::string& stop_to) const
{
    domain::Stop* stop_from_ptr = GetStop(stop_from);
    domain::Stop* stop_to_ptr = GetStop(stop_to);

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
