// stdlib
#include <iostream>
#include <unordered_set>
#include <memory>
#include <stack>

// pybind
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

// ssipp
#include "ext/mgpt/global.h"
#include "ext/mgpt/atom_states.h"
#include "ext/mgpt/problems.h"
#include "ext/mgpt/actions.h"
#include "heuristics/heuristic_factory.h"
#include "heuristics/heuristic_iface.h"
#include "heuristics/action_heuristic.h"
#include "heuristics/lm_cut.h"
#include "ssps/ppddl_adaptors.h"
#include "planners/planner_iface.h"
#include "planners/planner_factory.h"
#include "planners/lrtdp.h"

namespace py = pybind11;
using std::stack;
using std::unordered_set;
using std::shared_ptr;
using std::make_shared;

// this gets a problem instance and does all necessary initialisation
static bool initialised = false;

problem_t *init_problem(const std::string &name) {
  // only allow one initialisation
  // TODO: make this a real exception. Maybe also make !pptr thing real
  // exception.
  if (initialised) {
    throw std::runtime_error("I've already been initialised; you can't "
                            "initialise me again.");
  }
  problem_t *pptr = problem_t::find(name);
  if (!pptr) {
    return nullptr;
  }
  gpt::problem = pptr;
  pptr->instantiate_actions();
  pptr->flatten();
  state_t::initialize(*pptr);
  initialised = true;
  return pptr;
}

double get_dead_end_value() {
  return gpt::dead_end_value.double_value();
}

void set_dead_end_value(double new_value) {
  gpt::dead_end_value = new_value;
}

OutcomeResult succ_iter_next(SuccessorIterator &self) {
  if (self.hasNext()) {
    OutcomeResult rv = *self;
    ++self;
    return rv;
  }
  throw py::stop_iteration();
}

// Turns CutResult::cuts into Python list of frozensets of strings. Probably
// easier than futzing with nested STL shit containing action_t*s which aren't
// allowed to die (& thus annoying to memory-manage on py side).
py::list safe_cuts(const CutResult &self) {
  py::list rv;
  for (auto const &cut : self.cuts) {
    py::set this_cut;
    for (auto const action_ptr : cut) {
      this_cut.add(py::str(action_ptr->name()));
    }
    rv.append(this_cut);
  }
  return rv;
}

std::string state_string_repr(const problem_t &problem, const state_t &s) {
  // yields str like "(p1 o1 o2) (p2 o3) (p3 o4 o5 o6) …"
  std::stringstream ost;
  // *ai returns index of atom (ushort_t)
  // ++ai skips forward until idx points to something true
  // obvs. this totally misses statics!
  bool first_run = true;
  for (auto ai = s.predicate_begin(); ai != s.predicate_end(); ++ai ) {
    if (!first_run) {
      ost << " ";
    }
    first_run = false;

    const Atom *atom = problem_t::atom_inv_hash_get(*ai);
    if (*ai % 2) {
      // negative, skip
      continue;
    }

    // positive, go!
    problem.print(ost, *atom);
  }
  for (auto si : problem.init_atoms_static()) {
    if (!first_run) {
      ost << " ";
    }
    problem.print(ost, *si);
  }
  return ost.str();
}

double hp_q_value(HeuristicPlanner &hp, const state_t &s, const action_t &a) {
  if (!hp.decideAction(s)) {
    // sanity check for goals, inapplicable actions, etc.
    throw py::value_error("no action for this state---is it a goal or sth?");
  }
  if (!a.enabled(s)) {
    return gpt::dead_end_value.double_value();
  }

  double qv_acc = a.cost(s).double_value();
  double p_acc = 0;
  ProbDistState pr;
  a.expand(s, pr);
  for (const auto &succ : pr) {
    const state_t &s_prime = succ.event();
    double prob = succ.prob();
    // make sure we're converged
    // does not matter whether we actually find an action
    hp.decideAction(s_prime);
    // TODO: what if this is a goal state?
    qv_acc += prob * hp.value(s_prime);
    p_acc += prob;
  }
  if (abs(p_acc - 1) > 1e-3) {
    std::cerr << "[hp_q_value] warning: probs only sum to " << p_acc << std::endl;
  }
  return std::min(qv_acc, gpt::dead_end_value.double_value());
}

