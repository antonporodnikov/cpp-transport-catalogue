#include "geo_test.h"

namespace geo {

namespace tests {

void ComputeDistanceFunc()
{
    const double EPSILON = 1e-6;

    Coordinates from = {55.611087, 37.208290};
    Coordinates to = {55.611087, 37.208290};
    assert(ComputeDistance(from, to) == 0);

    from = {55.611087, 37.208290};
    to = {55.595884, 37.209755};
    assert((ComputeDistance(from, to) - 1693.0) < EPSILON);
}

void GeoModule()
{
    ComputeDistanceFunc();
}

}

}
