#ifndef PLANNER_RANDOM_H
#define PLANNER_RANDOM_H

#include <iostream>

#include "planner_iface.h"

#include "../ssps/ssp_iface.h"
#include "../ssps/ssp_utils.h"


class action_t;

/*******************************************************************************
 *
 * planner Random
 *
 ******************************************************************************/
class PlannerRandom : public Planner {
 public:
  PlannerRandom(SSPIface const& ssp) : Planner(), ssp_(ssp) { }
  /*
   * Planner Interface
   */
  action_t const* decideAction(state_t const& s) override {
    return randomAction(ssp_, s);
  }
  action_t const* decideAction(state_t const& s) const override {
    return randomAction(ssp_, s);
  }
  void trainForUsecs(uint64_t) override { }
  void initRound() override { }
  void endRound() override { }
  void resetRoundStatistics() override { };
  void statistics(std::ostream& os, int level) const override;

 private:
  SSPIface const& ssp_;
};

#endif // PLANNER_RANDOM_H
