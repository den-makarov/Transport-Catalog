#ifndef BUSSTOPMAP_H
#define BUSSTOPMAP_H

#include <set>
#include <unordered_map>
#include <optional>

#include "busstop.h"
#include "bus.h"
#include "BusRoute.h"

#include "graph.h"
#include "router.h"

class BusStopMap
{
public:
  class RoutePoint;
  using StopBoard = std::set<Bus> *;
  using RouteId = std::unordered_map<Bus, BusRoute, BusHasher>::iterator;
  using ConstRouteId = std::unordered_map<Bus, BusRoute, BusHasher>::const_iterator;
  using Results = std::vector<RoutePoint>;

  struct Weight
  {
    double weight;
    const std::string * bus_number;
    
    Weight operator+(const Weight& other) const {
      Weight new_weight(weight + other.weight, 
                        other.bus_number);

      return new_weight;
    }

    bool operator<(const Weight& other) const {
      return weight < other.weight;
    }

    bool operator>(const Weight& other) const {
      return weight > other.weight;
    }

    bool operator>=(double value) const {
      return weight >= value;
    }

    Weight(double val, const std::string * str)
    : weight(val)
    , bus_number(str)
    {}

    Weight(double val) 
    : weight(val)
    , bus_number(nullptr)
    {}
  };

  BusStopMap() {}

  enum class ActivityType {
    STOP,
    BUS,
    TOTAL_TIME
  };

  class RoutePoint {
  public:
    RoutePoint(BusStopMap::ActivityType t, const std::string& n, double weight)
      : type(t)
      , name(n)
      , time(weight)
    {}

    const std::string& GetName() const {
      return name;
    }

    double GetTime() const {
      return time;
    }

    void UpdateTime(double t) {
      time = t;
    }

    bool IsStop() const {
      return type == ActivityType::STOP;
    }
  private:
    BusStopMap::ActivityType type;
    const std::string& name;
    double time;
  };

  void AddSettings(double new_velocity, int new_wait_time) {
    velocity = new_velocity * 1000.0 / 60.0;
    wait_time = new_wait_time * 1.0;
    BuildPathGraph();
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
  Results GetOptimalPath(const std::string& stop_from, const std::string& stop_to) const;
private:
  size_t GetBusesCountOnStop(const std::string& stop, std::optional<BusStop> stop_id);
  void BuildPathGraph();
  BusRoute::Distance GetDistance(BusRoute::BusStopId from, BusRoute::BusStopId to);
  Results ParseOptimalPath(const Graph::Router<Weight>::RouteInfo& result,
                                           double weight) const;

  std::unordered_map<BusStop, std::set<Bus>, BusStopHasher> stops;
  std::unordered_map<Bus, BusRoute, BusHasher> buses;

  double velocity;
  double wait_time;
  size_t stopsXbuses = 0;

  std::unordered_map<Graph::VertexId, std::pair<const std::string&, const std::string&>> stop_route_ids;
  std::unordered_map<std::string, std::vector<Graph::VertexId>> stops_vertexes;
  
  std::unique_ptr<Graph::DirectedWeightedGraph<Weight>> path_graph;
  std::unique_ptr<Graph::Router<Weight>> path_router;
  mutable std::vector<std::optional<Graph::Router<Weight>::RouteInfo>> optimal_routes;
  const std::string wait_stop = "wait_stop";
};

#endif // BUSSTOPMAP_H
