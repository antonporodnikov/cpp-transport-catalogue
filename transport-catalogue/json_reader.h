#pragma once

#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"

#include <istream>

using transport_catalogue::TransportCatalogue;

namespace json_reader {

class JsonReader {
public:
    explicit JsonReader(std::istream& input);

    void UpdateCatalogue(TransportCatalogue& catalogue);

    void PrintStat(TransportCatalogue& catalogue, std::ostream& output);

    const domain::RequestQueue& GetRequestQueue() const;

    map_renderer::RenderSettingsRequest GetRenderSettings() const;

private:
    domain::RequestQueue request_queue_;
    map_renderer::RenderSettingsRequest render_settings_; 

    void ParseStopRequest(const json::Node& stop_request);

    void ParseBusRequest(const json::Node& bus_request);

    void ParseBaseRequests(const json::Node& base_requests);

    void ParseStatRequest(const json::Node& stat_request);

    void ParseStatRequests(const json::Node& stat_requests);

    svg::Color FormatColor(const json::Node& color);

    void ParseRenderSettings(const json::Node& render_settings);

    void ParseJSON(std::istream& input);

    void ProcessingStopRequest(TransportCatalogue& catalogue,
        const domain::StopRequest& request);

    void ProcessingDistances(TransportCatalogue& catalogue,
        const domain::StopRequest& request);

    void ProcessingBusRequest(TransportCatalogue& catalogue,
        const domain::BusRequest& request);

    json::Node ComputeStatRequest(TransportCatalogue& catalogue,
        const domain::StatRequest& request);

    json::Array ComputeJSON(TransportCatalogue& catalogue);
};

}
