#include <iostream>
#include <memory>

#include "ext/mgpt/actions.h"
#include "ext/mgpt/domains.h"
#include "ext/mgpt/global.h"
#include "ext/mgpt/problems.h"
#include "ext/mgpt/states.h"

#include "heuristics/heuristic_factory.h"

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


/*
 * readArguments: read the arguments from the command line and set the
 * appropriated flags in the scope gpt::
 */
bool readArguments(int& argc, char**& argv, std::vector<char*>& remaining_args);

/*
 * printUsageGlobal: display the flags used for setting the variables in
 * the scope gpt::
 */
void printUsageGlobal(std::ostream& os);

/*
 * printUsageLocal: display the flags of a given binary. This method should be
 * implemented for each file containing a main()
 */
void printUsageLocal(std::ostream& os);


void printBanner(std::ostream& os) {
  os << "This is SSiPP (v 0.1)" << std::endl
     << "(Developed by F. Trevizan (felipe@trevizan.org)\n"
     << "GIT HASH: " << GIT_HASH << " @ " << HOSTNAME << "\n"
     << "COMPILED ON: " << __DATE__ << " at " << __TIME__ << "\n"
     << "COMPILER: " << __VERSION__ << "\n"
     << "USED CFLAGS: " << CFLAGS_USED << "\n"
     << std::endl << std::endl;
}


void printUsageLocal(std::ostream& os) {
  os << "Usage: solver_ssp <option>* "
     << "[<domain-and-problem-file> | <domain-file> <problem-file> [problem-name]]"
     << std::endl << std::endl;
}


