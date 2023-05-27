#pragma once

#include "map_renderer.h"
#include "transport_catalogue.h"

using transport_catalogue::TransportCatalogue;

namespace request_handler {

class RequestHandler {
public:
    RequestHandler(const TransportCatalogue& db,
        const map_renderer::MapRenderer& renderer);

    const std::map<std::string, domain::Bus*> GetRoutes() const;

    svg::Document RenderMap() const;

private:
    const TransportCatalogue& db_;
    const map_renderer::MapRenderer& renderer_;
};

}
