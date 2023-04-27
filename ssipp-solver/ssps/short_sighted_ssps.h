#ifndef SHORT_SIGHTED_SPACE_H
#define SHORT_SIGHTED_SPACE_H

#include <unordered_map>
#include <memory>

#include "../utils/die.h"
#include "../ext/mgpt/global.h"
#include "../ext/mgpt/states.h"
#include "prob_dist_state.h"
#include "ssp_utils.h"
#include "ssp_iface.h"
#include "../utils/utils.h"  // randInIntervalInclusive_l

/*
 * Implementation of StateConstIteIface for a HashMap, i.e., the states used as
 * key of the HashMap are considered as the reachable. This can also be seen as
 * a const_iterator over the keys of the HashMap.
 */
template<typename T>
class HashMapStateConstIte : public StateConstIteIface {
 public:
  // Ctor for the begin of the HashsetState obtained from reachableStatesFrom.
  // The iterators will share an pointer to the object
  HashMapStateConstIte(typename HashMapState<T>::const_iterator it,
                       typename HashMapState<T>::const_iterator end)
    : it_(it), end_(end)
  { }
  HashMapStateConstIte& operator++() { ++it_; return *this; }
  bool operator!=(HashMapStateConstIte const& rhs) const {
    return it_ != rhs.it_;
  }
  bool operator!=(StateConstIteIfaceUniqPtr const& rhs) const {
    // Hack for efficiency: nullptr represents the end of the range
    if (!rhs)  return it_ != end_;

    HashMapStateConstIte const* conv_rhs =
                          dynamic_cast<HashMapStateConstIte const*>(rhs.get());
    if (conv_rhs) {
      return operator!=(*conv_rhs);
    }
    else {
      return true;
    }
  }
  state_t const& operator*() const { return it_->first; }
  StateConstIteIfaceUniqPtr clone() const {
    return StateConstIteIfaceUniqPtr(new HashMapStateConstIte(it_, end_));
  }

 private:
  typename HashMapState<T>::const_iterator it_;
  typename HashMapState<T>::const_iterator end_;
};



/*
 * NOTE: the value function v (hash_t) that represents the heuristic value added
 * to the fringe states are NOT const in order to cache the heuristic value.
 * This issue is related with the fact that terminal states (artificial goals in
 * this case) are not updated by the Bellman operator, instead, it is considered
 * a fixed constant. Therefore, fringe states that are never an internal state
 * of a different short-sighted SSP do not have their value function updated to
 * their heuristic value and this leads to excessive calls to the heuristic. By
 * making v NOT const, when terminalCost(s) is called, then H(s) is assigned to
 * V(s) if V(s) is not defined. This is the only change that ShortSightedSSP
 * makes to v.
 */
class ShortSightedSSP : public SSPIface {
 public:
  ShortSightedSSP(SSPIface const& ssp, state_t const& s0, hash_t& v,
      std::string name)
    : ssp_(ssp), s0_(s0), v_(v), name_(name)
  { }

  ShortSightedSSP(ShortSightedSSP&& rhs) = default;
  ShortSightedSSP(ShortSightedSSP const& rhs) = delete;

  std::string const& name() const { return name_; }

  state_t const& s0() const { return s0_; }

  bool isGoal(state_t const& s) const {
    auto it = ss_hash_.find(s);
    DIE(it != ss_hash_.end(), "short-sighted space is not defined for s", -1);
    // If a state is an original goal, it is also marked as a FRINGE
    return it->second == StateType::FRINGE;
  }

  bool hasApplicableActions(state_t const& s) const {
    DIE(!isGoal(s), "s is not supposed to be a goal in the SS-SSP", 171);
    return ssp_.hasApplicableActions(s);
  }

  bool isApplicable(state_t const& s, action_t const& a) const {
    DIE(!isGoal(s), "s is not supposed to be a goal in the SS-SSP", 171);
    return ssp_.isApplicable(s,a);
  }

  ActionConstRange applicableActions(state_t const& s) const {
    DIE(!isGoal(s), "s is not supposed to be a goal in the SS-SSP", 171);
    return ssp_.applicableActions(s);
  }

  // Actions are NOT changed
  void expand(action_t const& a, state_t const& s, ProbDistStateIface& pr) const
  {
    ssp_.expand(a, s, pr);
  }

  // Cost of actions are NOT changed
  Rational cost(state_t const& s, action_t const& a) const {
    return ssp_.cost(s,a);
  }

