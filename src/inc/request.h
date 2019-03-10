#ifndef REQUEST_H
#define REQUEST_H

#include "stringparser.h"

#include <memory>

class Request
{
public:
  using RequestHolder = std::unique_ptr<Request>;

  enum class Type {
    STOP_DECLARATION,
    BUS_INFO
  };

  Request(Type t) : type(t) {}

  static RequestHolder Create(Type type);
  virtual void ParseFrom(std::string_view input) = 0;
  virtual ~Request() = default;

  const Type type;
};

class RouteDefRequest : public Request {
  RouteDefRequest() : Request(Type::ROUTE_DEFINITION) {}
  void ParseFrom(std::string_view input) override;
};

class StopDeclRequest : public Request {
  StopDeclRequest() : Request(Type::ROUTE_DEFINITION) {}
  void ParseFrom(std::string_view input) override;
};

class BusInfoRequest : public Request {
  BusInfoRequest() : Request(Type::ROUTE_DEFINITION) {}
  void ParseFrom(std::string_view input) override;
};

#endif // REQUEST_H
