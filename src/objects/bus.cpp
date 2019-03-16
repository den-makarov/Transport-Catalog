#include "bus.h"

bool operator<(const Bus& lhs, const Bus& rhs) {
  return lhs.GetNumber() < rhs.GetNumber();
}
// Bus::Bus()
// {

// }