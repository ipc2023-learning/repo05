#ifndef PLANNER_LRTDP_H
#define PLANNER_LRTDP_H

#include <cmath>
#include <iostream>

#include "planner_iface.h"

#include "../ext/mgpt/actions.h"
#include "../ssps/bellman.h"
#include "../ext/mgpt/hash.h"
#include "../ssps/ssp_utils.h"

class heuristic_t;


/*******************************************************************************
 *
 * planner LRTDP: Labeled RTDP
 *
 ******************************************************************************/

class PlannerLRTDP : public OptimalPlanner {
 public:
  // When use_as_replanner is true, then only one trial is performed before
  // choosing an action, instead of solving the problem optimally before
  // choosing an action (default behaviour).
  PlannerLRTDP(SSPIface const& ssp, heuristic_t& heur, double epsilon,
      size_t max_trace_size = 1000000, bool use_as_replanner = false,
      bool use_as_greedy_planner = false);

  // Using the given hash as value function, i.e., the values there will be used
  // as a lower bound on V* and all the updates on V will change the hash v.
  PlannerLRTDP(SSPIface const& ssp, hash_t& v, double epsilon,
      size_t max_trace_size = 1000000, bool use_as_replanner = false,
      bool use_as_greedy_planner = false)
    : OptimalPlanner(), ssp_(ssp), internal_v_(nullptr), v_(v), epsilon_(epsilon),
      max_trace_size_(max_trace_size), use_as_replanner_(use_as_replanner),
      use_as_greedy_planner_(use_as_greedy_planner)
  { }

  ~PlannerLRTDP() { }

  /*
   * Planner Interface
   */
  action_t const* decideAction(state_t const& s) override {
    if (isSolved(s)) {
      // The state is solved, so using returning the action that stays in the
      // policy envelop
      return Bellman::constGreedyAction(s, v_, ssp_, AcceptStateIfSolved(*this));
    }
    else if (use_as_replanner_) {
      // Since LRTDP is being used has a replanner, then we don't solve the
      // whole problem, just do one trial (check-solved included).
      trial(s);
      return Bellman::greedyAction(s, v_, ssp_);
    }
    else if (use_as_greedy_planner_) {
      // as above, but we don't even do a trial; it's assumed that
      // trainForUsecs() has been called for an appropriate length of time
      // beforehand (ideally long enough for convergence).
      return Bellman::greedyAction(s, v_, ssp_);
    }
    else {
      // s is NOT solved and LRTDP is not being used as a replanner, then we
      // solve the problem and return the action inside the "optimal"
      // (epsilon-consistent) envelop.
      solve(s);
      return Bellman::constGreedyAction(s, v_, ssp_, AcceptStateIfSolved(*this));
    }
  }

  action_t const* decideAction(state_t const& s) const override {
    return Bellman::constGreedyAction(s, v_, ssp_, AcceptStateIfSolved(*this));
  }

  void trainForUsecs(uint64_t max_time_usec) override {
    if (!runForUsec(max_time_usec, [this]() { solve(ssp_.s0()); })) {
      std::cout << "[LRTDP::trainForUsecs]: training finished before "
                << " convergence." << std::endl;
    }
  }

  void initRound() override { }
  void endRound() override { }
  void resetRoundStatistics() override { };
  void statistics(std::ostream &os, int level) const override;


  /*
   * Heuristic Planner Interface
   */
  double value(state_t const& s) const { return v_.value(s); }

  /*
   * Optimal Planner Interface
   */
  double optimalSolution() override {
    if (!isSolved(ssp_.s0())) { solve(ssp_.s0()); }
    return v_.value(ssp_.s0());
  }


  /*
   * LRTDP check-solved procedure. See paper for more details.
   *
   * This function is a public static template because it is also used by
   * Labeled SSiPP. It lives inside LRTDP to make clear that Labeled SSiPP is
   * using an unaltered version of check-solved. Both LRTDP and Labeled SSiPP
   * have their private version of check-solved that calls this template with
   * the appropriated functors and variables.
   */
  template<typename IsSolvedFunc, typename SetSolvedFunc> static
  bool checkSolved(SSPIface const& ssp, hash_t& v, IsSolvedFunc is_solved,
      SetSolvedFunc set_solved, hashEntry_t* node, double epsilon,
      ProbDistState& pr);


 private:
  /*
   * Methods to manipulate the flag solved.
   */
  // Mark a state s as solved
  void setSolved(state_t const& s) { solved_states_.insert(s); }

  bool isSolved(state_t const& s) const {
    return ssp_.isGoal(s) || (solved_states_.find(s) != solved_states_.end());
  }

  // Functor to accept only states that are marked as solved. This is used for
  // constGreedyAction to make sure the selected actions are inside the envelop
  // of the computed policy.
  class AcceptStateIfSolved {
   public:
    AcceptStateIfSolved(PlannerLRTDP const& parent) : parent_(parent) { }
    bool operator()(state_t const& s) {
      return parent_.isSolved(s);
    }
   private:
    PlannerLRTDP const& parent_;
  };


