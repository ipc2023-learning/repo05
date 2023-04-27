#ifndef HEURISTICS_DETERMINIZATION_BASED_ATOM_ABC_H
#define HEURISTICS_DETERMINIZATION_BASED_ATOM_ABC_H

#include "heuristic_iface.h"

#include "../ext/mgpt/global.h"
#include "../ext/mgpt/states.h"
#include "../utils/die.h"
#include "../ext/mgpt/problems.h"

#ifndef DEBUG_H_DET
#define DEBUG_H_DET 0
#endif

/*******************************************************************************
 *
 * Functor to sort actions by increasing cost
 *
 ******************************************************************************/
class ActionCostIncreasing {
 public:
  ActionCostIncreasing() : cost_idx_(ACTION_COST) { }
  ActionCostIncreasing(size_t cost_idx) : cost_idx_(cost_idx) { }
  bool operator()(action_t const* lh, action_t const* rh ) const {
    return (lh->cost(cost_idx_) < rh->cost(cost_idx_) );
  }
 private:
  size_t cost_idx_;
};

/*
 * determinizationBasedAtomHeuristic
 *
 * This class implements the basic methods to compute H_add, H_max and similar
 * heuristics for a probabilistic problem.
 *
 * The approach used is similar to Florent paper on AAAI'11 except that a
 * determinization is explicitly used as opposed to do the "on fly"
 * computation as suggested by that paper.
 *
 */
class determinizationBasedAtomHeuristic : public FactoredHeuristic
{
 public:
  determinizationBasedAtomHeuristic(problem_t const& problem,
      size_t cost_idx = ACTION_COST,
      bool use_self_loop_relaxation = false);
  virtual ~determinizationBasedAtomHeuristic();

  // heuristic_t interface
  virtual double computeValue(state_t const& s) = 0;
  // heuristic_t interface

 protected:
  // * Compute the cost of the set of atoms using atom_rp_cost. If this
  //   function returns the:
  //   - sum of all costs, then H_add is obtained
  //   - max of all costs, then H_add is obtained
  virtual double costSetOfAtoms(atomList_t const& atoms) const = 0;

  // * Compute the cost of each atom from state s using the h_add definition.
  // * The values are stored in atom_rp_cost and action_rp_cost
  // * If supporter != NULL, then supporter[i] is the index of operator_ptr_ of
  //   the action that minimizes the h_add cost to add atom(i).
  // * It is expected that |supporter| >= problem_t::number_atoms()
  void computeCostOfAtoms(state_t const& s,
                          deterministicAction_t const** supporter = NULL);

  // heuristic_t interface
  virtual double value(state_t const& s) = 0;

  // The index of the cost function being used
  size_t cost_idx_;
  // Dead end penalty for the target cost function
  double dead_end_value_;

  // Using  gpt::__strong_relaxation instead
  problem_t const* relaxation_;
//  const problem_t &relaxation_; // strong relaxation, i.e., result of removing
//                                // the deletes from the STRIPS representation
//                                // of the all-outcomes determinization of the
//                                // original problem

  // List of deterministic actions in the all-outcome determinization. During
  // the ctor, it will also be sorted according to the cost of the action,
  // from lower costs to higher costs
  std::vector<deterministicAction_t const*> operator_ptr_;
  // has_prec_[i] is the list of indexes of operator_ptr_ such that, for all
  // j \in has_prec_[i], atom(i) \in precondition(operator_ptr_[j])
  // Since operator_ptr_ is sorted according to the actions costs:
  //        cost(has_prec_[i][j]) <= cost(has_prec_[i][j+1])
  std::vector<std::list<size_t>> has_prec_;

  // Size: # of atoms
  // Position i: the cost of making atom(i) true in the current state
  // Note: Equivalent to g_s(atom(i)) in Florent paper
  double* atom_rp_cost;

  // Size: same as operator_ptr_
  // Position i: additive cost of making all atoms in the preconditions of
  //             action operator_ptr_[i] true
  // Note:
  //  - this is used for speed up purposes (caching) since action_rp_cost[i]
  //    equals sum_[j \in prec(operator_ptr_[i])] atom_rp_cost[j]
  //  - Equivalent to g_s(prec(operator_ptr_[i])) for the sum operator (h_add)
  //    in Florent paper
  double* action_rp_cost;
};



/*
 * Template for atom based heuristic computed on any determinization
 */
template<typename AggregationFunctor,
         bool use_self_loop_relax,
         char const* nameArg,
         char const* short_name>
class HDetAtomTemplate : public determinizationBasedAtomHeuristic {

 public:
  HDetAtomTemplate(problem_t const& problem, size_t cost_idx = ACTION_COST)
    : determinizationBasedAtomHeuristic(problem, cost_idx, use_self_loop_relax)
  {
    name_ = std::string(nameArg);
  }
  ~HDetAtomTemplate() { }

  double computeValue(state_t const& s) {
#if 1
    return value(s);
#else
    double val = value(s);
    std::cout << short_name << "(";
    s.full_print(std::cout, gpt::problem, true, false);
    std::cout << ") = " << val << std::endl;
    return val;
#endif
  }

 private:
  // Calls the aggregator function
  double costSetOfAtoms(atomList_t const& atoms) const override {
    double cost = aggregationFunctor_(atoms, atom_rp_cost);
    if (cost > dead_end_value_) {
      return dead_end_value_;
    }
    else {
      return cost;
    }
  }

  double value(state_t const& s) {
    if (relaxation_->goalT().holds(s, relaxation_->nprec())) {
      // This is a goal state in the relaxation, so returning 0
      return 0.0;
    }
    computeCostOfAtoms(s);
    double v = costSetOfAtoms(relaxation_->goalT().atom_list(0));
//    std::cout << short_name << "_{"
//              << gpt::problem->domain().functions().name(cost_idx_)
//              << "}(" << s.toStringFull(gpt::problem) << ") = "
//              << v << std::endl;
    return v;
  }

  // Aggregator functor. Changing this results in h-max or h-add
  AggregationFunctor aggregationFunctor_;
};
#endif  // HEURISTICS_DETERMINIZATION_BASED_ATOM_ABC_H
