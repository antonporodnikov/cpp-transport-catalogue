#include "serialization.h"

namespace serialization {

SerializationMachine::SerializationMachine(TransportCatalogue& catalogue)
    : catalogue_(catalogue)
{
}

void SerializationMachine::SetSettings(const std::string& file_name)
{
    serialization_settings_.file_name = file_name;
}

void SerializationMachine::Serialize()
{
    std::ofstream ofs(serialization_settings_.file_name.c_str(),
        std::ios::binary);

    SerializeStops();
    SerializeStopsToDistance();
    SerializeBuses();

    tcb_.SerializeToOstream(&ofs);
}

void SerializationMachine::Deserialize()
{
    std::ifstream ifs(serialization_settings_.file_name.c_str(),
        std::ios::binary);
    tcb_.ParseFromIstream(&ifs);

    DeserializeStops();
    DeserializeStopsToDistance();
    DeserializeBuses();
}

transport_catalogue_serialize::Stop SerializationMachine::SerializeStop(
    const domain::Stop& stop) const
{
    transport_catalogue_serialize::Stop stop_proto;

    stop_proto.set_name(stop.name);
    stop_proto.mutable_coords()->set_lat(stop.coords.lat);
    stop_proto.mutable_coords()->set_lng(stop.coords.lng);
    stop_proto.set_edge_id(stop.edge_id);

    return stop_proto;
}

transport_catalogue_serialize::StopsToDistance SerializationMachine::SerializeStopsToDistanceElement(
    domain::Stop* from, domain::Stop* to, const int distance) const
{
    transport_catalogue_serialize::StopsToDistance stops_distance_proto;

    stops_distance_proto.set_from(from->name);
    stops_distance_proto.set_to(to->name);
    stops_distance_proto.set_distance(distance);

    return stops_distance_proto;
}

transport_catalogue_serialize::Bus SerializationMachine::SerializeBus(
    const domain::Bus& bus) const
{
    transport_catalogue_serialize::Bus bus_proto;

    bus_proto.set_name(bus.name);
    for (const auto& stop : bus.stops)
    {
        bus_proto.add_stops(stop->name);
    }
    bus_proto.set_is_round(bus.is_round);

    return bus_proto;
}


void SerializationMachine::SerializeStops()
{
    for (const domain::Stop& stop : catalogue_.GetAllStops())
    {
        *tcb_.add_stops() = std::move(SerializeStop(stop));
    }
}

void SerializationMachine::SerializeStopsToDistance()
{
    for (const auto& [from_to, distance] : catalogue_.GetStopsToDistance())
    {
        *tcb_.add_stops_to_distance() = std::move(SerializeStopsToDistanceElement(
            from_to.first, from_to.second, distance));
    }
}

void SerializationMachine::SerializeBuses()
{
    for (const domain::Bus& bus : catalogue_.GetAllBuses())
    {
        *tcb_.add_buses() = std::move(SerializeBus(bus));
    }
}

void SerializationMachine::DeserializeStop(
    const transport_catalogue_serialize::Stop& stop)
{
    geo::Coordinates coords;
    coords.lat = stop.coords().lat();
    coords.lng = stop.coords().lng();

    catalogue_.AddStop(stop.name(), coords);
}

void SerializationMachine::DeserializeStopsToDistanceElement(
    const transport_catalogue_serialize::StopsToDistance& stops_to_distance)
{
    catalogue_.AddDistance(stops_to_distance.from(), stops_to_distance.to(),
        stops_to_distance.distance());
}

void SerializationMachine::DeserializeBus(
    const transport_catalogue_serialize::Bus& bus)
{
    std::vector<std::string> stops_temp;
    for (int i = 0; i < bus.stops_size(); ++i)
    {
        stops_temp.push_back(bus.stops(i));
    }

    catalogue_.AddBus(bus.name(), stops_temp, bus.is_round());
}

void SerializationMachine::DeserializeStops()
{
    for (int i = 0; i < tcb_.stops_size(); ++i)
    {
        DeserializeStop(tcb_.stops(i));
    }
}

void SerializationMachine::DeserializeStopsToDistance()
{
    for (int i = 0; i < tcb_.stops_to_distance_size(); ++i)
    {
        DeserializeStopsToDistanceElement(tcb_.stops_to_distance(i));
    }
}

void SerializationMachine::DeserializeBuses()
{
    for (int i = 0; i < tcb_.buses_size(); ++i)
    {
        DeserializeBus(tcb_.buses(i));
    }
}

}
