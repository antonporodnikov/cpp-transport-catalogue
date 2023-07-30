#include "json_reader.h"
#include "serialization.h"
#include "transport_catalogue.h"

#include <fstream>
#include <iostream>
#include <string_view>

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    transport_catalogue::TransportCatalogue catalogue;
    serialization::SerializationMachine sm(catalogue);
    json_reader::JsonReader json_reader(catalogue, sm, std::cin);
        
    if (mode == "make_base"sv) {
        json_reader.UpdateCatalogue();
        json_reader.Serialize();

    } else if (mode == "process_requests"sv) {
        json_reader.Deserialize();
        json_reader.PrintStat(std::cout);

    } else {
        PrintUsage();
        return 1;
    }
}
