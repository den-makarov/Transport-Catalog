#include "BusStopMap.h"

BusStopMap::Results BusStopMap::GetOptimalPath(const std::string &stop_from,
                                               const std::string &stop_to) const {
  if(path_router) {
    auto from_stop_it = stops_vertexes.find(stop_from);
    auto to_stop_it = stops_vertexes.find(stop_to);

    if(from_stop_it != stops_vertexes.end()
       && to_stop_it != stops_vertexes.end()) {
      auto result = path_router->BuildRoute(from_stop_it->second,
                                            to_stop_it->second);
      optimal_routes.push_back(result);

      if(result) {
        return ParseOptimalPath(result.value(),
                                result.value().weight.weight);
      }
    }
  }
  return BusStopMap::Results();
}

BusStopMap::Results BusStopMap::ParseOptimalPath(const Graph::Router<Weight>::RouteInfo& result,
                                                 double weight) const {
  BusStopMap::Results points(1, {BusStopMap::Type::TOTAL_TIME,
                                 "total_time",
                                 weight + wait_time});
  auto route_id = result.id;
  auto edge_count = result.edge_count;
  const std::string * previous_bus = nullptr;

  for(size_t idx = 0; idx < edge_count; idx++) {
    auto edge = path_graph->GetEdge(path_router->GetRouteEdge(route_id, idx));
    auto current_bus = edge.weight.bus_number;

    if(current_bus != previous_bus) {
      /* It was new bus. Add in responces information about stop */
      auto stop_from_it = stop_route_ids.find(edge.from);
      if(stop_from_it != stop_route_ids.end()) {
        const auto& stop_from = stop_from_it->second;
        BusStopMap::RoutePoint stop(BusStopMap::Type::STOP,
                                    stop_from,
                                    wait_time);
        points.push_back(std::move(stop));

//        /* Update total time */
//        if(idx == 0) {
//          points[0].UpdateTime(points[0].GetTime() + wait_time);
//        }
      }

    }

    BusStopMap::RoutePoint bus(BusStopMap::Type::BUS,
                               *edge.weight.bus_number,
                               edge.weight.weight);

    points.push_back(std::move(bus));
    previous_bus = current_bus;
  }
  return points;
}

void BusStopMap::BuildPathGraph() {
  for(const auto& stop : stops) {
    stopsXbuses += stop.second.size();
  }

  path_graph = std::make_unique<Graph::DirectedWeightedGraph<Weight>>(stopsXbuses);

  Graph::VertexId vertex_idx = 0;
  Graph::VertexId vertex_to = vertex_idx;
  Graph::VertexId vertex_from = vertex_to;

  /* Fill graph with vertexes */
  for(const auto& [bus, route] : buses) {
    const auto& stops = route.GetStopsOnRoute();
    auto prev_stop = *stops.begin();
    /* for each bus route itterate on all stops */
    for(const auto& stop : stops) {
      /* Before find stop and its vertex update previous vertex */
      vertex_from = vertex_to;

      /* Try to find already added stop and its vertex */
      auto stop_it = stops_vertexes.find(stop->GetName());
      if(stop_it != stops_vertexes.end()) {
        /* It was added earlier, so use vertex for calculations */

        /* Only to avoid UB just to check that it was realy added */
        auto vertex_it = stop_route_ids.find(stop_it->second);
        if(vertex_it != stop_route_ids.end()) {
          vertex_to = vertex_it->first;
          /* The stop definitely was added and its vertex already in graph */

          /* So, just add stop with bus */
          //vertex_it->second.push_back({bus, stop->GetName()});
        } else {
          /* something went wrong */
        }
      } else {
        /* It is the first stop in base and has to be the first vertex */
        stops_vertexes[stop->GetName()] = vertex_idx;
        stop_route_ids.insert({vertex_idx, stop->GetName()});
        /* Update vertex of destination with new value added to base */
        vertex_to = vertex_idx;
        vertex_idx++;
      }

      /* Calculate weight for all segments in a route */
      if(prev_stop != stop) {
        auto distance = GetDistance(prev_stop, stop);
        if(route.IsOneDirection()) {
          /* Add vertex in forward direction for round route */
          double w = distance.forward != 0
              ? distance.forward / velocity
              : distance.back / velocity;
          Weight weight(w, wait_time, &bus.GetNumber());
          Graph::Edge<Weight> edge = {vertex_from,
                                      vertex_to,
                                      weight};
          path_graph->AddEdge(edge);
        } else {
          /* Add vertex in forward direction for ordinary route */
          double forward_w = distance.forward / velocity;
          Weight weight(forward_w, wait_time, &bus.GetNumber());
          Graph::Edge<Weight> forward_edge = {vertex_from,
                                              vertex_to,
                                              weight};
          path_graph->AddEdge(forward_edge);

          /* Add vertex in back direction for ordinary route */
          double back_w = distance.back / velocity;
          weight = Weight(back_w, wait_time, &bus.GetNumber());
          Graph::Edge<Weight> back_edge = {vertex_to,
                                           vertex_from,
                                           weight};
          path_graph->AddEdge(back_edge);
        }
      }
      prev_stop = stop;
    }
  }
  path_router = std::make_unique<Graph::Router<Weight>>(*path_graph);
}

BusRoute::Distance BusStopMap::GetDistance(BusRoute::BusStopId from,
                                           BusRoute::BusStopId to) {
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
