#include "action_heuristic.h"
#include <fstream>

using SIW = SuccessorIteratorWrapper;
using CI = SuccessorIterator;

double SuccessorEvaluator::state_value(const state_t &state) {
  return hash_.get(state)->value();
}

void SuccessorEvaluator::dump_table(const std::string &dest) const {
  std::ofstream out_file(dest);
  hash_.dump(out_file);
  out_file.close();
}

const SIW::Outcome SIW::effect(size_t i) const {
  if (!is_probabilistic_) {
    // it's a deterministic action
    assert(i == 0);
    return {1.0, det_action_->effect()};
  }
  assert(i < prob_action_->size());
  return {prob_action_->probability(i), prob_action_->effect(i)};
}

size_t SIW::numEffects() const {
  if (is_probabilistic_) {
    return prob_action_->size();
  }
  // deterministic actions only have one effect
  return 1;
}

SIW::SuccessorIteratorWrapper(const state_t &state, const action_t &action,
                         SuccessorEvaluator &evaluator)
  : state_(state), action_(action), evaluator_(evaluator) {
  prob_action_ = dynamic_cast<const probabilisticAction_t*>(&action_);
  if (prob_action_ == nullptr) {
    // it's a deterministic action
    det_action_ = dynamic_cast<const deterministicAction_t*>(&action_);
    assert(det_action_ != nullptr);
    is_probabilistic_ = false;
    num_outcomes_ = 1;
  } else {
    is_probabilistic_ = true;
    num_outcomes_ = prob_action_->size();
  }
}

SIW SuccessorEvaluator::succ_iter(const state_t &state,
                                  const action_t &action) {
  return SIW(state, action, *this);
};

CI SIW::begin() {
  return CI(*this, 0);
}

CI SIW::end() {
  // just beyond the end
  return CI(*this, num_outcomes_);
}

OutcomeResult CI::operator*() const {
  const SIW::Outcome outcome = wrapper_.effect(idx_);
  state_t new_state(wrapper_.state_);
  outcome.second.affect(wrapper_.state_, new_state);
  double val = wrapper_.evaluator_.state_value(new_state);
  return OutcomeResult(new_state, outcome.first.double_value(), val);
}

bool CI::operator!=(const CI &other) {
  // whatever, compare wrapper by address :P
  return &(other.wrapper_) != &wrapper_
    || other.idx_ != idx_;
}

CI CI::operator++() {
  // prefix, change this one and return copy of previous state
  CI old_it(*this);
  ++idx_;
  return old_it;
};
