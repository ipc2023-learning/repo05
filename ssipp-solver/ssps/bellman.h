#ifndef BELLMAN_H
#define BELLMAN_H
#pragma once

#include <iostream>

#include "../utils/die.h"
#include "../ext/mgpt/global.h"       // gpt::dead_end_value
#include "../utils/exceptions.h"   // PlannerGaveUpException

#include "ssp_iface.h"
#include "prob_dist_state.h"
#include "../ext/mgpt/hash.h"
#include "../ext/mgpt/atom_states.h"

/*
 * Functors for approving successor states when computing q-values for
 * evaluation (constGreedyAction and constQValue)
 */
class AlwaysAcceptStateFunctor {
 public:
  bool operator()(state_t const& s) {
    return true;
  }
};

class AlwaysAcceptActionFunctor {
 public:
  bool operator()(state_t const& s, action_t const& a) {
    return true;
  }
};
/******************************************************************************/

/*
 * The Bellman namespace contains all the standard Bellman-related operators
 * (e.g., Q-Value, Greedy action). They are in a namespace to avoid conflict
 * with specialized implementations of the same methods (if any).
 *
 * All the method bellow have as their last argument a value function (i.e., 
 * hash_t object) and an SSP which are used as "context". Since these parameters
 * usually do not change while solving a given SSP, the following is a useful
 * trick:
 *
 * auto qValue = std::bind(Bellman::qValue, std::placeholders_1,
 *                          std::placeholders_2, hash, ssp);
 *
 * Now qValue is a local reference to function with 2 paramaters: s and a
 */
namespace Bellman {
  /*
   * qValue: computes Q(s,a). It uses V(s'), therefore, if P(s'|s,a) > 0 and
   * V(s') is not defined, then H(s') is used and H(s') is assigned to V(s').
   * The V(s') <- H(s') assignment side-effect can be turned off by making
   * gpt::hash_all false.
   *
   * Formally, the q-value is associated to the following definition of the
   * Bellman operator B:
   * BV(s) = C_t(s)       if s \in G
   *       = min_a Q(s,a) if s \not \in G
   *          = min_a C(s,a) + \sum_s' P(s'|s,a) V(s')
   *
   * Assumptions:
   *  - a in A(s)
   *
   * Guarantees:
   *  - Returned value is <= dead_end_value
   *
   * Returns:
   *  - min{ Q(s,a), dead_end_value }
   */
  inline double qValue(state_t const& s, action_t const& a, hash_t& v,
      SSPIface const& ssp)
  {
    DIE(a.enabled(s), "Action not applicable in the given state", 167);
    double q_value = ssp.cost(s, a).double_value();

    // qValue can be called recursively. This is specially true if the
    // heuristic used also call qValue. For instance, in the call of qValue for
    // an SSP, we might have to compute H-min-min(s') and this can be computed
    // using VI/LRTDP/etc over the all outcomes determinization, resulting in
    // another call to qValue.
#define MAX_DEPTH_Q_VALUE 4
    static ProbDistState pr_array[MAX_DEPTH_Q_VALUE];
    static size_t pr_idx = 0;
#if not defined NDEBUG
    FANCY_DIE_IF(pr_idx >= MAX_DEPTH_Q_VALUE, 171,
        "Max depth of qValue was reached. Increased its value. Current value "
        "is %d.", MAX_DEPTH_Q_VALUE);
#endif

    ProbDistState& pr = pr_array[pr_idx];
    pr_idx++;
    ssp.expand(a, s, pr);
    for (auto const& it : pr) {
      state_t const& s_prime = it.event();
      double const prob_s_prime = it.prob();
      /*
       * NOTE(fwt): if s_prime is a terminal state then V(s) = C_t(s) by
       * definition. Thus we handle this special case here.
       *
       * OPTIMIZATION: If there is no need to handle terminal costs, then this
       * could be removed to obtain a small performance improvement.
       */
      if (ssp.isGoal(s_prime))
        q_value += prob_s_prime * ssp.terminalCost(s_prime).double_value();
      else
        q_value += prob_s_prime * v.value(s_prime);
    }
    pr_idx--;
    // qvalue could be greater than dead_end_value if all states it reaches are
    // dead ends, so:
    //           Q(s,a) = C(s,a) + dead_end_value > dead_end_value
    return std::min(q_value, gpt::dead_end_value.double_value());

  }