struct StateHash {
  size_t operator()(const state_t &s) const {
    return s.hash_value();
  }
};

struct StateEq {
  bool operator()(const state_t &a, const state_t &b) const {
    // this functor may be unnecessary, but want to keep explicit in case I
    // switch to state ptrs later (which I think the hash might accept silently
    // & compare on address)
    return a == b;
  }
};

// extract (state, action) mapping for greedy policy w.r.t. p uses standard DFS;
// stops after visiting too many states and sets flag
py::tuple extract_policy(Planner &p,
                         const SSPIface &ssp,
                         const state_t &s0,
                         unsigned int max_states=0x100000) {
  // need to use shared_ptrs so that we can keep states around
  // FIXME: replace states with smart ptrs if this method becomes a hotpoint
  // (probably too much copying)
  py::dict sa_dict;
  stack<state_t> open;
  unordered_set<state_t, StateHash, StateEq> closed;
  open.push(s0);
  closed.insert(s0);
  bool overflowed = false;

  while (open.size() > 0) {
    const state_t next = open.top();
    open.pop();
    const action_t *best_act = p.decideAction(next);
    // sometimes we hit dead ends
    if (!best_act) {
      continue;
    }
    py::object pynext = py::cast(next);
    sa_dict[pynext] = py::cast(best_act);

    // get successors
    // ProbDistState is weird wrapper around array<pair<state_t, double>>
    ProbDistState pr;
    ssp.expand(*best_act, next, pr);
    for (const auto &succ : pr) {
      const state_t &new_s = succ.event();
      bool is_unseen = closed.insert(new_s).second;
      if (is_unseen) {
        open.push(new_s);
      }
    }

    if (sa_dict.size() >= max_states && open.size() > 0) {
      overflowed = true;
      break;
    }
  }

  py::tuple rv = py::make_tuple(sa_dict, overflowed);
  return rv;
}

// returns a list of successor states and transition probabilities for a given
// state under a given action
py::list successors(const SSPIface &ssp, const state_t &s, const action_t &a) {
  ProbDistState pr;
  ssp.expand(a, s, pr);
  py::list successors;
  for (const auto &succ : pr) {
    const state_t new_s = succ.event();
    successors.append(py::make_tuple(succ.prob(), new_s));
  }
  return successors;
}

// wraps action.cost() to return double instead of rational
double action_t_cost(const action_t &action, const state_t &s) {
  Rational r = action.cost(s);
  return r.double_value();
}

// (for some reason I get ownership-related errors, as described below, when I
// try to return a py::list here)
typedef std::unique_ptr<action_t const, py::nodelete> action_uptr;
std::vector<action_uptr> ssp_applicable_actions(const SSPIface &ssp, state_t const &s) {
  ActionConstRange actions = ssp.applicableActions(s);
  std::vector<action_uptr> rv {};
  for (auto it = actions.begin(); it != actions.end(); ++it) {
    action_t const &a = *it;
    action_uptr a_ptr(&a);
    // We need to wrap each element of the list in a "nodelete" pointer to
    // indicate that the elements are managed on the C++ side, but the list is
    // not. Otherwise we get `RuntimeError: return_value_policy = copy, but the
    // object is non-copyable!`. See docs at
    // https://pybind11.readthedocs.io/en/stable/advanced/classes.html#non-public-destructors
    rv.emplace_back(std::move(a_ptr));
  }
  return rv;
}

