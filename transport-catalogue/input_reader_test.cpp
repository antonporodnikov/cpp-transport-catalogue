#include "input_reader_test.h"

namespace input {

namespace tests {

namespace details {

void ProcessingFunc()
{
    std::istringstream input{
        "4\n"
        "   Bus   256: Biryulyovo > Biryusinka > Biryulyovo\n"
        "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka\n"
        "   Stop   Tolstopaltsevo: 55.611087, 37.208290\n"
        "Stop Marushkino: 55.595884, 37.209755\n"
    };
    const RequestQueue result = input::Processing(input);

    assert(result.Stops.at(0) == 
        "   Stop   Tolstopaltsevo: 55.611087, 37.208290");
    assert(result.Stops.at(1) == 
        "Stop Marushkino: 55.595884, 37.209755");
    assert(result.Buses.at(0) == 
        "   Bus   256: Biryulyovo > Biryusinka > Biryulyovo");
    assert(result.Buses.at(1) == 
        "Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka");
}

void StopFunc()
{
    using input::parsers::Stop;

    {
        std::string stop_request = "Stop Tolstopaltsevo: 55.611087, 37.208290";
        input::Stop stop = Stop(stop_request);

        assert(stop.name == "Tolstopaltsevo");
        assert((stop.coords == geo::Coordinates{55.611087, 37.208290}));
    }
    {

        std::string stop_request = " Stop  Marushkino : 55.595884 , 37.209755 ";
        input::Stop stop = Stop(stop_request);

        assert(stop.name == "Marushkino");
        assert((stop.coords == geo::Coordinates{55.595884, 37.209755}));
    }
}

void BusFunc()
{
    using input::parsers::Bus;

    {
        std::string bus_request = "Bus 256: Pushkin > Universam > Pushkin";
        std::vector<std::string> route = {"Pushkin", "Universam", "Pushkin"};
        std::tuple<std::string, std::vector<std::string>, bool> result =
            {"256", route, true};
        assert((Bus(bus_request) == result));
    }
    {
        std::string bus_request = " Bus K 256:   Pushkin - Universam  - Mega ";
        std::vector<std::string> route = {"Pushkin", "Universam", "Mega"};
        std::tuple<std::string, std::vector<std::string>, bool> result =
            {"K 256", route, false};
        assert((Bus(bus_request) == result));
    }
}

void StopsDistanceFunc()
{
    std::string stop_request = "Stop Biryulyovo Zapadnoye: 55.574371, \
        37.6517, 7500m to Rossoshanskaya ulitsa, 1800m to Biryusinka, \
        2400m to Universam";
    std::pair<std::string, std::vector<std::string>> result = {
        "Biryulyovo Zapadnoye", {"7500m to Rossoshanskaya ulitsa",
        "1800m to Biryusinka", "2400m to Universam"}
    };

    std::pair<std::string, std::vector<std::string>> test =
        input::parsers::StopsDistance(stop_request);
    assert(test == result);
}

}

void InputReaderModule()
{
    using namespace input::tests::details;

    ProcessingFunc();
    StopFunc();
    BusFunc();
    StopsDistanceFunc();
}

}

}
