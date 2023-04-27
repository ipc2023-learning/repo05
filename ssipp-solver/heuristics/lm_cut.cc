#include <iostream>
#include <limits>
#include <vector>

#include "lm_cut.h"
#include "../utils/die.h"
#include "../ext/mgpt/global.h"



LMCutHeuristic::LMCutHeuristic(problem_t const& problem, size_t cost_idx)
  : determinizationBasedAtomHeuristic(problem, cost_idx, false),
    // use operator_ptr_.size() to get number of actions, and
    // problem.number_atoms() to get atom count.
    n_actions(operator_ptr_.size()), n_atoms(problem.number_atoms()),
    prec_op_vbits_(n_actions), adds_op_vbits_(n_actions)
{
  name_ = "lm-cut heuristic";
//  std::cout << "[LMCut] number of atoms in the problem: "
//            << problem.number_atoms() << std::endl;

  for (auto &it : prec_op_vbits_) {
    it.resize(n_atoms);
  }
  for (auto &it : adds_op_vbits_) {
    it.resize(n_atoms);
  }

  for (size_t i = 0; i < operator_ptr_.size(); ++i) {
    deterministicAction_t const& op = *operator_ptr_[i];
    // Prec
    prec_op_vbits_[i].reset();
    atomList_t const& prec = op.precondition().atom_list(0);
    for (size_t j = 0; j < prec.size(); ++j) {
      prec_op_vbits_[i][prec.atom(j)] = true;
    }
    // Adds
    adds_op_vbits_[i].reset();
    atomList_t const& adds = op.effect().s_effect().add_list();
    for (size_t j = 0; j < adds.size(); ++j) {
      adds_op_vbits_[i][adds.atom(j)] = true;
    }
  }
}



