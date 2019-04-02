#include "BusStopMap.h"

std::vector<BusStopMap::RoutePoint> BusStopMap::GetOptimalPath(const std::string &stop_from, const std::string &stop_to) const {
  if(path_router) {
    auto from_stop_it = stops_vertexes.find(stop_from);
    auto to_stop_it = stops_vertexes.find(stop_to);

    if(from_stop_it != stops_vertexes.end() && to_stop_it != stops_vertexes.end()) {
      auto result = path_router->BuildRoute(from_stop_it->second[0], to_stop_it->second[0]);
      optimal_routes.push_back(result);

      if(result) {
        return ParseOptimalPath(result.value(), result.value().weight);
      }
    }
  }
  return std::vector<BusStopMap::RoutePoint>();
}

std::vector<BusStopMap::RoutePoint> BusStopMap::ParseOptimalPath(const Graph::Router<double>::RouteInfo& result,
                                                                 double weight) const {
  std::vector<BusStopMap::RoutePoint> points(1, {BusStopMap::Type::TOTAL_TIME, "total_time", weight});
  auto route_id = result.id;
  auto edge_count = result.edge_count;

  for(size_t idx = 0; idx < edge_count; idx++) {
    auto edge_id = path_router->GetRouteEdge(route_id, idx);
    auto edge = path_graph->GetEdge(edge_id);
    auto stop_from_it = stop_route_ids.find(edge.from);
    auto stop_to_it = stop_route_ids.find(edge.to);

    if(stop_from_it != stop_route_ids.end() && stop_to_it != stop_route_ids.end()) {
      const auto& stop_from = stop_from_it->second;
      const auto& stop_to = stop_to_it->second;

      if(stop_from.second == stop_to.second) {
        /* It is a stop */
        BusStopMap::RoutePoint stop(BusStopMap::Type::STOP, stop_from.second, edge.weight);
        points.push_back(std::move(stop));
      } else {
        /* It is a bus */
        if(idx == 0) {
          points[0].UpdateTime(points[0].GetTime() + wait_time);
          BusStopMap::RoutePoint stop(BusStopMap::Type::STOP, stop_from.second, wait_time);
          points.push_back(std::move(stop));
        }

        BusStopMap::RoutePoint bus(BusStopMap::Type::BUS, stop_from.first.GetNumber(), edge.weight);
        points.push_back(std::move(bus));
      }
    }
  }
  return points;
}

void BusStopMap::BuildPathGraph() {
  for(const auto& bus : buses) {
    stopsXbuses += bus.second.GetStopsCount();
  }

  path_graph = std::make_unique<Graph::DirectedWeightedGraph<double>>(stopsXbuses);

  Graph::VertexId vertex = 0;
  /* Fill graph with vertexes */
  for(const auto& [bus, route] : buses) {
    const auto& stops = route.GetStopsOnRoute();
    auto prev_stop = *stops.begin();

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
      prev_stop = stop;
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

BusRoute::Distance BusStopMap::GetDistance(BusRoute::BusStopId from, BusRoute::BusStopId to) {
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
  return {forward_distance, back_distance};
}