bool readArguments(int& argc, char**& argv, std::vector<char*>& remaining_args)
{
  if (argc == 1) goto usage;
  ++argv;
  --argc;
  while (argc > 0 && argv[0][0] == '-') {
    if (strlen(argv[0]) == 2) {
      // Single letter argument
      switch (argv[0][1]) {
        case 'a':
          gpt::hash_all = (gpt::hash_all?false:true);
          ++argv;
          --argc;
          break;
        case 'A':
          gpt::show_applied_policy = true;
          argv += 1;
          argc -= 1;
          break;
        case 'b':
          gpt::tmp_dir = argv[1];
          argv += 2;
          argc -= 2;
          break;
        case 'c':
          gpt::cutoff = atoi( argv[1] );
          argv += 2;
          argc -= 2;
          break;
        case 'C':
          if (!strncasecmp(argv[1], "ignore", 6)) {
            gpt::use_action_cost = false;
            gpt::use_state_cost = false;
            gpt::normalize_action_cost = false;
          } else if (!strncasecmp(argv[1], "action", 6)) {
            gpt::use_action_cost = true;
            gpt::use_state_cost = false;
            gpt::normalize_action_cost = false;
            if (!strcasecmp(argv[1], "action:normalize"))
              gpt::normalize_action_cost = true;
            else if (strlen(argv[1]) > 6) {
              std::cout << "Option '-C " << argv[1]
                << "' not recognized" << std::endl;
              exit(-1);
            }
          } else if (!strncasecmp(argv[1], "full", 4)) {
            gpt::use_action_cost = true;
            gpt::use_state_cost = true;
            gpt::normalize_action_cost = false;
            if (!strcasecmp(argv[1], "full:normalize"))
              gpt::normalize_action_cost = true;
            else if (strlen(argv[1]) > 4) {
              std::cout << "Option '-C " << argv[1]
                << "' not recognized" << std::endl;
              exit(-1);
            }
          } else {
            std::cout << "The option '-C " << argv[1]
              << "' was not recognized..."
              << std::endl;
            exit(-1);
          }
          argv += 2;
          argc -= 2;
          break;
        case 'd':
          gpt::dead_end_value = (size_t) gpt::heuristic_weight * atoi(argv[1]);
          argv += 2;
          argc -= 2;
          break;
        case 'D':
          gpt::max_time_in_secs = atoi(argv[1]);

          if (gpt::max_time_in_secs > 0) {
            struct rlimit rl = { (rlim_t) gpt::max_time_in_secs,
                                 (rlim_t) gpt::max_time_in_secs };
            if (setrlimit(RLIMIT_CPU, &rl)) {
              std::cout << "ERROR trying to set the CPU time limit\n";
              std::cerr << "ERROR trying to set the CPU time limit\n";
            } else
              std::cout << "CPU-TIME LIMIT: " << gpt::max_time_in_secs
                << " secs" << std::endl;
          }
          argv += 2;
          argc -= 2;
          break;
        case 'e':
          gpt::epsilon = atof( argv[1] );
          argv += 2;
          argc -= 2;
          break;
        case 'h':
          gpt::default_hp = false;
          //          !strncmp(gpt::heuristic,argv[1],9) && (strlen(argv[1])==19) && !strcmp(&gpt::heuristic[10],&argv[1][10]);
          gpt::heuristic = argv[1];
          argv += 2;
          argc -= 2;
          break;
        case 'i':
          gpt::initial_hash_size = atoi( argv[1] );
          argv += 2;
          argc -= 2;
          break;
        case 'm':
          gpt::max_database_size = atoi( argv[1] );
          argv += 2;
          argc -= 2;
          break;
        case 'M':
          gpt::max_turn = (uint32_t) atoi(argv[1]);
          argv += 2;
          argc -= 2;
          break;
        case 'p':
          gpt::default_hp = false;
          gpt::algorithm = argv[1];
          argv += 2;
          argc -= 2;
          break;
        case 'P':
          gpt::show_computed_policy = true;
          argv += 1;
          argc -= 1;
          break;
        case 'r':
          gpt::seed = atoi(argv[1]);
          argv += 2;
          argc -= 2;
          break;
        case 'R':
          gpt::total_execution_rounds = (uint32_t) atoi(argv[1]);
          DIE(gpt::stopCriterion == NONE,
              "Another stop criterion was specified before", -1);
          gpt::stopCriterion = NUM_ROUNDS;
          argv += 2;
          argc -= 2;
          break;
        case 's':
          gpt::simulations = atoi( argv[1] );
          argv += 2;
          argc -= 2;
          break;
        case 't':
          gpt::print_turn_details = true;
          argv += 1;
          argc -= 1;
          break;
        case 'v':
          gpt::verbosity = atoi( argv[1] );
          argv += 2;
          argc -= 2;
          break;
        case 'w':
          gpt::heuristic_weight = atof(argv[1]);
          gpt::dead_end_value = gpt::dead_end_value * (size_t) gpt::heuristic_weight;
          argv += 2;
          argc -= 2;
          break;
        case 'x':
          gpt::xtra = atoi( argv[1] );
          argv += 2;
          argc -= 2;
          break;
        case 'X':
          gpt::execution_simulator = argv[1];
          argv += 2;
          argc -= 2;
          break;
        case 'z':
          gpt::domain_analysis = (gpt::domain_analysis?false:true);
          ++argv;
          --argc;
          break;
        default:
          std::cout << "Flag not recognized: " << argv[0] << std::endl;
          goto usage;
      }  // end switch
    }  // end if strlen(argv[0]) == 2
    else if (strlen(argv[0]) > 2 && argv[0][1] == '-') {
      // Long option, i.e., --something
      if (!strcasecmp(*argv, "--legend")) {
        gpt::legend_file = std::string(argv[1]);
        argv += 2;
        argc -= 2;
      } else if (!strcasecmp(*argv, "--ff_path")) {
        gpt::ff_path = std::string(argv[1]);
        argv += 2;
        argc -= 2;
      } else if (!strcasecmp(*argv, "--lama_path")) {
        gpt::lama_path = std::string(argv[1]);
        argv += 2;
        argc -= 2;
      } else if (!strncasecmp(*argv, "--follow_ff:", 12)) {
        gpt::followable_ffreplan_param = std::string(argv[0]);
        argv += 1;
        argc -= 1;
      } else if (!strcasecmp(*argv, "--suppress_round_info")) {
        gpt::suppress_round_info = true;
        argv += 1;
        argc -= 1;
      } else if (!strcasecmp(*argv, "--ignore_effects_with_prob_less_than")) {
        gpt::ignore_effects_with_prob_less_than = atof(argv[1]);
        argv += 2;
        argc -= 2;
      } else if (!strcasecmp(*argv, "--conv_s0")) {
        DIE(gpt::stopCriterion == NONE,
            "Another stop criterion was specified before", -1);
        gpt::stopCriterion = CONV_S0;
        argv += 1;
        argc -= 1;
      } else if (!strcasecmp(*argv, "--conv_cost")) {
        DIE(gpt::stopCriterion == NONE,
            "Another stop criterion was specified before", -1);
        gpt::given_value_for_vStar_s0 = atof(argv[1]);
        gpt::stopCriterion = CONV_COST;
        argv += 2;
        argc -= 2;
      } else if (!strcasecmp(*argv, "--max_rss_kb")) {
        gpt::max_rss_kb = atoi(argv[1]);
        argv += 2;
        argc -= 2;
      } else if (!strcasecmp(*argv, "--max_cpu_time_sec")) {
        gpt::max_cpu_sys_time_usec = 1000000 * (uint64_t) atoi(argv[1]);
        argv += 2;
        argc -= 2;
      } else if (!strcasecmp(*argv, "--no_rand_action_ordering")) {
        gpt::randomize_actionsT_order = false;
        argv += 1;
        argc -= 1;
      } else if (!strcasecmp(*argv, "--train_for")) {
        gpt::train_for_usecs = 1000000 * (uint64_t) atoi(argv[1]);
        argv += 2;
        argc -= 2;
      } else {
        std::cout << "Flag not recognized: " << argv[0] << std::endl;
        goto usage;
      }
    }  // end if strlen(argv[0]) > 2
    else {
      goto usage;
    }
  }

  if (argc == 0 || argc > 3) {
usage:
    printUsageGlobal(std::cout);
    if (argc > 0) {
      std::cerr << "\n\nERROR! Parameter '" << argv[0] << "' not recognized!"
                << " Didn't try the parameters after that one too...\n" << std::endl;
    }
    else {
      std::cerr << "\n\nERROR! Not enough parameters" << std::endl;
    }
    exit(7);
  }
  else {
    for (int i = 0; i < argc; i++) {
      remaining_args.push_back(argv[i]);
    }
    return true;
  }
}

