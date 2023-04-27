#ifndef PLANNER_VI_H
#define PLANNER_VI_H

#include <iostream>

#include "planner_iface.h"

#include "../ext/mgpt/actions.h"
#include "../ssps/bellman.h"
#include "../ext/mgpt/hash.h"
#include "../ssps/ssp_utils.h"
#include "../utils/utils.h"

class heuristic_t;

/*******************************************************************************
 *
 * planner VI
 *
 ******************************************************************************/

class PlannerVI : public OptimalPlanner
{
 public:
   PlannerVI(SSPIface const& ssp, heuristic_t& heur, double epsilon)
     : OptimalPlanner(), ssp_(ssp),
       internal_v_(new hash_t(gpt::initial_hash_size, heur)),
       v_(*internal_v_), epsilon_(epsilon), solved_(false)
  { }

   PlannerVI(SSPIface const& ssp, hash_t& v, double epsilon)
     : OptimalPlanner(), ssp_(ssp), internal_v_(nullptr), v_(v),
       epsilon_(epsilon), solved_(false)
  { }


   ~PlannerVI() { }

  /*
  * Planner Interface
  */
  action_t const* decideAction(state_t const& s) override {
    solve();
    return Bellman::constGreedyAction(s, v_, ssp_);
  }

  action_t const* decideAction(state_t const& s) const override {
    return Bellman::constGreedyAction(s, v_, ssp_);
  }

  void trainForUsecs(uint64_t max_time_usec) override {
    if (!runForUsec(max_time_usec, [this] { solve(); })) {
      std::cout << "[VI::trainForUsecs]: training finished before convergence."
                << std::endl;
    }
  }

  void initRound() override { }
  void endRound() override { }
  void resetRoundStatistics() override { };
  void statistics(std::ostream& os, int level) const override;

  /*
   * Heuristic Planner Interface
   */
  double value(state_t const& s) const override { return v_.value(s); }

  /*
   * Optimal Planner Interface
   */
  double optimalSolution() override {
    solve();
    return v_.value(ssp_.s0());
  }

  /*
   * Value Iteration Methods
   */
  // Main method for the PlannerVI that call the static solve and update the
  // solved_ flag.
  void solve() {
    if (!solved_) {
      solve(ssp_, v_, epsilon_);
      solved_ = true;
    }
  }

  /* 
   * This method applied value iteration in the given space until v is epsilon
   * consistent. This method is static because there is no internal data that
   * needs to be saved. So, other algorithms can call this method directly
   * without creating a PlannerVI object.
   *
   * If space is provided (instead of using the 2nd version of the method), then
   * updates are applied ONLY in the states in space. A state s in space will be
   * updated even if s is not reachable from s0 in ssp.
   *
   * It returns the number of iterations used to converge to an
   * epsilon-consistent solution.
   */
  static size_t solve(SSPIface const& ssp, hash_t& v, double epsilon);


 private:
  SSPIface const& ssp_;
  std::unique_ptr<hash_t> internal_v_;
  hash_t& v_;
  double epsilon_;
  bool solved_;
};

#endif  // PLANNER_VI_H
