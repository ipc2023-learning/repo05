#ifndef PLANNER_GREEDY_H
#define PLANNER_GREEDY_H

#include <iostream>

#include "planner_iface.h"

#include "../ssps/ssp_iface.h"
#include "../ext/mgpt/actions.h"
#include "../ssps/bellman.h"

class problem_t;
class hash_t;
class heuristic_t;


/*
 * Given a hash, representing V, follow the greedy action according to it
 */
class PlannerGreedy : public HeuristicPlanner {
 public:
  PlannerGreedy(SSPIface const& ssp, hash_t const& hash_table);
  ~PlannerGreedy() { }

  /*
   * Planner Interface
   */
  action_t const* decideAction(state_t const& s) override {
    return Bellman::constGreedyAction(s, v_, ssp_);
  }
  action_t const* decideAction(state_t const& s) const override {
    return Bellman::constGreedyAction(s, v_, ssp_);
  }
  void trainForUsecs(uint64_t) override { }
  void initRound() override { }
  void endRound() override { }
  void resetRoundStatistics() override { };
  void statistics(std::ostream& os, int level) const override { }

  /*
   * Heuristic Planner Interface
   */
  double value(state_t const& s) const override { return v_.value(s); }

 private:
  SSPIface const& ssp_;
  hash_t const& v_;
};

#endif // PLANNER_GREEDY_H
