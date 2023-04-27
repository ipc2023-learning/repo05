#ifndef HEURISTICS_ACTION_HEURISTIC_H
#define HEURISTICS_ACTION_HEURISTIC_H

#include "heuristic_iface.h"
#include "../ext/mgpt/hash.h"
#include <memory>

/* Header defines utilities for evaluating the worth of actions, rather than
states. Relies on standard state evaluation heuristics. */

class SuccessorIteratorWrapper;
class SuccessorIterator;

class SuccessorEvaluator {
  std::shared_ptr<heuristic_t> heuristic_;
  hash_t hash_;

public:
 SuccessorEvaluator(std::shared_ptr<heuristic_t> heuristic)
    : heuristic_(heuristic),
      hash_(gpt::initial_hash_size, *heuristic) {}
  ~SuccessorEvaluator() {}
  // get heuristic value for state, consulting cache
  double state_value(const state_t &state);
  SuccessorIteratorWrapper succ_iter(const state_t &state, const action_t &action);
  // dump hash table to a file somewhere (identified by path)
  void dump_table(const std::string &dest) const;

  friend class SuccessorIteratorWrapper;
};

struct OutcomeResult {
  OutcomeResult(const state_t state, double probability, double value)
  : state(state), probability(probability), value(value) {}

  const state_t state;
  const double probability;
  const double value;
};


class SuccessorIteratorWrapper {
  const state_t &state_;
  const action_t &action_;
  bool is_probabilistic_;
  int num_outcomes_;
  const probabilisticAction_t *prob_action_ = nullptr;
  const deterministicAction_t *det_action_ = nullptr;
  SuccessorEvaluator &evaluator_;
  typedef std::pair<Rational, const effect_t&> Outcome;

  friend class SuccessorEvaluator;

  size_t numEffects() const;
  const Outcome effect(size_t i) const;
  SuccessorIteratorWrapper(const state_t &state, const action_t &action,
                          SuccessorEvaluator &evaluator);

public:
  friend class SuccessorIterator;

  SuccessorIterator begin();
  SuccessorIterator end();
};

// this ought to be a nested class, but it isn't because SWIG can't deal with
// those (as of 3.0.10---even though the docs claim it should be able to!)
class SuccessorIterator {
  const SuccessorIteratorWrapper &wrapper_;
  int idx_, max_idx_;

  friend class SuccessorIteratorWrapper;

 SuccessorIterator(const SuccessorIteratorWrapper &wrapper, int idx = 0)
   : wrapper_(wrapper), idx_(idx), max_idx_(wrapper.numEffects()) {}

 public:
 SuccessorIterator(const SuccessorIterator &other)
   : wrapper_(other.wrapper_), idx_(other.idx_), max_idx_(other.max_idx_) {}
  SuccessorIterator operator++();
  OutcomeResult operator*() const;
  bool operator!=(const SuccessorIterator &other);
  bool hasNext() {return idx_ < max_idx_;}
};

#endif // HEURISTICS_ACTION_HEURISTIC_H
