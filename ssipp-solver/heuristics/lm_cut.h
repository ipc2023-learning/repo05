#ifndef HEURISTIC_LMCUT_H
#define HEURISTIC_LMCUT_H

#include <iostream>
#include <bitset>
#include <vector>
#include <algorithm>  // std::min

#include "determinization_based_atom_abc.h"
#include "h_max.h"

#include "../ext/mgpt/states.h"
#include "../ext/mgpt/problems.h"

using CutList = std::vector<std::set<const action_t*>>;

#define BOOST_DYNAMIC_BITSET_DONT_USE_FRIENDS
#include <boost/dynamic_bitset.hpp>
#include <boost/functional/hash.hpp>
using DynBitSet = boost::dynamic_bitset<>;
namespace std {
  template<> struct hash<DynBitSet> {
    size_t operator()(DynBitSet const& bs) const {
      size_t rv = 0;
      boost::hash_combine(rv, bs.m_num_bits);
      boost::hash_combine(rv, bs.m_bits);
      return rv;
    }
  };
};

struct CutResult {
  double value;
  CutList cuts;
};

/*******************************************************************************
 *
 * LM-Cut Heuristic
 *
 * Adapted from Patrik's HSP (http://users.cecs.anu.edu.au/~patrik/un-hsps.html)
 * specifically CostTable::compute_lmcut
 *
 ******************************************************************************/
class LMCutHeuristic : public determinizationBasedAtomHeuristic {
 public:
  LMCutHeuristic(problem_t const& problem, size_t cost_idx = ACTION_COST);
  ~LMCutHeuristic() { }

  /*
   * heuristic_t interface
   */
  double computeValue(state_t const& s) override { return value(s); }

  CutResult valueAndCuts(state_t const& s) {
    if (relaxation_->goalT().holds(s, relaxation_->nprec())) {
      return CutResult {value: 0.0, cuts: CutList {}};
    }
    return computeLMCut(s, relaxation_->goalT().atom_list(0), true);
  }

 protected:

  /*
   * heuristic_t interface
   */
  double value(state_t const& s) override {
    if (relaxation_->goalT().holds(s, relaxation_->nprec())) {
      // This is a goal state in the relaxation, so returning 0
      return 0.0;
    }
    double v = computeLMCut(s, relaxation_->goalT().atom_list(0)).value;
    // std::cout << "Hlmcut_{" << gpt::problem->domain().functions().name(cost_idx_)
    //           << "}(" << s.toStringFull(gpt::problem) << ") = " << v
    //           << std::endl;
    return v;
  }


  /*
   * determinizationBasedAtomHeuristic interface
   */
  double costSetOfAtoms(atomList_t const& atoms) const override {
    return std::min(maxCost(atoms, atom_rp_cost), dead_end_value_);
  }

 private:

  /*
   * Computes the LMCut heuristic from s to make all atoms in target_atoms true
   */
  CutResult computeLMCut(state_t const& s, atomList_t const& target_atoms, bool save_cut = false);

  void extendedGoals(atomList_t const& target_atoms,
                           std::vector<double> const& action_cost,
                           DynBitSet& rv) const
  {
    extendedGoalsRec(target_atoms, action_cost, rv);
  }

  void extendedGoalsRec(atomList_t const& target_atoms,
                        std::vector<double> const& action_cost,
                        DynBitSet& ext_goal_set) const;

  DynBitSet findCut(DynBitSet const& ext_goal_set,
                            std::vector<double> const& action_cost) const;

  // If allowed_actions is nullptr, then all actions are considered
  void computeCostOfAtomsForConstrainedActionSet(
                              state_t const& s,
                              std::vector<double> const& action_cost,
                              DynBitSet const* allowed_actions = nullptr);

  /*
   * Members
   */
  // Vectors for caching the preconditions and add effects of actions as bitset.
  // This is useful because speed up intersection operations.
  int n_actions, n_atoms;
  std::vector<DynBitSet> prec_op_vbits_;
  std::vector<DynBitSet> adds_op_vbits_;
  MaxCostSetOfAtoms maxCost;
};

#endif  // HEURISTIC_LMCUT_H
