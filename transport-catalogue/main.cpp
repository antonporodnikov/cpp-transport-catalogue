#include "input_reader.h"
#include "transport_catalogue.h"
#include "stat_reader.h"

#include <iostream>

int main()
{
    transport_catalogue::TransportCatalogue catalogue;

    input_reader::ProcessingInput(catalogue, std::cin);
    
    stat_reader::OutputResponse(catalogue, std::cin);

    return 0;
}
