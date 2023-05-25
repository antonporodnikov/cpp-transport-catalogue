#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"

#include <sstream>  // FOR TESTS

int main()
{
    transport_catalogue::TransportCatalogue catalogue;

    json_reader::JsonReader doc(std::cin);
    doc.UpdateCatalogue(catalogue);

    const map_renderer::MapRenderer map(doc.GetRenderSettings());

    const request_handler::RequestHandler request(catalogue, map);

    svg::Document test = request.RenderMap();
    test.Render(std::cout);

    return 0;
}
