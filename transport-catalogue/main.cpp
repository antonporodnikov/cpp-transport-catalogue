#include "json_reader.h"

#include <sstream>  // FOR TESTS

int main()
{
    // std::istringstream input{
    //     R"({
    //         "base_requests": [
    //             {
    //                 "type": "Bus",
    //                 "name": "114",
    //                 "stops": ["Morskoy vokzal", "Rivierskiy most"],
    //                 "is_roundtrip": false
    //             },
    //             {
    //                 "type": "Stop",
    //                 "name": "Rivierskiy most",
    //                 "latitude": 43.587795,
    //                 "longitude": 39.716901,
    //                 "road_distances": {"Morskoy vokzal": 850}
    //             },
    //             {
    //                 "type": "Stop",
    //                 "name": "Morskoy vokzal",
    //                 "latitude": 43.581969,
    //                 "longitude": 39.719848,
    //                 "road_distances": {"Rivierskiy most": 850}
    //             }
    //         ],
    //         "stat_requests": [
    //             { "id": 1, "type": "Stop", "name": "Rivierskiy most" },
    //             { "id": 2, "type": "Bus", "name": "114" }
    //             { "id": 3, "type": "Stop", "name": "Tesovaya most" },
    //             { "id": 4, "type": "Bus", "name": "113" }
    //         ]
    //     })"
    // };

    const domain::RequestQueue queue =
        json_reader::parsers::ParseJSON(std::cin);

    transport_catalogue::TransportCatalogue catalogue;

    json_reader::ProcessingInput(catalogue, queue);

    json_reader::ProcessingOutput(catalogue, queue, std::cout);

    return 0;
}
