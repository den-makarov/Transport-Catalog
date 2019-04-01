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
    wait_time = new_wait_time * 1.0 / 60.0;
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

  std::optional<double> GetOptimalPath(const std::string& stop_from,
                                                               const std::string& stop_to) const {
    std::optional<double> total_time = {std::nullopt};
    if(path_router) {
      auto from_stop_it = stops_vertexes.find(stop_from);
      auto to_stop_it = stops_vertexes.find(stop_to);

      if(from_stop_it != stops_vertexes.end() && to_stop_it != stops_vertexes.end()) {
        auto result = path_router->BuildRoute(from_stop_it->second[0], to_stop_it->second[0]);
        optimal_routes.push_back(result);
        if(result) {
          total_time = result.value().weight - 2 * wait_time;
        }
      }
    }
    return total_time;
  }

private:
  void BuildPathGraph() {
    for(const auto& bus : buses) {
      stopsXbuses += bus.second.GetStopsCount();
    }

    path_graph = std::make_unique<Graph::DirectedWeightedGraph<double>>(stopsXbuses);

    Graph::VertexId vertex = 0;
    /* Fill graph with vertexes */
    for(const auto& [bus, route] : buses) {
      const auto& stops = route.GetStopsOnRoute();
      const auto prev_stop = *stops.begin();

      /* for each bus route itterate on all stops */
      for(const auto& stop : stops) {
        /* Save for each Stop vertexes that could be touch by every route */
        stops_vertexes[stop->GetName()].push_back(vertex);
        stop_route_ids.insert({vertex, {bus, stop->GetName()}});

        /* Calculate weight for all segments in a route */
        if(prev_stop != stop) {
          auto distance = GetDistance(prev_stop, stop);
          if(route.IsOneDirection()) {
            /* Add vertex in forward direction for round route */
            double weight = distance.forward != 0
                            ? distance.forward / velocity
                            : distance.back / velocity;
            Graph::Edge<double> edge = {vertex - 1, vertex, weight};
            path_graph->AddEdge(edge);
          } else {
            /* Add vertex in forward direction for ordinary route */
            double forward_weight = distance.forward / velocity;
            Graph::Edge<double> forward_edge = {vertex - 1, vertex, forward_weight};
            path_graph->AddEdge(forward_edge);

            /* Add vertex in back direction for ordinary route */
            double back_weight = distance.back / velocity;
            Graph::Edge<double> back_edge = {vertex, vertex - 1, back_weight};
            path_graph->AddEdge(back_edge);
          }
        }
        vertex++;
      }
    }
    /* Add edges on each stop in order to change route */
    for(const auto& [stop, vertexes] : stops_vertexes) {
      for(const auto& vertex : vertexes) {
        for(const auto& other : vertexes) {
          if(other != vertex) {
            Graph::Edge<double> edge = {vertex, other, wait_time};
            path_graph->AddEdge(edge);
          }
        }
      }
    }

    path_router = std::make_unique<Graph::Router<double>>(*path_graph);
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
  double wait_time;
  size_t stopsXbuses = 0;

  std::unordered_map<Graph::VertexId, std::pair<const Bus&, const std::string&>> stop_route_ids;
  std::unordered_map<std::string, std::vector<Graph::VertexId>> stops_vertexes;
  std::unique_ptr<Graph::DirectedWeightedGraph<double>> path_graph;
  std::unique_ptr<Graph::Router<double>> path_router;
  mutable std::vector<std::optional<Graph::Router<double>::RouteInfo>> optimal_routes;
};

#endif // BUSSTOPMAP_H
