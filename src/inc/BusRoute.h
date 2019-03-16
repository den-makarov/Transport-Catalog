#ifndef BUS_ROUTE_H
#define BUS_ROUTE_H

#include "busstop.h"

#include <cstdint>
#include <set>
#include <vector>
#include <utility>
#include <optional>
#include <iostream>

class BusRoute
{
public:
  BusRoute(bool direction)
    : one_direction(direction)
  {}

  BusRoute& AddStop(BusStop stop) {
    auto it = stops.insert(std::move(stop));
    route.push_back(it.first);
    return *this;
  }

  std::optional<BusStop> GetStop(const std::string& name) const {
    BusStop dummy(name, {0.0, 0.0});
    auto it = stops.find(dummy);
    if(it != stops.end()) {
      return {*it};
    } else {
      return {std::nullopt};
    }
  }

  void PrintRoute(std::ostream& out) const {
    for(auto stop : route) {
      out << stop->GetName();
      if(one_direction) {
        out << " > ";
      } else {
        out << " - ";
      }
    }
  }
private:
  bool one_direction;
  std::set<BusStop> stops;
  double length;
  using BusStopId = std::set<BusStop>::iterator;
  std::vector<BusStopId> route;
};

#endif // BUS_ROUTE_H
