#ifndef BUSSTOPMAP_H
#define BUSSTOPMAP_H

#include <set>
#include <unordered_map>
#include <optional>

#include "busstop.h"
#include "bus.h"
#include "BusRoute.h"

class BusStopMap
{
public:
  using StopBoard = std::set<Bus> *;
  using RouteId = std::unordered_map<Bus, BusRoute, BusHasher>::iterator;
  using ConstRouteId = std::unordered_map<Bus, BusRoute, BusHasher>::const_iterator;

  BusStopMap() {}

  void AddSettings(int new_velocity, int new_wait_time) {
    velocity = new_velocity;
    wait_time = new_wait_time;
  }

  void AddStop(const BusStop& new_stop) {
    stops.insert({new_stop, {}});
  }

  void AddBus(const Bus& new_bus, BusRoute new_route) {
    auto it = buses.insert(std::pair<const Bus, BusRoute>(new_bus, BusRoute(true)));
    it.first->second = std::move(new_route);
    const auto & route = it.first->second.GetStopsOnRoute();
    for(const auto & stop : route) {
      stops.at(*stop).insert(new_bus);
    }
  }

  std::optional<BusStop> GetStopByName(const std::string& stop_name) const {
    BusStop dummy({stop_name, {0.0, 0.0}});
    auto it = stops.find(dummy);
    if(it != stops.end()) {
      return {it->first};
    } else {
      return {std::nullopt}; 
    }
  }

  std::optional<const std::set<Bus>*> GetStopBoardByName(const std::string& stop_name) const {
    BusStop dummy({stop_name, {0.0, 0.0}});
    auto it = stops.find(dummy);
    if(it != stops.end()) {
      return {&(it->second)};
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
  std::unordered_map<BusStop, std::set<Bus>, BusStopHasher> stops;
  std::unordered_map<Bus, BusRoute, BusHasher> buses;
  int velocity;
  int wait_time;
};

#endif // BUSSTOPMAP_H
