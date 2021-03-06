#ifndef REQUEST_H
#define REQUEST_H

#include "stringparser.h"
#include "bus.h"
#include "BusRoute.h"
#include "BusStopMap.h"
#include "json.h"

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
    STOP_INFO,
    ROUTE_SETTINGS,
    ROUTE_INFO
  };

  Request(Type t) : type(t) {}

  static RequestHolder Create(Type type);
  virtual void ParseFrom(std::string_view input) = 0;
  virtual void ParseFrom(const std::map<std::string, Json::Node>& request) = 0;
  virtual ~Request() = default;

  const Type type;
protected:
  void ReadString(std::string& str, const std::map<std::string, Json::Node>& request, const std::string& filter) {
    const auto it = request.find(filter);
    if(it != request.end()) {
      if(it->second.index() == STRING_NODE) {
        str = it->second.AsString();
      }
    }
  }

  void ReadNumber(double& number, const std::map<std::string, Json::Node>& request, const std::string& filter) {
    const auto it = request.find(filter);
    if(it != request.end()) {
      if(it->second.index() == NUMBER_NODE) {
        number = it->second.AsDouble();
      }
    }
  }
};

template <typename ResultType>
class PrintRequest : public Request {
public:
  using Request::Request;
  virtual ResultType Process(const BusStopMap& map) const = 0;
  virtual ~PrintRequest() = default;
  int GetId() const {
    return id;
  }
protected:
  int id = 0;
};

class ModifyRequest : public Request {
public:
  using Request::Request;
  virtual void Process(BusStopMap& map) = 0;
  virtual ~ModifyRequest() = default;
};

class RouteSettings : public ModifyRequest {
public:
  RouteSettings()
  : ModifyRequest(Type::ROUTE_SETTINGS)
  {}
  void ParseFrom(const std::map<std::string, Json::Node>& request) override;
  void ParseFrom(std::string_view input) override {
    /* UNSUPPORTED */
    (void)input;
  }
  void Process(BusStopMap& map) override;
private:
  int wait_time = 1;
  double velocity = 1.0;
};

class RouteDefRequest : public ModifyRequest {
public:
  RouteDefRequest()
  : ModifyRequest(Type::ROUTE_DEFINITION)
  {}

  void ParseFrom(std::string_view input) override;
  void ParseFrom(const std::map<std::string, Json::Node>& request) override;
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
  void ParseFrom(const std::map<std::string, Json::Node>& request) override;
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
  void ParseFrom(const std::map<std::string, Json::Node>& request) override;
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
  void ParseFrom(const std::map<std::string, Json::Node>& request) override;
  std::string Process(const BusStopMap& map) const override;
private:
  std::string stop_name;
};

class RouteInfoRequest : public PrintRequest<std::string> {
public:
  RouteInfoRequest()
  : PrintRequest(Type::ROUTE_INFO)
  {}

  void ParseFrom(std::string_view input) override {
    (void)input;
    /* UNSUPPORTED */
  }
  void ParseFrom(const std::map<std::string, Json::Node>& request) override;
  std::string Process(const BusStopMap& map) const override;
private:
  std::string stop_from;
  std::string stop_to;
};

#endif // REQUEST_H
