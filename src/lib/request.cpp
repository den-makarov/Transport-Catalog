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
  case Type::ROUTE_SETTINGS:
    return make_unique<RouteSettings>();
  case Type::ROUTE_INFO:
    return make_unique<RouteInfoRequest>();
  }
  return {nullptr};
}

/*------------------------------------------------------------------*/

void RouteSettings::ParseFrom(const std::map<std::string, Json::Node>& request) {
  double d_value;
  ReadNumber(d_value, request, "bus_wait_time");
  wait_time = static_cast<int>(d_value);

  ReadNumber(d_value, request, "bus_velocity");
  velocity = d_value;
}

void RouteSettings::Process(BusStopMap& map) {
  map.AddSettings(velocity, wait_time);
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
    if(dir_it->second.index() == BOOL_NODE) {
      one_direction = dir_it->second.AsBool();
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

  double latitude;
  double longitude;
  ReadNumber(latitude, request, "latitude");
  ReadNumber(longitude, request, "longitude");
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
          unsigned long distance = static_cast<unsigned long>(distance_node.AsDouble());
          distances->insert({stop, distance});
        }
      }
    }
  }
}

/*------------------------------------------------------------------*/

string BusInfoRequest::Process(const BusStopMap& map) const {
  ostringstream result;
  string spaces = "    ";
  auto route_id = map.GetRouteByBus(bus_name);
  if(route_id) {
    const auto& route = route_id.value()->second;
    auto params = route.GetRouteParams();
    
    result << spaces << "\"route_length\": " << fixed << setprecision(0) << params.distance << ",\n";
    result << spaces << "\"request_id\": " << fixed << setprecision(0) << id << ",\n";
    result << spaces << "\"curvature\": " << fixed << setprecision(6) << params.distance / params.geo_length << ",\n";
    result << spaces << "\"stop_count\": " << params.stops << ",\n";
    result << spaces << "\"unique_stop_count\": " << params.unique_stops;
  } else {
    result << spaces << "\"request_id\": " << fixed << setprecision(0) << id << ",\n";
    result << spaces << "\"error_message\": \"not found\"";
  }
  return result.str();

  // result << "Bus " << result << "Bus " << bus_name << ": ";
  // auto route_id = map.GetRouteByBus(bus_name);
  // if(route_id) {
  //   const auto& route = route_id.value()->second;
  //   auto params = route.GetRouteParams();
  //   result << params.stops << " stops on route, ";
  //   result << params.unique_stops << " unique stops, ";
  //   result << fixed << setprecision(0) << params.distance << " route length, ";
  //   result << setprecision(8) << params.distance / params.geo_length << " curvature";
  // } else {
  //   result << "not found";
  // }
  // return result.str();
}

void BusInfoRequest::ParseFrom(const std::map<std::string, Json::Node>& request) {
  ReadString(bus_name, request, "name");
  double d_id;
  ReadNumber(d_id, request, "id");
  id = static_cast<int>(d_id);
}

/*------------------------------------------------------------------*/

string StopInfoRequest::Process(const BusStopMap& map) const {
  ostringstream result;
  string spaces = "    ";
  auto board_id = map.GetStopBoardByName(stop_name);
  if(board_id) {
    const auto board = board_id.value();
    auto size = board->size();
    if(size) {
      result << spaces << "\"buses\": [\n";
      for(const auto & bus : *board) {
        result << spaces << "  \"" << bus.GetNumber() << "\"";
        if(--size != 0) {
          result << ",";
        }
        result << "\n";
      }
      result << spaces << "],\n";
    } else {
      result << spaces << "\"buses\": [],\n";
    }
    result << spaces << "\"request_id\": " << fixed << setprecision(0) << id;
  } else {
    result << spaces << "\"request_id\": " << fixed << setprecision(0) << id << ",\n";
    result << spaces << "\"error_message\": \"not found\"";
  }
  return result.str();

  // result << "Stop " << stop_name << ": ";
  // auto board_id = map.GetStopBoardByName(stop_name);
  // if(board_id) {
  //   const auto board = board_id.value();
  //   if(board->size()) {
  //     result << "buses";
  //     for(const auto & bus : *board) {
  //       result << " " << bus.GetNumber();
  //     }
  //   } else {
  //     result << "no buses";
  //   }
  // } else {
  //   result << "not found";
  // }
  // return result.str();
}

