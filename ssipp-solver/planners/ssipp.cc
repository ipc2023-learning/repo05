#include <cmath>
#include <vector>

#include "ssipp.h"
#include "lrtdp.h"
#include "vi.h"

#include "../ssps/bellman.h"
#include "../utils/die.h"
#include "../ext/mgpt/global.h"
#include "../ext/mgpt/hash.h"
#include "../ext/mgpt/problems.h"
#include "../simulators/simulator.h"
#include "../ext/mgpt/states.h"
#include "../utils/utils.h"




/******************************************************************************
 *
 * ShortSightedSSPFactory
 *
 *****************************************************************************/
void ShortSightedSSPFactory::init(std::deque<std::string>& flags) {
  if (flags.size() != 2) {
    std::cout << "ERROR! Unexpected number of parameters" << std::endl;
    usage(std::cout);
    exit(-1);
  }

  if (flags.front() == "max_depth") {
    s4p_method_ = ShortSightedMethod::MAX_DEPTH;
    flags.pop_front();
    max_depth_cfg_.t = atoi(flags.front().c_str());
    if (max_depth_cfg_.t <= 0) {
      std::cout << "ERROR! Max depth must be > 0. Received '"
                << max_depth_cfg_.t << "' instead. Quitting."  << std::endl;
      exit(-1);
    }
    flags.pop_front();
    std::cout << "[S4P Factory] Using max-depth with t = " << max_depth_cfg_.t
              << std::endl;
  }
  else if (flags.front() == "random_max_depth"
            || flags.front() == "max_depth_random")
  {
    s4p_method_ = ShortSightedMethod::RANDOM_MAX_DEPTH;
    flags.pop_front();
    std::deque<std::string> internal_flags = splitString(flags.front(), ",");
    if (internal_flags.size() != 2) {
      std::cout << "ERROR! Expecting 'X,Y' where X and Y are positive integers"
                << " and received '" << flags.front() << "' instead. Quitting."
                << std::endl;
      exit(-1);
    }
    random_max_depth_cfg_.lb = atoi(internal_flags.front().c_str());
    internal_flags.pop_front();
    random_max_depth_cfg_.ub = atoi(internal_flags.front().c_str());
    flags.pop_front();
    std::cout << "[S4P Factory] Using random-max-depth with t in {"
              << random_max_depth_cfg_.lb << ",...,"
              << random_max_depth_cfg_.ub << "} " << std::endl;
  }
  else if (flags.front() == "min_prob_traj"    // More descriptive name
            || flags.front() == "min_trace_p"  // Legacy name
            || flags.front() == "traj_based")  // NIPS'12 name
  {
    s4p_method_ = ShortSightedMethod::MIN_PROB_TRAJECTORY;
    flags.pop_front();
    min_prob_cfg_.min_p = Rational(flags.front().c_str());
    if (min_prob_cfg_.min_p <= 0 || min_prob_cfg_.min_p > 1) {
      std::cout << "ERROR! min_prob is not 0 < min_prob <= 1. Received '"
                << min_prob_cfg_.min_p << "' instead. Quitting."  << std::endl;
      exit(-1);
    }
    flags.pop_front();
    std::cout << "[S4P Factory] Using min-prob-traj with min_p = "
              << min_prob_cfg_.min_p << std::endl;
  }
  else if (flags.front() == "greedy") {
    s4p_method_ = ShortSightedMethod::GREEDY;
    flags.pop_front();
    greedy_cfg_.max_nodes = atoi(flags.front().c_str());
    if (greedy_cfg_.max_nodes <= 0) {
      std::cout << "ERROR! Max depth must be > 0. Received '"
                << greedy_cfg_.max_nodes << "' instead. Quitting."  << std::endl;
      exit(-1);
    }
    flags.pop_front();
    std::cout << "[S4P Factory] Using greedy with max_nodes = "
              << greedy_cfg_.max_nodes << std::endl;
  }
  else {
    std::cerr << "ERROR: Unknown short-sighted method: '"
              << flags.front() << "'. Quitting." << std::endl;
    usage(std::cerr);
    exit(1);
  }
  initialized_ = true;
}


