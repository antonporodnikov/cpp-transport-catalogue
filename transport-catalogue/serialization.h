#pragma once

#include "domain.h"
#include "transport_catalogue.h"

#include <transport_catalogue.pb.h>

#include <fstream>
#include <string>
#include <utility>

using transport_catalogue::TransportCatalogue;

namespace serialization {

struct SerializationSettings {
    std::string file_name;
};

class SerializationMachine {
public:
    SerializationMachine(TransportCatalogue& catalogue,
        const SerializationSettings& serialization_settings);
    
    void Serialize();

    void Deserialize();

private:
    const SerializationSettings serialization_settings_;
    TransportCatalogue& catalogue_; 
    transport_catalogue_serialize::TransportCatalogueBase tcb_;

    transport_catalogue_serialize::Stop SerializeStop(
        const domain::Stop& stop) const;

    transport_catalogue_serialize::StopsToDistance SerializeStopsToDistanceElement(
        domain::Stop* from, domain::Stop* to, const int distance) const;

    transport_catalogue_serialize::Bus SerializeBus(
        const domain::Bus& bus) const;

    void SerializeStops();

    void SerializeStopsToDistance();

    void SerializeBuses();

    void DeserializeStop(const transport_catalogue_serialize::Stop& stop);

    void DeserializeStopsToDistanceElement(
        const transport_catalogue_serialize::StopsToDistance& stops_to_distance);

    void DeserializeBus(const transport_catalogue_serialize::Bus& bus);

    void DeserializeStops();

    void DeserializeStopsToDistance();

    void DeserializeBuses();
};

}
