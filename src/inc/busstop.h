#ifndef BUSSTOP_H
#define BUSSTOP_H

#include <string>
#include <cmath>
#include <map>
#include <utility>
#include <memory>

struct Location {
  double longitude;
  double latidute;
};

class BusStop
{
public:
  using DistanceSet = std::shared_ptr<std::map<std::string, unsigned long>>;

  BusStop(const std::string& n, Location geo)
    : location(geo)
    , name(n)
  {
    std::hash<std::string> s_hash;
    stop_hash = s_hash(name);
  }

  BusStop(const BusStop& other) {
    stop_hash = other.stop_hash;
    location = other.location;
    name = other.name;
    distances = other.distances;
  }

  void AddDistanceInfo(DistanceSet new_set) {
    distances = new_set;
  }

  std::optional<unsigned long> GetDistanceInfo(const std::string& stop_name) const {
    auto it = distances->find(stop_name);
    if(it != distances->end()) {
      return {it->second};
    } else {
      return {std::nullopt};
    }
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
  DistanceSet distances;
};

bool operator<(const BusStop& lhs, const BusStop& rhs);
bool operator==(const BusStop& lhs, const BusStop& rhs);
double CalcDistance(Location from, Location to);

struct BusStopHasher {
  size_t operator()(const BusStop& stop) const {
    return stop.GetHash();
  }
};

#endif // BUSSTOP_H
