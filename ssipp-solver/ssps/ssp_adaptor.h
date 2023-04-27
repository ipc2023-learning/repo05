#ifndef SSP_ADAPTOR_H
#define SSP_ADAPTOR_H

#include <iostream>

#include "ssp_iface.h"
#include "../ext/mgpt/atom_states.h"  // explicitly using state_t (new_s0_)
#include "../ext/mgpt/hash.h"

/*
 * Adaptor to represent an SSP = <S', s0', G', A', P, C', C_t'> given an
 * SSPIface for an SSP = <S, s_0, G, A, P, C, C_t> where:
 *
 *  - S' \subseteq S
 *  - s0' \in S'
 *  - G \subset G', i.e., there might be MORE goals in the new problem
 *  - A'(s) \subseteq A(s), i.e., there might be LESS applicable actions
 *
 * The C' and C'_t can be arbitrarily changed.
 *
 * The restrictions in the given SSPIface are enforced through the given
 * functors: 
 *
 * - IsGoalFunctor: __adds__ goals to the problem by returning true. If false is
 *   returned, then the original ssp goal test is used for the given state.
 *
 * - IsApplicableFunctor: __removes__ applicable actions by returning false. If
 *   true is returned, then the original ssp isApplicable method is called.
 *
 * - CostActionFunctor: __changes__ the cost of an action arbitrarily
 *   (respecting C(s,a) > 0). If a value <= 0 is returned, then the original
 *   cost is used.
 *
 * - TerminalCostFunctor: __changes__ the terminal cost arbitrarily (respecting
 *   C_t(s) >= 0). If a value < 0 is returned, then the original terminal cost
 *   is used.
 *
 * - ReachableStateFunctor: __do not change__ anything. This method is used only
 *   for efficiency reasons. For instance, if S' is computed in order to find
 *   G', then it can be passed to the SSPAdaptor through this functor to avoid
 *   recomputing the set of reachable states from s0.
 *
 * The functors (defined in the end of this file) SameGoals,
 * SameApplicableActions, SameActionCost, SameTerminalCost, and
 * OnDemandReachableStates represent the identity function, i.e., no change over
 * the original problem on their respective parameter.
 *
 * Functors are copied to SSPAdaptor, therefore they cannot be modified, i.e.,
 * changing the original functor will have no effect in the subproblem. To
 * obtain a subproblem that changes "dynamically" (e.g., disables set of
 * provenly suboptimal actions that increases as the algorithm performs search
 * as in FTVI), the functor needs a reference for the data structure
 * representing this restrictions. As the restrictions change, the functor and
 * the subproblem change too.
 *
 * For simplicity, use the method makeSSPAdaptor (definition after SSPAdaptor).
 */

template<typename IsGoalFunctor,
         typename IsApplicableFunctor,
         typename CostActionFunctor,
         typename TerminalCostFunctor,
         typename ReachableStateFunctor>
class SSPAdaptor : public SSPIface {
 public:
  SSPAdaptor(SSPIface const& ssp,
      std::string const& name,
      state_t const& new_s0,
      IsGoalFunctor const& is_goal,
      IsApplicableFunctor const& is_applicable,
      CostActionFunctor const& cost,
      TerminalCostFunctor const& terminal_cost,
      ReachableStateFunctor const& reachable_states)
    : ssp_(ssp), name_(name), new_s0_(new_s0), is_goal_(is_goal),
      is_applicable_(is_applicable), cost_(cost), terminal_cost_(terminal_cost),
      reachable_states_(reachable_states)
  { }

  std::string const& name() const override { return name_; }

  state_t const& s0() const override { return new_s0_; }

  // Returns true if s \in G' \SUPseteq G
  bool isGoal(state_t const& s) const override {
    return is_goal_(s) || ssp_.isGoal(s);
  }

  // Returns true if |A(s)| > 0
  bool hasApplicableActions(state_t const& s) const override {
    auto const& range = applicableActions(s);
    return range.begin() != range.end();
  }

  // Returns true if a in A(s)
  bool isApplicable(state_t const& s, action_t const& a) const override {
    return is_applicable_(s, a) && ssp_.isApplicable(s, a);
  }

