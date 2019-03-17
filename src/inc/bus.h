#ifndef BUS_H
#define BUS_H

#include <memory>

#include "stringparser.h"

class Bus
{
public:
  Bus(const std::string& bus_number)
  : number(bus_number)
  {
    std::hash<std::string> s_hash;
    bus_hash = s_hash(number);
  }

  //static Bus FromString(std::string_view str);

  size_t GetHash() const {
    return bus_hash;
  }

  const std::string& GetNumber() const {
    return number;
  }

private:
  std::string number;
  size_t bus_hash;
};

bool operator<(const Bus& lhs, const Bus& rhs);
bool operator==(const Bus& lhs, const Bus& rhs);

struct BusHasher {
  size_t operator()(const Bus& bus) const {
    return bus.GetHash();
  }
};


#endif // BUS_H
