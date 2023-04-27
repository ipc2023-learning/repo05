#ifndef PLANNERS_LABELED_SSIPP_H
#define PLANNERS_LABELED_SSIPP_H

#include <iostream>

#include "planner_iface.h"
#include "ssipp.h"
#include "lrtdp.h"  // checkSolved template

#include "../ext/mgpt/actions.h"
#include "../ext/mgpt/hash.h"
#include "../ssps/prob_dist_state.h"
#include "../ssps/ssp_adaptor.h"
#include "../ssps/ssp_iface.h"


/*
 * Implementation of LabeledSSiPP: Labeled Short-Sighted Probabilistic Planner
 * as defined in
 * http://felipe.trevizan.org/papers/trevizan14:depth.pdf
 *
 * Bibtex entry:
   @article{trevizan14:depth,
     author = {Trevizan, F. and Veloso, M.},
     title = {{D}epth-based {S}hort-sighted {S}tochastic {S}hortest {P}ath {P}roblems},
     journal = {Artificial Intelligence},
     year = {2014},
     volume = {216},
     pages = {179 - 205},
     url = {felipe.trevizan.org/papers/trevizan14:depth.pdf},
   }
 *
 */

class heuristic_t;


class PlannerLabeledSSiPP : public OptimalPlanner {
 public:
  PlannerLabeledSSiPP(SSPIface const& ssp, heuristic_t &heur, double epsilon,
      std::string flags, size_t max_trace_size = 1000000,
      bool use_as_replanner = false)
    : OptimalPlanner(), ssp_(ssp),
      internal_v_(new hash_t(gpt::initial_hash_size, heur)), v_(*internal_v_),
      epsilon_(epsilon), max_trace_size_(max_trace_size),
      use_as_replanner_(use_as_replanner), solved_states_(),
      unsolved_sub_ssp_(ssp, "Unsolved SSP - LSSiPP", ssp.s0(),
                        AcceptStateIfSolved(*this),
                        SameApplicableActions(),
                        SameActionCost(),
                        TerminalCostFromV(v_),
                        OnDemandReachableStates()),
      ssipp_(unsolved_sub_ssp_, v_, epsilon, flags)
  {
    std::cout << "[Labeled SSiPP] SSiPP and unsolved_sub_ssp_ constructed"
              << std::endl;
  }


  /*
   * Planner Interface
   */
  action_t const* decideAction(state_t const& s) override {
//    std::cout << "[LSSiPP]: decideAction s = " << s
//              << " -- V(s) = " << v_.value(s) << std::endl;
    if (isSolved(s)) {
//      std::cout << "  [LSSiPP]: s is already solved. Using constGreedyAction\n";
      // No need to push a solved state to the visited_states_ stack since it
      // is already labeled as solved
      return Bellman::constGreedyAction(s, v_, ssp_, AcceptStateIfSolved(*this));
    }
    else if (use_as_replanner_) {
      // Using LSSiPP as replanner, thus we save the current state to
      // checkSolve it and forward the decideAction to SSiPP
      visited_states_.push(s);
      action_t const* a = ssipp_.decideAction(s);
//      std::cout << "  [LSSiPP]: return SSiPP action a = "
//                << (a ? a->name() : "NULL") << std::endl;
      return a;
    }
    else {
      // LSSiPP is not used as a replanner, i.e., it is being used as LRTDP,
      // therefore we solve the SSP from s and return the optimal action for s.
      // Notice that solve will simulate rounds and will make usage of the
      // initRound and endRound methods. To avoid unintended interaction, the
      // asserts bellow were added to check that no state was mark as visited
      // before calling solve and no state is left in the visited stack. If
      // there is an unintended interaction, then move to the trial-based
      // approach (trial code is commented out in the .cc file).
      FANCY_DIE_IF(!visited_states_.empty(), 111, "Unintended interaction "
                    "between Simulator and LocalSimulator might happen");
      solve(s);
      FANCY_DIE_IF(!visited_states_.empty(), 111, "Unintended interaction "
                    "between Simulator and LocalSimulator might happen");
      return Bellman::constGreedyAction(s, v_, ssp_, AcceptStateIfSolved(*this));
    }
  }


  action_t const* decideAction(state_t const& s) const override {
    return Bellman::constGreedyAction(s, v_, ssp_, AcceptStateIfSolved(*this));
  }


  void trainForUsecs(uint64_t max_time_usec) override {
    if (!runForUsec(max_time_usec, [this]() { solve(ssp_.s0()); })) {
      std::cout << "[LSSiPP::trainForUsecs]: training finished before "
                << " convergence." << std::endl;
    }
  }

  // Function called by the Planner interface every time a new round of the
  // evaluation starts.
  void initRound() override {
    // Equivalent to the missing stack.clear();
    visited_states_ = std::stack<state_t>();
    ssipp_.initRound();
    assert(visited_states_.empty());
  }


