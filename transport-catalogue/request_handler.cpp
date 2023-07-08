#include "request_handler.h"

namespace request_handler {

MapRequestHandler::MapRequestHandler(const TransportCatalogue& db,
    const map_renderer::MapRenderer& renderer)
    : db_(db), renderer_(renderer)
{
}

const std::map<std::string, domain::Bus*> MapRequestHandler::GetRoutes() const
{
    return db_.GetRoutes();
}
    
svg::Document MapRequestHandler::RenderMap() const
{
    return renderer_.RenderMap(GetRoutes());
}

RouterRequestHandler::RouterRequestHandler(const graph::Router<double>& router)
    : router_(router)
{
}

std::optional<graph::Router<double>::RouteInfo> RouterRequestHandler::BuildRoute(
    graph::VertexId from, graph::VertexId to) const
{
    return router_.BuildRoute(from, to);
}

}
