#pragma once

#include "geo.h"

#include <string>
#include <vector>

namespace domain {

struct Stop {
    std::string name;
    geo::Coordinates coords;
};

struct Bus {
    std::string name;
    std::vector<Stop*> stops;
};

}
