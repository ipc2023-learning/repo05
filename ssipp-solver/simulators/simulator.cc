#include <string.h>

#include "../utils/die.h"
#include "../ext/mgpt/rational.h"
#include "simulator.h"
#include "../ext/mgpt/states.h"
#include "../utils/utils.h"
#include "../ext/mgpt/problems.h"

#include "../planners/planner_iface.h"

/*
 * RoundSummary
 */
std::ostream& operator<<(std::ostream& out, RoundSummary const& r) {
  out << "[Round Summary]: status = " << r.exitStatus << std::endl
      << "[Round Summary]: # of actions = " << r.totalActionsApplied << std::endl
      << "[Round Summary]: CPU+System time = " << r.totalCpuPlusSystemTime << std::endl
      << "[Round Summary]: Total COST = " << r.accumulatedCost << std::endl;
  return out;
}


/*******************************************************************************
 *
 * Simulator
 *
 ******************************************************************************/

// static
Simulator* createSimulator(SSPIface const& ssp, std::string const& name) {
  Simulator* simulator = NULL;
  if (!strcasecmp(name.c_str(), "local")) {
    simulator = new LocalSimulator(ssp);
  }
  else if (!strcasecmp(name.c_str(), "mdpsim")) {
    std::cerr << "The support for mdpsim was removed for now."
              << std::endl;
    exit(-1);
  }
  else {
    std::cerr << "Invalid execution simulator" << std::endl;
  }
  return simulator;
}


action_t const* Simulator::requestPlannerAction(Planner* planner,
                                                state_t const s)
{
  return planner->decideAction(s);
}

void Simulator::initRound(Planner* planner) {
  planner->initRound();
}

void Simulator::endRound(Planner* planner) {
  planner->endRound();
}


RoundSummary const Simulator::simulateRoundFrom(state_t const* s,
    Planner* planner, uint32_t max_turn, uint64_t max_cpu_sys_time_usec)
{
  DIE(planner != NULL, "Null planner", 1);
  state_t cur_s;
  if (s == NULL)
    cur_s = getInitialState();
  else
    cur_s = *s;

  RoundSummary r(0, 0, EndOfRoundStatus::PLANNER_GAVE_UP);

  uint64_t start_time = get_cpu_and_sys_time_usec();
  initRound(planner);

  try {
    while (true) {
      if (output_turns_) {
        std::cout << std::endl;
      }

      // Checking several termination criteria
      if (ssp_.isGoal(cur_s)) {
        r.accumulatedCost += ssp_.terminalCost(cur_s);
        r.exitStatus = EndOfRoundStatus::GOAL_REACHED;
        break;
      }
      else if (r.totalActionsApplied >= max_turn) {
        r.exitStatus = EndOfRoundStatus::MAX_TURNS_REACHED;
        break;
      }
      else if (max_cpu_sys_time_usec > 0 &&
          (get_cpu_and_sys_time_usec() - start_time) > max_cpu_sys_time_usec)
      {
        r.exitStatus = EndOfRoundStatus::TIMEOUT;
        break;
      }
      else if (isSimulationOver()) {
        r.exitStatus = EndOfRoundStatus::CANCELED_BY_SIMULATOR;
        break;
      }

      if (output_turns_) {
        std::cout << "T[" << r.totalActionsApplied << "] s = "
                  << cur_s.toStringFull(gpt::problem)
                  << std::endl;
      }

      // Put try here to catch undefined policy and eventually in the future to
      // catch dead-end
      action_t const* a = requestPlannerAction(planner, cur_s);

      if (save_policy_)
        pi_.set(cur_s, a);

      if (a == NULL) {
        // The planner gave up from the problem
        if (output_turns_)
          std::cout << "T[" << r.totalActionsApplied << "] dead-end\n";
        r.exitStatus = EndOfRoundStatus::DEADEND;
        break;
      }
      else if (!a->enabled(cur_s)) {
        if (output_turns_)
          std::cout << "T[" << r.totalActionsApplied << "] INVALID ACTION "
                            << a->name() << std::endl;
        r.exitStatus = EndOfRoundStatus::INVALID_ACTION;
        break;
      }
      else {
        if (gpt::use_action_cost) {
          // ASSUMPTION(fwt): Since the cost/reward is independent of the
          // probabilistic effects (notice that we assume and also try to
          // enforce this, if the cost/reward is not what you expect, this
          // assumption is a major candidate for the reason), we don't need to
          // know what is the resulting state of applying a in the cur_s.

          r.accumulatedCost += ssp_.cost(cur_s,*a);
        }
        if (output_turns_) {
          std::cout << "T[" << r.totalActionsApplied << "] a = " << a->name()
                    << std::endl;
          if (gpt::use_action_cost) {
            std::cout << "T[" << r.totalActionsApplied << "]"
                      << " cost += " << ssp_.cost(cur_s,*a) << std::endl;
          }
        }
        r.totalActionsApplied++;
        cur_s = modifyState(a, cur_s);
      }
    }
  }
  catch (PlannerGaveUpException& e) {
    r.exitStatus = EndOfRoundStatus::PLANNER_GAVE_UP;
  }
  r.totalCpuPlusSystemTime = get_cpu_and_sys_time_usec() - start_time;
  endRound(planner);
  return r;
}


