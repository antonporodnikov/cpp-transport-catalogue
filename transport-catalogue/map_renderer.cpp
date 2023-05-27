#include "map_renderer.h"

namespace map_renderer {

namespace details {

bool IsZero(double value)
{
    return std::abs(value) < EPSILON;
}

ColorPalettePicker::ColorPalettePicker(
    const std::vector<svg::Color>& color_palette)
    : color_palette_(color_palette), color_palette_size_(color_palette.size())
{
}

const svg::Color ColorPalettePicker::GetColor()
{
    const svg::Color color = color_palette_.at(index_++);

    if (!(index_ < color_palette_size_))
    {
        index_ = 0;
        return color;
    }

    return color;
}

}

MapRenderer::MapRenderer(RenderSettingsRequest render_settings)
    : render_settings_(render_settings)
{
}

svg::Document MapRenderer::RenderMap(
    const std::map<std::string, domain::Bus*>& routes) const
{
    svg::Document doc;

    const double WIDTH = render_settings_.width;
    const double HEIGHT = render_settings_.height;
    const double PADDING = render_settings_.padding;

    std::vector<geo::Coordinates> geo_coords;
    for (const auto& [name, route] : routes)
    {
        for (auto it = route->stops.begin(); it != route->stops.end(); ++it)
        {
            geo_coords.push_back((*it)->coords);
        }
    }

    const details::SphereProjector proj{geo_coords.begin(), geo_coords.end(),
        WIDTH, HEIGHT, PADDING};

    details::ColorPalettePicker color_picker1(render_settings_.color_palette);
    for (const auto& route : routes)
    {
        if (!(route.second->stops.empty()))
        {
            const svg::Color route_color = color_picker1.GetColor();
            RenderRoute(route.second, proj, route_color, doc);
        }
    }

    details::ColorPalettePicker color_picker2(render_settings_.color_palette);
    for (const auto& route : routes)
    {
        if (!(route.second->stops.empty()))
        {
            const svg::Color route_color = color_picker2.GetColor();
            RenderRouteName(route.second, proj, route_color, doc);
        }
    }

    std::vector<domain::Stop*> stops;
    for (const auto& route : routes)
    {
        for (const auto& stop : route.second->stops)
        {
            stops.push_back(stop);
        }
    }

    std::sort(stops.begin(), stops.end(),
        [](domain::Stop* lhs, domain::Stop* rhs)
        {
            return lhs->name < rhs->name;
        });
    auto last = std::unique(stops.begin(), stops.end());
    stops.erase(last, stops.end());

    RenderStopsPoints(stops, proj, doc);

    RenderStopsNames(stops, proj, doc);

    return doc;
}

void MapRenderer::RenderRoute(const domain::Bus* route,
    const details::SphereProjector& proj, const svg::Color& color,
    svg::Document& doc) const
{
    std::vector<geo::Coordinates> geo_coords;
    for (auto it = route->stops.begin(); it != route->stops.end(); ++it)
    {
        geo_coords.push_back((*it)->coords);
    }

    svg::Polyline route_line;
    route_line.SetFillColor("none")
              .SetStrokeColor(color)
              .SetStrokeWidth(render_settings_.line_width)
              .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
              .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    for (const auto geo_coord : geo_coords)
    {
        const svg::Point screen_coord = proj(geo_coord);
        route_line.AddPoint(screen_coord);
    }

    doc.Add(std::move(route_line));
}

bool MapRenderer::IsRoundRoute(const domain::Bus* route) const
{
    auto route_start = route->stops.begin();
    auto route_end = std::next(route->stops.end(), -1);
    for (; route_start != route_end; ++route_start, --route_end)
    {
        if (*route_start != *route_end)
        {
            return true;
        }
    }

    return false;
}

void MapRenderer::RenderRouteName(const domain::Bus* route,
    const details::SphereProjector& proj, const svg::Color& color,
    svg::Document& doc) const
{
    const bool is_round_route = IsRoundRoute(route);

    svg::Text route_name_pad;
    route_name_pad.SetOffset({render_settings_.bus_label_offset.first,
        render_settings_.bus_label_offset.second})
                  .SetFontSize(render_settings_.bus_label_font_size)
                  .SetFontFamily("Verdana")
                  .SetFontWeight("bold")
                  .SetData(route->name)
                  .SetFillColor(render_settings_.underlayer_color)
                  .SetStrokeColor(render_settings_.underlayer_color)
                  .SetStrokeWidth(render_settings_.underlayer_width)
                  .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                  .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    
    svg::Text route_name;
    route_name.SetOffset({render_settings_.bus_label_offset.first,
        render_settings_.bus_label_offset.second})
                  .SetFontSize(render_settings_.bus_label_font_size)
                  .SetFontFamily("Verdana")
                  .SetFontWeight("bold")
                  .SetData(route->name)
                  .SetFillColor(color);

    const svg::Point screen_coords = proj(route->stops.at(0)->coords);
    doc.Add(route_name_pad.SetPosition(screen_coords));
    doc.Add(route_name.SetPosition(screen_coords));

    if (!is_round_route)
    {
        int route_middle_index = (route->stops.size()) / 2;
        const svg::Point screen_coords_to =
            proj(route->stops.at(route_middle_index)->coords);
        doc.Add(route_name_pad.SetPosition(screen_coords_to));
        doc.Add(route_name.SetPosition(screen_coords_to));
    }
}

void MapRenderer::RenderStopsPoints(const std::vector<domain::Stop*> stops,
    const details::SphereProjector& proj, svg::Document& doc) const
{
    for (const domain::Stop* stop : stops)
    {
        const svg::Point screen_coords = proj(stop->coords);
        svg::Circle crcl;
        crcl.SetCenter(screen_coords)
            .SetRadius(render_settings_.stop_radius)
            .SetFillColor("white");
        
        doc.Add(crcl);
    }
}

void MapRenderer::RenderStopsNames(const std::vector<domain::Stop*> stops,
    const details::SphereProjector& proj, svg::Document& doc) const
{
    for (const domain::Stop* stop : stops)
    {
        svg::Text route_name_pad;
        route_name_pad.SetOffset({render_settings_.stop_label_offset.first,
            render_settings_.stop_label_offset.second})
                    .SetFontSize(render_settings_.stop_label_font_size)
                    .SetFontFamily("Verdana")
                    .SetData(stop->name)
                    .SetFillColor(render_settings_.underlayer_color)
                    .SetStrokeColor(render_settings_.underlayer_color)
                    .SetStrokeWidth(render_settings_.underlayer_width)
                    .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        
        svg::Text route_name;
        route_name.SetOffset({render_settings_.stop_label_offset.first,
            render_settings_.stop_label_offset.second})
                    .SetFontSize(render_settings_.stop_label_font_size)
                    .SetFontFamily("Verdana")
                    .SetData(stop->name)
                    .SetFillColor("black");

        const svg::Point screen_coords = proj(stop->coords);
        doc.Add(route_name_pad.SetPosition(screen_coords));
        doc.Add(route_name.SetPosition(screen_coords));
    }
}

}
