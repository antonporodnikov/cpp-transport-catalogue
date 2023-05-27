#pragma once

#include "domain.h"

#include <deque>
#include <set>
#include <stdexcept>
#include <unordered_map>

namespace transport_catalogue {

namespace hashers {

struct StringViewHasher {
    std::size_t operator()(const std::string_view name) const
    {
        const int SHIFT = 27;

        const int mult = name.size() + SHIFT;
        const auto letter_it = name.begin();
        std::size_t first = hasher_(*letter_it) * mult; 
        std::size_t second = hasher_(*letter_it + mult) * (mult * mult); 

        return (first + second) * (mult * mult * mult);
    }

private:
    std::hash<char> hasher_;
};

struct StopPtrsHasher {
    std::size_t operator()(const std::pair<
        domain::Stop*, domain::Stop*> stops) const
    {
        const int MULT = 54;

        std::size_t first = (hasher_(stops.first)) * MULT;
        std::size_t second = (hasher_(stops.second)) * (MULT * MULT);

        return (first + second) * (MULT * MULT * MULT);
    }

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

    domain::Bus* GetBus(const std::string& name) const;

    std::map<std::string, domain::Bus*> GetRoutes() const;

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

    int GetDistance(const std::string& stop_from,
        const std::string& stop_to) const;
};

}
