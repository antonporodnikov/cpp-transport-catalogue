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
    const std::string& request)
{
    const std::string LABEL = "Stop";
    const std::string name = details::GetName(request, LABEL);

    std::cout << LABEL << ' ' << name << ": ";

    const std::set<std::string_view> buses_to_stop =
        catalogue.GetBusesToStop(name);

    if (buses_to_stop.empty())
    {
        std::cout << "no buses" << std::endl;

        return;
    }

    std::cout << "buses";
    for (std::string_view bus : buses_to_stop)
    {
        std::cout << ' ' << bus;
    }

    std::cout << std::endl;
}

void PrintStopRequest(TransportCatalogue& catalogue, std::string& request)
{
    try
    {
        PrintValidStopRequest(catalogue, request);
    }
    catch (const std::invalid_argument&)
    {
        std::cout << "not found" << std::endl;
    }
}

void PrintValidBusRequest(TransportCatalogue& catalogue,
    const std::string& request)
{
    const std::string LABEL = "Bus";
    const std::string name = details::GetName(request, LABEL);

    std::cout << LABEL << ' ' << name << ": "
        << catalogue.ComputeStopsCount(name) << " stops on route, "
        << catalogue.ComputeUniqueStopsCount(name) << " unique stops, ";

    std::cout << catalogue.ComputeRouteLength(name) << " route length, "
        << catalogue.ComputeCurvature(name) << " curvature";

    std::cout << std::endl;
}

void PrintBusRequest(TransportCatalogue& catalogue, std::string& request)
{
    try
    {
        PrintValidBusRequest(catalogue, request);
    }
    catch (const std::invalid_argument&)
    {
        std::cout << "not found" << std::endl;
    }
}

}

namespace parsers {

void ParseRequests(TransportCatalogue& catalogue, std::istream& input)
{
    std::string request;
    std::getline(input, request);
    const int request_count = std::stoi(request);
    for (int i = 0; i < request_count; ++i)
    {
        std::getline(input, request);
        
        if (details::IsStop(request))
        {
            prints::PrintStopRequest(catalogue, request);

            continue;
        }

        prints::PrintBusRequest(catalogue, request);
    }
}

}

void OutputResponse(TransportCatalogue& catalogue, std::istream& input)
{
    parsers::ParseRequests(catalogue, input);
}

}
