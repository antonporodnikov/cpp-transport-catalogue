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

void SerializationMachine::Serialize(
    const map_renderer::RenderSettingsRequest& render_settings,
    const transport_router::TransportRouterSettings& router_settings,
    const graph::DirectedWeightedGraph<double>& graph,
    const graph::Router<double>& router)
{
    std::ofstream ofs(serialization_settings_.file_name.c_str(),
        std::ios::binary);

    SerializeStops();
    SerializeStopsToDistance();
    SerializeBuses();
    SerializeRenderSettings(render_settings);
    SerializeRouterSettings(router_settings);
    SerializeGraph(graph);
    SerializeRouter(router);

    tcb_.SerializeToOstream(&ofs);
}

void SerializationMachine::Deserialize(
    map_renderer::RenderSettingsRequest& render_settings,
    transport_router::TransportRouterSettings& router_settings,
    graph::DirectedWeightedGraph<double>& graph,
    graph::Router<double>& router)
{
    std::ifstream ifs(serialization_settings_.file_name.c_str(),
        std::ios::binary);
    tcb_.ParseFromIstream(&ifs);

    DeserializeStops();
    DeserializeStopsToDistance();
    DeserializeBuses();
    DeserializeRenderSettings(render_settings);
    DeserializeRouterSettings(router_settings);
    DeserializeGraph(graph);
    DeserializeRouter(router, graph);
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

void SerializationMachine::SerializeColor(const svg::Color& color,
    map_renderer_serialize::Color& color_proto)
{
    if (std::holds_alternative<std::monostate>(color))
    {
        color_proto.set_is_none(true);
    }
    else if (std::holds_alternative<std::string>(color))
    {
        const std::string& color_name = std::get<std::string>(color);
        color_proto.set_name(color_name);
    }
    else if (std::holds_alternative<svg::Rgb>(color))
    {
        auto& rgb_proto = *color_proto.mutable_rgb();
        const svg::Rgb& rgb = std::get<svg::Rgb>(color);
        rgb_proto.set_red(rgb.red);
        rgb_proto.set_blue(rgb.blue);
        rgb_proto.set_green(rgb.green);
    }
    else if (std::holds_alternative<svg::Rgba>(color))
    {
        auto& rgba_proto = *color_proto.mutable_rgba();
        const svg::Rgba& rgba = std::get<svg::Rgba>(color);
        rgba_proto.set_red(rgba.red);
        rgba_proto.set_blue(rgba.blue);
        rgba_proto.set_green(rgba.green);
        rgba_proto.set_opacity(rgba.opacity);
    }
}

void SerializationMachine::SerializeRenderSettings(
    const map_renderer::RenderSettingsRequest& render_settings)
{
    map_renderer_serialize::MapRenderer render_settings_proto;

    render_settings_proto.set_width(render_settings.width);
    render_settings_proto.set_height(render_settings.height);
    render_settings_proto.set_padding(render_settings.padding);
    render_settings_proto.set_line_width(render_settings.line_width);
    render_settings_proto.set_stop_radius(render_settings.stop_radius);
    render_settings_proto.set_bus_label_font_size(render_settings.bus_label_font_size);
    render_settings_proto.set_bus_label_offset_x(render_settings.bus_label_offset.first);
    render_settings_proto.set_bus_label_offset_y(render_settings.bus_label_offset.second);
    render_settings_proto.set_stop_label_font_size(render_settings.stop_label_font_size);
    render_settings_proto.set_stop_label_offset_x(render_settings.stop_label_offset.first);
    render_settings_proto.set_stop_label_offset_y(render_settings.stop_label_offset.second);
    render_settings_proto.set_underlayer_width(render_settings.underlayer_width);
    
    SerializeColor(render_settings.underlayer_color,
        *render_settings_proto.mutable_underlayer_color());
    
    for (const svg::Color& color : render_settings.color_palette)
    {
        SerializeColor(color, *render_settings_proto.add_color_palette());
    }

    *tcb_.mutable_render_settings() = render_settings_proto;
}

void SerializationMachine::SerializeRouterSettings(
    const transport_router::TransportRouterSettings& router_settings)
{
    router_serialize::RouterSettings router_settings_proto;

    router_settings_proto.set_bus_wait_time(router_settings.bus_wait_time);
    router_settings_proto.set_bus_velocity(router_settings.bus_velocity);

    *tcb_.mutable_router_settings() = router_settings_proto;
}

void SerializationMachine::DeserializeStop(
    const transport_catalogue_serialize::Stop& stop)
{
    geo::Coordinates coords;
    coords.lat = stop.coords().lat();
    coords.lng = stop.coords().lng();

    catalogue_.AddStop(stop.name(), coords);
}

graph_serialize::Edge SerializationMachine::SerializeEdge(
    const graph::Edge<double>& edge)
{
    graph_serialize::Edge edge_proto;

    edge_proto.set_from(edge.from);
    edge_proto.set_to(edge.to);
    edge_proto.set_weight(edge.weight);
    edge_proto.set_bus_name(edge.bus_name);
    edge_proto.set_span_count(edge.span_count);

    return edge_proto;
}

graph_serialize::IncidenceList SerializationMachine::SerializeIncidenceList(
    const graph::DirectedWeightedGraph<double>::IncidentEdgesRange& incidence_list)
{
    graph_serialize::IncidenceList incidence_list_proto;

    for (const auto& edge_id : incidence_list)
    {
        incidence_list_proto.add_edge_id(edge_id);
    }

    return incidence_list_proto;
}

void SerializationMachine::SerializeGraph(
    const graph::DirectedWeightedGraph<double>& graph)
{
    graph_serialize::Graph graph_proto;

    for (int i = 0; i < graph.GetEdgeCount(); ++i)
    {
        *graph_proto.add_edges() = SerializeEdge(graph.GetEdge(i));
    }

    for (int i = 0; i < graph.GetVertexCount(); ++i)
    {
        *graph_proto.add_incidence_list() = SerializeIncidenceList(
            graph.GetIncidentEdges(i));
    }

    *tcb_.mutable_graph() = graph_proto;
}

router_serialize::RID SerializationMachine::SerializeRID(
    const graph::Router<double>::RouteInternalData& rid)
{
    router_serialize::RID rid_proto;

    rid_proto.set_weight(rid.weight);
    
    if (rid.prev_edge.has_value())
    {
        rid_proto.set_prev_edge(*(rid.prev_edge));
        rid_proto.set_prev_edge_is_set(true);
    }
    else
    {
        rid_proto.set_prev_edge_is_set(false);
    }

    return rid_proto;
}

router_serialize::OptionalRID SerializationMachine::SerializeOptionalRID(
    const std::optional<graph::Router<double>::RouteInternalData>&
    optional_rid)
{
    router_serialize::OptionalRID optional_rid_proto;

    if (optional_rid.has_value())
    {
        *optional_rid_proto.mutable_rid() = SerializeRID(*optional_rid);
        optional_rid_proto.set_rid_is_set(true);
    }
    else
    {
        optional_rid_proto.set_rid_is_set(false);
    }

    return optional_rid_proto;
}

router_serialize::RepeatedRID SerializationMachine::SerializeRepeatedRID(
    const std::vector<std::optional<graph::Router<double>::RouteInternalData>>&
    repeated_rid)
{
    router_serialize::RepeatedRID repeated_rid_proto;

    for (const auto& optional_rid : repeated_rid)
    {
        *repeated_rid_proto.add_optional_rid() =
            SerializeOptionalRID(optional_rid);
    }

    return repeated_rid_proto;
}

void SerializationMachine::SerializeRouter(const graph::Router<double>& router)
{
    router_serialize::RepeatedRIDs repeated_rids_proto;    
    const graph::Router<double>::RoutesInternalData& repeated_rids =
        router.GetRIDs();
    
    for (const auto& repeated_rid : repeated_rids)
    {
        *repeated_rids_proto.add_rids() =
            SerializeRepeatedRID(repeated_rid);
    }

    *tcb_.mutable_router_rid() = repeated_rids_proto;
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

svg::Color SerializationMachine::DeserializeColor(
    const map_renderer_serialize::Color& color_proto)
{
    const auto& rgba_proto = color_proto.rgba();
    const auto& rgb_proto = color_proto.rgb();

    switch(color_proto.data_case())
    {
        case map_renderer_serialize::Color::kIsNone:
            return std::monostate{};
        case map_renderer_serialize::Color::kName:
            return color_proto.name();
        case map_renderer_serialize::Color::kRgba:
            return svg::Rgba(rgba_proto.red(), rgba_proto.green(),
                rgba_proto.blue(), rgba_proto.opacity());
        case map_renderer_serialize::Color::kRgb:
            return svg::Rgb(rgb_proto.red(), rgb_proto.green(), rgb_proto.blue());
    }

    return std::monostate{};
}

void SerializationMachine::DeserializeRenderSettings(
    map_renderer::RenderSettingsRequest& render_settings)
{
    auto& rs_proto = *tcb_.mutable_render_settings();

    render_settings.width = rs_proto.width();
    render_settings.height = rs_proto.height();
    render_settings.padding = rs_proto.padding();
    render_settings.line_width = rs_proto.line_width();
    render_settings.stop_radius = rs_proto.stop_radius();
    render_settings.bus_label_font_size = rs_proto.bus_label_font_size();
    render_settings.bus_label_offset.first = rs_proto.bus_label_offset_x();
    render_settings.bus_label_offset.second = rs_proto.bus_label_offset_y();
    render_settings.stop_label_font_size = rs_proto.stop_label_font_size();
    render_settings.stop_label_offset.first = rs_proto.stop_label_offset_x();
    render_settings.stop_label_offset.second = rs_proto.stop_label_offset_y();
    render_settings.underlayer_width = rs_proto.underlayer_width();

    render_settings.underlayer_color = DeserializeColor(rs_proto.underlayer_color());

    for (const auto& color : rs_proto.color_palette())
    {
        render_settings.color_palette.push_back(DeserializeColor(color));
    }
}

void SerializationMachine::DeserializeRouterSettings(
    transport_router::TransportRouterSettings& router_settings)
{
    auto& rs_proto = *tcb_.mutable_router_settings();

    router_settings.bus_wait_time = rs_proto.bus_wait_time();
    router_settings.bus_velocity = rs_proto.bus_velocity();
}

graph::Edge<double> SerializationMachine::DeserializeEdge(
    const graph_serialize::Edge edge_proto)
{
    graph::Edge<double> edge;

    edge.from = edge_proto.from();
    edge.to = edge_proto.to();
    edge.weight = edge_proto.weight();
    edge.bus_name = edge_proto.bus_name();
    edge.span_count = edge_proto.span_count();

    return edge;
}

graph::DirectedWeightedGraph<double>::IncidenceList
SerializationMachine::DeserializeIncedenceList(
    const graph_serialize::IncidenceList incedence_list_proto)
{
    graph::DirectedWeightedGraph<double>::IncidenceList incedence_list;

    for (const auto& edge_id : incedence_list_proto.edge_id())
    {
        incedence_list.push_back(edge_id);
    }

    return incedence_list;
}

void SerializationMachine::DeserializeGraph(
    graph::DirectedWeightedGraph<double>& graph)
{
    std::vector<graph::Edge<double>> edges;
    for (const auto& edge : tcb_.mutable_graph()->edges())
    {
        edges.push_back(DeserializeEdge(edge));
    }
    graph.SetEdges(edges);

    std::vector<graph::DirectedWeightedGraph<double>::IncidenceList> incedence_lists;
    for (const auto& incedence_list : tcb_.mutable_graph()->incidence_list())
    {
        incedence_lists.push_back(DeserializeIncedenceList(incedence_list));
    }
    graph.SetIncidenceLists(incedence_lists);
}

graph::Router<double>::RouteInternalData
SerializationMachine::DeserializeRID(const router_serialize::RID& rid_proto)
{
    graph::Router<double>::RouteInternalData rid;
    rid.weight = rid_proto.weight();
    if (rid_proto.prev_edge_is_set())
    {
        rid.prev_edge = rid_proto.prev_edge();
        return rid;
    }

    rid.prev_edge = std::nullopt;
    return rid;
}

std::optional<graph::Router<double>::RouteInternalData>
SerializationMachine::DeserializeOptionalRID(
    const router_serialize::OptionalRID& optional_rid_proto)
{
    std::optional<graph::Router<double>::RouteInternalData> optional_rid;
    if (optional_rid_proto.rid_is_set())
    {
        optional_rid = DeserializeRID(optional_rid_proto.rid());

        return optional_rid;
    }

    optional_rid = std::nullopt;
    return optional_rid;
}

std::vector<std::optional<graph::Router<double>::RouteInternalData>>
SerializationMachine::DeserializeRepeatedRID(
    const router_serialize::RepeatedRID& repeated_rid_proto)
{
    std::vector<std::optional<graph::Router<double>::RouteInternalData>> repeated_rid;
    for (const auto& optional_rid : repeated_rid_proto.optional_rid())
    {
        repeated_rid.push_back(DeserializeOptionalRID(optional_rid));
    }

    return repeated_rid;
}

void SerializationMachine::DeserializeRouter(graph::Router<double>& router,
    const graph::DirectedWeightedGraph<double>& graph)
{
    graph::Router<double>::RoutesInternalData routes_internal_data;
    for (const auto& rid : tcb_.mutable_router_rid()->rids())
    {
        routes_internal_data.push_back(DeserializeRepeatedRID(rid));
    }

    router.SetRIDs(routes_internal_data);
    router.SetGraph(graph);
}

}
