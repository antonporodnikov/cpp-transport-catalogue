#pragma once

#include <cstdint>
#include <stdexcept>

namespace transport_router {

struct TransportRouterSettings {
     uint16_t bus_wait_time;
     double bus_velocity;
};

}
