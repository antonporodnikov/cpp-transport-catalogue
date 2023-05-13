#include "transport_catalogue.h"

namespace transport_catalogue {

void TransportCatalogue::FillStops(const std::vector<std::string>& stops)
{
    input::Stop stop_obj;
    for (const std::string& stop : stops)
    {
        input::Stop stop_obj = input::parsers::Stop(stop);
        stop_to_buses_[stop_obj.name] = {};        
        stops_.push_back(std::move(stop_obj));
    }    
}

void TransportCatalogue::CreateStopnameToStop()
{
    for (auto it = stops_.begin(); it != stops_.end(); it = std::next(it, 1))
    {
        stopname_to_stop_[it->name] = &(*it);
    }
}

void TransportCatalogue::FillBuses(const std::vector<std::string>& buses)
{
    for (const std::string& bus : buses)
    {
        std::tuple<std::string, std::vector<std::string>, bool> bus_data =
            input::parsers::Bus(bus);
        
        std::string& bus_name = std::get<0>(bus_data);

        std::vector<input::Stop*> bus_stops;
        for (const std::string& bus_stop : std::get<1>(bus_data))
        {
            stop_to_buses_.at(bus_stop).insert(bus_name);
            bus_stops.push_back(stopname_to_stop_.at(bus_stop));
        }
        
        if (!std::get<2>(bus_data))
        {
            std::vector<input::Stop*> bus_stops_circular(bus_stops);
            for (auto it = bus_stops.rbegin() + 1; it != bus_stops.rend();
                it = std::next(it, 1))
            {
                bus_stops_circular.push_back(*it);
            }

            buses_.push_back(input::Bus(bus_name, bus_stops_circular));

            continue;
        }

        buses_.push_back(input::Bus(bus_name, bus_stops));
    }
}

void TransportCatalogue::CreateBusnameToBus()
{
    for (auto it = buses_.begin(); it != buses_.end(); it = std::next(it, 1))
    {
        busname_to_bus_[it->name] = &(*it);
    }
}

void TransportCatalogue::AddDistance(
    const std::pair<std::string, std::vector<std::string>> stop)
{
    const std::string& stop_from = stop.first;
    const std::vector<std::string>& stops_to = stop.second;

    if (stops_to.empty())
    {
        return;
    }

    for (const std::string& stop_to : stops_to)
    {
        const std::pair<std::string, int> stop_to_and_distance =
            input::parsers::Distance(stop_to);

        input::Stop* stop_from_ptr = stopname_to_stop_.at(stop_from);
        input::Stop* stop_to_ptr = stopname_to_stop_.at(
            stop_to_and_distance.first);
        int distance = stop_to_and_distance.second;

        stops_distance_[{stop_from_ptr, stop_to_ptr}] = distance;
    }
}

void TransportCatalogue::FillStopsDistance(
    const std::vector<std::string>& stops)
{
    for (const std::string& stop : stops)
    {
        const auto stop_data = input::parsers::StopsDistance(stop);
        AddDistance(stop_data);
    }
}

void TransportCatalogue::RefreshCatalogue(const input::RequestQueue& request)
{
    FillStops(request.Stops);
    CreateStopnameToStop();
    FillBuses(request.Buses);
    CreateBusnameToBus();
    FillStopsDistance(request.Stops);
}

const std::deque<input::Stop>& TransportCatalogue::GetStops() const
{
    return stops_;
}

const TransportCatalogue::StopnameToStopType&
    TransportCatalogue::GetStopnameToStop() const
{
    return stopname_to_stop_;
}

const std::deque<input::Bus>& TransportCatalogue::GetBuses() const
{
    return buses_;
}

const TransportCatalogue::BusnameToBusType&
    TransportCatalogue::GetBusnameToBus() const
{
    return busname_to_bus_;
}

const TransportCatalogue::StopToBusesType&
    TransportCatalogue::GetStopToBuses() const
{
    return stop_to_buses_;
}

const TransportCatalogue::StopsDistanceType&
    TransportCatalogue::GetStopsDistance() const
{
    return stops_distance_;
}

}
