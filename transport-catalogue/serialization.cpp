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
    const map_renderer::RenderSettingsRequest& render_settings)
{
    std::ofstream ofs(serialization_settings_.file_name.c_str(),
        std::ios::binary);

    SerializeStops();
    SerializeStopsToDistance();
    SerializeBuses();
    SerializeRenderSettings(render_settings);

    tcb_.SerializeToOstream(&ofs);
}

void SerializationMachine::Deserialize(
    map_renderer::RenderSettingsRequest& render_settings)
{
    std::ifstream ifs(serialization_settings_.file_name.c_str(),
        std::ios::binary);
    tcb_.ParseFromIstream(&ifs);

    DeserializeStops();
    DeserializeStopsToDistance();
    DeserializeBuses();
    DeserializeRenderSettings(render_settings);
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

}