std::vector<RoundSummary> const Simulator::simulateNRoundsFrom(
      uint32_t n,
      state_t const* s,
      Planner* planner,
      uint32_t max_turn,
      uint64_t max_cpu_sys_time_per_round_usec)
{
  std::vector<RoundSummary> result(n);
  for (uint32_t i = 0; i < n; ++i) {
    if (!gpt::suppress_round_info) {
      if (i == 0) {
        gpt::parsing_cpu_time = get_cpu_and_sys_time_usec();
        std::cout << "<cpu+sys-time-since-start>"
          << gpt::parsing_cpu_time
          << "</cpu+sys-time-since-start>"
          << std::endl;
      }
      std::cout << "[Round Summary]: round # = " << i << std::endl;
      std::cout.flush();
    }
    result[i] = simulateRoundFrom(s, planner, max_turn,
                                  max_cpu_sys_time_per_round_usec);
    if (!gpt::suppress_round_info) {
      std::cout << result[i] << std::endl;
    }
  }
  return result;
}



/*******************************************************************************
 *
 * LocalSimulator
 *
 ******************************************************************************/

state_t const LocalSimulator::modifyState(action_t const* a,
                                           state_t const s)
{
  DIE(a != NULL, "Expecting a non-NULL action", 157);
  state_t s_prime = s;
  a->affect(s_prime);
  return s_prime;
}


state_t const LocalSimulator::getInitialState() {
  return ssp_.s0();
}


std::vector<RoundSummary> const LocalSimulator::simulateNRounds(
      uint32_t n,
      Planner const* planner,
      uint32_t max_turn,
      uint64_t max_cpu_sys_time_per_round_usec)
{
  std::vector<RoundSummary> result(n);
  for (uint32_t i = 0; i < n; ++i) {
    if (!gpt::suppress_round_info) {
      if (i == 0) {
        gpt::parsing_cpu_time = get_cpu_and_sys_time_usec();
        std::cout << "<cpu+sys-time-since-start>"
          << gpt::parsing_cpu_time
          << "</cpu+sys-time-since-start>"
          << std::endl;
      }
      std::cout << "[Round Summary]: round # = " << i << std::endl;
      std::cout.flush();
    }
    result[i] = simulateRound(planner, max_turn,
                              max_cpu_sys_time_per_round_usec);
    if (!gpt::suppress_round_info) {
      std::cout << result[i] << std::endl;
    }
  }
  return result;
}



RoundSummary const LocalSimulator::simulateRound(Planner const* planner,
    uint32_t max_turn, uint64_t max_cpu_sys_time_usec)
{
  DIE(planner != NULL, "Null planner", 1);
  state_t cur_s = getInitialState();

  RoundSummary r(0, 0, EndOfRoundStatus::PLANNER_GAVE_UP);

  uint64_t start_time = get_cpu_and_sys_time_usec();


  try {
    while (true) {
      if (output_turns_) {
        std::cout << std::endl;
      }

      // Checking several termination criteria
      if (ssp_.isGoal(cur_s)) {
        r.accumulatedCost += ssp_.terminalCost(cur_s);
        r.exitStatus = EndOfRoundStatus::GOAL_REACHED;
        break;
      }
      else if (r.totalActionsApplied >= max_turn) {
        r.exitStatus = EndOfRoundStatus::MAX_TURNS_REACHED;
        break;
      }
      else if (max_cpu_sys_time_usec > 0 &&
          (get_cpu_and_sys_time_usec() - start_time) > max_cpu_sys_time_usec)
      {
        r.exitStatus = EndOfRoundStatus::TIMEOUT;
        break;
      }
      else if (isSimulationOver()) {
        r.exitStatus = EndOfRoundStatus::CANCELED_BY_SIMULATOR;
        break;
      }

      if (output_turns_) {
        std::cout << "T[" << r.totalActionsApplied << "] s = "
                  << cur_s.toStringFull(gpt::problem)
                  << std::endl;
      }

      // Put try here to catch undefined policy and eventually in the future to
      // catch dead-end
      action_t const* a = planner->decideAction(cur_s);

      if (save_policy_)
        pi_.set(cur_s, a);

      if (a == NULL) {
        // The planner gave up from the problem
        if (output_turns_)
          std::cout << "T[" << r.totalActionsApplied << "] dead-end\n";
        r.exitStatus = EndOfRoundStatus::DEADEND;
        break;
      }
      else if (!a->enabled(cur_s)) {
        if (output_turns_)
          std::cout << "T[" << r.totalActionsApplied << "] INVALID ACTION "
                    << a->name() << std::endl;
        r.exitStatus = EndOfRoundStatus::INVALID_ACTION;
        break;
      }
      else {
        if (gpt::use_action_cost) {
          // ASSUMPTION(fwt): Since the cost/reward is independent of the
          // probabilistic effects (notice that we assume and also try to
          // enforce this, if the cost/reward is not what you expect, this
          // assumption is a major candidate for the reason), we don't need to
          // know what is the resulting state of applying a in the cur_s.

          r.accumulatedCost += ssp_.cost(cur_s,*a);
        }

        if (output_turns_) {
         std::cout << "T[" << r.totalActionsApplied << "] a = " << a->name()
                    << std::endl;
          if (gpt::use_action_cost) {
            std::cout << "T[" << r.totalActionsApplied << "]"
                      << " cost += " << ssp_.cost(cur_s,*a) << std::endl;
          }
        }
        r.totalActionsApplied++;
        cur_s = modifyState(a, cur_s);
      }
    }
  }
  catch (PlannerGaveUpException& e) {
    r.exitStatus = EndOfRoundStatus::PLANNER_GAVE_UP;
  }
  r.totalCpuPlusSystemTime = get_cpu_and_sys_time_usec() - start_time;
  return r;
}

