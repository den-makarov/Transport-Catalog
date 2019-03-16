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
  bus_stops_name.push_back(string(ReadToken(input)));
  
  if(input.size() > 3) {
    if(input[0] == '>') {
      one_direction = true;
      while(input.size()) {
        bus_stops_name.push_back(string(ReadToken(input, "> ")));
      }
    } else {
      one_direction = false;
      while(input.size()) {
        bus_stops_name.push_back(string(ReadToken(input, "- ")));
      }
    }
  }
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