// static
void ShortSightedSSPFactory::usage(std::ostream& os) {
  os
    << "ShortSightedSSPFactory takes at least 2 parameters:\n"
    << "\t<short_sighted_method>:<threshold>*\n"
    << "<short_sighted_method> = {max-depth, random-max-depth, "
    << "min_prob_traj, greedy}\n"
    << "<threshold> depends on the short_sighted_method, respectively:\n"
    << std::endl
    << "  max_depth         -> max depth of the space\n"
    << "  max_depth_random  -> a pair t_min,t_max. The value of t is randomly\n"
    << "                       draw from [t_min,t_max] for each short-sighted SSP\n"
    << "  min_prob_traj     -> min probability of the trace from s to s'\n"
    << "  greedy            -> (soft) maximum number of states allowed in a short-sighted SSP\n"
    << std::endl;
}



/******************************************************************************
 *
 * PlannerSSiPP
 *
 *****************************************************************************/

/*
 * CTOR
 */
void PlannerSSiPP::parseParameters(std::string const& flags_str) {
  std::deque<std::string> flags = splitString(flags_str, ":");

  if (flags.size() == 0 || flags.front() != "") {
    std::cerr << "ERROR! The following parameters passed to SSiPP could not "
              << "be parsed:";
    for (auto const& s : flags) {
      std::cerr << " '" << s << "'";
    }
    std::cerr << ".\n";
    usage(std::cerr);
    std::cerr << "\n\nQuitting." << std::endl;
    exit(1);
  }
  // Poping "" since the first char was ':' between ssipp and its first parameter
  flags.pop_front();

  // Parsing the local epsilon, i.e., the value used for declaring convergence
  // inside a short-sighted SSP. Note this is an optional parameter
  if (stringToDouble(flags.front(), short_sighted_epsilon_, true)) {
    // Using the given epsilon as local epsilon
    flags.pop_front();
  }
  else {
    // Parsing failed, so using the same epsilon as SSiPP for the overall
    // problem.
    short_sighted_epsilon_ = epsilon_;
  }
  DIE(short_sighted_epsilon_ > 0, "short_sighted_epsilon_ <= 0", -1);
  std::cout << "Using short_sighted_epsilon = " << short_sighted_epsilon_
            << " and epsilon = " << epsilon_ << std::endl;

  // Ignoring the choice of solver since vi is not working
  if (flags.front() == "lrtdp")   opt_planner_class_ = OptAlg::LRTDP;
  else if (flags.front() == "vi") opt_planner_class_ = OptAlg::VI;
  else {
    std::cerr << "ERROR: Unknown search algorithm for SSiPP: '"
              << flags.front() << "'" << std::endl;
    usage(std::cerr);
    exit(1);
  }
  flags.pop_front();
  std::cout << "[SSiPP] Using " << opt_planner_class_ << " as optimal planner"
            << std::endl;

  s4p_factory_.init(flags);

  if (flags.size() != 0) {
    std::cerr << "ERROR! The following parameters passed to SSiPP could not "
              << "be parsed:";
    for (auto const& s : flags) {
      std::cerr << " '" << s << "'";
    }
    std::cerr << ".\n";
    usage(std::cerr);
    std::cerr << "\n\nQuitting." << std::endl;
    exit(1);
  }
}


void PlannerSSiPP::usage(std::ostream& os) const {
  os
    << "ERROR! SSiPP takes the following parameters:\n"
    << "\tssipp:[epsilon]:<search_alg>:<FACTORY-PARAMS>*\n"
    << "[epsilon] = epsilon used for declaring convergence inside a \n"
    << "            short-sighted SSP. If omitted, the global epsilon is used.\n"
    << "<search_alg> = {lrtdp, vi}\n\n"
    << "<FACTORY-PARAMS>:" << std::endl;
  ShortSightedSSPFactory::usage(os);
}



/*
 * DECIDEACTION
 */