  /*
   * greedyActionAndMinQValue finds the greedy action to be applied in s w.r.t.
   * to the current hash. It used the qValue function, so it has side-effect on
   * the hash (see comments on qValue). Use constGreedyAction for a similar
   * version of this function without side-effects on the hash.
   *
   * This method also takes a functor that is used to approve the applicable
   * actions. That is, if a \in A(s) and accept_action(s,a) is FALSE, then the
   * action a is ignored. This is useful for planners that restrict the action
   * space (e.g., FTVI).
   *
   * If there are tied actions, the first one is returned. This should be fine
   * since actions are being randomized after parsing in problem_t::flatten().
   *
   * Def.: A'(s) is the effective applicable actions on s, i.e.,
   *                   A'(s) = A(s) \setminus accept(s,-)
   * Returns:
   *  = <nullptr, 0> if s is a goal state
   *  = <nullptr, dead_end_value> if:
   *    - A'(s) is empty
   *    - For all a \in A'(s), Q(s,a) >= dead_end_value
   *  = <a_g, q_min> otherwise, where:
   *    - q_min := min_{a \in A'(s)} min{Q(s,a), dead_end_value}
   *    - a_g is the argmin version of q_min
   *
   * Guarantees:
   *  - q_min <= dead_end_value (inherited from qValue)
   */
  template<typename AcceptActionFunctor>
  std::pair<action_t const*, double> greedyActionAndMinQValueFiltered(
      state_t const& s, hash_t& hash, SSPIface const& ssp,
      AcceptActionFunctor accept_action)
  {
    if (ssp.isGoal(s)) {
      double terminal_cost = ssp.terminalCost(s).double_value();
      return std::make_pair(nullptr, terminal_cost);
    }
    action_t const* greedy_a = nullptr;
    double min_q_value = gpt::dead_end_value.double_value();

    for (auto const& a : ssp.applicableActions(s)) {
      double q_value = qValue(s, a, hash, ssp);
      if (q_value < min_q_value) {
        greedy_a = &a;
        min_q_value = q_value;
      }
    }  // for each applicable action
    return std::make_pair(greedy_a, min_q_value);
  }


  /*
   * Useful shorthands
   */
  inline std::pair<action_t const*, double> greedyActionAndMinQValue(
      state_t const& s, hash_t& hash, SSPIface const& ssp)
  {
    return greedyActionAndMinQValueFiltered(s, hash, ssp,
                                                  AlwaysAcceptActionFunctor());

  }

  inline action_t const* greedyAction(state_t const& s, hash_t& hash,
      SSPIface const& ssp)
  {
    action_t const* a = nullptr;
    std::tie(a, std::ignore) = greedyActionAndMinQValueFiltered(s, hash, ssp,
                                                  AlwaysAcceptActionFunctor());
    return a;
  }

  inline double minQValue(state_t const& s, hash_t& hash, SSPIface const& ssp)
  {
    double min_q_value = 0;
    std::tie(std::ignore, min_q_value) = greedyActionAndMinQValueFiltered(s,
                                       hash, ssp, AlwaysAcceptActionFunctor());
    return min_q_value;
  }


  /*
   * Performs an in-place Bellman update. It returns the new greedy action and
   * min q-value
   */
  inline std::pair<action_t const*, double> update(state_t const& s,
      hash_t& hash, SSPIface const& ssp)
  {
    auto pair = greedyActionAndMinQValueFiltered(s,
                                       hash, ssp, AlwaysAcceptActionFunctor());
    hash.update(s, pair.second);
    return pair;
  }

  /*
   * Computes the Bellman residual. If update is true, it also performs an
   * update
   */
  inline double residual(state_t const& s, hash_t& hash, SSPIface const& ssp,
      bool update = false)
  {
    double min_q_value = minQValue(s, hash, ssp);
    double residual = fabs(hash.value(s) - min_q_value);
    if (update)
      hash.update(s, min_q_value);
    return residual;
  }

