#include "BusStopMap.h"

BusStopMap::Results BusStopMap::GetOptimalPath(const std::string &stop_from,
                                               const std::string &stop_to) const {
  if(path_router) {
    auto from_stop_it = stops_vertexes_map.find(stop_from);
    auto to_stop_it = stops_vertexes_map.find(stop_to);

    if(from_stop_it != stops_vertexes_map.end()
       && to_stop_it != stops_vertexes_map.end()) {
      auto result = path_router->BuildRoute(from_stop_it->second * 2,
                                            to_stop_it->second * 2);
      if(result) {
        return ParseOptimalPath(result.value(),
                                result.value().weight);
      }
    }
  }
  return BusStopMap::Results();
}

BusStopMap::Results BusStopMap::ParseOptimalPath(const Graph::Router<Weight>::RouteInfo& result,
                                                 double weight) const {
  BusStopMap::Results points(1, {BusStopMap::ActivityType::TOTAL_TIME,
                                 "total_time",
                                 weight, 0});
  auto route_id = result.id;
  auto edge_count = result.edge_count;

  for(size_t idx = 0; idx < edge_count; idx++) {
    auto edge = path_graph->GetEdge(path_router->GetRouteEdge(route_id, idx));
    auto bus_number = edges_buses_array[path_router->GetRouteEdge(route_id, idx)].first;
    auto span_count = edges_buses_array[path_router->GetRouteEdge(route_id, idx)].second;
    if(bus_number == &wait_stop) {
      /* It is a way to wait stop */
      auto stop_name = vertexes_stops_array[edge.from / 2];
      if(stop_name != nullptr) {
        BusStopMap::RoutePoint stop(BusStopMap::ActivityType::STOP,
                                    *stop_name,
                                    wait_time,
                                    0);
        points.push_back(stop);
      }
    } else if(bus_number != nullptr) {
      BusStopMap::RoutePoint bus(BusStopMap::ActivityType::BUS,
                                 *bus_number,
                                 edge.weight,
                                 span_count);
      points.push_back(bus);
    }
  }
  
  path_router->ReleaseRoute(result.id);
  return points;
}

void BusStopMap::BuildPathGraph() {
  auto vertexes_count = CountRequiredVertexes();
  FillVertexes(vertexes_count);
  path_graph = std::make_unique<Graph::DirectedWeightedGraph<Weight>>(vertexes_count * 2);

  for(const auto& [bus, route] : buses) {
    const auto& route_stops = route.GetStopsOnRoute();

    for(auto stop = route_stops.begin(); stop != route_stops.end(); ++stop) {
      Graph::VertexId vertex_from = 2 * stops_vertexes_map.at(stop.operator*()->GetName());
      
      /* Add edge to wait stop */
      Graph::VertexId vertex_to = vertex_from + 1;
      Graph::Edge<Weight> edge = {vertex_from, vertex_to, wait_time};
      edges_buses_array[path_graph->AddEdge(edge)] = {&wait_stop, 0};
      
      /* Add edges to all next stops */
      vertex_from = vertex_to;
      auto prev_stop_it = stop;
      BusRoute::Distance distance = {0, 0};
      size_t span_count = 1;
      for(auto stop_it = std::next(stop); stop_it != route_stops.end(); ++stop_it) {
        distance += GetDistance(*prev_stop_it, *stop_it);
        prev_stop_it = stop_it;
        vertex_to = 2 * stops_vertexes_map.at(stop_it.operator*()->GetName());
        
        if(route.IsOneDirection()) {
          /* Add vertex in forward direction for round route */
          double w = distance.forward != 0
              ? distance.forward / velocity
              : distance.back / velocity;
          Weight weight(w);
          Graph::Edge<Weight> edge = {vertex_from, vertex_to, w};
          edges_buses_array[path_graph->AddEdge(edge)] = {&bus.GetNumber(), span_count};
        } else {
          /* Add vertex in forward direction for ordinary route */
          double forward_w = distance.forward / velocity;
          Weight weight(forward_w);
          Graph::Edge<Weight> forward_edge = {vertex_from, vertex_to, weight};
          edges_buses_array[path_graph->AddEdge(forward_edge)] = {&bus.GetNumber(), span_count};

          /* Add vertex in back direction for ordinary route */
          weight = distance.back / velocity;
          Graph::Edge<Weight> back_edge = {vertex_to + 1, vertex_from - 1, weight};
          edges_buses_array[path_graph->AddEdge(back_edge)] = {&bus.GetNumber(), span_count};
        }

        span_count++;
      }
    }    
  }
  path_router = std::make_unique<Graph::Router<Weight>>(*path_graph);
}

size_t BusStopMap::CountRequiredVertexes() {
  size_t stopsXbuses = 0;

  for(const auto& stop : stops) {
    if(stop.second.size() > 0) {
      stopsXbuses++;
    }
  }
  return stopsXbuses;
}

void BusStopMap::FillVertexes(size_t count) {
  edges_buses_array.resize(count * count * 4, {nullptr, 0});
  vertexes_stops_array.reserve(count);
  stops_vertexes_map.reserve(count);

  Graph::VertexId idx = 0;
  for(const auto& stop : stops) {
    if(stop.second.size() > 0) {
      vertexes_stops_array[idx] = &stop.first.GetName();
      stops_vertexes_map[stop.first.GetName()] = idx++;
    }
  }
}

size_t BusStopMap::GetBusesCountOnStop(std::optional<BusStop> stop_id) {
  size_t buses_count_at_stop = 0;
  if(stop_id) {
    auto buses_set = stops.find(stop_id.value());
    if(buses_set != stops.end()) {
      buses_count_at_stop = buses_set->second.size();
    }
  }
  return buses_count_at_stop;
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