CutResult LMCutHeuristic::computeLMCut(state_t const& s,
                                       atomList_t const& target_atoms,
                                       bool save_cut)
{
//  std::cout << "[LMCut] =======" << std::endl;
  size_t const n_relax_actions = operator_ptr_.size();
  CutResult rv;

  // Each iteration of LM-Cuts shifts the costs of action, therefore we need to
  // represent it separately and initially all actions have their original costs
  std::vector<double> action_cost(n_relax_actions, 0);
  for (size_t ia = 0; ia < n_relax_actions; ++ia) {
    action_cost[ia] = operator_ptr_[ia]->cost(cost_idx_).double_value();
    // All actions should be STRIPS actions, i.e., deterministic and without
    // conditionals. LMCuts is base on this assumption
    assert(operator_ptr_[ia]->effect().c_effect().size() == 0);
  }

  // Populates atom_rp_cost and action_rp_cost with the H_1 cost (i.e., single
  // atom costs)
  computeCostOfAtomsForConstrainedActionSet(s, action_cost);

  // Test if the goal is reachable
  if (maxCost(target_atoms, atom_rp_cost) >= dead_end_value_) {
    rv.value = dead_end_value_;
    return rv;
  }


  DynBitSet allowed_actions(n_actions);

  double total_cost = 0;
  DynBitSet ext_goal_set(n_atoms);  // Declared here to avoid alloc/dealloc
  // size_t ite = 0;
  while (maxCost(target_atoms, atom_rp_cost) > 0) {
    // DEBUG
    //    ite++;
//    std::cout << "[LMCut] ite = " << ite << " cost(target) = "
//              << maxCost(target_atoms, atom_rp_cost) << std::endl;

    ext_goal_set.reset();
    extendedGoals(target_atoms, action_cost, ext_goal_set);
//    std::cout << "[LMCut] ext_goal_set (size = " << ext_goal_set.count()
//              << ")= {";
//    for (size_t i = 0; i < problem_.number_atoms(); ++i) {
//      if (ext_goal_set[i]) {
//        std::cout << i << ", ";
//      }
//    }
//    std::cout << "}" << std::endl;

    // Setting all bits to 1. If LMCUT_MAX_ACTIONS > n_relax_actions, then some
    // indexes of allowed_actions will be true but without a corresponding
    // action. This is OK because the cut starts with an all-false vector and
    // then they are intersected. Moreover, when computing the cost of the cut,
    // the loop is done over the number of existing actions.
    allowed_actions.set();



    // TODO(fwt): When using boost::dynamic_bitset, do benchmark to see what is
    // better the approach bellow or make
    //    allowed_actions[ia] = (prec_op_vbits_[ia] & ext_goal_set).none();
    // With the fixed size (bitset), the approach bellow is better.
    //
    // Forbidding any action a that has an extended goal as precondition
    for (size_t ia = 0; ia < n_relax_actions; ++ia) {
      if ((prec_op_vbits_[ia] & ext_goal_set).any()) {
        allowed_actions[ia] = false;
//        std::cout << "[LMCut] Forbidding op " << ia << std::endl;
      }
    }


    // H1 restricted to the allowed actions (given restriction plus the ones
    // derived from ext_goal_set
    computeCostOfAtomsForConstrainedActionSet(s, action_cost, &allowed_actions);
    // std::cerr << "revised h^1 table:" << std::endl;
    // table->write_pddl(std::cerr, instance);


    DynBitSet cut = findCut(ext_goal_set, action_cost);
    assert(cut.count() > 0);

    // cut = intersection(cut, allowed_actions)
    cut &= allowed_actions;
    assert(cut.count() > 0);  // Non-empty intersection
//    std::cout << "[LMCut]: |cut intersection allowed| = " << cut.count() << std::endl;


    // cut_cost = min_{a s.t. cut[a] == true} cost(a)
    double cut_cost = -1;
    for (size_t ic = 0; ic < n_relax_actions; ++ic) {
      if (cut[ic] && (cut_cost < 0 || action_cost[ic] < cut_cost))
        cut_cost = action_cost[ic];
    }
//    std::cout << "[LMCut] cut_cost = " << cut_cost << std::endl;
    // std::cerr << "cut: ";
    // instance.write_action_set(std::cerr, cut);
    // std::cerr << ", cost = " << c_cut << std::endl;


    FANCY_DIE_IF(cut_cost <= 0, 666, "cut_cost is %f <= 0!!!", cut_cost);
    total_cost += cut_cost;

    // Decrease the cost of all actions in the cut by cut_cost
    for (size_t ic = 0; ic < n_relax_actions; ++ic) {
      if (cut[ic]) {
//        std::cout << "[LMCut] decreasing cost of of op[" << ic << "] from "
//                  << action_cost[ic] << " to " << (action_cost[ic] - cut_cost)
//                  << std::endl;
        action_cost[ic] -= cut_cost;
      }
    }

    if (save_cut) {
      std::set<const action_t*> this_cut;
      for (size_t ic = 0; ic < n_relax_actions; ++ic) {
        if (cut[ic]) {
          this_cut.emplace(operator_ptr_[ic]->original_action());
        }
      }
      rv.cuts.emplace_back(this_cut);
    }

    // Update the H1 table using ALL the actions and the shifted cost function
    computeCostOfAtomsForConstrainedActionSet(s, action_cost);
  }
//  std::cout << "[LMCut] total cut cost = " << total_cost << std::endl;
  rv.value = total_cost;
  return rv;
}


void LMCutHeuristic::extendedGoalsRec(atomList_t const& target_atoms,
                                      std::vector<double> const& action_cost,
                                      DynBitSet& ext_goal_set) const
{
  size_t constexpr size_t_max = std::numeric_limits<size_t>::max();

  // find all H^1-maximizers in the subgoal set, and check if any of them
  // already belongs to the extended goal set
  double max_cost = 0.0;
  size_t argmax_cost = size_t_max;
  bool max_in_ext_goals = false;
  for (size_t it = 0; it < target_atoms.size(); ++it) {
    atom_t k = target_atoms.atom(it);
    if (atom_rp_cost[k] > max_cost) {
      max_cost = atom_rp_cost[k];
      max_in_ext_goals = ext_goal_set[k];
      argmax_cost = k;
    }
    else if (atom_rp_cost[k] == max_cost) {
      max_in_ext_goals |= ext_goal_set[k];
    }
  }

  // if no maximizer is in the extended goal set, we have to add one
  // (argmax_cost), and recurse on all actions that add it and have zero cost.
  //
  // if argmax_cost == size_t_max, all atoms in target_atoms have H^1 value zero.
  if (!max_in_ext_goals && argmax_cost != size_t_max) {
    ext_goal_set[argmax_cost] = true;
    for (size_t ia = 0; ia < operator_ptr_.size(); ++ia) {
      deterministicAction_t const& a = *operator_ptr_[ia];
      if (a.effect().s_effect().add_list().find(argmax_cost)
          && action_cost[ia] == 0)
      {
        extendedGoalsRec(a.precondition().atom_list(0),
                         action_cost, ext_goal_set);
      }
    }
  }
}


