#include "greedy.h"

PlannerGreedy::PlannerGreedy(SSPIface const& ssp, hash_t const& v)
  : HeuristicPlanner(), ssp_(ssp), v_(v)
{ }
