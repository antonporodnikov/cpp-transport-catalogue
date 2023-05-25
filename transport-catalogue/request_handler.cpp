#include "request_handler.h"

namespace request_handler {

RequestHandler::RequestHandler(const TransportCatalogue& db,
    const map_renderer::MapRenderer& renderer)
    : db_(db), renderer_(renderer)
{
}

const std::map<std::string, domain::Bus*> RequestHandler::GetRoutes() const
{
    return db_.GetRoutes();
}
    
svg::Document RequestHandler::RenderMap() const
{
    return renderer_.RenderMap(GetRoutes());
}

}
