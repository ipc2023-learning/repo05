#ifndef SSP_IFACE_H
#define SSP_IFACE_H

#include <iostream>
#include <memory>

#include "../ext/mgpt/rational.h"
#include "prob_dist_state.h"
#include "ssp_utils.h"
#include "../utils/iterator_wrapper.h"

class state_t;
class action_t;


/*
 * Instantiating iterator templates from iterator_wrapper.h
 */

// iterator for action_t const&
using ActionConstIteIfaceUniqPtr = ConstIteratorIfaceUniqPtr<action_t const&>;
using ActionConstIteIface = ConstIteratorIface<action_t const&>;
using ActionConstIteWrapper = ConstIteratorWrapper<action_t const&>;
using ActionConstRange = ConstRange<action_t const&>;

// iterator for state_t const&
using StateConstIteIfaceUniqPtr = ConstIteratorIfaceUniqPtr<state_t const&>;
using StateConstIteIface = ConstIteratorIface<state_t const&>;
using StateConstIteWrapper = ConstIteratorWrapper<state_t const&>;
using StateConstRange = ConstRange<state_t const&>;


/*
 * Represents an SSP = <S, s_0, G, A, P, C, C_t> where:
 *
 *  - S: set of states
 *  - s_0 \in S: initial state
 *  - G \subset S: set of goal states
 *  - A: set of actions. A(s) represents the actions applicable in s
 *  - P: probability distribution of successor state, i.e., P(s'|s,a)
 *  - C: S x A -> R*+: cost of applying action a on state s. C(s,a) > 0.
 *  - C_t: S -> R+: cost of reaching a state s \in G. C_t(s) >= 0.
 *
 */
class SSPIface {
 public:
  virtual ~SSPIface() { }

  virtual std::string const& name() const = 0;

  // Returns s0, the initial state
  virtual state_t const& s0() const = 0;

  // Returns true if s \in G
  virtual bool isGoal(state_t const& s) const = 0;


  // Returns true if |A(s)| > 0
  virtual bool hasApplicableActions(state_t const& s) const = 0;

  // Returns true if a in A(s)
  virtual bool isApplicable(state_t const& s, action_t const& a) const = 0;

  // Returns the range of all actions applicable in s. To loop over them, do:
  // for (auto const& action : applicableActions(s)) { <code> }
  virtual ActionConstRange applicableActions(state_t const& s) const = 0;


  /*
   * Populates pr with P(.|s,a)
   *
   * Expand populates the ProbDist pr with the reachable states s' from s when
   * applying action a and their probabilities, i.e., P(s'|s,a). NOTICE that
   * there is no guarantee that each event of pr is disjoint, i.e., the same
   * state s' might appear more than once and P(s'|s,a) equals the sum of the
   * prob of all the entries associated with s'. This guarantee of disjoint
   * event is given by the ProbDistStateIface passed to expand (map
   * implementations will give such guarantee while vector implementation
   * won't).
   *
   * GUARANTEE: expand WILL clear the pr before populating it.
   */
  virtual void expand(action_t const& action, state_t const& state,
                      ProbDistStateIface& pr) const = 0;

  // Returns the cost of the action, i.e., the cost being optimized.
  virtual Rational cost(state_t const& s, action_t const& a) const = 0;

  // Returns C_t(s). C_t(s) must be greater or EQUAL to 0.
  virtual Rational terminalCost(state_t const& s) const = 0;

  /*
   * Generate a hash set with all the reachable states from s0.
   *
   * Notice that this function should not be used very often since it can
   * potentially generate the whole state space, even if there are some states
   * that can be trivially shown to not be part of the optimal policy envelop.
   *
   * FWT: In most cases, this function is just a wrapper for reachableStatesFrom
   * (ssp_utils.h). The only reason for this function to be here is to improve
   * the efficiency of solving subproblems represented by an SSPAdaptor or other
   * extensions of SSPIface.  Almost always, in the process of generating a set
   * of new goal states, the set of reachable states of that subproblem is also
   * obtained. Thus, instead of computing this space twice, we can computed only
   * once by overloading this method in the SSPAdaptor with a functor.
   *
   * FWT: A different approach would be to make reachableStatesFrom a template
   * function over the ssp class and provide general implementation. Then
   * subproblems would provide their own specialization. Probably this template
   * approach is more elegant but can be too verbose, while the current approach
   * is really straightforward with the only downside of polluting the SSPIface
   * interface (and an extra virtual call when reachableStates is used).
   */
  virtual StateConstRange reachableStates() const = 0;
};


/* 
 * Wrapper around HashsetState. This wrapper can be used for a given
 * HashsetState const ref, in which case the reference has to be valid for the
 * life time of this object. Another usage is to wrap reachableStatesFrom
 * (ssp_utils.h) and keep the returned HashsetState valid until all iterators
 * are deallocated.
 */
class HashsetStateConstIte : public StateConstIteIface {
 public:
  // Ctor for the begin of the HashsetState obtained from reachableStatesFrom.
  // The iterators will share an pointer to the object
  HashsetStateConstIte(SSPIface const& ssp, state_t const& s)
    : reachable_states_sptr_(new HashsetState(std::move(reachableStatesFrom(ssp, s)))),
      reachable_states_(*reachable_states_sptr_),
      hash_it_(reachable_states_.begin())
  { }


