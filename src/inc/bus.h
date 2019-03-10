#ifndef BUS_H
#define BUS_H

#include "stringparser.h"

class Bus
{
public:
  Bus();
  Bus(std::string_view input) {}
private:
  int number;
  Route route;
};

#endif // BUS_H
