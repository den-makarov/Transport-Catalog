#ifndef BUSSTOPMAP_H
#define BUSSTOPMAP_H

#include <unordered_set>
#include <unordered_map>
#include <optional>

#include "busstop.h"
#include "bus.h"
#include "BusRoute.h"

class BusStopMap
{
public:
  using RouteId = std::unordered_map<Bus, BusRoute, BusHasher>::iterator;
  using ConstRouteId = std::unordered_map<Bus, BusRoute, BusHasher>::const_iterator;

  BusStopMap() {}
  BusRoute::BusStopId AddStop(const BusStop& new_stop) {
    return stops.insert(new_stop).first;
  }

  void AddBus(const Bus& new_bus, const BusRoute& new_route) {
    buses.insert({new_bus, new_route});
  }

  std::optional<BusRoute::BusStopId> GetStopByName(const std::string& stop_name) {
    BusStop dummy({stop_name, {0.0, 0.0}});
    auto it = stops.find(dummy);
    if(it != stops.end()) {
      return {it};
    } else {
      return {std::nullopt}; 
    }
  }

  std::optional<ConstRouteId> GetRouteByBus(const std::string& bus_name) const {
    Bus dummy(bus_name);
    auto it = buses.find(dummy);
    if(it != buses.end()) {
      return {it};
    } else {
      return {std::nullopt}; 
    }
  }

private:
  std::unordered_set<BusStop, BusStopHasher> stops;
  std::unordered_map<Bus, BusRoute, BusHasher> buses;
};

#endif // BUSSTOPMAP_H
