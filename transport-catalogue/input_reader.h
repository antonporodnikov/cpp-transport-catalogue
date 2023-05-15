#pragma once

#include "geo.h"
#include "transport_catalogue.h"

#include <istream>

using transport_catalogue::structs::Stop;
using transport_catalogue::TransportCatalogue;

namespace input_reader {

struct RequestQueue {
    std::vector<std::string> StopsQueue;
    std::vector<std::string> BusesQueue;
};

namespace details {

bool IsStop(const std::string& label_and_name);

void CutSpaces(std::string& text);

void CutLabel(std::string& request, const std::string&& label);

std::string GetName(const std::string& request);

void CutName(std::string& request);

geo::Coordinates GetCoords(const std::string& request);

void CutCoords(std::string& request);

void FillRoute(std::vector<std::string>& route, const std::string& request,
    const std::string&& del);

std::vector<std::string> GetRoute(const std::string& request);

std::string GetStopTo(const std::string& dist_stop);

int GetDistance(const std::string& dist_stop);

void FillDistances(std::vector<std::pair<std::string, int>>& stops_to,
    const std::string& request);

}

namespace parsers {

RequestQueue ParseInput(std::istream& input);

std::pair<std::string, geo::Coordinates> ParseStopRequest(std::string& request);

std::pair<std::string, std::vector<std::string>> ParseBusRequest(
    std::string& request);

void ParseDistanceRequest(TransportCatalogue& catalogue, std::string& request);

}

void ProcessingInput(TransportCatalogue& catalogue, std::istream& input);

}
