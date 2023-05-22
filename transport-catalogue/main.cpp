#include "input_reader.h"
#include "stat_reader.h"

int main()
{
    transport_catalogue::TransportCatalogue catalogue;

    input_reader::ProcessingInput(catalogue, std::cin);
    
    stat_reader::OutputResponse(catalogue, std::cin, std::cout);

    return 0;
}
