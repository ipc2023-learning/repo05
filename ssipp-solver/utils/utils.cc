#include <ctime>
#include <iostream>
#include <iomanip>
#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>

#include "utils.h"

#include "../ext/mgpt/states.h"

#include "../ext/mgpt/actions.h"
#include "../ext/mgpt/problems.h"


extern "C" {
  #include "../ext/mgpt/md4c.h"
};


const void __my_warning_function__(const char* f, int l, std::string frm, ...) {
#ifndef NO_COLOR
  std::cout << YELLOW;
#endif

  std::cout << "[" << f << ":" << l <<"]: ";
  va_list args;
  va_start(args, frm);
  vprintf(frm.c_str(), args);
  va_end(args);
  std::cout << std::endl;
#ifndef NO_COLOR
  std::cout << RESET_COLOR;
#endif
}

const void __my_debug_manager__(char const* f, int l, std::string const& t,
                              std::string const& msg, ...)
{
#ifdef NO_DEBUG_MANAGER
  return;
#else
  if (gpt::debug_signals.find(t) != gpt::debug_signals.end()) {
    std::cout << "[" << f << ":" << l << "]: ";
    va_list args;
    va_start(args, msg);
    vprintf(msg.c_str(), args);
    va_end(args);
    std::cout << std::endl;
  }
#endif
}

std::set<std::string> parseDebugSignals() {
#ifdef NO_DEBUG_MANAGER
  std::set<std::string> s;
  return s;
#else
  std::cout << DEBUG_SIGNALS << std::endl;
  std::string list_of_signals(DEBUG_SIGNALS);
  std::deque<std::string> signals = splitString(list_of_signals, ",");
  std::set<std::string> set_signals;
  for (std::string const& s : signals)
    set_signals.insert(s);
  return set_signals;
#endif
}


/*
 * Eventually use boost:
    #include <boost/algorithm/string.hpp>
    std::vector<std::string> strs;
    boost::split(strs, "string to split", boost::is_any_of("\t "));
 */
std::deque<std::string> splitString(std::string const& s,
                                    std::string const& delims)
{
  DIE(delims.size() == 1, "Only one delim is allowed for now", -1);
  std::deque<std::string> deque;
  if (s.size() == 0)
    return deque;

  size_t begin = 0;
  size_t delim_pos = s.find(delims);
  while (delim_pos != std::string::npos) {
    deque.push_back(s.substr(begin, delim_pos-begin));
    begin = delim_pos + 1;
    delim_pos = s.find(delims, delim_pos+1);
  }
  deque.push_back(s.substr(begin));
  return deque;
}


std::string const get_human_readable_timestamp() {
  time_t now = time(0);
  tm* ltm = localtime(&now);
  char buffer[100];
  strftime(buffer, 100, "%Y-%m-%d %H:%M:%S", ltm);
  return std::string(buffer);
}


std::string const SystemResourcesDeadline::explanation() const {
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
//    std::cout << "cpu_time vs threshold_time_ && max_rss vs threshold_max_rss_" << std::endl;
  std::ostringstream ost;
  ost << "[SystemResourcesDeadline] ";
  if (threshold_time_ > 0 && cpu_time > threshold_time_) {
    ost << "CPU+System time deadline reached. ";
  }
  if (threshold_max_rss_ > 0 && max_rss > threshold_max_rss_) {
    ost << "Maximum Resident Memory (RSS) reached (threshold was "
        << threshold_max_rss_ << ").";
  }
  return ost.str();

#else
  std::cerr << "RUSAGE_SELF and/or RUSAGE_CHILDREN not defined!" << std::endl;
  exit(-1);
  return std::string("ERROR: RUSAGE_SELF and/or RUSAGE_CHILDREN not defined!");
#endif
}


void DEBUG_show_probability_of_adding_atoms(state_t const& s) {
  HashAtomtToRational atom_prob;
  std::cout << "=================================================="
            << std::endl;
  std::cout << "s = ";
  s.full_print(std::cout, gpt::problem, true, false);
  std::cout << std::endl;

  for (size_t i = 0; i < gpt::problem->actionsT().size(); ++i) {
    if (gpt::problem->actionsT()[i]->enabled(s)) {
      gpt::problem->actionsT()[i]->print_full();
      std::cout << std::endl
        << "--------------------------------------------------"
        << std::endl;
      atom_prob.clear();
      gpt::problem->actionsT()[i]->probability_of_adding_atoms(s, atom_prob);
      for (HashAtomtToRational::iterator it = atom_prob.begin();
          it != atom_prob.end(); ++it)
      {
        Atom const* atom = problem_t::atom_inv_hash_get(it->first);
        std::cout << "\tP(" << it->first << ": "
          << (it->first % 2 ? "not " : "");
        gpt::problem->print(std::cout, *atom);
        std::cout << ") = " << it->second << std::endl;
        FANCY_DIE_IF(it->second > 1, 171,
            "Probability of atom %u is %f", it->first,
            it->second.double_value());
      }
    }
  }
  std::cout << "=================================================="
            << std::endl;
}

std::string getHostname() {
  char name[150];
  memset(name, 0, 150);
  gethostname(name, 150);
  return std::string(name);
}