  /*
   * Equivalent methods for CONST HASH_T
   *
   * FWT: all methods have const in their name because of the different
   * signature.
   */
  /*
   * Auxiliary method for constGreedyAction.
   *
   * It computes Q(s,a). If f P(s'|s,a) > 0 and V(s') is not defined, then
   * H(s') is used BUT V(s') is not set as H(s') since this is a const method.
   *
   * Assumptions:
   *  - a in A(s)
   *
   * Guarantees:
   *  - Returned value is <= dead_end_value
   *  - hash is not changed
   *
   * Returns:
   *  - dead_end_value if P(s'|s,a) > 0 and accept_state(s') is FALSE. Notice
   *    that this should not be a problem when trying to find the greedy action
   *    because if, for all a, Q(s,a) is dead_end_value, then it doesn't matter
   *    the reason (either due to real cost or failure to be approved) as s will
   *    be considered a dead end and no action should be applied.
   *  - min{ Q(s,a), dead_end_value } otherwise
   */
  template<typename AcceptStateFilterFunctor>
  double constQValue(state_t const& s, action_t const& a, hash_t const& v,
      SSPIface const& ssp, AcceptStateFilterFunctor accept_state)
  {
    DIE(a.enabled(s), "Action not applicable in the given state", 167);

    double q_value = ssp.cost(s, a).double_value();

#if not defined NDEBUG
    static bool pr_in_use = false;
    FANCY_DIE_IF(pr_in_use, 171, "Turns out const qValue can actually be "
        "recursively called too. Need to implement this case for the "
        "ProbDistIface approach");
    pr_in_use = true;
#endif

    static ProbDistState pr;
    ssp.expand(a, s, pr);
    for (auto const& ip : pr) {
      state_t const& s_prime = ip.event();
      if (!accept_state(s_prime)) {
        q_value = gpt::dead_end_value.double_value();
        break;
      }
      // See the equivalent comment in the non-const version of qValue
      if (ssp.isGoal(s_prime))
        q_value += ip.prob() * ssp.terminalCost(s_prime).double_value();
      else
        q_value += ip.prob() * v.value(s_prime);
    }
#if not defined NDEBUG
    pr_in_use = false;
#endif
    // qvalue could be greater than dead_end_value if all states it reaches are
    // dead ends, so:
    //           Q(s,a) = C(s,a) + dead_end_value > dead_end_value
    return std::min(q_value, gpt::dead_end_value.double_value());
  }


  // Shorthand for the always accept state case
  inline double constQValue(state_t const& s, action_t const& a,
      hash_t const& hash, SSPIface const& ssp)
  {
    return constQValue(s, a, hash, ssp, AlwaysAcceptStateFunctor());
  }

  /*
   * constGreedyAction find the greedy action to be applied in s w.r.t. to the
   * current hash. It is supposed to be used during EVALUATION of a planner,
   * i.e., when no changes in the data structures are allowed. Main client of
   * this method if eval_planner through planner_t::next() const.
   *
   * This method also takes a functor that is used to approve the successor 
   * states of each action a. That is, if P(s'|s,a) > 0 and accept_state(s) is
   * FALSE, then the action a is ignored. This is useful for planners that have
   * an internal data structure that prevents states from being visited (e.g.,
   * solved flag in LRTDP).
   *
   * If there are tied actions, the first one is returned. This should be fine
   * since actions are being randomized after parsing in problem_t::flatten().
   *
   * Assumptions: the state s is defined in the current hash (this is more an
   * evaluation assumption than this method assumption).
   *
   * Returns:
   *  = nullptr if
   *    - s is a goal state
   *    - s has no applicable actions (trivial dead-end)
   *    - Q(s,a) >= dead_end_value for all a in A(s) s.t. all successor of
   *                                        (s,a) are approved by accept_state.
   *  = non-nullptr otherwise
   *
   */
  template<typename AcceptStateFilterFunctor>
  action_t const* constGreedyAction(state_t const& s, hash_t const& hash,
      SSPIface const& ssp, AcceptStateFilterFunctor accept_state)
  {
    if (ssp.isGoal(s) || !ssp.hasApplicableActions(s))
      return nullptr;
    if (!hash.find(s))
      throw PlannerGaveUpException();

    action_t const* greedy_a = nullptr;
    double min_q_value = gpt::dead_end_value.double_value();

    for (auto const& a : ssp.applicableActions(s)) {
      double q_value = constQValue(s, a, hash, ssp, accept_state);
      if (q_value < min_q_value) {
        greedy_a = &a;
        min_q_value = q_value;
      }
    }  // for each applicable action
    return greedy_a;
  }

  // Shorthand for the always accept state case
  inline action_t const* constGreedyAction(state_t const& s, hash_t const& hash,
      SSPIface const& ssp)
  {
    return constGreedyAction(s, hash, ssp, AlwaysAcceptStateFunctor());
  }

}

#endif  // BELLMAN_H
