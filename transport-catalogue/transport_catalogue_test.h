#pragma once

#include "input_reader.h"
#include "transport_catalogue.h"

#include <cassert>
#include <sstream>

namespace transport_catalogue {

namespace tests {

namespace details {

void FillStopsFunc();

void CreateStopnameToStopTableFunc();

void FillBusesFunc();

void CreateBusnameToBusTableFunc();

}

void TransportCatalogueModule();

}

}
