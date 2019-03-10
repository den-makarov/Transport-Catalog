#ifndef BUSSTOP_H
#define BUSSTOP_H

#include <string>
#include <cmath>

struct Location {
  double longitude;
  double latidute;
};

class BusStop
{
public:
  BusStop(const std::string& n, Location geo)
    : location(geo)
    , name(n)
  {}

  const std::string& GetName() const {
    return name;
  }

  Location GetLocation() const {
    return location;
  }
private:
  Location location;
  std::string name;
};

bool operator<(const BusStop& lhs, const BusStop& rhs) {
  return lhs.GetName() < rhs.GetName();
}

struct BusStopHasher {
  size_t operator()(const BusStop& stop) {
    return s_hash(stop.GetName());
  }
  std::hash<std::string> s_hash;
};

double CalcDistance(Location from, Location to);

#endif // BUSSTOP_H