void printUsageGlobal(std::ostream& os) {
  printBanner(os);
  printUsageLocal(os);
  os << std::endl << std::endl
     << "==================================================================================="
     << std::endl
     << "GLOBAL Options:" << std::endl << std::endl
     << "  -d <dead-end-value>     (default = " << gpt::dead_end_value << ", see options bellow)" << std::endl
     << "  -e <epsilon>            (default = " << gpt::epsilon << ")" << std::endl
     << "  -h <heuristic-stack>    (default = \"smartZero\")" << std::endl
     << "  -M <max_turn>           (max number of actions applicable in each round. Default = "
     << gpt::max_turn << ")" << std::endl
     << "  -p <planner>            (default = lrtdp)" << std::endl
     << "  -r <random-seed>        (default = random)" << std::endl
     << "  -R <rounds>             (default = 30)" << std::endl
     << "  -t                      (Turn on the output of each turn detail)" << std::endl
     << "  --max_rss_kb <K>        (Maximum resident memory allowed in KB. Default = unlimited)" << std::endl
     << "  --max_cpu_time_sec <T>  (Maximum CPU time allowed in seconds. Default = unlimited)" << std::endl
     << "  --suppress_round_info" << std::endl
     << "  <planner>         := random | vi | lrtdp | ssipp | labeledssipp"
     << std::endl
     << "  <heuristic-stack> := <heuristic-stack> '|' <heuristic> | <heuristic>" << std::endl
     << "  <heuristic>       := simpleZero | smartZero | h-max | h-add | lm-cut " << std::endl
     << std::endl
     << "A stop criterion must be provided, that is, one of the following flags must be passed: " << std::endl
     << "  -R <number of round>" << std::endl
     << "  --conv_cost <cost> <limit>  wait until |V(s0) - cost| <= epsilon" << std::endl
     << "  --conv_s0 <limit>           wait the epsilon-consistent of V to V*" << std::endl
     << "where <limit> is either (or both) --max_rss_kb <K> or --max_cpu_time_sec <T>"
     << std::endl << std::endl;
}


