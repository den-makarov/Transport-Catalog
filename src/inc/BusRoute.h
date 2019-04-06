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

  struct Distance {
    unsigned long forward;
    unsigned long back;
  };

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
  BusRoute(BusRoute&& other);
  BusRoute& operator=(BusRoute&& other);
  //  BusRoute(const BusRoute& other) {
  //    std::cout << "Warning: Route Copied!" << std::endl;
  //  }

  void AddStop(BusStop stop);
  std::optional<BusStop> GetStop(const std::string& name) const;
  void PrintRoute(std::ostream& out) const;
  RouteParams GetRouteParams() const;
  const std::vector<BusStopId>& GetStopsOnRoute() const { return route; }
  size_t GetStopsCount() const { return route.size(); }
  size_t GetUniqueStopsCount() const { return stops.size(); }
  bool IsOneDirection() const { return one_direction; }

private:
  bool one_direction;
  std::unordered_set<BusStop, BusStopHasher> stops;
  double geo_length;
  mutable double total_distance;
  std::vector<BusStopId> route;
};

#endif // BUS_ROUTE_H
