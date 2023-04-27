#ifndef UTILS_H
#define UTILS_H

#include <cstring>
#include <iostream>
#include <list>
#include <math.h>
#include <set>
#include <sstream>
#include <stack>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <sys/resource.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <vector>
#include <functional>

#include "die.h"
#include "../ext/mgpt/global.h"


#ifndef DEBUG_ATOMLIST
#define DEBUG_ATOMLIST 0
#endif


class state_t;

/******************************************************************************
 * Macros to print text in color.
 * To turn it off, define the NO_COLOR (-DNO_COLOR)                          */

#ifndef NO_COLOR
#define IN_COLOR(code, body) code << body << RESET_COLOR
#else
#define IN_COLOR(code, body) body

#endif
#ifndef RESET_COLOR
#define RESET_COLOR "\033[m"
#define BRIGHT_GREEN "\033[32m"
#define BRIGHT_RED "\033[31m"
#define YELLOW "\033[33m"
#define BRIGHT_PURPLE "\033[35m"
#define BRIGHT_CYAN "\033[36m"
#endif
/******************************************************************************/


/* Debug macros */

/*
 * WARNING MACRO
 */
#ifdef IGNORE_WARNINGS
#define WARNING(frm, ...) { while(false) { } }
#else
#define WARNING(frm, ...) { __my_warning_function__(__FILE__, __LINE__, frm, ##__VA_ARGS__); }
#endif
const void __my_warning_function__(char const* f, int l, std::string frm, ...);

/*
 * DEBUG MESSAGES MACROS. These macros receive a token as id and these token id
 * need to be active during compiling time to be active (i.e., to print). Here
 * is how to active the tokens t1 and t2:
 *
 * make DEBUG_SIGNALS="t1,t2" all
 */
#ifndef DEBUG_SIGNALS
#define NO_DEBUG_MANAGER
#endif

#ifdef NDEBUG
#define DEBUG_MSG(t, msg, ...) { while(false) { } }
#define DEBUG_TOKEN(t, msg, ...) { while(false) { } }
#define DEBUG_EXEC(t, code) { while(false) { } }
#else
#define DEBUG_MSG(t, msg, ...) { __my_debug_manager__(__FILE__, __LINE__, t, msg, ##__VA_ARGS__); }
#define DEBUG_TOKEN(t, token) DEBUG_MSG(t, #token " is %d", token)
/*
 * Example of multi-line code usage:
    DEBUG_EXEC("uct", std::cout << "Search tree dump:" << std::endl;
                      search_tree.dump());
 */
#define DEBUG_EXEC(t, code) { if (gpt::debug_signals.find(t) != gpt::debug_signals.end()) { code; } }
#endif

const void __my_debug_manager__(char const* f, int l, std::string const& t,
                                std::string const& msg, ...);


std::set<std::string> parseDebugSignals();



/******************************************************************************/
/*                    Profiling Macros                                       */

#define START_TIMING(label) { while(false) { } }
#define STOP_TIMING(label) { while(false) { } }
#define EXEC_AND_TIME(label, code) { code; }

/******************************************************************************/
/*                    Statistics Macros                                       */

#define LOG_VAR(label, v) { while(false) { } }
#define COND_LOG_VAR(cond, label, v) { while(false) { } }
#define LOG_VAR_SINGLE(label, v) { while(false) { } }
#define LOG_VAR_SINGLE_INC(label, v) { while(false) { } }


/******************************************************************************/




/******************************************************************************/


/*
 * Util functions
 */
// Return current time as YYYY-MM-DD HH:mm:SS
std::string const get_human_readable_timestamp();

inline uint64_t get_time_usec() {
  struct timespec ts;
  if (clock_gettime(CLOCK_MONOTONIC, &ts)) {
    perror("get_time_usec() failed: ");
    exit(1);
  }
  return (uint64_t)ts.tv_sec * 1000000 + (uint64_t)ts.tv_nsec / 1000;
}

/*
 * for more robust version of a function that gets the cputime, see:
 * http://nadeausoftware.com/articles/2012/03/c_c_tip_how_measure_cpu_time_benchmarking
 */
inline uint64_t get_cputime_usec() {
#if defined(RUSAGE_SELF) && defined(RUSAGE_CHILDREN)
  struct rusage rusage;
  uint64_t cputime = 0;
  int rt = getrusage(RUSAGE_SELF, &rusage);
  DIE(rt != -1, "getrusage(RUSAGE_SELF) returned -1", -1);
  cputime += 1000000 * rusage.ru_utime.tv_sec + rusage.ru_utime.tv_usec;
  rt = getrusage(RUSAGE_CHILDREN, &rusage);
  DIE(rt != -1, "getrusage(RUSAGE_CHILDREN) returned -1", -1);
  cputime += 1000000 * rusage.ru_utime.tv_sec + rusage.ru_utime.tv_usec;
  return cputime;
#else
  std::cerr << "RUSAGE_SELF and/or RUSAGE_CHILDREN not defined!" << std::endl;
  exit(-1);
  return 0;
#endif
}


