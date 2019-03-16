#ifndef BUS_H
#define BUS_H

#include <memory>

#include "stringparser.h"
#include "BusRoute.h"

class Bus
{
public:
  Bus(const std::string& number, BusRoute new_route) 
  : route(std::make_unique<BusRoute>(new_route))
  {
    std::hash<std::string> s_hash;
    bus_hash = s_hash(number);
  }

  static Bus FromString(std::string_view str);

  size_t GetHash() const {
    return bus_hash;
  }

  const std::string& GetNumber() const {
    return number;
  }

  const auto& GetRoute() const {
    return route;
  }

private:
  std::string number;
  std::unique_ptr<BusRoute> route;
  size_t bus_hash;
};

bool operator<(const Bus& lhs, const Bus& rhs);

struct BusHasher {
  size_t operator()(const Bus& bus) const {
    return bus.GetHash();
  }
};


#endif // BUS_H
