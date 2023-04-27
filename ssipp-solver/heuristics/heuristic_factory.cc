#include <iostream>
#include <memory>
#include <sstream>
#include <string.h>

#include "heuristic_factory.h"
#include "constant_value.h"
#include "h_add.h"
#include "h_max.h"
#include "lm_cut.h"

#include "../utils/die.h"
#include "../ext/mgpt/global.h"

/*******************************************************************************
 *
 * heuristic_t
 *
 ******************************************************************************/
// static
extern "C" char* strtok_r(char*, const char*, char**) throw();

std::shared_ptr<heuristic_t> createHeuristic(SSPIface const& ssp,
                                             std::string const& heuristic)
{
  std::shared_ptr<heuristic_t> heur = nullptr;
  char *strstack = strdup(heuristic.c_str());
  char *lptr, *ptr = strtok_r(strstack, "| ", &lptr);
  // FIXME: there is no point in supporting multiple heuristic names now that
  // interface has changed. Can probably throw out the loop and strtok-ing.
  while (ptr != NULL) {
    /*
     * heuristic_t, i.e., SSPIface based heuristics
     */
    if (!strcasecmp(ptr, "simpleZero")) {
      heur = std::make_shared<ZeroHeuristic>();
    }
    else if (!strcasecmp(ptr, "smartZero")) {
      heur = std::make_shared<SmartZeroHeuristic>(ssp);
    }
    else {
      /*
       * FactoredHeuristic or descendants
       */
      // TODO(fwt): refactor the code to avoid this
      problem_t const& problem = *gpt::problem;

      if (!strcasecmp(ptr, "lm-cut")) {
        heur = std::make_shared<LMCutHeuristic>(problem);
      }
      else if (!strcasecmp(ptr, "h-add")) {
        heur = std::make_shared<HAddAllOutcomesDet>(problem, ACTION_COST);
      }
      else if (!strcasecmp(ptr, "h-max")) {
        heur = std::make_shared<HMaxAllOutcomesDet>(problem, ACTION_COST);
      }
      else {
        std::cerr << "ERROR! undefined heuristic `" << ptr << "'" << std::endl;
        exit(-1);
      }
    }  // case in which the heuristic is a FactoredHeuristic
    ptr = strtok_r(NULL, "| ", &lptr);
  }
  free(strstack);
  return heur;
}
