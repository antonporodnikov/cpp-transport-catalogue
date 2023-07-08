#pragma once

#include "map_renderer.h"
#include "router.h"
#include "transport_catalogue.h"

#include <optional>

using transport_catalogue::TransportCatalogue;

namespace request_handler {

class MapRequestHandler {
public:
    MapRequestHandler(const TransportCatalogue& db,
        const map_renderer::MapRenderer& renderer);

    const std::map<std::string, domain::Bus*> GetRoutes() const;

    svg::Document RenderMap() const;

private:
    const TransportCatalogue& db_;
    const map_renderer::MapRenderer& renderer_;
};

class RouterRequestHandler {
public:
    RouterRequestHandler(const graph::Router<double>& router);
    
    std::optional<graph::Router<double>::RouteInfo> BuildRoute(
        graph::VertexId from, graph::VertexId to) const;

private:
    const graph::Router<double>& router_;
};

}
