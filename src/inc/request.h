#ifndef REQUEST_H
#define REQUEST_H

#include "stringparser.h"
#include "bus.h"
#include "BusRoute.h"
#include "BusStopMap.h"

#include <memory>

class Request
{
public:
  using RequestHolder = std::unique_ptr<Request>;

  enum class Type {
    STOP_DECLARATION,
    ROUTE_DEFINITION,
    BUS_INFO
  };

  Request(Type t) : type(t) {}

  static RequestHolder Create(Type type);
  virtual void ParseFrom(std::string_view input) = 0;
  virtual ~Request() = default;

  const Type type;
};

template <typename ResultType>
class PrintRequest : public Request {
public:
  using Request::Request;
  virtual ResultType Process(const BusStopMap& map) const = 0;
  virtual ~PrintRequest() = default;
};

class ModifyRequest : public Request {
public:
  using Request::Request;
  virtual void Process(BusStopMap& map) const = 0;
  virtual ~ModifyRequest() = default;
};

class RouteDefRequest : public ModifyRequest {
public:
  RouteDefRequest()
  : ModifyRequest(Type::ROUTE_DEFINITION)
  {}

  void ParseFrom(std::string_view input) override;

  void Process(BusStopMap& map) const override {
    Bus new_bus(bus_name);
    BusRoute new_route(one_direction);
    for(const auto& stop_name : bus_stops_name) {
      auto stop_id = map.GetStopByName(stop_name);
      if(stop_id) {
        BusStop stop(*(stop_id.value()));
        new_route.AddStop(std::move(stop));
      }
    }
    map.AddBus(std::move(new_bus), std::move(new_route));
  }
private:
  std::string bus_name;
  std::vector<std::string> bus_stops_name;
  bool one_direction;
};

class StopDeclRequest : public ModifyRequest {
public:
  StopDeclRequest()
  : ModifyRequest(Type::STOP_DECLARATION)
  {}

  void ParseFrom(std::string_view input) override;

  void Process(BusStopMap& map) const override {
    map.AddStop({stop_name, geo});
  }
private:
  std::string stop_name;
  Location geo;
};

class BusInfoRequest : public PrintRequest<std::string> {
public:
  BusInfoRequest()
  : PrintRequest(Type::BUS_INFO)
  {}

  void ParseFrom(std::string_view input) override;

  std::string Process(const BusStopMap& map) const override {
    std::ostringstream result;
    result << "Bus " << bus_name << ": ";
    auto route_id = map.GetRouteByBus(bus_name);
    if(route_id) {
      const auto& route = route_id.value()->second;
      auto params = route.GetRouteParams();
      result << params.stops << " stops on route, ";
      result << params.unique_stops << " unique stops, ";
      result << params.length << " route length\n";
    } else {
      result << "not found\n";
    }
    return result.str();
  }
private:
  std::string bus_name;
};

#endif // REQUEST_H