  // Ctor for a pre-computed HashsetState. Notice that this case relies in the
  // fact that set will be deallocated only after all the HashsetStateConstIte
  // using it are deallocated. This is useful for instance to SCC-based methods
  // that pre-compute the SCCs.
  HashsetStateConstIte(HashsetState const& set,
                       HashsetState::const_iterator hash_it)
    : reachable_states_sptr_(nullptr), reachable_states_(set), hash_it_(hash_it)
  { }

  HashsetStateConstIte& operator++() { ++hash_it_; return *this; }
  bool operator!=(HashsetStateConstIte const& rhs) const {
    return hash_it_ != rhs.hash_it_;
  }
  bool operator!=(StateConstIteIfaceUniqPtr const& rhs) const {
    // Hack for efficiency: nullptr represents the end of the range
    if (!rhs)  return hash_it_ != reachable_states_.end();

    HashsetStateConstIte const* conv_rhs =
                      dynamic_cast<HashsetStateConstIte const*>(rhs.get());
    if (conv_rhs) {
      return operator!=(*conv_rhs);
    }
    else {
      return true;
    }
  }
  state_t const& operator*() const { return *hash_it_; }
  StateConstIteIfaceUniqPtr clone() const {
    return StateConstIteIfaceUniqPtr(
        new HashsetStateConstIte(reachable_states_sptr_, reachable_states_,
                                  hash_it_));
  }

 private:
  /*
   * Ctor for the clone method. It assumes that rs_sptr points to
   * reachable_states. This is always true for the life time of
   * HashsetStateConstIte and thus it is safe to use this constructor internally
   * (private). This is more efficient than having an if clause in clone to
   * choose between 2 different constructors.
   */
  HashsetStateConstIte(std::shared_ptr<HashsetState const> rs_sptr,
                       HashsetState const& reachable_states,
                       HashsetState::const_iterator hash_it)
    : reachable_states_sptr_(rs_sptr),
      reachable_states_(reachable_states),
      hash_it_(hash_it)
  { }

  std::shared_ptr<HashsetState const> reachable_states_sptr_;
  HashsetState const& reachable_states_;
  HashsetState::const_iterator hash_it_;
};


/*
 * Wrapper for the reachableStatesFrom that returns a StateConstRange to be used
 * in the implementations of SSPIface
 */
inline
StateConstRange wrappedReachableStates(SSPIface const& ssp, state_t const& s) {
//  // Computing the set of reachable states and moving it (efficient operation)
//  // to a newly allocated hashset managed by a shared_ptr
//  std::shared_ptr<HashsetState> reachable(new HashsetState(
//                                 std::move(reachableStatesFrom(ssp, s))));
  // Creating a StateConstIteIfaceUniqPtr using the specialized
  // HashsetStateConstIte that holds a pointer to the reachable states.
  StateConstIteIfaceUniqPtr begin(new HashsetStateConstIte(ssp, s));
  return StateConstRange(std::move(begin), nullptr);
}


/*
 * Template that implements the iterator over applicable actions of s.
 *
 * This is provided since it is the should be the most common implementation
 * of ActionConstIteIface. Iterator is any iterator (provides *, prefix ++ and
 * !=).
 *
 * NOTE: clone() must be overloaded if ApplicableActionIterator is extended
 * (e.g., to represent a subset of A(s)), otherwise the cloned iterators will
 * be objects from ApplicableActionIterator and functionality will be lost.
 *
 */
template<typename Iterator>
class ApplicableActionIterator : public ActionConstIteIface {
 public:
  ApplicableActionIterator(Iterator const& it, Iterator const& end,
                           SSPIface const& ssp, state_t const& s)
    : it_(it), end_(end), ssp_(ssp), s_(s)
  {
    init();
  }

  bool acceptAction(state_t const& s, action_t const& a)  {
    return ssp_.isApplicable(s, a);
  }

  ApplicableActionIterator& operator++() {
    ++it_;
    while (it_ != end_ && !acceptAction(s_, operator*())) {
      ++it_;
    }
    return *this;
  }

  bool operator!=(ApplicableActionIterator const& rhs) const {
    // It doesn't make sense to compare iterators of different actions or even
    // different SSPs, so putting this asserts just to double check.
    // 
    // SSPIface doesn't provide operator==, we can't do 
    // assert(ssp_ == conv_rhs->ssp_); 
    assert(! (end_ != rhs.end_));
    return it_ != rhs.it_;
  }

  bool operator!=(ActionConstIteIfaceUniqPtr const& rhs) const {
    // Hack for efficiency: nullptr represents the end of the range
    if (!rhs)  return it_ != end_;

    ApplicableActionIterator const* conv_rhs =
                      dynamic_cast<ApplicableActionIterator const*>(rhs.get());
    if (conv_rhs) {
      return operator!=(*conv_rhs);
    }
    else {
      return true;
    }
  }

  action_t const& operator*() const { return *it_; }

  ActionConstIteIfaceUniqPtr clone() const {
    return ActionConstIteIfaceUniqPtr(
        new ApplicableActionIterator(it_, end_, ssp_, s_));
  }

 private:
  // Finds the first valid entry
  void init() {
    if (it_ != end_ && !acceptAction(s_, operator*())) {
      // The increment operator will forward it_ until we find an executable
      // action or it_ == end
      operator++();
    }
  }

  Iterator it_, end_;
  SSPIface const& ssp_;
  state_t const& s_;
};

#endif  // SSP_IFACE_H
