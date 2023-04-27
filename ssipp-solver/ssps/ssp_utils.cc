#include <iostream>
#include <queue>

#include "../utils/die.h"
#include "ssp_utils.h"
#include "ssp_iface.h"
#include "../ext/mgpt/atom_states.h"
#include "../ext/mgpt/actions.h"
#include "../utils/utils.h"


HashsetState const reachableStatesFrom(SSPIface const& ssp, state_t const& root)
{
  typedef std::queue<state_t> QueueState;
  QueueState queue;
  HashsetState reachable;
  size_t deadline_counter = 0;

  // Invariant: every state in queue is also in reachable
  queue.push(root);
  reachable.insert(root);

  std::cout << "[reachableStatesFrom] Starting from s = "
            << root.toStringFull(gpt::problem) << std::endl;
  uint64_t before_start_search = get_cputime_usec();

  while (!queue.empty()) {
    gpt::incCounterAndCheckDeadlineEvery(deadline_counter, 10000);
    state_t const& s = queue.front();
//    std::cout << "  s = " << s.toStringFull(gpt::problem, false, false);

    if (!ssp.isGoal(s)) {
//      size_t applicable = 0;
//      size_t added = 0;
      for (auto const& a : ssp.applicableActions(s)) {
//        applicable++;
        ProbDistState pr;
        ssp.expand(a, s, pr);
        for (auto const& ip : pr) {
          state_t const& s_prime = ip.event();
          if (reachable.find(s_prime) == reachable.end()) {
            reachable.insert(s_prime);
            queue.push(s_prime);
//            added++;
          }
        }
      }
//      std::cout << " -- Expanded " << applicable << " actions "
//                << " and added " << added << " states" << std::endl;
    }
//    else {
//      std::cout << " -- is goal. Not expanding" << std::endl;
//    }
    queue.pop();
  }
  uint64_t time_spent_usecs = get_cputime_usec() - before_start_search;
  std::cout << "[reachableStatesFrom] Total states = " << reachable.size()
            << std::endl
            << "[reachableStatesFrom] Total time = " << time_spent_usecs
            << std::endl;

  return reachable;
}


action_t const* randomAction(SSPIface const& ssp, state_t const& s) {
  // Doing reservoir sampling to go through the applicable actions only once
  // since that can be an expensive operation (chain of adaptors, etc) in
  // comparison with randomly drawing numbers.
  action_t const* chosen_a = nullptr;
  size_t total_actions = 0;
  for (action_t const& a : ssp.applicableActions(s)) {
    total_actions++;
    if (rand0to1_d() < 1.0 / (double) total_actions) {
      chosen_a = &a;
    }
  }
  DIE(chosen_a != nullptr || !ssp.hasApplicableActions(s),
      "state has applicable action but none were chosen.", -1);
  return chosen_a;
}