  Rational terminalCost(state_t const& s) const {
    DIE(isGoal(s), "Expecting Goal state", 171);
    if (ssp_.isGoal(s)) {
      return ssp_.terminalCost(s);
    }
    return Rational(v_.value(s));
  }

  StateConstRange reachableStates() const {
    StateConstIteIfaceUniqPtr begin(
        new HashMapStateConstIte<StateType>(ss_hash_.begin(), ss_hash_.end()));
    return StateConstRange(std::move(begin), nullptr);
  }

  bool contains(state_t const& s) const { return isDefinedFor(s); }


  /*
   * Named Constructors for the different types of Short-Sighted SSPs
   */

  /*
   * MAX-DEPTH
   * 
   * Defined in AIJ'14: http://felipe.trevizan.org/papers/trevizan14:depth.pdf
   * 
   * Generate the space of all states reachable using up to max_depth actions.
   */
  static std::unique_ptr<ShortSightedSSP> newMaxDepth(SSPIface const& ssp,
      state_t const& s, hash_t& fringe_heuristic, size_t max_depth,
      size_t max_space_size = 0, uint64_t max_cpu_time_usec = 0);


  /* 
   * TRAJECTORY-BASED
   *
   * Defined in NIPS'12: http://felipe.trevizan.org/papers/trevizan12:trajectory.pdf
   *
   * Generates an SSP by considering states with a trace probability at least
   * min_p. Formally, let
   *   P_max(s,s') := max_{T a trajectory s.t. T[0] = s, T[end] = s'} P(T)
   *
   * a state s' is in the (s, min_p)-trajectory-based SSP if:
   *  - there exists a state s" s.t. P_max(s,s") >= min_p AND
   *  - there exists a s.t. P(s'|s",a) > 0
   *
   * For more details of the definition, see the paper "Trajectory-Based
   * Short-Sighted Probabilistic Planning" from NIPS 2012 (in the paper min_p is
   * referred to as rho).
   *
   * If max_space_size > 0, then the expansion of the trajectory-based
   * short-sighted SSP will stop when |S'| == max_space_size; however, the final
   * size of S' might be bigger than max_space_size because the states marked
   * for expansion will be added to the short-sighted SSP as artificial goals.
   * The total amount of extra states added depends on the branching factor of
   * the original SSP.
   */
  static std::unique_ptr<ShortSightedSSP> newTrajectoryBased(SSPIface const& ssp,
      state_t const& s, hash_t& fringe_heuristic, Rational min_p,
      size_t max_space_size = 0, uint64_t max_cpu_time_usec = 0);


  /*
   * GREEDY
   *
   * Defined in FWT's thesis: http://felipe.trevizan.org/papers/thesis.pdf
   *
   * Generates a short-sighted SSP with approximate size max_space_size (approx.
   * because of the completeness conditions) expanding states with smaller value
   * of v first.
   */
  static std::unique_ptr<ShortSightedSSP> newGreedy(SSPIface const& ssp,
      state_t const& s, hash_t& v, size_t max_space_size,
      uint64_t max_cpu_time_usec = 0);


 private:
  /*
   * Enums and typedefs
   */
  // FRINGE represents the Artificial Goals UNION the Original Goals inside the
  // short-sighted problem.
  enum class StateType {INTERNAL, FRINGE};
  using ShortSightedHash = HashMapState<StateType>;

  /*
   * Private functions
   */
  bool isDefinedFor(state_t const& s) const {
    return ss_hash_.find(s) != ss_hash_.end();
  }

  bool insertState(state_t const& s) {
    if (isDefinedFor(s))
      return false;
    ss_hash_[s] = StateType::INTERNAL;
    return true;
  }

  void setAsFringe(state_t const& s) {
    DIE(isDefinedFor(s), "short-sighted space is not defined for s", -1);
    ss_hash_[s] = StateType::FRINGE;
  }

  void setAsInternal(state_t const& s) {
    DIE(isDefinedFor(s), "short-sighted space is not defined for s", -1);
    ss_hash_[s] = StateType::INTERNAL;
  }

  size_t totalStates() const { return ss_hash_.size(); }

  // Debug function that checks if the current short-sighted SSP meets all the
  // sufficient conditions given in the NIPS'12 paper (definition 5).
  bool satisfiesSufficientConditions() const;

  /*
   * Member variables
   */
  SSPIface const& ssp_;
  state_t const& s0_;
  hash_t& v_;
  std::string const name_;
  ShortSightedHash ss_hash_;
};
#endif  //SHORT_SIGHTED_SPACE_H
