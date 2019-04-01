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
  using StopBoard = std::set<Bus> *;
  using RouteId = std::unordered_map<Bus, BusRoute, BusHasher>::iterator;
  using ConstRouteId = std::unordered_map<Bus, BusRoute, BusHasher>::const_iterator;

  BusStopMap() {}

  void AddSettings(double new_velocity, int new_wait_time) {
    velocity = new_velocity;
    wait_time = new_wait_time;
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

private:
  void BuildPathGraph() {
    for(const auto& bus : buses) {
      stopsXbuses += bus.second.GetStopsCount();
    }

    path_graph = std::make_unique<Graph::DirectedWeightedGraph<double>>(stopsXbuses);

    Graph::VertexId vertex = 0;
    for(const auto& [bus, route] : buses) {
      const auto& stops = route.GetStopsOnRoute();
      
      const auto prev_stop = *stops.begin();
      for(const auto& stop : stops) {
        stops_vertexes[stop->GetName()].push_back(vertex);
        stop_route_ids.insert({vertex, {bus, stop->GetName()}});
        vertex++;
        if(prev_stop != stop) {
          auto distance = GetDistance(prev_stop, stop);
          if(route.IsOneDirection()) {
            
          } else {

          }
        }
      }
    }
  }

  BusRoute::Distance GetDistance(BusRoute::BusStopId from, BusRoute::BusStopId to) {
    unsigned long forward_distance = 0;
    unsigned long back_distance = 0;
    
    auto dist_to = from->GetDistanceInfo(to->GetName());
    auto dist_back = to->GetDistanceInfo(from->GetName());

    if(dist_to) {
      forward_distance = dist_to.value();
    } else if (dist_back) {
      forward_distance = dist_back.value();
    }

    if(dist_back) {
      back_distance = dist_back.value();
    } else if(dist_to) {
      back_distance = dist_to.value();
    }
    return {forward_distance, forward_distance};
  }

  std::unordered_map<BusStop, std::set<Bus>, BusStopHasher> stops;
  std::unordered_map<Bus, BusRoute, BusHasher> buses;

  double velocity;
  int wait_time;
  size_t stopsXbuses = 0;

  std::unordered_map<Graph::VertexId, std::pair<const Bus&, const std::string&>> stop_route_ids;
  std::unordered_map<std::string, std::vector<Graph::VertexId>> stops_vertexes;
  std::unique_ptr<Graph::DirectedWeightedGraph<double>> path_graph;
};

#endif // BUSSTOPMAP_H