  // Returns the range of all actions A'(s). A'(s) \subseteq A(s) and is further
  // restricted by the functor IsApplicableFunctor
  ActionConstRange applicableActions(state_t const& s) const override {
    ActionConstRange range = ssp_.applicableActions(s);
    ActionConstIteIfaceUniqPtr begin(
      new ApplicableActionIterator<ActionConstIteWrapper>(
                                  range.begin(), range.end(), *this, s));
    return ActionConstRange(std::move(begin), nullptr);
  }

  void expand(action_t const& a, state_t const& s, ProbDistStateIface& pr)
    const override
  {
    ssp_.expand(a, s, pr);
  }

  Rational cost(state_t const& s, action_t const& a) const override {
    Rational c = cost_(s,a);
    return (c > Rational(0) ? c : ssp_.cost(s,a));
  }

  Rational terminalCost(state_t const& s) const override {
    Rational c = terminal_cost_(s);
    return (c >= Rational(0) ? c : ssp_.terminalCost(s));
  }

  StateConstRange reachableStates() const override {
    return reachable_states_(*this);
  }

 private:
  SSPIface const& ssp_;
  std::string const name_;
  state_t const new_s0_;
  IsGoalFunctor is_goal_;
  IsApplicableFunctor is_applicable_;
  CostActionFunctor cost_;
  TerminalCostFunctor terminal_cost_;
  ReachableStateFunctor reachable_states_;
};


/******************************************************************************
 *
 * Template functions for constructing an SSPAdaptor (it allows template
 * parameters deduction
 *
 *****************************************************************************/
/*
 * Short-hand method to produce an SSPAdaptor that allows the usage of auto and
 * template deduction:
 *
 * auto ssp_prime = sspadaptor::makeadaptor("new", ssp.s0,
 *                                          SameGoals(),
 *                                          SameApplicableActions(),
 *                                          SameActionCost(),
 *                                          SameTerminalCost()
 *                                          OnDemandReachableStates());
 *
 * ssp_prime is now an sspadaptor equivalent to the original ssp.
 *
 * Notice that the compiler optimizes the value return (or inline this function)
 * and no copy or move constructor or assign operation is used. So, it should
 * have no overhead.
 */
template<typename A, typename B, typename C, typename D, typename E>
SSPAdaptor<A,B,C,D,E> makeSSPAdaptor(SSPIface const& ssp,
    std::string const& name, state_t const& new_s0, A const&  is_goal,
    B const& is_applicable, C const& cost, D const& terminal_cost,
    E const& reachable_states)
{
  return SSPAdaptor<A,B,C,D,E>(ssp, name, new_s0, is_goal, is_applicable,
      cost, terminal_cost, reachable_states);
}

// Same as above but allocates and returns a raw pointer to the new SSPAdaptor
template<typename A, typename B, typename C, typename D, typename E>
SSPAdaptor<A,B,C,D,E>* newSSPAdaptor(SSPIface const& ssp,
    std::string const& name, state_t const& new_s0, A const&  is_goal,
    B const& is_applicable, C const& cost, D const& terminal_cost,
    E const& reachable_states)
{
  return new SSPAdaptor<A,B,C,D,E>(ssp, name, new_s0, is_goal, is_applicable,
      cost, terminal_cost, reachable_states);
}


/******************************************************************************
 *
 * Most commonly used functors with SSPAdaptor
 *
 *****************************************************************************/
/*
 * Functors for SSPAdaptor representing no extra restriction.
 */
class SameGoals {
 public:
  bool operator()(state_t const&) const { return false; }
};
class SameApplicableActions {
 public:
  bool operator()(state_t const&, action_t const&) const { return true; }
};
class SameActionCost {
 public:
  Rational operator()(state_t const&, action_t const&) const {
    return Rational(-1);
  }
};
class SameTerminalCost {
 public:
  Rational operator()(state_t const&) const { return Rational(-1); }
};
class OnDemandReachableStates {
 public:
  StateConstRange operator()(SSPIface const& ssp) const {
    return wrappedReachableStates(ssp, ssp.s0());
  }
};


