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
  }
  return {nullptr};
}

void RouteDefRequest::ParseFrom(string_view input) {
  bus_name = string(ReadToken(input, ": "));
  // cout << "Bus " << bus_name << ":_";
  
  if(IsContent(input, " > ")) {
    one_direction = true;
    while(input.size()) {
      bus_stops_name.push_back(string(ReadToken(input, " > ")));
      // cout << bus_stops_name.back() << "_";
    }
  } else {
    one_direction = false;
    while(input.size()) {
      bus_stops_name.push_back(string(ReadToken(input, " - ")));
      // cout << bus_stops_name.back() << "_";
    }
  }

  // if(one_direction) {
  //   cout << "One Direction. Stops " << bus_stops_name.size() << "\n";
  // } else {
  //   cout << "Back to Back. Stops " << bus_stops_name.size() << "\n";
  // }
}

void StopDeclRequest::ParseFrom(string_view input) {
  stop_name = string(ReadToken(input, ": "));
  double latitude = stod(string(ReadToken(input, ", ")));
  double longitude = ConvertToDouble(input);
  geo = {longitude, latitude};
//  cout << setprecision(8) << geo.latidute << " :: " << geo.longitude << "\n";
}

void BusInfoRequest::ParseFrom(string_view input) {
  bus_name = string(ReadToken(input, "*"));
}

string BusInfoRequest::Process(const BusStopMap& map) const {
  ostringstream result;
  result << "Bus " << bus_name << ": ";
  auto route_id = map.GetRouteByBus(bus_name);
  if(route_id) {
    const auto& route = route_id.value()->second;
    auto params = route.GetRouteParams();
    result << params.stops << " stops on route, ";
    result << params.unique_stops << " unique stops, ";
    result << setprecision(8) << params.length << " route length";
  } else {
    result << "not found";
  }
  return result.str();
}