inline uint64_t get_cpu_and_sys_time_usec() {
#if defined(RUSAGE_SELF) && defined(RUSAGE_CHILDREN)
  struct rusage rusage;
  uint64_t cpuSystime = 0;
  int rt = getrusage(RUSAGE_SELF, &rusage);
  DIE(rt != -1, "getrusage(RUSAGE_SELF) returned -1", -1);
  cpuSystime += 1000000 * (rusage.ru_utime.tv_sec + rusage.ru_stime.tv_sec)
                  + rusage.ru_utime.tv_usec + rusage.ru_stime.tv_usec;
  rt = getrusage(RUSAGE_CHILDREN, &rusage);
  DIE(rt != -1, "getrusage(RUSAGE_CHILDREN) returned -1", -1);
  cpuSystime += 1000000 * (rusage.ru_utime.tv_sec + rusage.ru_stime.tv_sec)
                  + rusage.ru_utime.tv_usec + rusage.ru_stime.tv_usec;
  return cpuSystime;
#else
  std::cerr << "RUSAGE_SELF and/or RUSAGE_CHILDREN not defined!" << std::endl;
  exit(-1);
  return 0;
#endif
}



inline size_t get_secs_since_epoch() {
  time_t seconds;
  seconds = time(NULL);
  return (size_t) seconds;
}


/*
 * Returns the max resident memory. According to the current kernel manpage at
 *           http://man7.org/linux/man-pages/man2/getrusage.2.html
 * the obtained value for the children (RUSAGE_CHILDREN) is "the resident set
 * size of the largest child, not the maximum resident set size of the process
 * tree". Therefore, this function returns the max resident memory of the
 * current process and the largest children. For now this is OK since the only
 * children we run is a call to FF.
 */
inline uint64_t get_max_resident_mem_in_kb() {
#if defined(RUSAGE_SELF) && defined(RUSAGE_CHILDREN)
  struct rusage rusage;
  uint64_t maxrss = 0;
  int rt = getrusage(RUSAGE_SELF, &rusage);
  DIE(rt != -1, "getrusage(RUSAGE_SELF) returned -1", -1);
  maxrss += rusage.ru_maxrss;
  rt = getrusage(RUSAGE_CHILDREN, &rusage);
  DIE(rt != -1, "getrusage(RUSAGE_CHILDREN) returned -1", -1);
  maxrss += rusage.ru_maxrss;
  return maxrss;
#else
  std::cerr << "RUSAGE_SELF and/or RUSAGE_CHILDREN not defined!" << std::endl;
  exit(-1);
  return 0;
#endif
}



inline float  rand0to1_f()         { return (float) drand48(); }
inline double rand0to1_d()         { return drand48(); }
inline double rand0toN_d(double n) { return drand48() * n; }
inline size_t rand0toN_l(size_t n) { return lrand48() % n; }
inline size_t randInIntervalInclusive_l(size_t min, size_t max) {
  return (lrand48() % (max-min+1)) + min;
}

std::deque<std::string> splitString(std::string const& s,
                                    std::string const& delims);

// TODO(fwt): Greatly improve this
inline std::string& trim_string(std::string& str) {
  // trim trailing spaces
  size_t endpos = str.find_last_not_of(" \t\n");
  if (std::string::npos != endpos) {
    str = str.substr(0, endpos+1);
  }
  // trim leading spaces
  size_t startpos = str.find_first_not_of(" \t\n");
  if (std::string::npos != startpos) {
    str = str.substr(startpos);
  }
  return str;
}


std::string getHostname();





inline bool e_less(double x, double y) {
  return ((y-x) > gpt::epsilon);
}

inline bool e_equal(double x, double y) {
  return (fabs(x-y) <= gpt::epsilon);
}



/******************************************************************************/
/*                         Deadline Classes                                   */
class CpuTimeDeadline : public Deadline {
 public:
  CpuTimeDeadline(uint64_t max_time_usec) : Deadline() {
    threshold_ = get_cputime_usec() + max_time_usec;
  }
  ~CpuTimeDeadline() { }
  bool isOver() const { return get_cputime_usec() > threshold_; }
  std::string const explanation() const {
    return std::string("[CpuTimeDeadline] CPU time exceeded.");
  }
  bool remainingTimeInUsec(uint64_t& remaining) const {
    uint64_t now = get_cputime_usec();
    if (now > threshold_) {
      remaining = 0;
    }
    else {
      remaining = threshold_ - now;
    }
    return true;
  }
 private:
  uint64_t threshold_;
};