  /*
   * LRTDP Methods from the original paper
   */
  // Driver method to solve an SSP. It returns the total number of trials needed
  // for epsilon-convergence.
  size_t solve(state_t const& s) {
    size_t i = 0;
    while (!isSolved(s)) {
      gpt::incCounterAndCheckDeadlineEvery(i, 100);
      trial(s);
    }
    return i;
  }

  // LRTDP trial method.
  void trial(state_t const& s);

  // LRTDP check-solved procedure. See paper for more details.
  bool checkSolved(hashEntry_t* node) {
    return checkSolved(
        ssp_, v_,
        // isSolved Functor (as lambda)
        [this](state_t const& s)-> bool { return isSolved(s); },
        // setSolved Functor (as lambda)
        [this](state_t const& s) { setSolved(s); },
        node, epsilon_, pr_);
  }


  /*
   * Member variables
   */
  SSPIface const& ssp_;
  std::unique_ptr<hash_t> internal_v_;
  hash_t& v_;
  double epsilon_;
  size_t max_trace_size_;
  bool use_as_replanner_;
  bool use_as_greedy_planner_;
  HashsetState solved_states_;
  ProbDistState pr_;
};


// static
template<typename IsSolvedFunc, typename SetSolvedFunc>
bool PlannerLRTDP::checkSolved(SSPIface const& ssp, hash_t& v,
    IsSolvedFunc isSolved, SetSolvedFunc setSolved, hashEntry_t* node,
    double epsilon, ProbDistState& pr)
{

  // List of open and closed nodes
  std::deque<hashEntry_t*> open;
  std::deque<hashEntry_t*> closed;

  // Set of explored nodes so far, i.e., the union of open and closed
  // represented as a set
  std::set<hashEntry_t*,std::less<hashEntry_t*> > set_open_U_closed;

  bool rv = true;

  if (!isSolved(*(node->state()))) {
    open.push_back(node);
    set_open_U_closed.insert(node);
  }

  size_t loopCounter = 0;
  while (!open.empty()) {
    gpt::incCounterAndCheckDeadlineEvery(loopCounter, 100);
    node = open.front();
    open.pop_front();
    closed.push_back(node);

#ifdef DEBUG_TRACE
    std::cout << "CheckSolved cur: ";
    node->state()->full_print(std::cout, gpt::problem, false, true);
    std::cout << std::endl;
#endif

    if (isSolved(*(node->state()))) {
      continue;
    }

    // Checking the residual
    double min_q_value = 0;
    action_t const* a_greedy = nullptr;
    std::tie(a_greedy, min_q_value) = Bellman::greedyActionAndMinQValue(
                                                     *(node->state()), v, ssp);

    if (a_greedy == nullptr) {
      // This node is a dead-end. If it were a goal, it would have been caught
      // isSolved above (all goals are solved by default). But just to make sure
      // and to avoid potential code maintenance problems, the assert bellow was
      // added.
      DIE(!ssp.isGoal(*(node->state())),
          "Expected dead end but received a goal state", 171);
      // Since the node is a dead end, min_q_value == dead_end_value
      if (std::abs(min_q_value - node->value()) > epsilon) {
        rv = false;
        node->update(min_q_value);
      }
      // Since the node is a dead-end, let's mark it as solved
      setSolved(*(node->state()));
      continue;
    }
    else if (std::abs(min_q_value - node->value()) > epsilon) {
      // Residual is too large
      rv = false;
      continue;
    }

    // If a node reached this section then it's not solved, not a dead-end and
    // its residual is smaller than epsilon
    DIE(!isSolved(*(node->state())), "Not expecting a solved node", -1);
    DIE(a_greedy != nullptr, "Not expecting a dead-end or goal node", -1);
    DIE(std::abs(min_q_value - node->value()) <= epsilon,
        "Expecting a node with residual smaller than epsilon", -1);

    // Expanding the current state to check its descendants
    ssp.expand(*a_greedy, *node->state(), pr);
    for (auto const& ip : pr) {
      hashEntry_t* entry = v.get(ip.event());

      // if descend is not solved and not explored yet
      if (!isSolved(*(entry->state()))
          && set_open_U_closed.find(entry) == set_open_U_closed.end())
      {
        open.push_front(entry);
        set_open_U_closed.insert(entry);
      }
    }  // for each descend of the current state
  }  // while there are nodes in the open list

  if (rv) {
    // The given node is solved, therefore we should label all the nodes
    // explored as solved too
    while (!closed.empty()) {
      setSolved(*(closed.front()->state()));
      closed.pop_front();
    }
  }
  else {
    // The given node is not solved, therefore we update it and all the
    // explored nodes in order to bring them closer to be solved
    while (!closed.empty()) {
      Bellman::update(*closed.front()->state(), v, ssp);
      closed.pop_front();
    }
  }
  return rv;
}

#endif  // PLANNER_LRTDP_H
