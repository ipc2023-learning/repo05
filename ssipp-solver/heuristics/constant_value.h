#ifndef HEURISTICS_CONSTANT_VALUE_H
#define HEURISTICS_CONSTANT_VALUE_H

#include "heuristic_iface.h"

#include "../ext/mgpt/global.h"
#include "../ext/mgpt/rational.h"
#include "../ext/mgpt/states.h"
#include "../utils/die.h"
#include "../ssps/ssp_iface.h"

#include <iostream>


/*******************************************************************************
 *
 * constant value heuristic
 *
 ******************************************************************************/
class ConstantValueHeuristic : public heuristic_t {
 public:
  ConstantValueHeuristic(double value)
    : heuristic_t("Constant-Value-H"), value_(value)
  { }
  ~ConstantValueHeuristic() { }

  /*
   * heuristic_t interface
   */
  double computeValue(state_t const& s) { return value_; }

 private:
  double value_;
};




/*******************************************************************************
 *
 * zero heuristic
 *
 ******************************************************************************/
class ZeroHeuristic : public ConstantValueHeuristic {
 public:
  ZeroHeuristic() : ConstantValueHeuristic(0.0) {
    name_ = "Zero Heuristic";
  }
};



/*******************************************************************************
 *
 * constant value heuristic for all states except goal, in which 0 is returned
 *
 ******************************************************************************/
class SmartConstantValueHeuristic : public heuristic_t {
 public:
  SmartConstantValueHeuristic(SSPIface const& ssp, double value)
    : heuristic_t("Smart Cons. Value H"), ssp_(ssp), value_(value)
  { }
  ~SmartConstantValueHeuristic() { }

  /*
   * heuristic_t interface
   */
  double computeValue(state_t const& s) {
    if (ssp_.isGoal(s))
      return ssp_.terminalCost(s).double_value();
    else if (!ssp_.hasApplicableActions(s))
      return gpt::dead_end_value.double_value();
    else
      return value_;
  }


 private:
  SSPIface const& ssp_;
  double value_;
};



/*******************************************************************************
 *
 * smart zero heuristic
 *
 ******************************************************************************/
class SmartZeroHeuristic : public SmartConstantValueHeuristic {
 public:
  SmartZeroHeuristic(SSPIface const& ssp) : SmartConstantValueHeuristic(ssp, 0)
  {
    name_ = "SmartZero H";
  }
};


#endif // HEURISTICS_CONSTANT_VALUE_H
