#ifndef ROUTE_H
#define ROUTE_H

#include "busstop.h"

#include <cstdint>
#include <set>
#include <vector>
#include <utility>
#include <optional>
#include <iostream>

class Route
{
public:
  Route(bool direction, size_t identifier)
    : one_direction(direction)
    , id(identifier)
  {}

  Route& AddStop(BusStop stop) {
    auto it = stops.insert(std::move(stop));
    route.push_back(it.first);
    return *this;
  }

  size_t GetId() const {
    return id;
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
  std::size_t id;
  std::set<BusStop> stops;
  double length;
  using BusStopId = std::set<BusStop>::iterator;
  std::vector<BusStopId> route;
};

#endif // ROUTE_H
