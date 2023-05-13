#pragma once

#include "geo.h"
#include "input_reader.h"
#include "transport_catalogue.h"

#include <algorithm>
#include <iomanip>
#include <iostream>

namespace stat_reader {

namespace compute {

double Length(const input::Bus* bus_ptr);

std::tuple<int, int, double> Data(
    const transport_catalogue::TransportCatalogue& catalogue,
    std::string& name);

int Curv(const transport_catalogue::TransportCatalogue& catalogue,
    std::string& name);

}

void ProcessOutputBus(const transport_catalogue::TransportCatalogue& catalogue,
    std::string& name);

void ProcessOutputStop(const transport_catalogue::TransportCatalogue& catalogue,
    std::string& name);

void Output(std::istream& input,
    const transport_catalogue::TransportCatalogue& catalogue);

}