DynBitSet LMCutHeuristic::findCut(DynBitSet const& ext_goal_set,
                                  std::vector<double> const& action_cost) const
{
  DynBitSet cut(n_actions);  // Default is zero
  assert(cut.count() == 0);

  // For each action a[ia]
  for (size_t ia = 0; ia < operator_ptr_.size(); ++ia) {
    assert(action_cost[ia] >= 0);
    if (action_cost[ia] > 0) {
      deterministicAction_t const& a = *operator_ptr_[ia];
      if ((adds_op_vbits_[ia] & ext_goal_set).any()) {
        // a[ia].adds has at least one of the ext_goal_set atoms
        atomList_t const& prec_a = a.precondition().atom_list(0);
        if (maxCost(prec_a, atom_rp_cost) < dead_end_value_) {
          // The precondition of a has finite cost, i.e., is achievable 
          cut[ia] = true;
        }
      }
    }
  }
  return cut;
}

void LMCutHeuristic::computeCostOfAtomsForConstrainedActionSet(state_t const& s,
    std::vector<double> const& action_cost,
    DynBitSet const* allowed_actions)
{
  assert(!relaxation_->goalT().holds(s, relaxation_->nprec()));

  // Offset. If negative atoms are represented, then positive atoms are the
  // even values of i.
  ushort_t inc = relaxation_->nprec() ? 1 : 2;
  _D(DEBUG_H_DET, std::cout << "inc == " << inc << std::endl;)

  // Position i: true if the cost of any atom in the precondition of
  //             operator_ptr_[i] has decreased.
  static DynBitSet aa(n_actions);
  aa.reset();

  for (size_t i = 0; i < operator_ptr_.size(); i++) {
    action_rp_cost[i] = std::numeric_limits<double>::max();
    if (operator_ptr_[i]->precondition().atom_list(0).size() == 0) {
      // This operator has no precondition, so it is already applicable and
      // needs to be considered
      _D(DEBUG_H_DET, std::cout << "operator_ptr[" << i << "] = "
                  << operator_ptr_[i]->name()
                  << " has no precondition and is allowed\n";)
      aa[i] = true;
    }
  }

  _D(DEBUG_H_DET, std::cout << "Computing " << name_ << "_"
      << gpt::problem->domain().functions().name(cost_idx_)
                            << "(s) for s = "
                            << s.toStringFull(relaxation_, true)
                            << std::endl;)
  for (ushort_t i = 0; i < problem_t::number_atoms(); i += inc) {
    // For some reason s.holds(i) is not returning true for the negation of
    // an atom. Probably it is related to the fact that the s is a state of
    // the original problem, not the relaxation.
    // If
    //  - inc equals one, we should also set the negative atoms.
    //  - i is odd, then i represent (not i-1)
    //  - !s.holds(i-1)
    // then
    //  - i-1 should hold in s.
    if (s.holds(i) || (inc == 1 && i % 2 == 1 && !s.holds(i-1))) {
      atom_rp_cost[i] = 0.0;
      _D(DEBUG_H_DET, std::cout << "  atom i=" << i
                                << " holds in s, so atom_rp_cost[i] = 0\n";
      );
      for (auto const& op_idx : has_prec_[i]) {
      // for every op_idx it that has i its precondition
        _D(DEBUG_H_DET, std::cout << "    make aa true for operator "
                                   << op_idx << std::endl;)
        aa[op_idx] = true;
      }
    }
    else {
      atom_rp_cost[i] = std::numeric_limits<double>::max();
    }
  }

  /*
   * Computing atom_rp_cost and action_rp_cost for the given state
   */
  bool done = false;
  while (!done) {
    done = true;  // Assuming we already converged

    for (size_t i = 0; i < operator_ptr_.size(); i++) {
      if (aa[i]) {
        _D(DEBUG_H_DET, std::cout << "Processing operator_ptr[" << i << "] = "
                    << operator_ptr_[i]->name()
                    << " (its aa was true)" << std::endl;)
        aa[i] = false;  // i needed to be updated and we're doing it now

        if (allowed_actions && !(*allowed_actions)[i]) {
          _D(DEBUG_H_DET,
              std::cout << "\tEarly exit because operator is not allowed\n";);
          continue;
        }


        bool finite_prec_cost =  true;
        atomList_t const& prec = operator_ptr_[i]->precondition().atom_list(0);
        // Searching for an atom in prec(i) that has infinity cost, i.e.,
        // double::max
        for (size_t j = 0; j < prec.size() && finite_prec_cost; ++j) {
          if (atom_rp_cost[prec.atom(j)] == std::numeric_limits<double>::max())
          {
            _D(DEBUG_H_DET, std::cout << "  One of its preconditions still "
                                      << "cost infinity, so giving up\n";)
            finite_prec_cost = false;
          }
        }

        if (finite_prec_cost) {
        // Every atom in the precondition of operator_ptr_[i] costs less than
        // infinity (double::max), so we can derive a plan in which the
        // action is applicable
          _D(DEBUG_H_DET, std::cout << "  Operator_ptr[" << i << "] = "
                                    << operator_ptr_[i]->name()
                                    << " is applicable!" << std::endl;
            operator_ptr_[i]->print_full();
            std::cout << std::endl;
          )

          atomList_t const& add_list =
                               operator_ptr_[i]->effect().s_effect().add_list();

          // prec_cost is combination cost of making the preconditions of action
          // operator_ptr_[i] true. Formally:
          //    prec_cost = COMBO_(k in prec[i]) cost_to_make_atom_true(k)
          // where COMBO can be summation (for the additive h) or maximization
          // (for maximal h)
          double prec_cost = std::min(maxCost(prec, atom_rp_cost),
                                      dead_end_value_);

          if (e_less((prec_cost), (action_rp_cost[i]))) {
            done = false;  // The cost of action operator_ptr_[i] decreased
                           // by more than epsilon, so we didn't converge.
            _D(DEBUG_H_DET, std::cout << "  Cost to make action applicable "
                        << "decreased from " << action_rp_cost[i]
                        << " to " << prec_cost << std::endl;)
            // set new prec_cost for applying action
            action_rp_cost[i] = prec_cost;
            // cost of reaching each atom in the add list, i.e., cost of the
            // preconditions of the current operator plus the cost of the
            // operator itself
            double cost_after_exec_i = prec_cost + action_cost[i];


            // Updating, if necessary, the cost of the atoms added by
            // operator_ptr_[i], the actions supporting the added atom and the
            // actions that need to be updated (vector aa)
            for (size_t k = 0; k < add_list.size(); k++) {
              atom_t a = add_list.atom(k);
              if (e_less((cost_after_exec_i), (atom_rp_cost[a])))
              // if atom_rp_cost[a] - (cost_after_exec_i) > epsilon
              {
                _D(DEBUG_H_DET, std::cout << "  Cost to make atom " << a
                            << " true decreased by "
                            << atom_rp_cost[a] - cost_after_exec_i
                            << " to " << cost_after_exec_i << std::endl;)
                atom_rp_cost[a] = cost_after_exec_i;
                for (auto const& op_idx : has_prec_[a]) {
                  aa[op_idx] = true;
                }
              }
            }  // for each atom a in add list of operator_ptr_[i]
          }  // if action_rp_cost[i] - prec_cost > epsilon
        }  // if every atom can be reached from cur_s with prec_cost < double::max
        else {
          _D(DEBUG_H_DET, std::cout << "Operator_ptr[" << i << "] = "
                      << operator_ptr_[i]->name()
                      << " is NOT applicable :(" << std::endl;)
        }
      }  // if aa[i] == true
      else {
        _D(DEBUG_H_DET, std::cout << "Operator_ptr[" << i << "] = "
                                  << operator_ptr_[i]->name()
                                  << " has aa false :(" << std::endl;)
      }
    }  // for each action o
  }  // while !done

  /*
   * The vectors atom_rp_cost and action_rp_cost have already converged to
   * their additive cost.
   */
}
