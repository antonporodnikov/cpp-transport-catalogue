#pragma once

#include "domain.h"
#include "map_renderer.h"
#include "svg.h"
#include "transport_catalogue.h"

#include <transport_catalogue.pb.h>
#include <map_renderer.pb.h>

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
    SerializationMachine(TransportCatalogue& catalogue);

    void SetSettings(const std::string& file_name);
    
    void Serialize(const map_renderer::RenderSettingsRequest& render_settings);

    void Deserialize(map_renderer::RenderSettingsRequest& render_settings);

private:
    SerializationSettings serialization_settings_;
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

    void SerializeColor(const svg::Color& color,
        map_renderer_serialize::Color& color_proto);

    void SerializeRenderSettings(
        const map_renderer::RenderSettingsRequest& render_settings);

    void DeserializeStop(const transport_catalogue_serialize::Stop& stop);

    void DeserializeStopsToDistanceElement(
        const transport_catalogue_serialize::StopsToDistance& stops_to_distance);

    void DeserializeBus(const transport_catalogue_serialize::Bus& bus);

    void DeserializeStops();

    void DeserializeStopsToDistance();

    void DeserializeBuses();

    svg::Color DeserializeColor(
        const map_renderer_serialize::Color& color_proto);

    void DeserializeRenderSettings(
        map_renderer::RenderSettingsRequest& render_settings);
};

}
