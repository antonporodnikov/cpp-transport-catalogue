#include "transport_router.h"

namespace transport_router {

using namespace std::string_literals;

TransportRouter::TransportRouter(const TransportRouterSettings& router_settings)
    : router_settings_(router_settings) {}

double TransportRouter::ComputeEdgeWeight(const double distance) const
{
    const double DISTANCE_CONVERT_VALUE = 1000.0;
    const double BUS_VELOCITY_CONVERT_VALUE = 60.0;

    return distance / DISTANCE_CONVERT_VALUE /
        router_settings_.bus_velocity * BUS_VELOCITY_CONVERT_VALUE;
}

void TransportRouter::AddEdgesForwards(const std::vector<domain::Stop*>& stops,
    const TransportCatalogue& catalogue,
    graph::DirectedWeightedGraph<double>& graph,
    const std::string& route_name) const
{
    const auto stops_to_distance = catalogue.GetStopsToDistance();

    for (int i = 0; i + 1 < static_cast<int>(stops.size()); ++i)
    {
        double weight = router_settings_.bus_wait_time;
        uint16_t span = 1;

        for (int j = i + 1; j < static_cast<int>(stops.size()); ++j)
        {
            if (stops.at(i) != stops.at(j))
            {
                auto it_temp = stops_to_distance.find({stops.at(j - 1), stops.at(j)});
                auto it_for_dist = (it_temp == stops_to_distance.end()) ?
                    stops_to_distance.find({stops.at(j), stops.at(j - 1)}) : it_temp;

                weight += ComputeEdgeWeight(it_for_dist->second);
                graph.AddEdge({stops.at(i)->edge_id, stops.at(j)->edge_id, weight,
                    route_name, span});

                ++span;
            }
        }
    }
}

void TransportRouter::AddEdgesBackwards(const std::vector<domain::Stop*>& stops,
    const TransportCatalogue& catalogue,
    graph::DirectedWeightedGraph<double>& graph,
    const std::string& route_name) const
{
    const auto stops_to_distance = catalogue.GetStopsToDistance();

    for (int i = static_cast<int>(stops.size()) - 1; i > 0; --i)
    {
        double weight = router_settings_.bus_wait_time;
        uint16_t span = 1;

        for (int j = i; j > 0; --j)
        {
            if (stops.at(i) != stops.at(j - 1))
            {
                auto it_temp = stops_to_distance.find({stops.at(j), stops.at(j - 1)});
                auto it_for_dist = (it_temp == stops_to_distance.end()) ?
                    stops_to_distance.find({stops.at(j - 1), stops.at(j)}) : it_temp;

                weight += ComputeEdgeWeight(it_for_dist->second);
                graph.AddEdge({stops.at(i)->edge_id, stops.at(j - 1)->edge_id, weight,
                    route_name, span});

                ++span;
            }
        }
    }
}

void TransportRouter::FillGraph(const TransportCatalogue& catalogue,
    graph::DirectedWeightedGraph<double>& graph) const
{
    for (const auto& [name, route] : catalogue.GetRoutes())
    {
        const std::vector<domain::Stop*>& stops = route->stops;

        if (stops.size() > 1)
        {
            AddEdgesForwards(stops, catalogue, graph, name);
            
            if (!(route->is_round))
            {
                AddEdgesBackwards(stops, catalogue, graph, name);
            }
        }
    }
}

}  // namespace transport_router
