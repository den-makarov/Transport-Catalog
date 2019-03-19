#ifndef BUS_ROUTE_H
#define BUS_ROUTE_H

#include <cstdint>
#include <unordered_set>
#include <set>
#include <unordered_map>
#include <vector>
#include <utility>
#include <optional>
#include <iostream>

#include "busstop.h"
#include "bus.h"

class BusRoute
{
public:
  using BusStopId = std::unordered_set<BusStop, BusStopHasher>::iterator;

  struct RouteParams {
    size_t stops;
    size_t unique_stops;
    double geo_length;
    double distance;
  };

  BusRoute(bool direction)
    : one_direction(direction)
    , total_distance(0)
  {}

  void AddStop(BusStop stop) {
    auto it = stops.insert(stop);
    if(route.size() == 0) {
      geo_length = 0.0;
      total_distance = 0;
    } else {
      auto point1 = route.back()->GetLocation();
      auto point2 = it.first->GetLocation();
      geo_length += CalcDistance(point1, point2);
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

  const std::vector<BusStopId>& GetStopsOnRoute() const {
    return route;
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

    if(total_distance == 0) {
      auto prev_id = *route.begin();
      for(const auto id : route) {
        auto dist1 = prev_id->GetDistanceInfo(id->GetName());
        auto dist2 = id->GetDistanceInfo(prev_id->GetName());
        prev_id = id;

        if(dist2) {
          total_distance += dist2.value();
        } else if (dist1) {
          total_distance += dist1.value();
        }
      }
    }
    params.unique_stops = stops.size();

    if(one_direction) {
      params.stops = route.size();
      params.geo_length = geo_length;
      params.distance = total_distance;
    } else {
      params.stops = (route.size() - 1) * 2 + 1;
      params.geo_length = geo_length * 2.0;
      params.distance = total_distance * 2;
    }

    return params;
  }

  bool IsOneDirection() const {
    return one_direction;
  }
private:
  bool one_direction;
  std::unordered_set<BusStop, BusStopHasher> stops;
  double geo_length;
  mutable double total_distance;
  std::vector<BusStopId> route;
};

#endif // BUS_ROUTE_H
