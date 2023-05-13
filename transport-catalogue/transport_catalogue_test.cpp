#include "transport_catalogue_test.h"

namespace transport_catalogue {

namespace tests {

namespace details {

void FillStopsFunc()
{
    const double EPSILON = 1e-6;

    std::vector<std::string> requests = {
        "   Stop   Tolstopaltsevo: 55.611087, 37.208290",
        "Stop Marushkino: 55.595884, 37.209755"
    };
    
    transport_catalogue::TransportCatalogue catalogue;
    catalogue.FillStops(requests);

    std::deque<input::Stop> stops = catalogue.GetStops();
    assert(stops.at(0).name == "Tolstopaltsevo");
    assert(stops.at(1).name == "Marushkino");
    assert((stops.at(0).coords.lat - 55.611087) < EPSILON);
    assert((stops.at(1).coords.lng - 37.209755) < EPSILON);
}

void FillBusesFunc()
{
    using StopnameToStopType = std::unordered_map<std::string_view,
        input::Stop*, transport_catalogue::details::ObjnameToObjHasher>; 

    std::istringstream input{
        "5\n"
        "Bus K 13: Mega > Novaya > Viha > Mega\n"
        "Bus 314A: Mega - Viha - Novaya\n"
        "Stop Mega: 55.611087, 37.208290\n"
        "Stop Novaya: 55.595884, 37.209755\n"
        "Stop Viha: 55.611087, 37.209755\n"
    };

    const input::RequestQueue request = input::Processing(input);

    transport_catalogue::TransportCatalogue catalogue;
    catalogue.FillStops(request.Stops);
    catalogue.CreateStopnameToStop();
    catalogue.FillBuses(request.Buses);
    const std::deque<input::Stop>& stops = catalogue.GetStops();
    StopnameToStopType stopname_to_stop = catalogue.GetStopnameToStop();
    const std::deque<input::Bus>& buses = catalogue.GetBuses();
    
    assert((buses[0].name == "K 13") && (buses[1].name == "314A"));
    assert(buses[0].stops[1] == stopname_to_stop.at("Novaya"));
    assert(buses[1].stops[3] == stopname_to_stop.at("Viha"));
}

void CreateStopnameToStopTableFunc()
{
    using StopnameToStopType = std::unordered_map<std::string_view,
        input::Stop*, transport_catalogue::details::ObjnameToObjHasher>; 

    const double EPSILON = 1e-6;

    std::vector<std::string> requests = {
        "   Stop   Tolstopaltsevo: 55.611087, 37.208290",
        "Stop Marushkino: 55.595884, 37.209755"
    };
    
    transport_catalogue::TransportCatalogue catalogue;
    catalogue.FillStops(requests);
    catalogue.CreateStopnameToStop();

    StopnameToStopType stopname_to_stop = catalogue.GetStopnameToStop();
    assert((stopname_to_stop.at("Tolstopaltsevo")->coords.lat - 55.611087) <
        EPSILON);
    assert((stopname_to_stop.at("Marushkino")->coords.lng - 37.209755) <
        EPSILON);
}

void CreateBusnameToBusTableFunc()
{
    using BusnameToBusType = std::unordered_map<std::string_view,
        input::Bus*, transport_catalogue::details::ObjnameToObjHasher>; 

    std::istringstream input{
        "5\n"
        "Bus K 13: Mega > Novaya > Viha > Mega\n"
        "Bus 314A: Mega - Viha - Novaya\n"
        "Stop Mega: 55.611087, 37.208290\n"
        "Stop Novaya: 55.595884, 37.209755\n"
        "Stop Viha: 55.611087, 37.209755\n"
    };

    const input::RequestQueue request = input::Processing(input);

    transport_catalogue::TransportCatalogue catalogue;
    catalogue.FillStops(request.Stops);
    catalogue.CreateStopnameToStop();
    catalogue.FillBuses(request.Buses);
    catalogue.CreateBusnameToBus();

    const std::deque<input::Bus>& buses = catalogue.GetBuses();
    const input::Bus* k_13 = &buses.at(0);
    BusnameToBusType busname_to_bus = catalogue.GetBusnameToBus();
    
    assert(busname_to_bus.at("K 13") == k_13);
}

}

void TransportCatalogueModule()
{
    using namespace transport_catalogue::tests::details;

    FillStopsFunc();
    CreateStopnameToStopTableFunc();
    FillBusesFunc();
    CreateBusnameToBusTableFunc();
}

}

}
