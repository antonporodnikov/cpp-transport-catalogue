#pragma once

#include "json.h"
#include "transport_catalogue.h"

#include <istream>

using transport_catalogue::TransportCatalogue;

namespace json_reader {

namespace parsers {

void ParseStopRequest(const json::Node& stop_request,
    domain::RequestQueue& queue);

void ParseBusRequest(const json::Node& bus_request,
    domain::RequestQueue& queue);

void ParseBaseRequests(const json::Node& base_requests,
    domain::RequestQueue& queue);

void ParseStatRequest(const json::Node& stat_request,
    domain::RequestQueue& queue);

void ParseStatRequests(const json::Node& stat_requests,
    domain::RequestQueue& queue);

domain::RequestQueue ParseJSON(std::istream& input);

}

void ProcessingStopRequest(TransportCatalogue& catalogue,
    const domain::StopRequest& request);

void ProcessingDistances(TransportCatalogue& catalogue,
    const domain::StopRequest& request);

void ProcessingBusRequest(TransportCatalogue& catalogue,
    const domain::BusRequest& request);

void ProcessingInput(TransportCatalogue& catalogue,
    const domain::RequestQueue& queue);

json::Node ComputeStatRequest(TransportCatalogue& catalogue,
    const domain::StatRequest& request);

json::Array ComputeJSON(TransportCatalogue& catalogue,
    const domain::RequestQueue& queue);

void ProcessingOutput(TransportCatalogue& catalogue,
    const domain::RequestQueue& queue, std::ostream& output);

}
