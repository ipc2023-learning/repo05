#ifndef HEURISTICS_H_MAX
#define HEURISTICS_H_MAX

#include "determinization_based_atom_abc.h"

#include "../ext/mgpt/states.h"
#include "../ext/mgpt/problems.h"


/*
 * Aggregation functor for h-max
 */
struct MaxCostSetOfAtoms {
  double operator()(atomList_t const& atoms, double* const& atom_rp_cost) const {
    double cost = 0;
    for (size_t i = 0; i < atoms.size(); ++i) {
      ushort_t a = atoms.atom(i);
      if (atom_rp_cost[a] > cost) {
        cost = atom_rp_cost[a];
      }
    }
    return cost;
  }
};

/*
 * HMaxAllOutcomesDet
 */
static constexpr const char __h_max_allout_name[] = "h-max-all-out-det";
static constexpr const char __h_max_allout_name_short[] = "H-max";
using HMaxAllOutcomesDet = HDetAtomTemplate<MaxCostSetOfAtoms,
                                            false,  // no self-loop relax
                                            __h_max_allout_name,
                                            __h_max_allout_name_short>;

/*
 * HMaxSelfLoop
 */
static constexpr const char __h_max_selfloop_name[] = "h-max-selfloop";
static constexpr const char __h_max_selfloop_name_short[] = "H-max-sl";
using HMaxSelfLoop = HDetAtomTemplate<MaxCostSetOfAtoms,
                                      true,  // using self-loop relax
                                      __h_max_selfloop_name,
                                      __h_max_selfloop_name_short>;

#endif // HEURISTICS_H_MAX
