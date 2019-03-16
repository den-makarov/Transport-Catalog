#include "request.h"

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

}

void StopDeclRequest::ParseFrom(std::string_view input) {

}

void BusInfoRequest::ParseFrom(std::string_view input) {
  bus = Bus::FromString(ReadToken(input));
}