void StopInfoRequest::ParseFrom(const std::map<std::string, Json::Node>& request) {
  ReadString(stop_name, request, "name");
  double d_id;
  ReadNumber(d_id, request, "id");
  id = static_cast<int>(d_id);
}

/*------------------------------------------------------------------*/

string RouteInfoRequest::Process(const BusStopMap& map) const {
  ostringstream result;
  string spaces = "    ";
  result << spaces << "\"request_id\": " << fixed << setprecision(0) << id << ",\n";
  auto optimal_route = map.GetOptimalPath(stop_from, stop_to);

  if(optimal_route.size() > 1) {
    result << spaces << "\"total_time\": "
           << fixed << setprecision(6)
           << optimal_route[0].GetTime() << ",\n";
    result << spaces << "\"items\": [\n";
    double bus_time = 0.0;
    size_t spans = 1;

    bool was_stop = false;
    for(size_t idx = 1; idx < optimal_route.size(); idx++) {
      if(optimal_route[idx].IsStop()){

        /* Print postponed bus */
        if(idx != 1) {
          result << spaces << spaces << spaces << "\"span_count\": " << spans << ",\n";
          result << spaces << spaces << spaces << "\"time\": "
                 << fixed << setprecision(6)
                 << bus_time << "\n";
          result << spaces << spaces << "}";
          if(idx == optimal_route.size() - 1) {
            result << "\n";
          } else {
            result << ",\n";
          }
        }

        /* stop */
        was_stop = true;
        result << spaces << spaces << "{\n";
        result << spaces << spaces << spaces << "\"type\": \"Wait\",\n";
        result << spaces << spaces << spaces << "\"stop_name\": \"" << optimal_route[idx].GetName() << "\",\n";
        result << spaces << spaces << spaces << "\"time\": "
               << fixed << setprecision(0)
               << optimal_route[idx].GetTime() << "\n";
        result << spaces << spaces << "},\n";
      } else {
        if(was_stop) {
          /* new bus */
          result << spaces << spaces << "{\n";
          result << spaces << spaces << spaces << "\"type\": \"Bus\",\n";
          result << spaces << spaces << spaces << "\"bus\": \"" << optimal_route[idx].GetName() << "\",\n";
          spans = 1;
          bus_time = optimal_route[idx].GetTime();
        } else {
          /* the same bus */
          spans++;
          bus_time += optimal_route[idx].GetTime();
        }
        was_stop = false;

        if(idx == optimal_route.size() - 1) {
          result << spaces << spaces << spaces << "\"span_count\": " << spans << ",\n";
          result << spaces << spaces << spaces << "\"time\": "
                 << fixed << setprecision(6)
                 << bus_time << "\n";
          result << spaces << spaces << "}";
          if(idx == optimal_route.size() - 1) {
            result << "\n";
          } else {
            result << ",\n";
          }
        }
      }
    }
    result << spaces << "]";
  } else if(optimal_route.size() == 1) {
    result << spaces << "\"total_time\": "
           << fixed << setprecision(6)
           << optimal_route[0].GetTime() << ",\n";
    result << spaces << "\"items\": [\n" << spaces << "]";
  } else {
    result << spaces << "\"error_message\": \"not found\"";
  }
  return result.str();
}

void RouteInfoRequest::ParseFrom(const std::map<std::string, Json::Node>& request) {
  ReadString(stop_from, request, "from");
  ReadString(stop_to, request, "to");
  
  double d_value;
  ReadNumber(d_value, request, "id");
  id = static_cast<int>(d_value);
}
