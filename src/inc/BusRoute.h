#ifndef BUS_ROUTE_H
#define BUS_ROUTE_H

#include "busstop.h"

#include <cstdint>
#include <unordered_set>
#include <vector>
#include <utility>
#include <optional>
#include <iostream>

class BusRoute
{
public:
  using BusStopId = std::unordered_set<BusStop, BusStopHasher>::iterator;

  struct RouteParams {
    size_t stops;
    size_t unique_stops;
    double length;
  };

  BusRoute(bool direction)
    : one_direction(direction)
  {}

  void AddStop(BusStop stop) {
    auto it = stops.insert(std::move(stop));
    if(route.size() == 0) {
      length = 0.0;
    } else {
      auto point1 = route.back()->GetLocation();
      auto point2 = it.first->GetLocation();
      length += CalcDistance(point1, point2);
    }
    route.push_back(it.first);
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

  RouteParams GetRouteParams() const {
    RouteParams params;

    params.unique_stops = stops.size();

    if(one_direction) {
      params.stops = route.size();
      params.length = length;
    } else {
      params.stops = (route.size() - 1) * 2 + 1;
      params.length = length * 2.0;
    }

    return params;
  }

  bool IsOneDirection() const {
    return one_direction;
  }
private:
  bool one_direction;
  std::unordered_set<BusStop, BusStopHasher> stops;
  double length;
  std::vector<BusStopId> route;
};

#endif // BUS_ROUTE_H
