/* Trivial script to take a bunch of PDDL files, a state, and a heuristic name,
and return the heuristic evaluation for that state. */

#include <iostream>
#include <fstream>

#include "ext/mgpt/actions.h"
#include "ext/mgpt/domains.h"
#include "ext/mgpt/global.h"
#include "ext/mgpt/problems.h"
#include "ext/mgpt/states.h"

#include "heuristics/heuristic_factory.h"
#include "heuristics/action_heuristic.h"
#include "heuristics/lm_cut.h"

#include "planners/planner_factory.h"

#include "simulators/simulator.h"

#include "ssps/ppddl_adaptors.h"
#include "ssps/policy.h"

#include "utils/exceptions.h"
#include "utils/utils.h"


#ifndef CFLAGS_USED
#define CFLAGS_USED "not defined"
#endif

#ifndef __VERSION__
#define __VERSION__ "UNKNOWN"
#endif

#ifndef GIT_HASH
#define GIT_HASH "UNKNOWN"
#endif

#ifndef HOSTNAME
#define HOSTNAME "UNKNOWN"
#endif


int main(int argc, char** argv) {
  // init stuff first (probably not necessary for heuristics)
  // gpt::start_time = get_time_usec();
  // ushort_t seed[3];
  // gpt::seed = time(NULL);
  // seed[0] = seed[1] = seed[2] = gpt::seed;
  // srand48(gpt::seed);
  // seed48(seed);

  // start by parsing args
  char USAGE[] = "USAGE: heur_eval pddl_file [pddl_file ...] problem heuristic state\n";
  if (argc < 5) {
    std::cout << USAGE;
    return -1;
  }
  for (int i = 1; i <= argc - 4; i++) {
    std::cout << "Reading '" << argv[i] << "'" << std::endl;
    if (!readPDDLFile(argv[i])) {
      std::cout << "couldn't read parse the above file" << std::endl;
      return -1;
    }
  }
  char *problem_name = argv[argc-3];
  char *heuristic_name = argv[argc-2];
  char *state_string = argv[argc-1];

  std::cout << "Loading problem '" << problem_name << "'" << std::endl;
  problem_t *problem = (problem_t*)problem_t::find(problem_name);
  if (!problem) {
    std::cout << "Couldn't load problem, dying" << std::endl;
    return -1;
  }
  gpt::problem = problem;
  std::cout << "Loaded problem " << problem->name() << std::endl;

  // instantiate actions
  try {
    problem->instantiate_actions();
    problem->flatten();
    state_t::initialize(*problem);
  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
    return -1;
  }

  // initialize heuristic and execute
  try {
    SSPfromPPDDL ssp(*problem);

    std::cout << "Loading lm-cut" << std::endl;
    std::shared_ptr<heuristic_t> lm_cut_heur_p = createHeuristic(ssp, "lm-cut");
    if (lm_cut_heur_p == nullptr) {
      std::cout << "Couldn't load lm-cut";
      return -1;
    }
    std::shared_ptr<LMCutHeuristic> lm_cut_p = std::dynamic_pointer_cast<LMCutHeuristic>(lm_cut_heur_p);
    if (lm_cut_p == nullptr) {
      std::cout << "Couldn't cast lm-cut";
      return -1;
    }

    std::cout << "Loading heuristic '" << heuristic_name << "'" << std::endl;
    std::shared_ptr<heuristic_t> heur_p = createHeuristic(ssp, heuristic_name);
    if (heur_p == nullptr) {
      std::cout << "No heuristics loaded :(" << std::endl;
      return -1;
    }
    SuccessorEvaluator evaluator(heur_p);

    // may or may not be necessary
    problem->no_more_atoms();
    // state_string should be "pred1 arg1,pred2 arg1 arg2" (etc.)
    state_t state = problem->get_intermediate_state(state_string);
    std::cout << "State: " << state << std::endl;

    double val = evaluator.state_value(state);
    std::cout << "Value of state: " << val << std::endl;

    CutResult cut_res = lm_cut_p->valueAndCuts(state);
    std::cout << "lm-cut value: " << cut_res.value << std::endl;
    std::cout << "lm-cut disjunctive landmarks:" << std::endl;
    for (const std::set<const action_t*> &cut_set : cut_res.cuts) {
      if (!cut_set.size()) {
        std::cout << "\t(empty)" << std::endl;
      } else {
        std::cout << "\t";
        for (const action_t* action : cut_set) {
          std::cout << action->name() << " ";
        }
        std::cout << std::endl;
      }
    }

    // we can also evaluate enabled actions
    std::cout << "Heuristic information for applicable actions:" << std::endl;
    int actions_seen = 0;
    for (auto const &action : ssp.applicableActions(state)) {
      std::cout << "Action ";
      action.print(std::cout);
      std::cout << ":" << std::endl;
      for (auto succ : evaluator.succ_iter(state, action)) {
        // use succ.{state,probability,value}
        std::cout << "\tState: " << succ.state << std::endl;
        std::cout << "\t\tProbability: " << succ.probability << std::endl;
        std::cout << "\t\tValue: " << succ.value << std::endl;
      }
      actions_seen++;
    }
    std::cout << "Done, processed " << actions_seen << " actions" << std::endl;

    std::string table_dest = "/tmp/ssipp-hash-table-from-heur-eval";
    std::cout << "Dumping file to '" << table_dest << "'" << std::endl;
    evaluator.dump_table(table_dest);
  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
    return -1;
  }

  return 0;
}
