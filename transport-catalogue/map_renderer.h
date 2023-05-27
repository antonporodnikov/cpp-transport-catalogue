#pragma once

#include "domain.h"
#include "svg.h"

#include <algorithm>
#include <cstdlib>
#include <optional>
#include <set>

namespace map_renderer {

struct RenderSettingsRequest {
    double width = 0.0;
    double height = 0.0;
    double padding = 0.0;
    double line_width = 0.0;
    double stop_radius = 0.0;
    int bus_label_font_size = 0;
    std::pair<double, double> bus_label_offset = {0.0, 0.0};
    int stop_label_font_size = 0;
    std::pair<double, double> stop_label_offset = {0.0, 0.0};
    svg::Color underlayer_color;
    double underlayer_width = 0.0;
    std::vector<svg::Color> color_palette;
};

namespace details {

inline const double EPSILON = 1e-6;
bool IsZero(double value);

class SphereProjector {
public:
    template <typename PointInputIt>
    SphereProjector(PointInputIt points_begin, PointInputIt points_end,
        double max_width, double max_height, double padding);

    svg::Point operator()(geo::Coordinates coords) const;

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

template <typename PointInputIt>
SphereProjector::SphereProjector(PointInputIt points_begin,
    PointInputIt points_end, double max_width, double max_height,
    double padding)
    : padding_(padding)
{
    if (points_begin == points_end)
    {
        return;
    }

    const auto [left_it, right_it] = std::minmax_element(
        points_begin, points_end,
        [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
    min_lon_ = left_it->lng;
    const double max_lon = right_it->lng;

    const auto [bottom_it, top_it] = std::minmax_element(
        points_begin, points_end,
        [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
    const double min_lat = bottom_it->lat;
    max_lat_ = top_it->lat;

    std::optional<double> width_zoom;
    if (!IsZero(max_lon - min_lon_))
    {
        width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
    }

    std::optional<double> height_zoom;
    if (!IsZero(max_lat_ - min_lat))
    {
        height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
    }

    if (width_zoom && height_zoom)
    {
        zoom_coeff_ = std::min(*width_zoom, *height_zoom);
    }
    else if (width_zoom)
    {
        zoom_coeff_ = *width_zoom;
    }
    else if (height_zoom)
    {
        zoom_coeff_ = *height_zoom;
    }
}

class ColorPalettePicker {
public:
    ColorPalettePicker(const std::vector<svg::Color>& color_palette);

    const svg::Color GetColor();

private:
    const std::vector<svg::Color>& color_palette_;
    const size_t color_palette_size_;
    uint64_t index_ = 0;
};

}

class MapRenderer {
public:
    MapRenderer(RenderSettingsRequest render_settings);

    svg::Document RenderMap(
        const std::map<std::string, domain::Bus*>& routes) const;

private:
    RenderSettingsRequest render_settings_;

    void RenderRoute(const domain::Bus* route,
        const details::SphereProjector& proj, const svg::Color& color,
        svg::Document& doc) const;
    
    bool IsRoundRoute(const domain::Bus* route) const;

    void RenderRouteName(const domain::Bus* route,
        const details::SphereProjector& proj, const svg::Color& color,
        svg::Document& doc) const;

    void RenderStopsPoints(const std::vector<domain::Stop*> stops,
        const details::SphereProjector& proj, svg::Document& doc) const;

    void RenderStopsNames(const std::vector<domain::Stop*> stops,
        const details::SphereProjector& proj, svg::Document& doc) const;
};

}
