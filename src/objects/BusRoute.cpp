#include "BusRoute.h"

BusRoute::BusRoute(BusRoute&& other) {
  std::swap(one_direction, other.one_direction);
  std::swap(stops, other.stops);
  std::swap(geo_length, other.geo_length);
  std::swap(total_distance, other.total_distance);
  std::swap(route, other.route);
}

BusRoute& BusRoute::operator=(BusRoute&& other) {
  std::swap(one_direction, other.one_direction);
  std::swap(stops, other.stops);
  std::swap(geo_length, other.geo_length);
  std::swap(total_distance, other.total_distance);
  std::swap(route, other.route);
  return *this;
}

void BusRoute::AddStop(BusStop stop) {
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

BusRoute::RouteParams BusRoute::GetRouteParams() const {
  RouteParams params;
  unsigned long back_distance = 0;
  unsigned long forward_distance = 0;

  if(total_distance == 0.0) {
    auto prev_id = *route.begin();
    for(const auto& id : route) {
      auto dist_to = prev_id->GetDistanceInfo(id->GetName());
      auto dist_back = id->GetDistanceInfo(prev_id->GetName());

      if(one_direction || id == prev_id) {
        if(dist_to) {
          total_distance += dist_to.value();
        } else if (dist_back) {
          total_distance += dist_back.value();
        }
      } else {
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

        total_distance += (forward_distance + back_distance);
      }

      prev_id = id;
    }
  }

  params.unique_stops = stops.size();

  if(one_direction) {
    params.stops = route.size();
    params.geo_length = geo_length;
  } else {
    params.stops = (route.size() - 1) * 2 + 1;
    params.geo_length = geo_length * 2.0;
  }
  params.distance = total_distance;

  return params;
}

std::optional<BusStop> BusRoute::GetStop(const std::string& name) const {
  BusStop dummy(name, {0.0, 0.0});
  auto it = stops.find(dummy);
  if(it != stops.end()) {
    return {*it};
  } else {
    return {std::nullopt};
  }
}


void BusRoute::PrintRoute(std::ostream& out) const {
  for(auto stop : route) {
    out << stop->GetName();
    if(one_direction) {
      out << " > ";
    } else {
      out << " - ";
    }
  }
}
