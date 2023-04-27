#ifndef PLANNER_H
#define PLANNER_H

#include <iostream>

#include "../ext/mgpt/actions.h"
#include "../ssps/policy.h"
#include "../simulators/simulator.h"
#include "../ssps/ssp_iface.h"

class hash_t;
class heuristic_t;


/*******************************************************************************
 *
 * planner (abstract class)
 *
 ******************************************************************************/

class Planner {
 public:
  virtual ~Planner() { }

  /****************************************************************************
   * Main Planner Methods
   ***************************************************************************/
  // Given a state, decide the next action to be applied.
  virtual action_t const* decideAction(state_t const& s) = 0;

  // This method if for evaluation purposes only. It should be used after
  // "training" the planner for a while. Try not to do a lot of computation
  // on it since it would be kind of cheating.
  virtual action_t const* decideAction(state_t const& s) const = 0;

  // Train/Warm-up the planner with a time deadline of usec microseconds
  virtual void trainForUsecs(uint64_t max_time_usec) = 0;

  /****************************************************************************
   * Round related functions
   ***************************************************************************/
  virtual void initRound() = 0;
  virtual void endRound()  = 0;
  virtual void resetRoundStatistics() = 0;

  // Method to print out information that might be useful once the planner is
  // deallocated
  virtual void statistics(std::ostream& os, int level) const = 0;
};


/*
 * FWT: HeuristicPlanner does not declare a hash because, in the future, each
 * algorithm will be allowed to have its own hash type, so it can hash more than
 * just V(s) (e.g., solved flag and other things).
 */
class HeuristicPlanner : public Planner {
 public:
  virtual ~HeuristicPlanner() { }

  // Returns the planners estimate of V*(s). This should be straightforward
  // since this is a Heuristic Planner and it should carry such estimate.
  virtual double value(state_t const& s) const = 0;
};


/*******************************************************************************
 *
 * OptimalPlanner (abstract class)
 *
 ******************************************************************************/

// FWT: Ideally, OptimalPlanner would derive from Planner, but this would create
// a diamond shape inheritance, i.e., extra complications for nothing.
class OptimalPlanner : public HeuristicPlanner {
 public:
  virtual ~OptimalPlanner() { }

  /*
   * Computes the "optimal solution" (epsilon consistent solution) for the
   * current SSP and returns V*(s0). No time cutoff is set, therefore it is good
   * to either manually set one or wrap this call with runForUsec (utils.h)
   */
  virtual double optimalSolution() = 0;
};

#endif // PLANNER_H
