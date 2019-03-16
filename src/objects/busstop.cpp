#include "busstop.h"

static constexpr double GEO_PI = 3.1415926535;
static constexpr double EARTH_RADIUS = 6371000;

using namespace std;

double CalcDistance(Location from, Location to) {
  double latA = from.latidute * GEO_PI / 180.0;
  double latB = to.latidute * GEO_PI / 180.0;
  double lonA = from.longitude * GEO_PI / 180.0;
  double lonB = to.longitude * GEO_PI / 180.0;
  double arc = acos(sin(latA) * sin(latB) +
                    cos(latA) * cos(latB) *
                    cos(lonA - lonB) ) * 180 / GEO_PI;
  return arc * EARTH_RADIUS;
}

bool operator<(const BusStop& lhs, const BusStop& rhs) {
  return lhs.GetName() < rhs.GetName();
}