  /*
   * Function called by the Planner interface every time a new round starts.
   *
   * At the end of each round, which for LRTDP means a trial, we try to mark the
   * states visited as solved.
   */
  void endRound() override {
    ssipp_.endRound();
//    std::cout << "[LSSiPP]: endRound. Total visited = "
//              << visited_states_.size() << std::endl;
    size_t loopCounter = 0;
    while (!visited_states_.empty()) {
      gpt::incCounterAndCheckDeadlineEvery(loopCounter, 100);
      // try labeling
      state_t const& s = visited_states_.top();
      if (!isSolved(s) && !checkSolved(s)) {
        break;
      }
      visited_states_.pop();
    }
    // Equivalent to the missing stack.clear();
    visited_states_ = std::stack<state_t>();
  }


  void resetRoundStatistics() { };
  void statistics(std::ostream& os, int level) const { }

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
   * Labeled SSiPP Methods
   */
  // Show the option of labeled ssipp.
  void usage(std::ostream& os) const;


 private:
  /*
   * Methods to manipulate the flag solved.
   *
   * For this planner, a bit mask in the hash table bits was employed. This is
   * not the most robust solution. See other comment in the enum.
   *
   * FUTURE: use a template hash table with an extra field for the solved flag
   */
  // Mark a state s as solved
  void setSolved(state_t const& s) { solved_states_.insert(s); }

  bool isSolved(state_t const& s) const {
    if (solved_states_.find(s) != solved_states_.end() || ssp_.isGoal(s)) {
      return true;
    }
    return false;
  }

  // Functor to accept only states that are marked as solved. This is used for
  // constGreedyAction to make sure the selected actions are inside the envelop
  // of the computed policy.
  class AcceptStateIfSolved {
   public:
    AcceptStateIfSolved(PlannerLabeledSSiPP const& parent) : parent_(parent) {}
    bool operator()(state_t const& s) const {
      return parent_.isSolved(s);
    }
   private:
    PlannerLabeledSSiPP const& parent_;
  };


  /*
   * LRTDP-like methods. All these methods are really similar to the LRTDP ones
   * and any improvements there can be applied here too.
   */
  // Driver method
  size_t solve(state_t const& s) {
    // This method should only be called if the optimal solution is desired,
    // therefore use_as_replanner_ must be false. This is necessary to be able
    // to use the decideAction method in the LocalSimulator instead of a trial
    // method as in LRTDP.
    assert(!use_as_replanner_);
    // Rounds will be simulated internally as a replanner until s is marked as
    // solved
    use_as_replanner_ = true;
    LocalSimulator simulator(unsolved_sub_ssp_);
    size_t i = 0;
    while (!isSolved(s)) {
      gpt::incCounterAndCheckDeadlineEvery(i, 100);
//      std::cout << "===> Simulating the " << i
//                << "-th round for the unsolved_sub_ssp_: V(s0) = "
//                << v_.value(ssp_.s0()) << "  -- |solved| = "
//                << solved_states_.size() << std::endl;
      // This takes care of the trial and then checkSolved approach of LRTDP
      // because: initRound will setup LSSiPP and SSiPP, decideAction will
      // accumulate the visited states (as would trial), and endRound will call
      // checkSolved.
//      RoundSummary const summary =
      simulator.simulateRoundFrom(&s, this, max_trace_size_);
//      std::cout << "[LSSiPP]: Trial ended. Reason " << summary.exitStatus
//                << "  --  # of actions applied: " << summary.totalActionsApplied
//                << std::endl;

      /*
       * Equivalent to the deprecated trial approach:
      ssipp_.initRound();
      trial(ssp_.s0());
      ssipp_.endRound();
       */
    }
    // By assumption use_as_replanner_ was false, so moving it back to false.
    // Although this shouldn't matter now that the optimal solution from s is
    // already computed.
    use_as_replanner_ = false;
    return i;
  }

  // the trial method is not necessary because a simulator is used to perform
  // the trial in the unsolved_sub_ssp_. For testing or debug purposes, the
  // trial method is still in the .cc but commented.
  void trial(state_t const& s);

  // Using the Check-Solved from LRTDP
  bool checkSolved(state_t const& s) {
    return PlannerLRTDP::checkSolved(
        ssp_, v_,
        // isSolved Functor (as lambda)
        [this](state_t const& s)-> bool { return isSolved(s); },
        // setSolved Functor (as lambda)
        [this](state_t const& s) { setSolved(s); },
        v_.get(s), epsilon_, pr_);
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
  HashsetState solved_states_;
  // SSP in which the goals are the original goal \union the states proved to
  // have converged (solve_states_). Since solved_states_ changes during the
  // execution of Labeled-SSiPP and this adaptor is pass to SSiPP, then the
  // short-sighted SSPs generated by SSiPP will be able to take advantage of the
  // new goals.
  SSPAdaptor<AcceptStateIfSolved,
             SameApplicableActions,
             SameActionCost,
             TerminalCostFromV,
             OnDemandReachableStates> unsolved_sub_ssp_;

  PlannerSSiPP ssipp_;
  std::stack<state_t> visited_states_;
  ProbDistState pr_;
};
#endif  // PLANNERS_LABELED_SSIPP_H
