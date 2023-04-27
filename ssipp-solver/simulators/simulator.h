#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <iostream>

#include "../ssps/policy.h"
#include "../ext/mgpt/states.h"
#include "../ssps/ssp_iface.h"






class Planner;
class problem_t;

enum class EndOfRoundStatus : int {
  GOAL_REACHED = 0,
  PLANNER_GAVE_UP,       // V(s) is not defined
  DEADEND,               // decideAction returned NULL
  MAX_TURNS_REACHED,
  TIMEOUT,
  INVALID_ACTION,
  CANCELED_BY_SIMULATOR  // External simulator canceled the simulation
};

inline std::ostream& operator<<(std::ostream& os, EndOfRoundStatus s) {
  switch (s) {
   case EndOfRoundStatus::GOAL_REACHED         : os << "goal-reached"; break;
   case EndOfRoundStatus::PLANNER_GAVE_UP      : os << "planner-gave-up"; break;
   case EndOfRoundStatus::DEADEND              : os << "deadend-reached"; break;
   case EndOfRoundStatus::MAX_TURNS_REACHED    : os << "max-turns-reached"; break;
   case EndOfRoundStatus::TIMEOUT              : os << "timed-out"; break;
   case EndOfRoundStatus::INVALID_ACTION       : os << "invalid-action"; break;
   case EndOfRoundStatus::CANCELED_BY_SIMULATOR: os << "canceled-by-sim"; break;
   default                               : os.setstate(std::ios_base::failbit);
  }
  return os;
}

class RoundSummary {
 public:
  // Here for the vector constructor
  RoundSummary() : accumulatedCost(0), totalActionsApplied(0),
      totalCpuPlusSystemTime(0),  exitStatus(EndOfRoundStatus::PLANNER_GAVE_UP)
  { }

  RoundSummary(uint32_t t, uint64_t cpu, EndOfRoundStatus s)
    : accumulatedCost(0), totalActionsApplied(t),
      totalCpuPlusSystemTime(cpu), exitStatus(s)
  { }
  ~RoundSummary() { }

  Rational accumulatedCost;
  uint32_t totalActionsApplied;
  uint64_t totalCpuPlusSystemTime;
  EndOfRoundStatus exitStatus;
  friend std::ostream& operator<<(std::ostream& out, RoundSummary const& r);
};


class Simulator {
 public:
  Simulator(SSPIface const& ssp, bool output_turns = false,
            bool save_policy = false) : ssp_(ssp),
                                        output_turns_(output_turns),
                                        save_policy_(save_policy) { }
  virtual ~Simulator() { }

  virtual void setOutputTurns(bool val) { output_turns_ = val; }
  virtual void setSavePolicy(bool val) { save_policy_ = val; }
  virtual bool isSavingPolicy() const { return save_policy_; }

  RoundSummary const simulateRound(
      Planner* planner,
      uint32_t max_turn,
      uint64_t max_cpu_sys_time_usec = 0)
  {
//    state_t const s0 = getInitialState();
    return simulateRoundFrom(NULL, planner, max_turn,
                             max_cpu_sys_time_usec);
  }

  virtual RoundSummary const simulateRoundFrom(
      state_t const* s,
      Planner* planner,
      uint32_t max_turn,
      uint64_t max_cpu_sys_time_usec = 0);


  std::vector<RoundSummary> const simulateNRounds(
      uint32_t n,
      Planner* planner,
      uint32_t max_turn,
      uint64_t max_cpu_sys_time_per_round_usec = 0)
  {
//    state_t const s0 = getInitialState();
    return simulateNRoundsFrom(n, NULL, planner, max_turn,
                               max_cpu_sys_time_per_round_usec);
  }

  virtual std::vector<RoundSummary> const simulateNRoundsFrom(
      uint32_t n,
      state_t const* s,
      Planner* planner,
      uint32_t max_turn,
      uint64_t max_cpu_sys_time_per_round_usec = 0);


  virtual DetPolicy& getSavedPolicy() { return pi_; }
  virtual DetPolicy const& getSavedPolicy() const { return pi_; }

 protected:
  SSPIface const& ssp_;
  bool output_turns_;
  bool save_policy_;
  DetPolicy pi_;

  /* Methods that make an interface with the simulator: OVERWRITE THEM */
  virtual action_t const* requestPlannerAction(Planner* planner,
                                               state_t const s);
  virtual void initRound(Planner* planner);
  virtual void endRound(Planner* planner);

  // This method checks if the current simulator has decided to finish the
  // simulation abruptly. This method is specially interesting for external
  // simulators, e.g., MDPSIM, in which the communication channel needs to be
  // checked
  virtual bool isSimulationOver() = 0;
  virtual state_t const modifyState(action_t const* a, state_t const s) = 0;
  virtual state_t const getInitialState() = 0;
};


Simulator* createSimulator(SSPIface const& ssp, std::string const& name);

class LocalSimulator : public Simulator {
 public:
  LocalSimulator(SSPIface const& ssp, bool output_turns = false,
            bool save_policy = false) : Simulator(ssp, output_turns,
                                                  save_policy) { }

  virtual ~LocalSimulator() { }

  /* Added for evaluation purposes, i.e., a simulation in which the planner is
   * not allowed to acquire more information about the environment. This is
   * enforced through the 'constness' of the planner, thus it assumes that the
   * planner won't cheat (mutables, const casts, etc).
   */
  std::vector<RoundSummary> const simulateNRounds(
      uint32_t n,
      Planner const* planner,
      uint32_t max_turn,
      uint64_t max_cpu_sys_time_per_round_usec = 0);

  RoundSummary const simulateRound(
      Planner const* planner,
      uint32_t max_turn,
      uint64_t max_cpu_sys_time_usec = 0);


 protected:
  virtual bool isSimulationOver() { return false; }
  virtual state_t const modifyState(action_t const* a, state_t const s);
  virtual state_t const getInitialState();
};

#endif // SIMULATOR_H
