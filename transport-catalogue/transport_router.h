#pragma once

#include "domain.h"
#include "graph.h"
#include "transport_catalogue.h"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace transport_router {

using TransportCatalogue = transport_catalogue::TransportCatalogue;

struct TransportRouterSettings {
     uint16_t bus_wait_time;
     double bus_velocity;
};

class TransportRouter {
public:
     TransportRouter(const TransportRouterSettings& router_settings);

     void FillGraph(const TransportCatalogue& catalogue,
          graph::DirectedWeightedGraph<double>& graph) const;

private:
     TransportRouterSettings router_settings_;

     double ComputeEdgeWeight(const double distance) const;

     void AddEdgesForwards(const std::vector<domain::Stop*>& stops,
          const TransportCatalogue& catalogue,
          graph::DirectedWeightedGraph<double>& graph,
          const std::string& route_name) const;

     void AddEdgesBackwards(const std::vector<domain::Stop*>& stops,
          const TransportCatalogue& catalogue,
          graph::DirectedWeightedGraph<double>& graph,
          const std::string& route_name) const;
};

}  // namespace transport_router
