#pragma once

#include "domain.h"
#include "graph.h"
#include "map_renderer.h"
#include "router.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "transport_router.h"

#include <transport_catalogue.pb.h>
#include <map_renderer.pb.h>

#include <fstream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

using transport_catalogue::TransportCatalogue;

namespace serialization {

struct SerializationSettings {
    std::string file_name;
};

class SerializationMachine {
public:
    SerializationMachine(TransportCatalogue& catalogue);

    void SetSettings(const std::string& file_name);
    
    void Serialize(const map_renderer::RenderSettingsRequest& render_settings,
        const transport_router::TransportRouterSettings& router_settings,
        const graph::DirectedWeightedGraph<double>& graph,
        const graph::Router<double>& router);

    void Deserialize(map_renderer::RenderSettingsRequest& render_settings,
        transport_router::TransportRouterSettings& router_settings,
        graph::DirectedWeightedGraph<double>& graph,
        graph::Router<double>& router);

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

    void SerializeRouterSettings(
        const transport_router::TransportRouterSettings& router_settings);
    
    graph_serialize::IncidenceList SerializeIncidenceList(
        const graph::DirectedWeightedGraph<double>::IncidentEdgesRange& incidence_list);

    graph_serialize::Edge SerializeEdge(const graph::Edge<double>& edge);

    void SerializeGraph(const graph::DirectedWeightedGraph<double>& graph);

    router_serialize::RID SerializeRID(
        const graph::Router<double>::RouteInternalData& rid);
    
    router_serialize::OptionalRID SerializeOptionalRID(const std::optional<
        graph::Router<double>::RouteInternalData>& optional_rid);

    router_serialize::RepeatedRID SerializeRepeatedRID(const std::vector<
        std::optional<graph::Router<double>::RouteInternalData>>& repeated_rid);

    void SerializeRouter(const graph::Router<double>& router);

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

    void DeserializeRouterSettings(
        transport_router::TransportRouterSettings& router_settings);
    
    graph::DirectedWeightedGraph<double>::IncidenceList DeserializeIncedenceList(
        const graph_serialize::IncidenceList incedence_list_proto);

    graph::Edge<double> DeserializeEdge(const graph_serialize::Edge edge_proto);

    void DeserializeGraph(graph::DirectedWeightedGraph<double>& graph);

    graph::Router<double>::RouteInternalData DeserializeRID(
        const router_serialize::RID& rid_proto);

    std::optional<graph::Router<double>::RouteInternalData>
    DeserializeOptionalRID(
        const router_serialize::OptionalRID& optional_rid_proto);

    std::vector<std::optional<graph::Router<double>::RouteInternalData>>
    DeserializeRepeatedRID(
        const router_serialize::RepeatedRID& repeated_rid_proto);

    void DeserializeRouter(graph::Router<double>& router,
        const graph::DirectedWeightedGraph<double>& graph);
};

}
