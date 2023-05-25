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

    details::ColorPalettePicker color_picker(render_settings_.color_palette);
    for (const auto& route : routes)
    {
        if (!(route.second->stops.empty()))
        {
            const svg::Color color = color_picker.GetColor();
            RenderRoute(route.second, proj, color, doc);
        }
    }

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

    svg::Polyline p;
    p.SetFillColor("none");
    p.SetStrokeColor(color);
    p.SetStrokeWidth(render_settings_.line_width);
    p.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    p.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    for (const auto geo_coord : geo_coords)
    {
        const svg::Point screen_coord = proj(geo_coord);
        p.AddPoint(screen_coord);
    }

    doc.Add(std::move(p));
}

}
