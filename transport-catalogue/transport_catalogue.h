#pragma once

#include "domain.h"

#include <deque>
#include <set>
#include <stdexcept>
#include <unordered_map>

namespace transport_catalogue {

namespace hashers {

struct StringViewHasher {
    std::size_t operator()(const std::string_view name) const;

private:
    std::hash<char> hasher_;
};

struct StopPtrsHasher {
    std::size_t operator()(const std::pair<
        domain::Stop*, domain::Stop*> stops) const;

private:
    std::hash<domain::Stop*> hasher_;
};

}

class TransportCatalogue {
public:
    using StopnameToStop = std::unordered_map<std::string_view,
        domain::Stop*, hashers::StringViewHasher>;
    
    using BusnameToBus = std::unordered_map<std::string_view,
        domain::Bus*, hashers::StringViewHasher>;

    using StopnameToBuses = std::unordered_map<std::string_view,
        std::set<std::string_view>, hashers::StringViewHasher>;

    using StopsToDistance = std::unordered_map<std::pair<
        domain::Stop*, domain::Stop*>, int, hashers::StopPtrsHasher>;

    void AddStop(const std::string& name, const geo::Coordinates& coords);

    void AddBus(const std::string& name,
        const std::vector<std::string>& stops);
    
    void AddDistance(const std::string& stop_from, const std::string& stop_to,
        int distance);

    domain::Stop* GetStop(const std::string& name) const;

    std::set<std::string_view> GetBusesToStop(
        const std::string& stop_name) const;

    int ComputeStopsCount(const std::string& bus_name) const;

    int ComputeUniqueStopsCount(const std::string& bus_name) const;

    double ComputeRouteLength(const std::string& name) const;

    double ComputeCurvature(const std::string& name) const;

private:
    std::deque<domain::Stop> stops_;
    std::deque<domain::Bus> buses_;
    StopnameToStop stopname_to_stop_;
    BusnameToBus busname_to_bus_;
    StopnameToBuses stopname_to_buses_;
    StopsToDistance stops_to_distance_;

    domain::Bus* GetBus(const std::string& name) const;

    int GetDistance(const std::string& stop_from,
        const std::string& stop_to) const;
};

}
