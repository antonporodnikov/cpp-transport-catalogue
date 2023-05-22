#include "stat_reader.h"

namespace stat_reader {

namespace details {

bool IsStop(const std::string& label_and_name)
{
    return label_and_name.find("Stop") != std::string::npos;
}

void CutSpaces(std::string& text)
{
    while (text[0] == ' ')
    {
        text.erase(0, 1);
    }

    while (*next(text.end(), -1) == ' ')
    {
        text.pop_back();
    }
}

std::string GetName(const std::string& request, const std::string& label)
{
    std::string name = request.substr(request.find(label) + label.size());
    CutSpaces(name);

    return name;
}

}

namespace prints {

void PrintValidStopRequest(TransportCatalogue& catalogue,
    const std::string& request, std::ostream& output)
{
    const std::string LABEL = "Stop";
    const std::string name = details::GetName(request, LABEL);

    output << LABEL << ' ' << name << ": ";

    const std::set<std::string_view> buses_to_stop =
        catalogue.GetBusesToStop(name);

    if (buses_to_stop.empty())
    {
        output << "no buses" << std::endl;

        return;
    }

    output << "buses";
    for (std::string_view bus : buses_to_stop)
    {
        output << ' ' << bus;
    }

    output << std::endl;
}

void PrintStopRequest(TransportCatalogue& catalogue, std::string& request,
    std::ostream& output)
{
    try
    {
        PrintValidStopRequest(catalogue, request, output);
    }
    catch (const std::invalid_argument&)
    {
        output << "not found" << std::endl;
    }
}

void PrintValidBusRequest(TransportCatalogue& catalogue,
    const std::string& request, std::ostream& output)
{
    const std::string LABEL = "Bus";
    const std::string name = details::GetName(request, LABEL);

    output << LABEL << ' ' << name << ": "
        << catalogue.ComputeStopsCount(name) << " stops on route, "
        << catalogue.ComputeUniqueStopsCount(name) << " unique stops, ";

    output << catalogue.ComputeRouteLength(name) << " route length, "
        << catalogue.ComputeCurvature(name) << " curvature";

    output << std::endl;
}

void PrintBusRequest(TransportCatalogue& catalogue, std::string& request,
    std::ostream& output)
{
    try
    {
        PrintValidBusRequest(catalogue, request, output);
    }
    catch (const std::invalid_argument&)
    {
        output << "not found" << std::endl;
    }
}

}

namespace parsers {

void ParseRequests(TransportCatalogue& catalogue, std::istream& input,
    std::ostream& output)
{
    std::string request;
    std::getline(input, request);
    const int request_count = std::stoi(request);
    for (int i = 0; i < request_count; ++i)
    {
        std::getline(input, request);
        
        if (details::IsStop(request))
        {
            prints::PrintStopRequest(catalogue, request, output);

            continue;
        }

        prints::PrintBusRequest(catalogue, request, output);
    }
}

}

void OutputResponse(TransportCatalogue& catalogue, std::istream& input,
    std::ostream& output)
{
    parsers::ParseRequests(catalogue, input, output);
}

}
