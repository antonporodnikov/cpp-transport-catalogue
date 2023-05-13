#include "geo.h"
#include "geo_test.h"
#include "input_reader.h"
#include "input_reader_test.h"
#include "stat_reader.h"
#include "transport_catalogue.h"
#include "transport_catalogue_test.h"

#include <iostream>
#include <sstream>

int main()
{
    using namespace std::string_literals;

    // {
    //     geo::tests::GeoModule();
    //     input::tests::InputReaderModule();
    //     transport_catalogue::tests::TransportCatalogueModule();
    //     std::cout << "Tests completed successfully"s << std::endl;
    // }

    const input::RequestQueue request = input::Processing(std::cin);

    transport_catalogue::TransportCatalogue catalogue;
    catalogue.RefreshCatalogue(request);

    stat_reader::Output(std::cin, catalogue);

    return 0;
}
