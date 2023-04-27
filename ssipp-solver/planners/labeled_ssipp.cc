#include "labeled_ssipp.h"
#include "ssipp.h"

#include "../ssps/bellman.h"
#include "../utils/die.h"
#include "../ext/mgpt/global.h"
#include "../ext/mgpt/hash.h"
#include "../ssps/prob_dist_state.h"
#include "../ext/mgpt/states.h"

void PlannerLabeledSSiPP::usage(std::ostream& os) const {
  os
    << "ERROR! LabeledSSiPP takes at least 2 parameters:\n"
    << "\tlabeledssipp:[epsilon]:<search_alg>:<FACTORY-PARAMS>*\n"
    << "[epsilon] = epsilon used for declaring convergence inside a \n"
    << "            short-sighted SSP. If omitted, the global epsilon is used.\n"
    << "<search_alg> = {lrtdp, vi}\n\n"
    << "<FACTORY-PARAMS>:" << std::endl;
  ShortSightedSSPFactory::usage(os);
}
