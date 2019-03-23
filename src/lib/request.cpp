#include <iomanip>

#include "request.h"

using namespace std;

Request::RequestHolder Request::Create(Request::Type type) {
  switch (type) {
  case Type::ROUTE_DEFINITION:
    return make_unique<RouteDefRequest>();
  case Type::STOP_DECLARATION:
    return make_unique<StopDeclRequest>();
  case Type::BUS_INFO:
    return make_unique<BusInfoRequest>();
  case Type::STOP_INFO:
    return make_unique<StopInfoRequest>();
  }
  return {nullptr};
}

/*------------------------------------------------------------------*/

void RouteDefRequest::Process(BusStopMap& map) {
  Bus new_bus(bus_name);
  BusRoute new_route(one_direction);
  for(const auto& stop_name : bus_stops_name) {
    auto stop_id = map.GetStopByName(stop_name);
    if(stop_id) {
      BusStop stop(stop_id.value());
      new_route.AddStop(std::move(stop));
    }
  }
  map.AddBus(new_bus, move(new_route));
}

void RouteDefRequest::ParseFrom(string_view input) {
  bus_name = string(ReadToken(input, ": "));
  
  if(IsContent(input, " > ")) {
    one_direction = true;
    while(input.size()) {
      bus_stops_name.push_back(string(ReadToken(input, " > ")));
    }
  } else {
    one_direction = false;
    while(input.size()) {
      bus_stops_name.push_back(string(ReadToken(input, " - ")));
    }
  }
}

void RouteDefRequest::ParseFrom(const std::map<std::string, Json::Node>& request) {
  ReadString(bus_name, request, "name");
  
  const auto dir_it = request.find("is_roundtrip");
  if(dir_it != request.end()) {
    if(dir_it->second.index() == NUMBER_NODE) {
      /* @TODO: Define string to bool parses and dedicated json type */
      //one_direction = it->second.AsInt();
      one_direction = true;
    }
  }

  const auto stops_array_it = request.find("stops");
  if(stops_array_it != request.end()) {
    if(stops_array_it->second.index() == ARRAY_NODE) {
      const auto& stops_array = stops_array_it->second.AsArray();
      for(const auto& item : stops_array) {
        if(item.index() == STRING_NODE) {
          bus_stops_name.push_back(item.AsString());
        }
      }
    }
  }
}

/*------------------------------------------------------------------*/

void StopDeclRequest::ParseFrom(string_view input) {
  stop_name = string(ReadToken(input, ": "));

  double latitude = stod(string(ReadToken(input, ", ")));
  double longitude = stod(string(ReadToken(input, ", ")));
  geo = {longitude, latitude};

  if(!distances) {
    distances = BusStop::DistanceSet(new map<string, unsigned long>({}));
  }
  while(input.size()) {
    unsigned long distance = stoul(string(ReadToken(input, "m to ")));
    string stop(ReadToken(input, ", "));
    distances->insert({stop, distance});
  }
}

void StopDeclRequest::ParseFrom(const std::map<std::string, Json::Node>& request) {
  ReadString(stop_name, request, "name");

  int latitude;
  int longitude;
  ReadInt(latitude, request, "latitude");
  ReadInt(longitude, request, "longitude");
  geo = {longitude, latitude};

  if(!distances) {
    distances = BusStop::DistanceSet(new map<string, unsigned long>({}));
  }

  const auto stops_map_it = request.find("road_distances");
  if(stops_map_it != request.end()) {
    if(stops_map_it->second.index() == MAP_NODE) {
      const auto& stops_map = stops_map_it->second.AsMap();
      for(const auto& [stop, distance_node] : stops_map) {
        if(distance_node.index() == NUMBER_NODE) {
          unsigned long distance = distance_node.AsInt();
          distances->insert({stop, distance});
        }
      }
    }
  }
}

/*------------------------------------------------------------------*/

string BusInfoRequest::Process(const BusStopMap& map) const {
  ostringstream result;
  result << "Bus " << bus_name << ": ";
  auto route_id = map.GetRouteByBus(bus_name);
  if(route_id) {
    const auto& route = route_id.value()->second;
    auto params = route.GetRouteParams();
    result << params.stops << " stops on route, ";
    result << params.unique_stops << " unique stops, ";
    result << fixed << setprecision(0) << params.distance << " route length, ";
    result << setprecision(8) << params.distance / params.geo_length << " curvature";
  } else {
    result << "not found";
  }
  return result.str();
}

void BusInfoRequest::ParseFrom(const std::map<std::string, Json::Node>& request) {
  ReadString(bus_name, request, "name");
  ReadInt(id, request, "id");
}

/*------------------------------------------------------------------*/

string StopInfoRequest::Process(const BusStopMap& map) const {
  ostringstream result;
  result << "Stop " << stop_name << ": ";
  auto board_id = map.GetStopBoardByName(stop_name);
  if(board_id) {
    const auto board = board_id.value();
    if(board->size()) {
      result << "buses";
      for(const auto & bus : *board) {
        result << " " << bus.GetNumber();
      }
    } else {
      result << "no buses";
    }
  } else {
    result << "not found";
  }
  return result.str();
}

void StopInfoRequest::ParseFrom(const std::map<std::string, Json::Node>& request) {
  ReadString(stop_name, request, "name");
  ReadInt(id, request, "id");
}
