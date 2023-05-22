#pragma once

#include "transport_catalogue.h"

#include <iostream>

using transport_catalogue::TransportCatalogue;

namespace stat_reader {

namespace details {

bool IsStop(const std::string& label_and_name);

void CutSpaces(std::string& text);

std::string GetName(const std::string& request, const std::string& label);

}

namespace prints {

void PrintValidStopRequest(TransportCatalogue& catalogue,
    const std::string& request, std::ostream& output);

void PrintStopRequest(TransportCatalogue& catalogue, std::string& request,
    std::ostream& output);

void PrintValidBusRequest(TransportCatalogue& catalogue,
    const std::string& request, std::ostream& output);

void PrintBusRequest(TransportCatalogue& catalogue, std::string& request,
    std::ostream& output);

}

namespace parsers {

void ParseRequests(TransportCatalogue& catalogue, std::istream& input,
    std::ostream& output);

}

void OutputResponse(TransportCatalogue& catalogue, std::istream& input,
    std::ostream& output);

}
