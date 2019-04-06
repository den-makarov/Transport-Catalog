#include "BusStopMap.h"

BusStopMap::Results BusStopMap::GetOptimalPath(const std::string &stop_from,
                                               const std::string &stop_to) const {
  if(path_router) {
    auto from_stop_it = stops_vertexes.find(stop_from);
    auto to_stop_it = stops_vertexes.find(stop_to);

    if(from_stop_it != stops_vertexes.end()
       && to_stop_it != stops_vertexes.end()) {
      auto result = path_router->BuildRoute(from_stop_it->second[0],
                                            to_stop_it->second[0]);
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
  BusStopMap::Results points(1, {BusStopMap::ActivityType::TOTAL_TIME,
                                 "total_time",
                                 weight});
  auto route_id = result.id;
  auto edge_count = result.edge_count;

  for(size_t idx = 0; idx < edge_count; idx++) {
    auto edge = path_graph->GetEdge(path_router->GetRouteEdge(route_id, idx));
    auto stop_from_it = stop_route_ids.find(edge.from);
    auto stop_to_it = stop_route_ids.find(edge.to);

    if( stop_from_it != stop_route_ids.end() 
        && stop_to_it != stop_route_ids.end() ) {
      if( stop_from_it->second.second == wait_stop ) {
        BusStopMap::RoutePoint stop(BusStopMap::ActivityType::STOP,
                                    stop_from_it->second.first,
                                    wait_time);
        points.push_back(std::move(stop));
      } else if ( stop_to_it->second.second != wait_stop ) {
        if(idx == 0) {
          BusStopMap::RoutePoint stop(BusStopMap::ActivityType::STOP,
                                      stop_from_it->second.first,
                                      wait_time);
          points.push_back(std::move(stop));
          points[0].UpdateTime(points[0].GetTime() + wait_time);
        }
        
        BusStopMap::RoutePoint stop(BusStopMap::ActivityType::BUS,
                                    stop_from_it->second.second,
                                    edge.weight.weight);
        points.push_back(std::move(stop));
        
        if(idx == edge_count - 1) {
          points[0].UpdateTime(points[0].GetTime() + wait_time / 2);
        }
      }
    }
    // auto current_bus = edge.weight.bus_number;

    // if(current_bus != previous_bus) {
    //   /* It was new bus. Add in responces information about stop */
    //   auto stop_from_it = stop_route_ids.find(edge.from);
    //   if(stop_from_it != stop_route_ids.end()) {
    //     const auto& stop_from = stop_from_it->second;

    //     BusStopMap::RoutePoint stop(BusStopMap::Type::STOP,
    //                                 stop_from,
    //                                 wait_time);
    //     points.push_back(std::move(stop));
    //   }
    // }

    // BusStopMap::RoutePoint bus(BusStopMap::Type::BUS,
    //                            *edge.weight.bus_number,
    //                            edge.weight.weight);
    // points.push_back(std::move(bus));
    // previous_bus = current_bus;
  }
  if(edge_count > 0) {
    points[0].UpdateTime(points[0].GetTime() - wait_time / 2);
  }

  return points;
}

void BusStopMap::BuildPathGraph() {
  // for(const auto& stop : stops) {
  //   stopsXbuses += stop.second.size();
  //   /* It is a stop with two or more buses. 
  //      Reserve vertex for changing buses */
  //   if(stop.second.size() > 1) {
  //     stopsXbuses += 1;
  //   }
  // }
  for(const auto& [bus, route] : buses) {
    stopsXbuses += route.GetStopsCount();
  }
  stopsXbuses *= 2;

  path_graph = std::make_unique<Graph::DirectedWeightedGraph<Weight>>(stopsXbuses);

  Graph::VertexId vertex_to = 0;
  Graph::VertexId vertex_from = vertex_to;
  /* Fill graph with vertexes */

  for(const auto& [bus, route] : buses) {
    
    const auto& route_stops = route.GetStopsOnRoute();
  
    /* Previous stop to exclude calcualtion on the same stops in one route */
    auto prev_stop = *route_stops.begin();

    /* for each bus's route itterate on all stops it has*/
    for(const auto& stop : route_stops) {
      
      /* Check if the stop already has vertexes */
      auto stop_vertex_it = stops_vertexes.find(stop->GetName());
      if(stop_vertex_it != stops_vertexes.end()) {
        /* The stop and its vertex was parsed within parsing previous route */
        stop_vertex_it->second.push_back(vertex_to);

        Weight weight(wait_time / 2, &bus.GetNumber());

        Graph::Edge<Weight> edge = {stop_vertex_it->second[0],
                                    vertex_to,
                                    weight};
        path_graph->AddEdge(edge);

        edge = {vertex_to, stop_vertex_it->second[0], weight};
        path_graph->AddEdge(edge);
      } else {
        /* Check if this is a stop with only one bus on it */
        /* If the stop has many buses, add waiting stop vertex */
        auto BusCount = GetBusesCountOnStop(stop->GetName(), route.GetStop(stop->GetName()));
        if(BusCount > 1) {
          /* Add vertex for waiting stop */
          /* This vertex should be at the beginning of the vector */
          /* In the following steps cycle shouldn't get here */
          stops_vertexes[stop->GetName()].push_back(vertex_to);
          stop_route_ids.insert({vertex_to, {stop->GetName(), wait_stop}});
          vertex_to++;
          
          Weight weight(wait_time / 2, &bus.GetNumber());
          
          Graph::Edge<Weight> edge = {vertex_to - 1,
                                      vertex_to,
                                      weight};
          path_graph->AddEdge(edge);
          
          edge = {vertex_to, vertex_to - 1, weight};
          path_graph->AddEdge(edge);
        }
        /* Add vertex of the stop */
        stops_vertexes[stop->GetName()].push_back(vertex_to);
      }
      stop_route_ids.insert({vertex_to, {stop->GetName(), bus.GetNumber()}});

      /* Calculate weight for all segments in a route */
      if(prev_stop != stop) {
        auto distance = GetDistance(prev_stop, stop);
        if(route.IsOneDirection()) {
          /* Add vertex in forward direction for round route */
          double w = distance.forward != 0
              ? distance.forward / velocity
              : distance.back / velocity;
          Weight weight(w, &bus.GetNumber());
          Graph::Edge<Weight> edge = {vertex_from,
                                      vertex_to,
                                      weight};
          path_graph->AddEdge(edge);
        } else {
          /* Add vertex in forward direction for ordinary route */
          double forward_w = distance.forward / velocity;
          Weight weight(forward_w, &bus.GetNumber());
          Graph::Edge<Weight> forward_edge = {vertex_from,
                                              vertex_to,
                                              weight};
          path_graph->AddEdge(forward_edge);

          /* Add vertex in back direction for ordinary route */
          weight.weight = distance.back / velocity;
          Graph::Edge<Weight> back_edge = {vertex_to,
                                           vertex_from,
                                           weight};
          path_graph->AddEdge(back_edge);
        }
      }
      prev_stop = stop;

      /* Recalculate vertexes */
      vertex_from = vertex_to;
      vertex_to++;
    }
  /* Check if it was round */
  }
  path_router = std::make_unique<Graph::Router<Weight>>(*path_graph);
}

size_t BusStopMap::GetBusesCountOnStop(const std::string& stop, std::optional<BusStop> stop_id) {
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
