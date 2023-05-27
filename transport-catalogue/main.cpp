#include "json_reader.h"
#include "request_handler.h"

int main()
{
    transport_catalogue::TransportCatalogue catalogue;

    json_reader::JsonReader doc(catalogue, std::cin);
    doc.UpdateCatalogue();
    doc.PrintStat(std::cout);

    return 0;
}