int main(int argc, char** argv) {

  gpt::start_time = get_time_usec();

  // Removing buffer from stdout/cout
#ifndef NDEBUG
  setvbuf(stdout, NULL, _IONBF, 0);
#endif

  Planner* planner = 0;
  ushort_t seed[3];

  // Setting the seed as time. If the seed is given as a parameter, the
  // parameter will overwrite this one
  gpt::seed = time(NULL);


  // set command line
  std::ostringstream cmd;
  for( int i = 0; i < argc; ++i )
    cmd << argv[i] << " ";

  // read arguments and print banner
  std::vector<char*> remaining_args;
  if (!readArguments(argc, argv, remaining_args))
    return -1;

  printBanner(std::cout);
  std::cout << "**" << std::endl;
  std::cout << "COMMAND: " << cmd.str() << std::endl;
  std::cout << "PLANNER: \"" << gpt::algorithm << "\"" << std::endl;
  std::cout << "HEURISTIC: \"" << gpt::heuristic << "\"" << std::endl;
  std::cout << "STOP CRITERION: ";

  if (gpt::stopCriterion == NONE) {
    std::cout << "Not expecified... Error! See help" << std::endl;
    return -1;
  }
  else if (gpt::stopCriterion == NUM_ROUNDS) {
    std::cout << "Fixed number of rounds (" << gpt::total_execution_rounds
              << ")" << std::endl;
  }
  else if (gpt::stopCriterion == CONV_S0) {
    std::cout << "Epsilon-convergence of V(s0). Epsilon = " << gpt::epsilon
      << std::endl;
  }
  else if (gpt::stopCriterion == CONV_COST) {
    std::cout << "Epsilon-convergence of V*(s0) to the given cost. "
      << "Cost Given = " << gpt::given_value_for_vStar_s0
      << "\tEpsilon = "  << gpt::epsilon << std::endl;
  }

  std::cout << "COST POLICY: ";
  if (gpt::use_action_cost) {
    if (gpt::use_state_cost)
      std::cout << "FULL (actions & states)";
    else
      std::cout << "ACTIONS only";

    if (gpt::normalize_action_cost)
      std::cout << ", NORMALIZED";
    else
      std::cout << ", unnormalized";
  } else
    std::cout << "IGNORE";
  std::cout << std::endl;
  std::cout << "DEAD-END VALUE = " << gpt::dead_end_value << std::endl;

  std::cout << "SEED: " << gpt::seed << std::endl;

  std::cout << "HOSTNAME: " << getHostname() << std::endl;

  if (gpt::ignore_effects_with_prob_less_than > 0.0 &&
      gpt::execution_simulator == "local")
  {
    std::cout << IN_COLOR(BRIGHT_RED,
        "SIMULATION IS CONSIDERING AN IMPRECISE MODEL! "
        "In order to avoid this, use mdpsim to decople planning "
        "model and simulation model") << std::endl;
  }

  // set random seeds
  seed[0] = seed[1] = seed[2] = gpt::seed;
  srand48(gpt::seed);
  seed48(seed);

  assert(remaining_args.size() > 0);
  assert(remaining_args.size() < 4);

  // Remaining parameters semantics by the length of the vector:
  //
  // Length: | 1st position             | 2nd position   | 3rd pos   |
  // --------|--------------------------|----------------|-----------|
  // 1       | domain + prob ppddl file | --             | --        |
  // 2       | domain ppddl file        | prob pddl file | --        |
  // 3       | domain ppddl file        | prob pddl file | prob name |

  // Always parse first argument (either domain+prob or domain ppddl)
  if (remaining_args.size() == 1) {
    if (!readPPDDLDomainFile(remaining_args[0])) {
      std::cout << "[main]: ERROR: couldn't read parse the Domain+Problem file `"
                << remaining_args[0] << "'" << std::endl;
      return -1;
    }
  }
  else {
    //remaining_args.size() > 1
    if (!readPPDDLDomainFile(remaining_args[0])) {
      // There is at least prob pddl file
      std::cout << "[main]: ERROR: couldn't read parse the domain file `"
                << remaining_args[0] << "'" << std::endl;
      return -1;
    }
    if (!readPPDDLProblemFile(remaining_args[1])) {
      // There is at least prob pddl file
      std::cout << "[main]: ERROR: couldn't read parse problem file `"
                << remaining_args[1] << "'" << std::endl;
      return -1;
    }
  }

  problem_t *problem = NULL;
  if (remaining_args.size() == 3) {
    problem = (problem_t*)problem_t::find(remaining_args[2]);
    if (!problem) {
      std::cout << "[main]: ERROR: problem `" << remaining_args[2]
                << "' is not defined." << std::endl;
      return -1;
    }
  }
  else {
    problem = problem_t::first_problem();
    if (!problem) {
      std::cout << "[main]: ERROR: no problem was defined." << std::endl;
      return -1;
    }
  }

  assert(problem);
  gpt::problem = problem;
  std::cout << "PROBLEM NAME: " << problem->name() << std::endl;
  std::cout << "**" << std::endl;

  if (gpt::verbosity >= 300) {
    std::cout << "[domain-begin]" << std::endl
      << problem->domain() << std::endl
      << "<domain-end>" << std::endl;
  }


  //xxxxxx move after timer is started
  // instantiate actions
  try {
//    START_TIMING("instantiating_acts");
    problem->instantiate_actions();
//    STOP_TIMING("instantiating_acts");

//    START_TIMING("flattening");
    problem->flatten();
//    STOP_TIMING("flattening");

//    START_TIMING("prob_initing");
    state_t::initialize(*problem);
//    STOP_TIMING("prob_initing");

    if (gpt::verbosity >= 300) {
      std::cout << "[problem-begin]" << std::endl << "goal: ";
      problem->goalT().print(std::cout);
      std::cout << std::endl << "[problem-end]" << std::endl;
    }
  }
  catch (std::exception& e) {
    std::cout << e.what() << std::endl;
    return -1;
  }


  if (gpt::verbosity >= 300)
    std::cout << "**" << std::endl;
  std::cout << "[begin-session]" << std::endl;

  // initialize algorithm + heuristic + planner and execute
  try {
    SSPfromPPDDL ssp(*problem);
    std::shared_ptr<heuristic_t> heur_p = createHeuristic(ssp, gpt::heuristic);
    if (!heur_p) {
      std::cout << "[ERROR] No heuristic named '" << gpt::heuristic
                << "' is available. Quitting" << std::endl;
      exit(-1);
    }
    planner = createPlanner(ssp, gpt::algorithm, *heur_p);

    if (!planner) {
      std::cout << "[ERROR] No SSP planner called '" << gpt::algorithm
                << "' was found. Quitting" << std::endl;
      exit(-1);
    }

    problem->no_more_atoms();

    // Setting-up the execution environment
    Simulator* execution_simulator = createSimulator(ssp,
                                                    gpt::execution_simulator);

    if (!execution_simulator)
      return -1;

    if (gpt::train_for_usecs > 0) {
      planner->trainForUsecs(gpt::train_for_usecs);
    }

    gpt::simulator = execution_simulator;

    // Saving the policy of the planner if needed
    if (gpt::show_applied_policy || gpt::show_computed_policy)
      execution_simulator->setSavePolicy(true);


    execution_simulator->setOutputTurns(gpt::print_turn_details);
    if (gpt::stopCriterion == NUM_ROUNDS) {
      std::shared_ptr<SystemResourcesDeadline> eval_deadline = nullptr;

      try {
        if (gpt::max_cpu_sys_time_usec > 0 || gpt::max_rss_kb > 0) {
          eval_deadline
            = std::make_shared<SystemResourcesDeadline>(gpt::max_cpu_sys_time_usec,
                                                        gpt::max_rss_kb);
          gpt::setDeadline(eval_deadline);
        }

        // Restarting the seed. The idea is, if pi_1 and pi_2 are the same
        // but obtained in a different way (and potentially using different
        // numbers of calls to the random generator), they should still be
        // evaluated equally.
        srand48(gpt::seed + 1);
        seed48(seed + 1);
        auto rounds = execution_simulator->simulateNRounds(
            gpt::total_execution_rounds, planner, gpt::max_turn);
        Rational avg_cost(0);
        for (auto const& r : rounds) {
          avg_cost += r.accumulatedCost;
        }
        avg_cost /= (double) gpt::total_execution_rounds;

        std::cout << "Observed Avg cost = " << avg_cost << std::endl;
      }
      catch (DeadlineReachedException& e) {
        std::cout << std::endl
                  << "[" << get_human_readable_timestamp() << "] "
                  << eval_deadline->explanation() << " Aborting..." << std::endl;
        throw;
      }
    }
    else if (gpt::stopCriterion == CONV_S0) {
      HeuristicPlanner* heur_planner = dynamic_cast<HeuristicPlanner*>(planner);
      if (!heur_planner) {
        std::cout << "The chosen planner is not an heuristic planner, "
          << "therefore the convergence over s0 is not possible." << std::endl;
        return -1;
      }
      OptimalPlanner* opt_planner = dynamic_cast<OptimalPlanner*>(planner);
      if (!opt_planner) {
        std::cout << "The chosen planner is not an optimal planner!"
          << "therefore the convergence over s0 implemented for opt planners "
          << "only for now" << std::endl;
        return -1;
      }
      else {
        state_t s0 = problem->get_initial_state();
        auto eval_deadline
          = std::make_shared<SystemResourcesDeadline>(gpt::max_cpu_sys_time_usec,
                                                      gpt::max_rss_kb);
        try {
          gpt::setDeadline(eval_deadline);
          opt_planner->optimalSolution();
        }
        catch (DeadlineReachedException& e) {
          std::cout << std::endl
              << "[" << get_human_readable_timestamp() << "] "
              << eval_deadline->explanation() << " Aborting..."  << std::endl;
          std::cout << "V_lb(s0) = " << heur_planner->value(s0) << std::endl;
          throw;
        }
      }

    }
    else if (gpt::stopCriterion == CONV_COST) {
      std::cout << "HACK! CAUTION: we declare that it has converge when "
        << "V*(s0) - V_cur(s0) <= epsilon where V*(s0) is given in the "
        << "command line. This should be used with care!" << std::endl;

      HeuristicPlanner* heur_planner = dynamic_cast<HeuristicPlanner*>(planner);
      if (!heur_planner) {
        std::cout << "The chosen planner is not an heuristic planner, "
          << "therefore the convergence to fixed value of s0 is not possible."
          << std::endl;
        return -1;
      }

      state_t s0 = problem->get_initial_state();
      auto eval_deadline
        = std::make_shared<SystemResourcesDeadline>(gpt::max_cpu_sys_time_usec,
                                                    gpt::max_rss_kb);
      try {
        gpt::setDeadline(eval_deadline);
        while (true) {
          execution_simulator->simulateRound(heur_planner, gpt::max_turn);
          if (gpt::given_value_for_vStar_s0 - heur_planner->value(s0) <= gpt::epsilon)
          {
            std::cout << "Convergence reached! V(s0) = "
              << heur_planner->value(s0) << " (error wrt to given value is "
              << (gpt::given_value_for_vStar_s0 - heur_planner->value(s0))
              << ")" << std::endl;
            break;
          }
        }
      }
      catch (DeadlineReachedException& e) {
        std::cout << std::endl
          << "[" << get_human_readable_timestamp() << "] "
          << eval_deadline->explanation() << " Aborting..."  << std::endl;
        std::cout << "V_lb(s0) = " << heur_planner->value(s0) << std::endl;
        throw;
      }
    }

    // Showing the policy of the planner if needed
    HeuristicPlanner* heur_planner = dynamic_cast<HeuristicPlanner*>(planner);
    if (heur_planner) {
      state_t s0 = problem->get_initial_state();
      double val_cur = heur_planner->value(s0);
      std::cout << "V(s0) = " << val_cur << std::endl;
    }

    if (gpt::show_applied_policy || gpt::show_computed_policy) {
      DetPolicy pi;
      if (gpt::show_applied_policy)
        pi = execution_simulator->getSavedPolicy();
      else if (gpt::show_computed_policy) {
        NOT_IMPLEMENTED;
      }
      std::cout << "<final-policy>" << std::endl
                << pi << "</final-policy>" << std::endl;
    }
    delete execution_simulator;
  }
  catch (std::exception& e) {
    std::cout << e.what() << std::endl;
    state_t::statistics(std::cout);
    std::cout << "Wall time: " << (get_time_usec() - gpt::start_time) << std::endl;
    std::cout << "CPU+Sys time: " << get_cpu_and_sys_time_usec() << std::endl;
    std::cout << "Max Resident Mem: " << get_max_resident_mem_in_kb() << " KB" << std::endl;
    std::cout << "[end-session] due to Exception" << std::endl;
    return -1;
  }


  // print statistics and clean
  planner->statistics(std::cout, gpt::verbosity);
  delete planner;

  state_t::statistics( std::cout );

  std::cout << "Wall time: " << (get_time_usec() - gpt::start_time) << std::endl;
  std::cout << "CPU+Sys time: " << get_cpu_and_sys_time_usec() << std::endl
            << "Ignoring Parsing CPU+Sys time: "
            << (get_cpu_and_sys_time_usec() - gpt::parsing_cpu_time)
            << std::endl;
  std::cout << "Max Resident Mem: " << get_max_resident_mem_in_kb() << " KB" << std::endl;

#ifdef USE_CACHE_PROB_OP_ADDS_ATOM
  if (gpt::total_prob_op_adds_atom_calls > 0) {
    std::cout << "Total calls to Prob Op adds atom: "
              << gpt::total_prob_op_adds_atom_calls
              << " -- "
              << gpt::total_saved_prob_op_adds_atom_calls
              << " ["
              << ((float) gpt::total_saved_prob_op_adds_atom_calls /
                          gpt::total_prob_op_adds_atom_calls)
              << "] of them were cached"
              << std::endl;
  }
#endif

  // return
#ifdef MEM_DEBUG
  std::cerr << "[end-session]" << std::endl;
#endif
  std::cout << "[end-session]" << std::endl;
  return( 0 );
}
