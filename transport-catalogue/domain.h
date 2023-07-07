#pragma once

#include "geo.h"

#include <map>
#include <string>
#include <variant>
#include <vector>

namespace domain {

struct Stop {
    std::string name;
    geo::Coordinates coords;
    uint16_t edge_id;
};

struct Bus {
    std::string name;
    std::vector<Stop*> stops;
    bool is_round;
};

struct StopRequest {
    std::string name;
    double lat = 0;
    double lng = 0;
    std::map<std::string, int> dists;
};

struct BusRequest {
    std::string name;
    std::vector<std::string> stops;
    bool is_round;
};

struct StatRequest {
    int id;
    std::string type;
    std::string name;
};

struct RouteRequest {
    int id;
    std::string type;
    std::string from;
    std::string to;
};

struct RequestQueue {
    std::vector<StopRequest> stops_requests;
    std::vector<BusRequest> buses_requests;
    std::vector<std::variant<StatRequest, RouteRequest>> stats_requests;
};

}
