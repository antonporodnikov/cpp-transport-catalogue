#include "stat_reader.h"

namespace stat_reader {

namespace compute {

double Length(const input::Bus* bus_ptr)
{
    double result = 0.0;
    for (auto it = bus_ptr->stops.begin();
        it != std::next(bus_ptr->stops.end(), -1); it = std::next(it, +1)) 
    {
        input::Stop* stop_a = *it;
        input::Stop* stop_b = *(it + 1);
        result += geo::ComputeDistance(stop_a->coords, stop_b->coords);
    }

    return result;
}

std::tuple<int, int, double> Data(
    const transport_catalogue::TransportCatalogue& catalogue,
    std::string& name)
{
    using BusnameToBusType = std::unordered_map<std::string_view,
        input::Bus*, transport_catalogue::details::ObjnameToObjHasher>; 
    
    const BusnameToBusType& busname_to_bus = catalogue.GetBusnameToBus();
    input::Bus* bus_ptr = busname_to_bus.at(name);
    const int stops_amount = bus_ptr->stops.size();

    std::vector<input::Stop*> stops = bus_ptr->stops;
    std::sort(stops.begin(), stops.end());
    int unique_stops = std::unique(stops.begin(), stops.end()) - stops.begin();

    const double route_lenght = Length(bus_ptr);

    return {stops_amount, unique_stops, route_lenght};
}

int Curv(const transport_catalogue::TransportCatalogue& catalogue,
    std::string& name)
{
    using StopsDistanceType = std::unordered_map<std::pair<input::Stop*,
        input::Stop*>, int, transport_catalogue::details::StopsDistanceHasher>;

    using BusnameToBusType = std::unordered_map<std::string_view,
        input::Bus*, transport_catalogue::details::ObjnameToObjHasher>; 
    
    const BusnameToBusType& busname_to_bus = catalogue.GetBusnameToBus();
    input::Bus* bus_ptr = busname_to_bus.at(name);

    const StopsDistanceType& stops_distance = catalogue.GetStopsDistance();

    int lenght = 0;
    auto first_stop = bus_ptr->stops.begin();
    auto second_stop = bus_ptr->stops.begin();
    second_stop++;
    for (; second_stop != bus_ptr->stops.end(); ++first_stop, ++second_stop)
    {
        if (stops_distance.count({*first_stop, *second_stop}) == 0)
        {
            lenght += stops_distance.at({*second_stop, *first_stop});

            continue;
        }

        lenght += stops_distance.at({*first_stop, *second_stop});
    }

    return lenght;
}

}

void ProcessOutputBus(const transport_catalogue::TransportCatalogue& catalogue,
    std::string& name)
{
    using BusnameToBusType = std::unordered_map<std::string_view,
        input::Bus*, transport_catalogue::details::ObjnameToObjHasher>; 
    
    const BusnameToBusType& busname_to_bus = catalogue.GetBusnameToBus();

    if (busname_to_bus.count(name) == 0)
    {
        std::cout << "Bus " << name << ": " << "not found" << std::endl;
        
        return;
    }

    std::tuple<int, int, double> route_data = compute::Data(catalogue, name);
    
    int stops = std::get<0>(route_data);
    int unique_stops = std::get<1>(route_data);
    int lenght = compute::Curv(catalogue, name);
    
    double lenght_raw = std::get<2>(route_data);
    double curv = lenght / static_cast<double>(lenght_raw);

    std::cout << "Bus " << name << ": "
        << stops << " stops on route, "
        << unique_stops << " unique stops, "
        << std::scientific << std::setprecision(5)
        << static_cast<double>(lenght) << " route length, ";

        std::cout << std::defaultfloat;
        std::cout << curv << " curvature" << std::endl;
}

void ProcessOutputStop(const transport_catalogue::TransportCatalogue& catalogue,
    std::string& name)
{
    using StopToBusesType = std::unordered_map<std::string,
        std::set<std::string>,
        transport_catalogue::details::ObjnameToObjHasherStr>;
    
    std::cout << "Stop " << name << ": ";

    const StopToBusesType& stop_to_buses = catalogue.GetStopToBuses();

    if (stop_to_buses.count(name) == 0)
    {
        std::cout << "not found" << std::endl;

        return;
    }

    if (stop_to_buses.at(name).empty())
    {
        std::cout << "no buses" << std::endl;

        return;
    }

    std::cout << "buses";
    const std::set<std::string>& buses_at_stop = stop_to_buses.at(name);
    for (auto it = buses_at_stop.begin(); it != buses_at_stop.end();
        it = std::next(it, +1))
    {
        std::cout << ' ' << *it;
    }

    std::cout << std::endl;
}

void Output(std::istream& input,
    const transport_catalogue::TransportCatalogue& catalogue)
{
    std::string request;
    std::getline(input, request);
    int request_count = std::stoi(request);
    for (int i = 0; i < request_count; ++i)
    {
        std::getline(input, request);
        std::string label_name = request.substr(0, request.find(':'));
        if (label_name.find("Bus") != std::string::npos)
        {
            std::string name = input::parsers::Output(request, "Bus");
            ProcessOutputBus(catalogue, name);

            continue;
        }

        std::string name = input::parsers::Output(request, "Stop");
        ProcessOutputStop(catalogue, name);
    }
}

}