action_t const* PlannerSSiPP::decideAction(state_t const& s) {
  static size_t decideActionCounter = 0;
  gpt::incCounterAndCheckDeadlineEvery(decideActionCounter, 10);
  gpt::checkDeadline();
  static size_t num_actions_same_s4p = 0;

//  std::cout << "=> SSiPP: s = " << s.toString() << " V = " << v_.value(s)
//            << std::endl;

  /*
   * Trivial cases with early return
   */
  if (ssp_.isGoal(s) || !ssp_.hasApplicableActions(s)) {
//    std::cout << "[SSiPP]: goal or deadend reached. Used "
//              << num_actions_same_s4p << " actions of the current s4p"
//              << std::endl;
    return nullptr;
  }

  // Creating a new s4p if necessary, that is, if either:
  //  - first decideAction call in this round (thus opt_planner_ == nullptr)
  //  - s is an artificial goal, i.e., s \in G' \setminus G. Notice that, since
  //    opt_planner != nullptr, then cur_s4p_ is also not null.
  if (!opt_planner_ || cur_s4p_->isGoal(s)) {
    cur_s4p_ = s4p_factory_.newShortSightedSSP(ssp_, s, v_);
    /*
     * Building a new Optimal Planner for the new Short-Sighted SSP
     */
    switch (opt_planner_class_) {
     case OptAlg::LRTDP:
      opt_planner_.reset(new PlannerLRTDP(*cur_s4p_, v_,
                                          short_sighted_epsilon_));
      break;
     case OptAlg::VI:
      opt_planner_.reset(new PlannerVI(*cur_s4p_, v_, short_sighted_epsilon_));
      break;
     default:
      std::cout << "[SSiPP]: Unexpected OptimalPlanner type '"
                << opt_planner_class_ << "' in decideAction" << std::endl;
      exit(-1);
    }
//    std::cout << "Build a new s4p. Used " << num_actions_same_s4p
//              << " actions of the previous one" << std::endl;
    // Optimally solving the s4p
    opt_planner_->optimalSolution();
//    for (auto const& sp : cur_s4p_->reachableStates()) {
//      std::cout << "      V(" << sp.toString() << ") = " << v_.value(sp)
//                << (cur_s4p_->isGoal(sp) ? " -- s4p goal" : "")
//                << (ssp_.isGoal(sp) ? " -- original goal" : "")
//                << std::endl;
//    }
    num_actions_same_s4p = 1;
  }
  else {
    // still using the current s4p solution
    num_actions_same_s4p++;
  }

  action_t const* a = opt_planner_->decideAction(s);
//  std::cout << "[ssipp::decideAction] n = " << num_actions_same_s4p
//            << " s = " << s.toString() << " a = " << (a ? a->name() : "null")
//            << std::endl;
  return a;
}



/*
 * Ad-hoc training method: simulate rounds until either:
 *  - Deadline is reached
 *  - 50 consecutive rounds reach the goal and delta V(s0) <= epsilon
 */
void PlannerSSiPP::trainForUsecs(uint64_t max_time_usec) {
  size_t const GOAL_THRESHOLD = 50;

  std::cout << "[SSiPP] Training for " << max_time_usec
            << " usecs or reaching the goal " << GOAL_THRESHOLD
            << " in a row" << std::endl;
  double v_old_s0 = 0;
  uint16_t goal_reached_in_a_row = 0;
  LocalSimulator simulator(ssp_);
  state_t const s0 = ssp_.s0();

  auto my_deadline
    = std::make_shared<SystemResourcesDeadline>(max_time_usec, gpt::max_rss_kb);
  if (!gpt::setDeadline(my_deadline)) {
    std::cerr
      << "Deadline already set and PlannerLabeledSSiPP::trainForUsecs "
      << "need to set a new deadline" << std::endl;
    exit(-1);
  }
  try {
    while (true) {
      RoundSummary const r = simulator.simulateRoundFrom(&s0, this,
                                                         gpt::max_turn);
      if (r.exitStatus == EndOfRoundStatus::GOAL_REACHED) {
        goal_reached_in_a_row++;
        if (goal_reached_in_a_row == GOAL_THRESHOLD
            && std::abs(v_.value(s0) - v_old_s0) < gpt::epsilon)
        {
          std::cout
            << "[PlannerSSiPP::trainForUsecs] "
            << "50 rounds reached a goal in a row and delta(V(s0)) < epsilon"
            << std::endl;
          break;
        }
        else {
          // Since we need 50 goals to reached in a row, we can update v_old_s0
          // only when a goal is reached to avoid some extra calls
          v_old_s0 = v_.value(s0);
        }
      }
      else {
        goal_reached_in_a_row = 0;
      }
    }
  }
  catch (DeadlineReachedException& e) {
    std::cout << "[PlannerSSiPP::trainForUsecs] "
      << "deadline reached before optimal solution was found."
      << std::endl;
  }
  gpt::removeDeadline();
}