/******************************************************************************
 *
 * Trivial implementation of functors for SSPAdaptor
 *
 *****************************************************************************/

/*
 * For Goals
 */
// Consider every state in the given hashset of states as a new goal
class ExtraGoalsFromSet {
 public:
  ExtraGoalsFromSet(HashsetState& new_goals) : new_goals_(new_goals) { }
  bool operator()(state_t const& s) const {
    return new_goals_.find(s) != new_goals_.end();
  }
 private:
  HashsetState& new_goals_;
};


/*
 * For Applicable Actions
 */
// Denies the applicability of any action in the internal hash map.
//
class ActionsToIgnoreFromHash {
 public:
  ActionsToIgnoreFromHash(HashStateToActiontPtrs& pairs_hash)
    : pairs_hash_(pairs_hash)
  {}
  bool operator()(state_t const& s, action_t const& a) const {
    auto it = pairs_hash_.find(s);
    if (it != pairs_hash_.end()) {
      // if the action is NOT in the HashsetActiontPtr, then we return true to
      // forward the decision to the original ssp.
      return (it->second.find(&a) == it->second.end());
    }
    // state s has no restriction, so returning true to forward decision to the
    // original ssp
    return true;
  }
 private:
  HashStateToActiontPtrs& pairs_hash_;
};



/*
 * For Action Cost
 */
// If the given (state, action) is in the hash, then return its cost. Otherwise,
// returns -1 to flag the adaptor to use the original action cost.
class ActionCostFromHash {
 public:
  ActionCostFromHash(HashStateActionToRational& new_cost) : new_cost_(new_cost)
  {}
  Rational operator()(state_t const& s, action_t const& a) const {
    auto state_it = new_cost_.find(s);
    if (state_it != new_cost_.end()) {
      auto action_it = state_it->second.find(&a);
      if (action_it != state_it->second.end()) {
        // the current (state,action) has a value in the hash, thus this value
        // is returned.
        assert(action_it->second > Rational(0));
        return action_it->second;
      }
    }
    // the pair (s,a) is not in the hash, so returning -1 to use the original
    // cost of (s,a)
    return Rational(-1);
  }
 private:
  HashStateActionToRational& new_cost_;
};



/*
 * For TerminalCost
 */
// If the given state is in the hash, then return its cost. Otherwise, returns
// -1 to flag the adaptor to use the original cost.
class TerminalCostFromHash {
 public:
  TerminalCostFromHash(HashStateToRational& new_cost) : new_cost_(new_cost) { }
  Rational operator()(state_t const& s) const {
    auto it = new_cost_.find(s);
    if (it != new_cost_.end()) {
      assert(it->second >= Rational(0));
      return it->second;
    }
    return Rational(-1);
  }
 private:
  HashStateToRational& new_cost_;
};


// If the given state is defined in the hash_t, then return its cost. Otherwise,
// returns -1 to flag the adaptor to use the original cost
class TerminalCostFromV {
 public:
  TerminalCostFromV(hash_t const& v) : v_(v) { }
  Rational operator()(state_t const& s) const {
    hashEntry_t* it = v_.find(s);
    if (it) {
      assert(it->value() >= 0);
      return Rational(it->value());
    }
    // s is not defined in v_, so forwarding the request to the original SSP
    return Rational(-1);
  }
 private:
  hash_t const& v_;
};



/*
 * For ReachableStates
 */
class PrecomputedReachableStates {
 public:
  PrecomputedReachableStates(HashsetState const& set) : set_(set) { }
  StateConstRange operator()(SSPIface const& ssp) const {
    // Creating a StateConstIteIfaceUniqPtr using the specialized
    // HashsetStateConstIte that holds a pointer to the reachable states.
    StateConstIteIfaceUniqPtr begin(new HashsetStateConstIte(set_,
                                                             set_.begin()));
    return StateConstRange(std::move(begin), nullptr);
  }
 private:
  HashsetState const& set_;
};
#endif  // SSP_ADAPTOR_H