class CpuPlusSystemTimeDeadline : public Deadline {
 public:
  CpuPlusSystemTimeDeadline(uint64_t max_time_usec) : Deadline() {
    threshold_ = get_cpu_and_sys_time_usec() + max_time_usec;
  }
  ~CpuPlusSystemTimeDeadline() { }
  bool isOver() const {
    return get_cpu_and_sys_time_usec() > threshold_;
  }
  std::string const explanation() const {
    return std::string("[CpuPlusSystemTimeDeadline] CPU+System time exceeded.");
  }
  bool remainingTimeInUsec(uint64_t& remaining) const {
    uint64_t now = get_cputime_usec();
    if (now > threshold_) {
      remaining = 0;
    }
    else {
      remaining = threshold_ - now;
    }
    return true;
  }
 private:
  uint64_t threshold_;
};


/*
 * Set a deadline as cpu+system and/or max resident memory. If max_time_usec is
 * 0, then cpu+system is ignored. If max_rss_kb is 0, then max resident memory
 * is ignored. Either max_time_usec or max_rss_kb must be defined.
 */
class SystemResourcesDeadline : public Deadline {
 public:
  SystemResourcesDeadline(uint64_t max_time_usec, uint64_t max_rss_kb)
    : Deadline(), threshold_max_rss_(max_rss_kb), threshold_time_(0)
  {
    if (max_time_usec > 0)
      threshold_time_ = get_cpu_and_sys_time_usec() + max_time_usec;
    else if (max_rss_kb == 0) {
      std::cerr << "SystemResourcesDeadline must define at least "
        << "max_time_usec or max_rss_kb. None were defined." << std::endl;
      exit(-1);
    }
  }
  ~SystemResourcesDeadline() { }

  // This method reimplements get_cpu_and_sys_time_usec and
  // get_max_resident_mem_in_kb for efficiency, i.e., save the double call.
  bool isOver() const {
#if defined(RUSAGE_SELF) && defined(RUSAGE_CHILDREN)
    struct rusage rusage;
    uint64_t cpu_time = 0;
    uint64_t max_rss = 0;
    int rt = getrusage(RUSAGE_SELF, &rusage);
    DIE(rt != -1, "getrusage(RUSAGE_SELF) returned -1", -1);
    cpu_time += 1000000 * (rusage.ru_utime.tv_sec + rusage.ru_stime.tv_sec)
                        + rusage.ru_utime.tv_usec + rusage.ru_stime.tv_usec;
    max_rss += rusage.ru_maxrss;
    // Now accounting for the childrens for cpu+system and largest children
    // for max_rss
    rt = getrusage(RUSAGE_CHILDREN, &rusage);
    DIE(rt != -1, "getrusage(RUSAGE_CHILDREN) returned -1", -1);
    cpu_time += 1000000 * (rusage.ru_utime.tv_sec + rusage.ru_stime.tv_sec)
                        + rusage.ru_utime.tv_usec + rusage.ru_stime.tv_usec;
    max_rss += rusage.ru_maxrss;
    return ((threshold_time_ > 0 && cpu_time > threshold_time_)
              || (threshold_max_rss_ > 0 && max_rss > threshold_max_rss_));
#else
    std::cerr << "RUSAGE_SELF and/or RUSAGE_CHILDREN not defined!" << std::endl;
    exit(-1);
    return true;
#endif
  }
  std::string const explanation() const;
  bool remainingTimeInUsec(uint64_t& remaining) const {
    // isOver reimplements get_cputime_usec() in order to get cpu_time and
    // memory usage at the same time, so it's fair to use get_cputime_usec()
    // here.
    uint64_t now = get_cputime_usec();
    if (now > threshold_time_) {
      remaining = 0;
    }
    else {
      remaining = threshold_time_ - now;
    }
    return true;
  }
 private:
  uint64_t threshold_max_rss_;
  uint64_t threshold_time_;
};


/*
 * This function is a wrapper for running a function (std::function<void(void)>
 * or lambda function) for the given amount of time and using the global memory
 * restriction.
 *
 * If the function finishes before the deadline, then true is returned. False is
 * returned otherwise.
 */
inline bool runForUsec(uint64_t max_time_usec, std::function<void(void)> f)
{
  bool finished_exec = true;
  auto my_deadline = std::make_shared<CpuTimeDeadline>(max_time_usec);
  if (!gpt::setDeadline(my_deadline)) {
    std::cerr << "Deadline already set and runForUsec needs to set a new "
              << "deadline" << std::endl;
    exit(-1);
  }
  try { f(); }
  catch (DeadlineReachedException& e) { finished_exec = false; }
  gpt::removeDeadline();
  return finished_exec;
}

/******************************************************************************/




/******************************************************************************/
/*            Conversion Util Functions                                       */

inline bool stringToDouble(std::string const& s, double& d,
                           bool fail_if_there_is_leftover = false)
{
  std::istringstream ist(s);
  char c;
  return (ist >> d) && !(fail_if_there_is_leftover && ist.get(c));
}
/******************************************************************************/
#endif  // UTILS_H
