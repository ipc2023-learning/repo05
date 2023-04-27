#ifndef SSPS_PPDDL_ADAPTORS_H
#define SSPS_PPDDL_ADAPTORS_H

#include "ssp_iface.h"

class problem_t;

/*
 * Adaptor class from problem_t to SSPIface
 */
class SSPfromPPDDL : public SSPIface {
 public:
  SSPfromPPDDL(problem_t const& problem) : p_(problem) { }

  // It was already a method from problem_t
  std::string const& name() const override { return p_.name(); }

  state_t const& s0() const override { return p_.s0(); }

  bool isGoal(state_t const& s) const override { return p_.goal().holds(s); }

  bool hasApplicableActions(state_t const& s) const override {
    auto range = applicableActions(s);
    return range.begin() != range.end();
  }

  bool isApplicable(state_t const& s, action_t const& a) const override {
    return !isGoal(s) && a.enabled(s);
  }

  class MgptActionTIteratorWrapper {
   public:
    MgptActionTIteratorWrapper(actionList_t::const_iterator it) : it_(it) { }
    MgptActionTIteratorWrapper& operator++() { ++it_; return *this; }
    bool operator!=(MgptActionTIteratorWrapper const& rhs) const {
      return it_ != rhs.it_;
    }
    action_t const& operator*() const { assert(*it_); return **it_; }
   private:
    actionList_t::const_iterator it_;
  };

  ActionConstRange applicableActions(state_t const& s) const override {
    // Returning an instance of **ApplicableActionIterator**, therefore, the
    // iterator will take care of checking if the actions are applicable or
    // not.
    ActionConstIteIfaceUniqPtr begin(
        new ApplicableActionIterator<MgptActionTIteratorWrapper>(
          MgptActionTIteratorWrapper(p_.actionsT_.begin()),
          MgptActionTIteratorWrapper(p_.actionsT_.end()),
                                         *this, s));
    return ActionConstRange(std::move(begin), nullptr);
  }

  /*
   * expand populates the ProbDist pr with the reachable states s' from s when
   * applying action a and their probabilities, i.e., P(s'|s,a). NOTICE that
   * there is no guarantee that each event of pr is disjoint, i.e., the same
   * state s' might appear more than once and P(s'|s,a) equals the sum of the
   * prob of all the entries associated with s'. This guarantee of disjoint
   * event is given by the ProbDistStateIface passed to expand (map
   * implementations will give such guarantee while vector implementation
   * won't).
   *
   * GUARANTEE: expand WILL clear the pr before populating it.
   *
   * It was already a method from problem_t
   */
  void expand(action_t const& a, state_t const& s, ProbDistStateIface& pr)
    const override
  {
    p_.expand(a, s, pr);
  }

  Rational cost(state_t const& s, action_t const& a) const override {
    return a.cost(s);
  }

  Rational terminalCost(state_t const& s) const override {
    return p_.terminalCost(s);
  }

  StateConstRange reachableStates() const override {
    return wrappedReachableStates(*this, s0());
  }

 private:
  problem_t const& p_;
};

#endif  // SSPS_PPDDL_ADAPTORS_H
