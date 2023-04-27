#include "planner_iface.h"
#include "lrtdp.h"

#include "../utils/die.h"
#include "../ext/mgpt/global.h"
#include "../ext/mgpt/hash.h"
#include "../ext/mgpt/states.h"

/*******************************************************************************
 *
 * planner LRTDP: Labeled RTDP
 *
 ******************************************************************************/

PlannerLRTDP::PlannerLRTDP(SSPIface const& ssp, heuristic_t& heur,
    double epsilon, size_t max_trace_size, bool use_as_replanner,
    bool use_as_greedy_planner)
  : OptimalPlanner(), ssp_(ssp), internal_v_(new hash_t(gpt::initial_hash_size, heur)),
    v_(*internal_v_), epsilon_(epsilon), max_trace_size_(max_trace_size),
    use_as_replanner_(use_as_replanner), use_as_greedy_planner_(use_as_greedy_planner)
{ }

void PlannerLRTDP::trial(state_t const& s) {

  // TODO(fwt): OPTIMIZATION: use a member variable vector to put the visited
  // states. This way we should save some time on memory allocation.
  std::stack<hashEntry_t*> visited;
  state_t cur_s = s;
  // Keeping a hashEntry_t around to save a few hash calls
  hashEntry_t *cur_node = v_.get(s);

  size_t trace_size = 0;
  while (true) {
    // This should always hold
    assert(cur_s == *(cur_node->state()));

#ifdef DEBUG_TRACE
    std::cout << "Trial cur: "
              << cur_s.toStringFull(gpt::problem, false, true)
              << " V = " << cur_node->value() << std::endl;
#endif

    // Stop criteria for this loop
    if (ssp_.isGoal(cur_s)) {
      // Not really necessary but just in case. Potential Pitfall
      //setSolved(cur_s);
      DEBUG_MSG("LrtdpTrialDebug", "Exiting while [state is goal]");
      break;
    }
    else if (cur_node->value() >= gpt::dead_end_value.double_value()) {
      setSolved(cur_s);
      DEBUG_MSG("LrtdpTrialDebug", "Exiting while [state is dead-end]");
      break;
    }
    else if (isSolved(cur_s)) {
      DEBUG_MSG("LrtdpTrialDebug", "Exiting while [state_t is solved]");
      break;
    }
    else if (trace_size > max_trace_size_) {
      std::cout << "Max trace size (" << max_trace_size_ << ") reached"
                << std::endl;
      DEBUG_MSG("LrtdpTrialDebug", "Exiting while [max trace size]");
      break;
    }

    visited.push(cur_node);
    action_t const* a_greedy = nullptr;
    std::tie(a_greedy, std::ignore) = Bellman::update(cur_s, v_, ssp_);

    if (a_greedy == nullptr) {
      DEBUG_MSG("LrtdpTrialDebug", "Exiting while [no action]");
      setSolved(cur_s);
      break;
    }

    a_greedy->affect(cur_s);
    cur_node = v_.get(cur_s);
    gpt::incCounterAndCheckDeadlineEvery(trace_size, 100);
  }  // while true

//  std::cout << "[LRTDP] End of trial. Total visited = " << visited.size()
//            << std::endl;
  while (!visited.empty()) {
    cur_node = visited.top();  // last visited state
    // try labeling
    if (!isSolved(*(cur_node->state())) && !checkSolved(cur_node)) {
      break;
    }
    visited.pop();
  }
  // TODO(fwt): OPTIMIZATION: if using a member variable for visited, then it
  // should be cleared here.
}



void PlannerLRTDP::statistics(std::ostream &os, int level) const {
  if (level >= 300)
    v_.print(os, ssp_);
}
