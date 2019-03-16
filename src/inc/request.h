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
  virtual void Process(BusStopMap& map) const = 0;
  virtual ~Request() = default;

  const Type type;
};

class RouteDefRequest : public Request {
public:
  RouteDefRequest() : Request(Type::ROUTE_DEFINITION) {}
  void ParseFrom(std::string_view input) override;

  void Process(BusStopMap& map) const override {

  }
private:
  std::string bus_name;
  std::vector<std::string> bus_stops_name;
  bool one_direction;
};

class StopDeclRequest : public Request {
public:
  StopDeclRequest() : Request(Type::STOP_DECLARATION) {}
  void ParseFrom(std::string_view input) override;

  void Process(BusStopMap& map) const override {
    map.AddStop({stop_name, geo});
  }
private:
  std::string stop_name;
  Location geo;
};

class BusInfoRequest : public Request {
public:
  BusInfoRequest() : Request(Type::BUS_INFO) {}
  void ParseFrom(std::string_view input) override;

  void Process(BusStopMap& map) const override {

  }
private:
  std::string bus_name;
};

#endif // REQUEST_H
