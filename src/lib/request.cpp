#include "request.h"

using namespace std;

Request::RequestHolder Request::Create(Request::Type type) {
  switch (type) {
  case Type::ROUTE_DEFINITION:
    return std::make_unique<RouteDefRequest>();
  case Type::STOP_DECLARATION:
    return std::make_unique<StopDeclRequest>();
  case Type::BUS_INFO:
    return std::make_unique<BusInfoRequest>();
  }
  return {nullptr};
}

void RouteDefRequest::ParseFrom(std::string_view input) {
  bus_name = string(ReadToken(input, ": "));
  // std::cout << "Bus " << bus_name << ":_";
  
  if(IsContent(input, " > ")) {
    one_direction = true;
    while(input.size()) {
      bus_stops_name.push_back(string(ReadToken(input, " > ")));
      // std::cout << bus_stops_name.back() << "_";
    }
  } else {
    one_direction = false;
    while(input.size()) {
      bus_stops_name.push_back(string(ReadToken(input, " - ")));
      // std::cout << bus_stops_name.back() << "_";
    }
  }

  // if(one_direction) {
  //   std::cout << "One Direction. Stops " << bus_stops_name.size() << "\n";
  // } else {
  //   std::cout << "Back to Back. Stops " << bus_stops_name.size() << "\n";
  // }
}

void StopDeclRequest::ParseFrom(std::string_view input) {
  stop_name = string(ReadToken(input, ": "));
  double latitude = stod(string(ReadToken(input, ", ")));
  double longitude = ConvertToDouble(input);
  geo = {longitude, latitude};
}

void BusInfoRequest::ParseFrom(std::string_view input) {
  bus_name = string(ReadToken(input));
}
