#pragma once

#include "geo.h"

#include <istream>
#include <string>
#include <tuple>
#include <vector>

namespace input {

struct RequestQueue {
    std::vector<std::string> Stops;
    std::vector<std::string> Buses;
};

struct Stop {
    Stop() = default;
    Stop(const std::string& stop_name, const geo::Coordinates& stop_coords);

    std::string name;
    geo::Coordinates coords;
};

struct Bus {
    Bus(const std::string& bus_name, const std::vector<Stop*>& bus_stops);

    std::string name;
    std::vector<Stop*> stops;
};

bool IsStop(const std::string& request);

RequestQueue Processing(std::istream& input);

namespace parsers {

namespace details {

void CutSpaces(std::string& text);

std::string CutName(const std::string& request, const std::string& label);

geo::Coordinates CutCoords(const std::string& request);

std::vector<std::string> CutRoute(const std::string& request,
    const std::string& del);

}

input::Stop Stop(const std::string& request);

std::tuple<std::string, std::vector<std::string>, bool>
    Bus(const std::string& request);

std::pair<std::string, std::vector<std::string>> StopsDistance(
    const std::string& request);

std::pair<std::string, int> Distance(const std::string& request);

std::string Output(const std::string& request, const std::string& label);

}

}
