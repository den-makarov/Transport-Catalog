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
  {
    std::hash<std::string> s_hash;
    stop_hash = s_hash(name);
  }

  size_t GetHash() const {
    return stop_hash;
  }
  const std::string& GetName() const {
    return name;
  }

  Location GetLocation() const {
    return location;
  }
private:
  Location location;
  std::string name;
  size_t stop_hash;
};

bool operator<(const BusStop& lhs, const BusStop& rhs);
double CalcDistance(Location from, Location to);

struct BusStopHasher {
  size_t operator()(const BusStop& stop) const {
    return stop.GetHash();
  }
};

#endif // BUSSTOP_H
