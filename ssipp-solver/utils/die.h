#ifndef _DIE_H_
#define _DIE_H_
#include <iostream>

#if defined(ENABLE_D)
#define _D(flag, code) { if (flag) { code; } }
#else
#define _D(flag, code) { while(false) { } }
#endif


#if defined(NDEBUG) || defined (NODIE)
#define DIE(cond, x, exit_code) { while(false) { } }
#else
#define DIE(cond, x, exit_code) { die(cond, x, exit_code, __FILE__, __LINE__); }
#endif

const void die(bool condition, const char* x, int exit_code, const char* f, int l);

#if defined(NDEBUG) || defined (NODIE)
#define FANCY_DIE_IF(cond, exit_code, frm, ...) { while(false) { } }
#else
/*
 * Example of FANCY_DIE_IF
 * FANCY_DIE_IF(i > threshold, "i == '%d' > '%d' == threshold", i, threshold);
 */
#define FANCY_DIE_IF(cond, exit_code, frm, ...) { __fwt_fancy_die_if_function__(cond, exit_code, __FILE__, __LINE__, frm, ##__VA_ARGS__); }
#endif
const void __fwt_fancy_die_if_function__(bool condition, int exit_code, char const* f, int l, std::string frm, ...);

#define NOT_IMPLEMENTED { __fwt_not_implemented__(__PRETTY_FUNCTION__, __FILE__, __LINE__); }

const void __fwt_not_implemented__(char const* func_name, char const* file_name, int line);

#endif
