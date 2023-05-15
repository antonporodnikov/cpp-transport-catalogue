#pragma once

#include "geo.h"

#include <deque>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace transport_catalogue {

namespace structs {

struct Stop {
    std::string name;
    geo::Coordinates coords;
};

struct Bus {
    std::string name;
    std::vector<Stop*> stops;
};

}

namespace hashers {

struct StringViewHasher {
    std::size_t operator()(const std::string_view name) const;

private:
    std::hash<char> hasher_;
};

struct StopPtrsHasher {
    std::size_t operator()(const std::pair<
        structs::Stop*, structs::Stop*> stops) const;

private:
    std::hash<structs::Stop*> hasher_;
};

}

class TransportCatalogue {
public:
    using StopnameToStop = std::unordered_map<std::string_view,
        structs::Stop*, hashers::StringViewHasher>;
    
    using BusnameToBus = std::unordered_map<std::string_view,
        structs::Bus*, hashers::StringViewHasher>;

    using StopnameToBuses = std::unordered_map<std::string_view,
        std::set<std::string_view>, hashers::StringViewHasher>;

    using StopsToDistance = std::unordered_map<std::pair<
        structs::Stop*,structs::Stop*>, int, hashers::StopPtrsHasher>;

    void AddStop(const std::string&& name, const geo::Coordinates&& coords);

    void AddBus(const std::string&& name,
        const std::vector<structs::Stop*>&& stops);
    
    void AddDistance(const std::string& stop_from, const std::string& stop_to,
        int distance);

    structs::Stop* GetStop(const std::string& name) const;

    std::set<std::string_view> GetBusesToStop(
        const std::string& stop_name) const;

    int ComputeStopsCount(const std::string& bus_name) const;

    int ComputeUniqueStopsCount(const std::string& bus_name) const;

    double ComputeRouteLength(const std::string& name) const;

    double ComputeCurvature(const std::string& name) const;

private:
    std::deque<structs::Stop> stops_;
    std::deque<structs::Bus> buses_;
    StopnameToStop stopname_to_stop_;
    BusnameToBus busname_to_bus_;
    StopnameToBuses stopname_to_buses_;
    StopsToDistance stops_to_distance_;

    structs::Bus* GetBus(const std::string& name) const;

    int GetDistance(const std::string& stop_from,
        const std::string& stop_to) const;
};

}
