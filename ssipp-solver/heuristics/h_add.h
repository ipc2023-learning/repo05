#ifndef HEURISTICS_H_ADD
#define HEURISTICS_H_ADD

#include "determinization_based_atom_abc.h"

#include "../ext/mgpt/states.h"
#include "../ext/mgpt/problems.h"


/*
 * Aggregation functor for h-add
 */
struct SumCostSetOfAtoms {
  double operator()(atomList_t const& atoms, double* const& atom_rp_cost) const {
    double cost = 0;
    for (size_t i = 0; i < atoms.size(); ++i) {
      ushort_t a = atoms.atom(i);
      cost += atom_rp_cost[a];
    }
    return cost;
  }
};

/*
 * HAddAllOutcomesDet
 */
static constexpr const char __h_add_allout_name[] = "h-add-all-out-det";
static constexpr const char __h_add_allout_name_short[] = "H-add";
using HAddAllOutcomesDet = HDetAtomTemplate<SumCostSetOfAtoms,
                                            false,  // no self-loop relax
                                            __h_add_allout_name,
                                            __h_add_allout_name_short>;

/*
 * HAddSelfLoop
 */
static constexpr const char __h_add_selfloop_name[] = "h-add-selfloop";
static constexpr const char __h_add_selfloop_name_short[] = "H-add-sl";
using HAddSelfLoop = HDetAtomTemplate<SumCostSetOfAtoms,
                                      true,  // using self-loop relax
                                      __h_add_selfloop_name,
                                      __h_add_selfloop_name_short>;

#endif // HEURISTICS_H_ADD
