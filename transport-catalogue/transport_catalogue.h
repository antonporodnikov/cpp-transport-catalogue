#pragma once

#include "geo.h"
#include "input_reader.h"

#include <deque>
#include <functional>
#include <iterator>
#include <set>
#include <string>
#include <unordered_map>

namespace transport_catalogue {

namespace details {

struct ObjnameToObjHasher {
    std::size_t operator()(std::string_view name) const
    {
        const int SHIFT = 27;

        const int factor = name.size() + SHIFT;
        auto letter = name.begin();
        std::size_t first = hasher_(*letter) * factor; 
        std::size_t second = hasher_(*letter + factor) * (factor * factor); 

        return (first + second) * (factor * factor * factor);
    }

private:
    std::hash<char> hasher_;
};

struct ObjnameToObjHasherStr {
    std::size_t operator()(const std::string& name) const
    {
        const int SHIFT = 27;

        const int factor = name.size() + SHIFT;
        auto letter = name.begin();
        std::size_t first = hasher_(*letter) * factor; 
        std::size_t second = hasher_(*letter + factor) * (factor * factor); 

        return (first + second) * (factor * factor * factor);
    }

private:
    std::hash<char> hasher_;
};

struct StopsDistanceHasher {
    std::size_t operator()(
        const std::pair<input::Stop*, input::Stop*> stops) const
    {
        const int MULT = 54;

        std::size_t first = (hasher_(stops.first)) * MULT;
        std::size_t second = (hasher_(stops.second)) * (MULT * MULT);

        return (first + second) * (MULT * MULT * MULT);
    }

private:
    std::hash<input::Stop*> hasher_;
};

}

class TransportCatalogue {
public:
    using StopnameToStopType = std::unordered_map<std::string_view,
        input::Stop*, details::ObjnameToObjHasher>; 
    
    using BusnameToBusType = std::unordered_map<std::string_view,
        input::Bus*, details::ObjnameToObjHasher>; 
    
    using StopToBusesType = std::unordered_map<std::string,
        std::set<std::string>, details::ObjnameToObjHasherStr>;

    using StopsDistanceType = std::unordered_map<std::pair<input::Stop*,
        input::Stop*>, int, details::StopsDistanceHasher>;

    void FillStops(const std::vector<std::string>& stops);

    void CreateStopnameToStop();

    void FillBuses(const std::vector<std::string>& buses);

    void CreateBusnameToBus();

    void FillStopsDistance(const std::vector<std::string>& stops);

    void RefreshCatalogue(const input::RequestQueue& request);

    const std::deque<input::Stop>& GetStops() const;

    const StopnameToStopType& GetStopnameToStop() const;

    const std::deque<input::Bus>& GetBuses() const;

    const BusnameToBusType& GetBusnameToBus() const;

    const StopToBusesType& GetStopToBuses() const;

    const StopsDistanceType& GetStopsDistance() const;

private:
    std::deque<input::Stop> stops_;
    std::deque<input::Bus> buses_;
    StopnameToStopType stopname_to_stop_;
    BusnameToBusType busname_to_bus_;
    StopToBusesType stop_to_buses_;
    StopsDistanceType stops_distance_;

    void AddDistance(
        const std::pair<std::string, std::vector<std::string>> stop);
};

}
