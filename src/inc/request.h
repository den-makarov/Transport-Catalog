#ifndef REQUEST_H
#define REQUEST_H

#include "stringparser.h"
#include "bus.h"
#include "BusRoute.h"
#include "BusStopMap.h"

#include <utility>
#include <memory>

class Request
{
public:
  using RequestHolder = std::unique_ptr<Request>;

  enum class Type {
    STOP_DECLARATION,
    ROUTE_DEFINITION,
    BUS_INFO,
    STOP_INFO
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
  virtual void Process(BusStopMap& map) = 0;
  virtual ~ModifyRequest() = default;
};

class RouteDefRequest : public ModifyRequest {
public:
  RouteDefRequest()
  : ModifyRequest(Type::ROUTE_DEFINITION)
  {}

  void ParseFrom(std::string_view input) override;
  void Process(BusStopMap& map) override;
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
  void Process(BusStopMap& map) override {
    BusStop stop({stop_name, geo});
    stop.AddDistanceInfo(distances);
    map.AddStop(std::move(stop));
  }
private:
  std::string stop_name;
  Location geo;
  BusStop::DistanceSet distances;
};

class BusInfoRequest : public PrintRequest<std::string> {
public:
  BusInfoRequest()
  : PrintRequest(Type::BUS_INFO)
  {}

  void ParseFrom(std::string_view input) override {
    bus_name = std::string(ReadToken(input, "*"));
  }
  std::string Process(const BusStopMap& map) const override;
private:
  std::string bus_name;
};

class StopInfoRequest : public PrintRequest<std::string> {
public:
  StopInfoRequest()
  : PrintRequest(Type::STOP_INFO)
  {}

  void ParseFrom(std::string_view input) override {
    stop_name = std::string(ReadToken(input, "*"));
  }
  std::string Process(const BusStopMap& map) const override;
private:
  std::string stop_name;
};

#endif // REQUEST_H
