#ifndef GLOBAL_H
#define GLOBAL_H

#include <cstdio>
#include <cstdint>
#include <string>
#include <memory>
#include <stack>
#include <set>

#include "../../utils/die.h"
#include "../../utils/exceptions.h"
#include "rational.h"

#ifndef UCHAR_MAX
#define UCHAR_MAX       255
#endif
#ifndef USHORT_MAX
#define USHORT_MAX      65535
#endif

#define NUMBER_NAME     "number"
#define OBJECT_NAME     "object"

#define GPTMAX(x,y)        ((x)>(y)?(x):(y))
#define GPTMIN(x,y)        ((x)<(y)?(x):(y))

// From boost library. The wierd "double" call is really necessary:
// http://www.iar.com/Global/Resources/Developers_Toolbox/C_Cplusplus_Programming/Tips%20and%20tricks%20using%20the%20preprocessor%20%28part%20two%29.pdf
#define STRINGIFY(s) XSTRINGIFY(s)
#define XSTRINGIFY(s) #s

#ifndef MAX_TRACE_SIZE
#define MAX_TRACE_SIZE 1000000
#endif

#define RESET_COLOR "\e[m"
#define BRIGHT_GREEN "\e[32m"
#define BRIGHT_RED "\e[31m"
#define YELLOW "\e[33m"
#define BRIGHT_PURPLE "\e[35m"
#define BRIGHT_CYAN "\e[36m"

// In the cost vector, the first index is the action cost, i.e., the function
// that is generally minimized and the only function in the case of an SSP
#define ACTION_COST 0

// FWT: using the current implementation of state_t and problem, the maximum
// number of atoms of a problem is USHORT_MAX
#if not defined MAX_ATOMS
#define MAX_ATOMS 16384
#endif

#include <bitset>
using AtomBitset = std::bitset<MAX_ATOMS>;


// FWT: gcc only
#ifdef __GNUC__
#define DEPRECATED __attribute__((deprecated))
#else
#define DEPRECATED
#endif

class heuristic_t;
class problem_t;
class Simulator;
class PlannerFFReplan;

enum DeterminizationType {MOST_LIKELY_OUTCOMES = 0, ALL_OUTCOMES};

class SignalManager {
 public:
  SignalManager();
  ~SignalManager() { }
 private:
  static void sigUsr1Handler(int signum);
  static void sigTermHandler(int signum);
};

enum StopCriterion {NONE = 0,    // The stop criterion is not defined yet
                    NUM_ROUNDS,  // Stop after a given number of rounds
                    CONV_S0,     // Stop when V(s0) has epsilon-converged
                    CONV_COST    // Stop when the average cost has epsilon-converged (?)
                  };

class Deadline {
 public:
  Deadline() { }
  virtual ~Deadline() { }
  // Returns true if the deadline is over and everything else should stop.
  virtual bool isOver() const = 0;
  // Returns a string explaning why the deadline is over
  virtual std::string const explanation() const = 0;
  // This returns true if there is a time deadline. In this case, remaining is
  // populated with the remaining time in usecs.
  virtual bool remainingTimeInUsec(uint64_t& remaining) const {
    return false;
  }
};


namespace gpt
{
  extern uint64_t train_for_usecs;
  extern uint64_t parsing_cpu_time;
#ifdef USE_CACHE_PROB_OP_ADDS_ATOM
  extern size_t total_saved_prob_op_adds_atom_calls;
  extern size_t total_prob_op_adds_atom_calls;
#endif
  extern bool randomize_actionsT_order;
  extern double given_value_for_vStar_s0;
  extern uint64_t max_cpu_sys_time_usec;
  extern uint64_t max_rss_kb;
  extern std::shared_ptr<Deadline> _deadline_;
  extern StopCriterion stopCriterion;
  extern SignalManager signalManager;
  extern std::string ff_path;
  extern std::string lama_path;
  extern std::string legend_file;
  extern std::set<std::string> debug_signals;
  extern double ignore_effects_with_prob_less_than;
  extern std::shared_ptr<PlannerFFReplan> followable_ffreplan;
  extern std::string followable_ffreplan_param;
  extern bool external_ff_ignore_forall_w_prob_effects;
  extern bool suppress_round_info;
  extern bool default_hp;
  extern std::string algorithm;
  extern bool domain_analysis;
  extern Rational dead_end_value;
  extern unsigned cutoff;
  extern double epsilon;
  extern bool hash_all;
  extern std::string heuristic;
  extern size_t initial_hash_size;
  extern unsigned max_database_size;
  extern bool noise;
  extern double noise_level;
  extern unsigned seed;
  extern int simulations;
  extern unsigned verbosity;
  extern unsigned warning_level;
  extern double heuristic_weight;
  extern size_t xtra;

  extern int max_time_in_secs;

  extern uint64_t start_time;
  extern bool print_turn_details;
  extern std::string execution_simulator;
  extern uint32_t total_execution_rounds;
  // extern bool run_to_convergence;

  /* replanner_threshold_t:
   *    The replanner will search for (replanner_threshold_t)-closed policies.*/
  // extern size_t replanner_threshold_t; // -T

  extern problem_t const* problem;
  extern Simulator const* simulator;

  extern bool show_applied_policy;
  extern bool show_computed_policy;
  extern uint32_t max_turn;

  // This variable holds the amount of shift applied to the cost of actions in
  // order to make all the actions to have cost > 0
  extern Rational cost_shift;

  // Enables the usage of cost of actions
  extern bool use_action_cost;
  // Enables the usage of cost of states (states and goal rewards). Only works
  // if the cost of actions is enabled
  extern bool use_state_cost;
  // Enables the normalization of the actions cost. That is, all the actions
  // cost will be guaranted to be >= 0
  extern bool normalize_action_cost;
  // Enables the dynamic choice of dead-end value. Since this guarantee is
  // probabilistic, we need an epsilon
  extern std::string tmp_dir;

  bool setDeadline(std::shared_ptr<Deadline> deadline);
  void removeDeadline();
  void checkDeadline();  // Throws DeadlineReached
  void incCounterAndCheckDeadlineEvery(size_t& counter, size_t mod);
};

inline bool gpt::setDeadline(std::shared_ptr<Deadline> deadline) {
  if (_deadline_)
    return false;
  else {
    _deadline_ = deadline;
    return true;
  }
}

inline void gpt::removeDeadline() { gpt::_deadline_ = nullptr; }

inline void gpt::checkDeadline() {
  if (_deadline_ && _deadline_->isOver())
    throw DeadlineReachedException(_deadline_->explanation());
}

inline void gpt::incCounterAndCheckDeadlineEvery(size_t& counter, size_t mod) {
  counter++;
  if (counter % mod == 0) {
    gpt::checkDeadline();
  }
}


typedef unsigned char uchar_t;
typedef unsigned short ushort_t;
typedef ushort_t atom_t;

inline void notify(void *ptr, std::string const& name) {
#ifdef MEM_DEBUG
  fprintf( stderr, "notify %s %p\n", name, ptr );
#endif
}

bool readPDDLFile(const char* name);
inline bool readPPDDLDomainFile(const char* name) {
  return readPDDLFile(name);
}

inline bool readPPDDLProblemFile(const char* name) {
  return readPDDLFile(name);
}
#endif // GLOBAL_H
