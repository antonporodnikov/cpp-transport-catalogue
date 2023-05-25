#pragma once

#include "map_renderer.h"
#include "transport_catalogue.h"

using transport_catalogue::TransportCatalogue;

namespace request_handler {

class RequestHandler {
public:
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(const TransportCatalogue& db,
        const map_renderer::MapRenderer& renderer);

    // Возвращает информацию о маршруте (запрос Bus)
    // std::optional<BusStat> GetBusStat(
    //     const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через
    // const std::unordered_set<BusPtr>* GetBusesByStop(
    //     const std::string_view& stop_name) const;

    const std::map<std::string, domain::Bus*> GetRoutes() const;

    // Этот метод будет нужен в следующей части итогового проекта
    svg::Document RenderMap() const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и
    // "Визуализатор Карты"
    const TransportCatalogue& db_;
    const map_renderer::MapRenderer& renderer_;
};

}
