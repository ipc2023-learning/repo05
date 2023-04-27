#include "die.h"
#include <cassert>
#include <iostream>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

const void die(bool condition, const char* x, int exit_code, const char* f,
    int l)
{
  if (! condition) {
    std::cerr << "Error: " << x << " in " << f << ":" << l << std::endl;
#ifdef DIE_WITH_ASSERT
    assert(false);
#endif
    exit(exit_code);
  }
}

const void __fwt_fancy_die_if_function__(bool condition, int exit_code,
    char const* f, int l, std::string frm, ...)
{
  if (condition) {
    std::cerr << "[Error in " << f << ":" << l << "]: ";
    va_list args;
    va_start(args, frm);
    vprintf(frm.c_str(), args);
    va_end(args);
    std::cout << std::endl;
#ifdef DIE_WITH_ASSERT
    assert(false);
#endif
    exit(exit_code);
  }
}

const void __fwt_not_implemented__(char const* func_name, char const* file_name,
    int line)
{
  std::cerr << "[Error in " << file_name << ":" << line << "]: function "
    << func_name << " not implemented!" << std::endl;
#ifdef DIE_WITH_ASSERT
  assert(false);
#endif
  exit(100);
}
