#ifndef BUSSTOPMAP_H
#define BUSSTOPMAP_H

#include <unordered_set>
#include <list>

#include "busstop.h"
#include "bus.h"

class BusStopMap
{
public:
  BusStopMap() {}
  void AddStop(const BusStop& new_stop) {
    stops.insert(new_stop);
  }
  void AddBus(const Bus& new_bus) {
    std::list<StopsId> route_map;
  }

private:
  using StopsId = std::unordered_set<BusStop, BusStopHasher>::iterator;
  std::unordered_set<BusStop, BusStopHasher> stops;
  std::unordered_map<Bus, typename std::list<StopsId>, BusHasher> buses;
  // std::unique_map<string, StopsId> buses;
};

#endif // BUSSTOPMAP_H