// Based in pybind11/tests/object.h since I'm sure as hell not re-implementing all
// this boilerplate myself. Might make sense to subclass, but I don't think it's
// public.
template <typename T> class ref {
public:
  // TODO: comment out crap that's not required to make this holder compile

  // null constructor
  ref() : m_ptr(nullptr) {}
  // constructor from ptr
  ref(T *ptr) : m_ptr(ptr) {if (m_ptr) T::register_use(m_ptr);}
  // copy constructor
  ref(const ref &r) : m_ptr(r.m_ptr) {if (m_ptr) T::register_use(m_ptr);}
  // move constructor
  ref(ref &&r) : m_ptr(r.m_ptr) {r.m_ptr = nullptr;}
  // destructor, deallocator
  ~ref() {if (m_ptr) T::unregister_use(m_ptr);}

  // move assignment
  ref& operator=(ref&& r) {
    if (*this == r) return *this;
    if (m_ptr) T::unregister_use(m_ptr);
    m_ptr = r.m_ptr;
    r.m_ptr = nullptr;
    // no need to unregister other, since we keep its ptr
    return *this;
  }
  // overwrite with reference
  ref& operator=(const ref& r) {
    if (m_ptr == r.m_ptr) return *this;
    if (m_ptr) T::unregister_use(m_ptr);
    m_ptr = r.m_ptr;
    if (m_ptr) T::register_use(m_ptr);
    return *this;
  }
  // overwrite with pointer
  ref& operator=(T *ptr) {
    if (m_ptr == ptr) return *this;
    if (m_ptr) T::unregister_use(m_ptr);
    m_ptr = ptr;
    if (m_ptr) T::register_use(m_ptr);
    return *this;
  }

  // comparisons
  bool operator==(const ref &r) const { return m_ptr == r.m_ptr; }
  bool operator!=(const ref &r) const { return m_ptr != r.m_ptr; }
  bool operator==(const T* ptr) const { return m_ptr == ptr; }
  bool operator!=(const T* ptr) const { return m_ptr != ptr; }

  // ptr ops
  T* operator->() { return m_ptr; }
  const T* operator->() const { return m_ptr; }
  T& operator*() { return *m_ptr; }
  const T& operator*() const { return *m_ptr; }
  operator T* () { return m_ptr; }

  // things pybind11 wants
  T* get_ptr() { return m_ptr; }
  const T* get_ptr() const { return m_ptr; }
private:
  T *m_ptr;
};

// "true" means "you can construct a new ref<T> from an Object* with no
// inconsistencies" (which may not be true for non-intrusive pointers, but is
// generally true for intrusive pointers)
PYBIND11_DECLARE_HOLDER_TYPE(T, ref<T>, true);

void screw_pickle() {
  // for some reason pybind11 (or maybe Python?) insists that it knows how to
  // pickle my objects, and subsequently crashes everything when it tries to
  // "unpickle" uninitialised bytes
  throw std::runtime_error("STOP! None of the SSiPP objects can be pickled!");
}

#define SCREW_PICKLE() .def("__reduce__", &screw_pickle) \
  .def(py::pickle( \
    [](void*) {screw_pickle(); return py::make_tuple(nullptr);}, \
    [](py::tuple) {screw_pickle(); return nullptr;}))

