#include <assert.h>
#include <math.h>
#include <set>
#include <stdlib.h>

#include "planner_factory.h"

#include "greedy.h"
#include "labeled_ssipp.h"
#include "lrtdp.h"
#include "random.h"
#include "ssipp.h"
#include "vi.h"

#include "../ext/mgpt/global.h"
#include "../ext/mgpt/hash.h"
#include "../simulators/simulator.h"
#include "../ext/mgpt/states.h"

Planner* createPlanner(SSPIface const& ssp, std::string const& name,
                       heuristic_t& heuristic) {
  if (!strcasecmp(name.c_str(), "random")) {
    return new PlannerRandom(ssp);
  }
  else if (!strcasecmp(name.c_str(), "vi")) {
    return new PlannerVI(ssp, heuristic, gpt::epsilon);
  }
  else if (!strcasecmp(name.c_str(), "glrtdp"))  {
    // Like LRTDP, but doesn't plan during decideAction(). Instead, it executes
    // greedily (hence "g" in "glrtdp"), assuming that all planning has been
    // done with trainForUsecs calls.
    return new PlannerLRTDP(ssp, heuristic, gpt::epsilon,
                            MAX_TRACE_SIZE, false, true);
  }
  else if (!strcasecmp(name.c_str(), "lrtdp"))  {
    return new PlannerLRTDP(ssp, heuristic, gpt::epsilon,
                            MAX_TRACE_SIZE, false);
  }
  else if (!strncasecmp(name.c_str(), "labeledssipp:", 12)) {
    return new PlannerLabeledSSiPP(ssp, heuristic, gpt::epsilon,
                                   name.substr(12));
  }
  else if (!strncasecmp(name.c_str(), "ssipp", 5)) {
     return new PlannerSSiPP(ssp, heuristic, gpt::epsilon,
                                name.substr(5));
  }
  std::cout << "[Warning] No SSP planner named '" << name << "'" << std::endl;
  return nullptr;
}
