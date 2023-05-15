#pragma once

#include "geo.h"
#include "transport_catalogue.h"

#include <iomanip>
#include <iostream>
#include <istream>

using transport_catalogue::structs::Stop;
using transport_catalogue::structs::Bus;
using transport_catalogue::TransportCatalogue;

namespace stat_reader {

namespace details {

bool IsStop(const std::string& label_and_name);

void CutSpaces(std::string& text);

std::string GetName(const std::string& request, const std::string& label);

}

namespace prints {

void PrintValidStopRequest(TransportCatalogue& catalogue,
    const std::string& request);

void PrintStopRequest(TransportCatalogue& catalogue, std::string& request);

void PrintValidBusRequest(TransportCatalogue& catalogue,
    const std::string& request);

void PrintBusRequest(TransportCatalogue& catalogue, std::string& request);

}

namespace parsers {

void ParseRequests(TransportCatalogue& catalogue, std::istream& input);

}

void OutputResponse(TransportCatalogue& catalogue, std::istream& input);

}