PYBIND11_MODULE(ssipp, m) {
  m.doc() = "Python interface to the SSiPP planner";
  m.attr("__version__") = py::str("dev");

  m.def("init_problem", &init_problem, "Fetch a problem by name (or None)",
        // we don't own this, SSiPP does
        py::return_value_policy::reference);
  m.def("readPDDLFile", &readPDDLFile,
        "Parse a PDDL file; True on success, False otherwise");
  m.def("get_dead_end_value", &get_dead_end_value,
        "Returns (fixed) V* for dead-end states");
  m.def("set_dead_end_value", &set_dead_end_value,
        "Sets a new (fixed) V* for dead-end states");
  m.def("createHeuristic", &createHeuristic,
        "Create a heuristic from an SSP and a name",
        // keep SSP alive at least as long as heuristic is
        py::keep_alive<1, 2>());
  m.def("extract_policy", &extract_policy,
        py::arg("p"), py::arg("ssp"), py::arg("s0"),
        py::arg("max_states")=0x100000,
        "Extract (state, action) pairs from planner's policy; return dict "
        "mapping s->a, plus flag saying whether search stopped early by "
        "exceeding state limit.");
  m.def("successors", &successors,
        py::arg("ssp"), py::arg("s"), py::arg("a"),
        "Find list of successor pairs (transition prob., state) for given "
        "action in given state.");

  py::class_<problem_t, ref<problem_t>>(m, "problem_t")
    .def("no_more_atoms", &problem_t::no_more_atoms,
         "Indicate to SSiPP that no more atoms will be added to the global tables")
    // state gets returned on stack, no need for ownership hacks
    .def("get_intermediate_state", &problem_t::get_intermediate_state,
         "Get a state for the current problem from a string (format is "
         "'pred-1 obj-1 obj-2, pred-2 obj-1 obj-3, ...'; static predicates "
         "are ignored)")
    .def("find_action", &problem_t::find_action, "Get an action_t by name",
         py::return_value_policy::reference)
    .def("string_repr", &state_string_repr,
         "Show state atoms in a string, *including* static atoms!")
    SCREW_PICKLE();

  py::class_<state_t>(m, "state_t")
    .def("toStringFullNoStatics", &state_t::toStringFull,
         "String representation of state (for debugging)",
         py::arg("problem"),
         py::arg("print_idx")=true,
         py::arg("print_braces")=true)
    .def("__hash__", [](const state_t &s) {
        return s.hash_value();
      })
    .def("__eq__", &state_t::operator==)
    SCREW_PICKLE();

  // use shared_ptr holder so that we can pass to SuccessorEvaluator
  py::class_<heuristic_t, std::shared_ptr<heuristic_t>>(m, "heuristic_t")
    .def("value", &heuristic_t::value,
        "Get value of a state")
    SCREW_PICKLE();

  //expose LMCutHeuristic separately (along with CutResult) so that we can retrieve cuts
  py::class_<LMCutHeuristic, heuristic_t, std::shared_ptr<LMCutHeuristic>>(m, "LMCutHeuristic")
    .def(py::init<const problem_t&>())
    .def("valueAndCuts", &LMCutHeuristic::valueAndCuts,
        "Return both lm-cut value of state and cuts")
    SCREW_PICKLE();

  py::class_<CutResult>(m, "CutResult")
    .def_property_readonly("cuts", &safe_cuts,
                          "List of sets of action names forming cuts")
    .def_readonly("value", &CutResult::value,
                  "lm-cut evaluation of state")
    SCREW_PICKLE();

  py::class_<action_t, ref<action_t>>(m, "action_t")
    .def("name", &action_t::name, "String representation of action")
    .def("cost", action_t_cost, "Cost of applying this action in a given state.")
    // address-based impls ugly, but not sure of a better way
    // maybe I don't need them?
    // .def("__hash__", [](const action_t &a) -> long {
    //   return 0x75433f08L ^ (long)&a;
    // })
    // .def("__eq__", [](const action_t &a, const action_t &b) -> bool {
    //   return &a == &b;
    // })
    SCREW_PICKLE();

  // ABC for SSPfromPPDDL only exposed so that we can use createHeuristic
  py::class_<SSPIface>(m, "SSPIface")
    .def("s0", &SSPIface::s0, "Get initial state for problem.")
    .def("isGoal", &SSPIface::isGoal, "Check whether given state is a goal state.")
    .def("applicableActions", ssp_applicable_actions,
         "List of actions applicable in this state.")
    SCREW_PICKLE();

  py::class_<SSPfromPPDDL, SSPIface>(m, "SSPfromPPDDL")
    .def(py::init<const problem_t &>(),
        // keep problem (2) alive so long as *this (1) is
        py::keep_alive<1, 2>())
    SCREW_PICKLE();

  py::class_<SuccessorEvaluator>(m, "SuccessorEvaluator")
    .def(py::init<std::shared_ptr<heuristic_t>>())
    .def("state_value", &SuccessorEvaluator::state_value,
        "Evaluate a single state")
    // should return a SuccessorIteratorWrapper
    .def("succ_iter", &SuccessorEvaluator::succ_iter,
        "Iterator over successor states for a given state/action pair",
        // SIW takes references, so as long as it's alive, all arguments
        // to succ_iter (including this SuccessorEvaluator) should remain alive
        // too
        py::keep_alive<0, 1>(),
        py::keep_alive<0, 2>(),
        py::keep_alive<0, 3>())
    .def("dump_table", &SuccessorEvaluator::dump_table,
        "Write internal hash table to a file at some path")
    SCREW_PICKLE();

  py::class_<OutcomeResult>(m, "OutcomeResult")
    // return type: state_t
    .def_readonly("state", &OutcomeResult::state,
                  "State that this outcome yields")
    // return type: double
    .def_readonly("probability", &OutcomeResult::probability,
                  "Probability of this outcome, from parent (action node)")
    // return type: double
    .def_readonly("value", &OutcomeResult::value,
                  "Heuristic evaluation of produced state")
    SCREW_PICKLE();

  py::class_<SuccessorIteratorWrapper>(m, "SuccessorIteratorWrapper")
    .def("__iter__", &SuccessorIteratorWrapper::begin,
        // iterator has a wrapper reference, so keep wrapper alive as long as
        // iterator is
        py::keep_alive<0, 1>(),
        "Turn iterable wrapper into iterator over outcomes of action")
    SCREW_PICKLE();

  py::class_<SuccessorIterator>(m, "SuccessorIterator")
    .def("__next__", &succ_iter_next,
          "Get the next outcome for this action")
    SCREW_PICKLE();

  // Interesting: pybind11 tries to return the exact type of something, even
  // when given a pointer to a parent instead. Can't find example in master,
  // but it's in this commit:
  // https://github.com/pybind/pybind11/commit/d7efa4ff7b04f987cdb3a3a9d1649be8f0a78bc1
  // Seems to work well for createPlanner (so long as the underlying class is
  // known to SSiPP)

  py::class_<Deadline, std::shared_ptr<Deadline>>(m, "Deadline")
    SCREW_PICKLE();
  py::class_<CpuTimeDeadline,
              Deadline,
              std::shared_ptr<CpuTimeDeadline>>(m, "CpuTimeDeadline")
    .def(py::init<uint64_t>(),
          "Time out after given number of microseconds.")
    SCREW_PICKLE();
  py::register_exception<DeadlineReachedException>(
    m, "DeadlineReachedException");

  // need to separately declare this pointer so that it's non-const; otherwise,
  // the compiler gets confused by the presence of an `rv method(sig)` and `rv
  // method(sig) const` in the same class.
  action_t const* (Planner::*decideActionHandle)(state_t const& s)
    = &Planner::decideAction;
  py::class_<Planner>(m, "Planner")
    .def("decideAction", decideActionHandle,
          "Solve SSP for given state, then return best action.",
          py::return_value_policy::reference)
    // next one only solves from SSP's s0 (bad)
    // .def("trainForUsecs", &Planner::trainForUsecs,
    //      "Try to solve SSP from (fixed) s0 for given time.")
    // next three don't matter much unless using SSiPP
    .def("initRound", &Planner::initRound,
        "Should be called at the beginning of each evaluation round. Only used by *SSiPP.")
    .def("endRound", &Planner::endRound,
        "Should be called at the end of each evaluation round. Only used by *SSiPP.")
    SCREW_PICKLE();
  py::class_<HeuristicPlanner, Planner>(m, "HeuristicPlanner")
    .def("value", &HeuristicPlanner::value,
          "Return best state value known so far")
    .def("q_value", &hp_q_value,
         "Average over subsequent returns to get Q(s, a)")
    SCREW_PICKLE();
  py::class_<OptimalPlanner, HeuristicPlanner>(m, "OptimalPlanner")
    SCREW_PICKLE();
  py::class_<PlannerLRTDP, OptimalPlanner>(m, "PlannerLRTDP")
    SCREW_PICKLE();
  m.def("createPlanner", &createPlanner,
        py::arg("problem"), py::arg("planner_name"), py::arg("heuristic"),
        // need to keep SSP and heuristic alive for as long as planner is
        py::keep_alive<0, 1>(), py::keep_alive<0, 3>(),
        "create a planner from an SSP, a planner name, and a heuristic",
        // this is the default, but putting here to make it explicit
        py::return_value_policy::take_ownership);
  m.def("setDeadline", &gpt::setDeadline,
        "Set deadline for any planner to solve problem.");
  m.def("removeDeadline", &gpt::removeDeadline,
        "Remove any planner deadline previously put in place.");
}
