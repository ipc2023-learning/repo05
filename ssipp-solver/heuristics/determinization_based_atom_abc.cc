#include <algorithm>  // sort
#include <cassert>
#include <iostream>
#include <limits>

#include "determinization_based_atom_abc.h"

#include "../utils/die.h"
#include "../ext/mgpt/atom_list.h"


/*******************************************************************************
 *
 * determinizationBasedAtomHeuristic: base for h_add, h_max and similar for
 * probabilistic problems.
 *
 ******************************************************************************/
determinizationBasedAtomHeuristic::determinizationBasedAtomHeuristic(
    problem_t const& problem, size_t cost_idx, bool use_self_loop_relaxation)
  : FactoredHeuristic("det-based-atom-H", problem),
    cost_idx_(cost_idx), dead_end_value_(gpt::dead_end_value.double_value())
{
  relaxation_ = &problem.strong_relaxation(use_self_loop_relaxation);

  if (gpt::verbosity > 300) {
    std::cout << "<hadd>: new, total " << problem.number_atoms()
      << "atoms, " << relaxation_->actionsT().size()
      << " actions." << std::endl;
  }

  assert(cost_idx_ == ACTION_COST);
  assert(dead_end_value_ > 0.0);

  atom_rp_cost = new double[problem.number_atoms()];
  action_rp_cost = new double[relaxation_->actionsT().size()];

  actionList_t::const_iterator it;

  _D(DEBUG_H_DET, std::cout << "Actions in the strong relaxation\n";)
  for (it = relaxation_->actionsT().begin();
       it != relaxation_->actionsT().end(); ++it)
  {
    const deterministicAction_t *dact =
      dynamic_cast<const deterministicAction_t*>(*it);
    _D(DEBUG_H_DET, std::cout << "====================" << std::endl;
          dact->print_full();
          std::cout << std::endl;)
    operator_ptr_.push_back(dact);
  }

  _D(DEBUG_H_DET, std::cout << "===== All actions printed" << std::endl;)
  std::sort(operator_ptr_.begin(), operator_ptr_.end(),
            ActionCostIncreasing(cost_idx_));

  has_prec_.resize(problem.number_atoms());
//  std::cout << "[" << name_ << "] List of operators ordered by increasing "
//            << "value of ";
//  if (cost_idx_ == ACTION_COST) {
//    std::cout << "COST";
//  }
//  else {
//    std::cout << problem.domain().functions().name(cost_idx_);
//  }
//  std::cout << " (" << cost_idx_ << "-th cost function)" << std::endl;

  // For each action o in the strong relaxation
  for (size_t o = 0; o < operator_ptr_.size(); ++o) {
//    std::cout << "operator_ptr[" << o << "] = " << operator_ptr_[o]->name()
//              << " -- cost = " << operator_ptr_[o]->cost(cost_idx_) << std::endl;
    // For each atom i in the precondition of o
    FANCY_DIE_IF(operator_ptr_[o]->precondition().size() != 1, 171,
        "Action %s has a (disjuntive) precondition of size %d",
        operator_ptr_[o]->name(),
        operator_ptr_[o]->precondition().atom_list(0).size());

    for (size_t i = 0;
         i < operator_ptr_[o]->precondition().atom_list(0).size(); ++i)
    {
      ushort_t atom = operator_ptr_[o]->precondition().atom_list(0).atom(i);
      has_prec_[atom].push_back(o);
    }
  }
  assert(relaxation_->goalT().size() == 1);
}



determinizationBasedAtomHeuristic::~determinizationBasedAtomHeuristic() {
  delete[] action_rp_cost;
  delete[] atom_rp_cost;
  if( gpt::verbosity > 300 )
    std::cout << "<hadd>: deleted" << std::endl;
}

