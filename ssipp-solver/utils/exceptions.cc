#include "exceptions.h"

std::ostream& operator<< (std::ostream& os, Exception const& e)
{
  return os << "<exception>: ERROR: " << e.msg_;
}
