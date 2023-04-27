#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <set>

#include "planner_iface.h"
#include "vi.h"

#include "../utils/die.h"
#include "../ext/mgpt/global.h"
#include "../ext/mgpt/hash.h"
#include "../ssps/ssp_utils.h"
#include "../ext/mgpt/states.h"


/*******************************************************************************
 *
 * planner VI
 *
 ******************************************************************************/

// static
size_t PlannerVI::solve(SSPIface const& ssp, hash_t& v, double epsilon) {
  double max_residual = 1.0 + epsilon;
  size_t iter = 0;
  size_t bellmanBackupCounter = 0;

  StateConstRange const& reachable_states = ssp.reachableStates();
  while (max_residual > epsilon) {
    max_residual = 0;
    iter++;
    for (state_t const& s : reachable_states) {
      gpt::incCounterAndCheckDeadlineEvery(bellmanBackupCounter, 1000);
      // Computing the Bellman residual and ALSO APPLYING a Bellman UPDATE
      double residual = Bellman::residual(s, v, ssp, true);
      max_residual = std::max(max_residual, residual);

#ifdef VI_SHOW_UPDATES
      std::cout << "[vi::update] #" << iter << ": ";
      s.debug_print(gpt::problem, &v);
      std::cout << " updated. New value: " << v.value(s)
                << "  (Res = " << residual << ")" << std::endl;
#endif
    }
#ifdef VI_SHOW_RESIDUAL
    std::cout << "[vi::update] Iteration over. Max residual = "
              << max_residual << std::endl;
#endif
  }

#ifdef VI_SHOW_SOLUTION
  std::cout << std::endl << "[vi::solution]";
  for (state_t const& s : reachable_states) {
    std::cout << "\t";
    s.full_print(std::cout, gpt::problem);

    double min_q_value = 0;
    action_t const* a_greedy = nullptr;
    std::tie(a_greedy, min_q_value) =
                                 Bellman::greedyActionAndMinQValue(s, v, ssp);
    std::cout << " -- V* = " << min_q_value
              << "  pi* = " << (a_greedy ? a_greedy->name() : "NULL")
              << std::endl;
  }
#endif
  return iter;
}


void PlannerVI::statistics(std::ostream &os, int level) const {
  if (level > 0) {
    os << "[vi]: hash size = " << v_.size() << std::endl;
    os << "[vi]: hash diameter = " << v_.diameter() << std::endl;
    os << "[vi]: hash dimension = " << v_.dimension() << std::endl;
  }
  if (level >= 300)
    v_.print(os, ssp_);
}