void determinizationBasedAtomHeuristic::computeCostOfAtoms(state_t const& s,
    deterministicAction_t const** supporter)
{

  assert(!relaxation_->goalT().holds(s, relaxation_->nprec()));

  // Offset. If negative atoms are represented, then positive atoms are the
  // even values of i.
  ushort_t inc = relaxation_->nprec() ? 1 : 2;
  _D(DEBUG_H_DET, std::cout << "inc == " << inc << std::endl;)

  // Position i: true if the cost of any atom in the precondition of
  //             operator_ptr_[i] has decreased.
  static std::vector<bool> aa(operator_ptr_.size(), false);

  for (size_t i = 0; i < operator_ptr_.size(); i++) {
    action_rp_cost[i] = std::numeric_limits<double>::max();
    aa[i] = false;
    if (operator_ptr_[i]->precondition().atom_list(0).size() == 0) {
      // This operator has no precondition, so it is already applicable and
      // needs to be considered
      _D(DEBUG_H_DET, std::cout << "operator_ptr[" << i << "] = "
                  << operator_ptr_[i]->name() << " has no precondition\n";)
      aa[i] = true;
    }
  }

  _D(DEBUG_H_DET, std::cout << "Computing " << name_ << "_"
      << gpt::problem->domain().functions().name(cost_idx_)
                            << "(s) for s = "
                            << s.toStringFull(relaxation_, true)
                            << std::endl;)
  for (ushort_t i = 0; i < problem_t::number_atoms(); i += inc) {
    if (supporter) {
      supporter[i] = NULL;
    }
    // For some reason s.holds(i) is not returning true for the negation of
    // an atom. Probably it is related to the fact that the s is a state of
    // the original problem, not the relaxation.
    // If
    //  - inc equals one, we should also set the negative atoms.
    //  - i is odd, then i represent (not i-1)
    //  - !s.holds(i-1)
    // then
    //  - i-1 should hold in s.
    if (s.holds(i) || (inc == 1 && i % 2 == 1 && !s.holds(i-1)))
    {
      atom_rp_cost[i] = 0.0;
      _D(DEBUG_H_DET, std::cout << "  atom i=" << i
                                << " holds in s, so atom_rp_cost[i] = 0\n";
      )
      for(std::list<size_t>::iterator it = has_prec_[i].begin();
          it != has_prec_[i].end(); it++)
      {
        _D(DEBUG_H_DET, std::cout << "    make aa true for operator "
                                   << *it << std::endl;)
        aa[*it] = true;
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
        size_t j = 0;  // idx of a positive atom

        // Searching for an atom in prec(i) that has infinity cost, i.e.,
        // double::max
        for (j=0; j < operator_ptr_[i]->precondition().atom_list(0).size(); j++)
        {
          if (atom_rp_cost[
                    operator_ptr_[i]->precondition().atom_list(0).atom(j)]
              == std::numeric_limits<double>::max())
          {
            _D(DEBUG_H_DET, std::cout << "  One of its preconditions still "
                                      << "cost infinity, so giving up\n";)
            break;
          }
        }

        if (j == operator_ptr_[i]->precondition().atom_list(0).size())
        // Every atom in the precondition of operator_ptr_[i] costs less than
        // infinity (double::max), so we can derive a plan in which the the
        // action is applicable
        {
          _D(DEBUG_H_DET, std::cout << "  Operator_ptr[" << i << "] = "
                                    << operator_ptr_[i]->name()
                                    << " is applicable!" << std::endl;
            operator_ptr_[i]->print_full();
            std::cout << std::endl;
          )

          const atomList_t& add_list =
              operator_ptr_[i]->effect().s_effect().add_list();

          // cost: combination cost of making the preconditions of action
          //       operator_ptr_[i] true. Formally:
          //    cost = COMBO_(k in prec[i]) cost_to_make_atom_true(k)
          // where COMBO can be summation (for the additive h) or maximization
          // (for maximal h)
          double cost =
              costSetOfAtoms(operator_ptr_[i]->precondition().atom_list(0));

          if (e_less((cost), (action_rp_cost[i]))) {
            done = false;  // The cost of action operator_ptr_[i] decreased
                           // by more than epsilon, so we didn't converge.
            _D(DEBUG_H_DET, std::cout << "  Cost to make action applicable "
                        << "decreased by " << action_rp_cost[i] - cost
                        << " to " << cost << std::endl;)
            /* set new cost for applying action */
            action_rp_cost[i] = cost;
            /* cost of reaching each atom in the add list, i.e., cost of the
             * preconditions of the current operator plus the cost of the
             * operator itself */
            double cost_after_exec_i = cost
                            + operator_ptr_[i]->cost(cost_idx_).double_value();


            // Updating, if necessary, the cost of the atoms added by
            // operator_ptr_[i], the actions supporting the added atom and the
            // actions that need to be updated (vector aa)
            for (size_t k = 0; k < add_list.size(); k++) {
              ushort_t a = add_list.atom(k);
              if (e_less((cost_after_exec_i), (atom_rp_cost[a])))
              // if atom_rp_cost[a] - (cost_after_exec_i) > epsilon
              {
                _D(DEBUG_H_DET, std::cout << "  Cost to make atom " << a
                            << " true decreased by "
                            << atom_rp_cost[a] - cost_after_exec_i
                            << " to " << cost_after_exec_i << std::endl;)
                atom_rp_cost[a] = cost_after_exec_i;
                if (supporter) {
                  supporter[a] = operator_ptr_[i];
                }
                for(std::list<size_t>::iterator it = has_prec_[a].begin();
                    it != has_prec_[a].end(); it++) {
                  aa[*it] = true;
                }
              }
            }  // for each atom a in add list of operator_ptr_[i]
          }  // if action_rp_cost[i] - cost > epsilon
        }  // if every atom can be reached from cur_s with cost < double::max
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
