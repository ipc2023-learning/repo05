/* TODO things that need to be fixed in this file:
 *
 * - There are a whole bunch of classes which need to be implemented and tested.
 *   Fix those first.
 * - std::maps and std::vectors which merely get converted into Python objects
 *   should be replaced with py::dicts, py::lists, etc. in some cases. Note that
 *   this will not work well for things which can't be manipulated on the C++
 *   side (i.e. immutable stuff).
 * - Should replace string types with a new PyType class that *optionally*
 *   contains a name. Will be hard to deal with (possibly anonymous) union types
 *   otherwise.
 * - Currently undecided on whether I should return pointers to heap-allocated
 *   custom class instances, or just return instances on the stack. pybind11 can
 *   certainly handle the latter, but I'm not sure whether there's any real
 *   advantage to (more complex) heap allocation.
 */

#include "pymdpsim.h"

// these are just conveniences for debug printing from GDB
// they're not used anywhere, so they'll disappear when compiled above -O0
template<class T>
void printer(const T &thing) {
  std::cerr << thing << std::endl;
}

template void printer<StateFormula>(const StateFormula&);
template void printer<ActionSchema>(const ActionSchema&);
template void printer<Action>(const Action&);
template void printer<Problem>(const Problem&);
template void printer<Domain>(const Domain&);

// stuff needed by mdpsim        ----------------------------------------------

/* The parse function. */
extern int yyparse();
/* File to parse. */
extern FILE* yyin;
/* Name of current file. */
string current_file;
/* Level of warnings. */
int warning_level = 1;
/* Verbosity level. */
int verbosity = 1;

// forward declarations          ----------------------------------------------


// actual implementation         ----------------------------------------------

void build_maps(const Problem *problem, AtomList &atom_vec,
                vector<const Action*> &action_vec) {
  AtomSet atom_set;
  // all instantiated actions (in initial state)
  ActionSet remaining_actions = problem->actions();
  ActionList actions;

  // atoms that can appear in the initial state.
  for (auto ai = problem->init_atoms().cbegin();
       ai != problem->init_atoms().cend(); ai++) {
    atom_set.insert(*ai);
    atom_vec.push_back(*ai);
  }

  for (auto ei = problem->init_effects().cbegin();
       ei != problem->init_effects().cend(); ei++) {
    (*ei)->listAtoms(atom_set, atom_vec);
  }

  // expand by looking at atoms which can be produced by enabled actions
  // (remove actions which have already been used)
  while (true) {
    // Get enabled actions in the list of unused actions.
    problem->enabled_actions_noValues(remaining_actions, actions, atom_set);

    /* If no enabled actions: step out. */
    if (actions.size() == 0)
      break;

    /* For each enabled action: */
    for (auto ai = actions.cbegin();
         ai != actions.cend(); ai++) {
      action_vec.push_back(*ai);

      /* Add atoms from this action's precondition */
      (*ai)->precondition().listAtoms(atom_set, atom_vec);

      /* Add atoms from this action's effects. */
      (*ai)->effect().listAtoms(atom_set, atom_vec);

      /* Remove action from unused actions. */
      auto af = remaining_actions.begin();
      while((*af) != (*ai)) {
        af++;
      }

      remaining_actions.erase(af);
    }

    actions.erase(actions.begin(), actions.end());
  }
}

bool parse_file(string pddl_path) {
  const char *name = pddl_path.c_str();
  yyin = fopen(name, "r");
  if (yyin == 0) {
    std::cerr << PACKAGE << ':' << name << ": " << strerror(errno)
              << std::endl;
    return false;
  }
  current_file = name;
  bool success = (yyparse() == 0);
  fclose(yyin);
  return success;
}

py::dict get_domains() {
  py::dict rv;
  for (auto i = Domain::begin(); i != Domain::end(); i++) {
    rv[i->first.c_str()] = py::cast(new PyDomain(i->second));
  }
  return rv;
}

py::dict get_problems() {
  py::dict rv;
  for (auto i = Problem::begin(); i != Problem::end(); i++) {
    rv[i->first.c_str()] = py::cast(new PyProblem(i->second));
  }
  return rv;
}

void screw_pickle() {
  // for some reason pybind11 (or maybe Python?) insists that it knows how to
  // pickle my objects, and subsequently crashes everything when it tries to
  // "unpickle" uninitialised bytes
  throw std::runtime_error(
      "STOP! None of the pymdpsim objects can be pickled!"
  );
}

#define SCREW_PICKLE() .def("__reduce__", &screw_pickle)  \
    .def("__getstate__", &screw_pickle) \
    .def("__setstate__", &screw_pickle)

PYBIND11_PLUGIN(mdpsim) {
  py::module m("mdpsim", "Python interface to the MDPSim Probabilistic PDDL "
               "(PPDDL) simulator");

  py::class_<State, std::shared_ptr<State>>(m, "State")
    .def("goal", &State::goal)
    // .def_property_readonly("reward_so_far", &State::reward_so_far)
    SCREW_PICKLE();

  // Low-level API
  py::class_<PyTerm>(m, "Term")
    .def("__repr__", &PyTerm::repr)
    .def("__eq__", &PyTerm::eq)
    .def("__hash__", &PyTerm::hash)
    .def_property_readonly("type", &PyTerm::type)
    .def_property_readonly("name", &PyTerm::name)
    SCREW_PICKLE();
  py::class_<PyVariable, PyTerm>(m, "Variable")
    SCREW_PICKLE();
  py::class_<PyPDDLObject, PyTerm>(m, "PDDLObject")
    SCREW_PICKLE();
  py::class_<PyPredicate>(m, "Predicate")
    .def("__repr__", &PyPredicate::repr)
    .def_property_readonly("name", &PyPredicate::name)
    .def_property_readonly("arg_types", &PyPredicate::arg_types)
    SCREW_PICKLE();
  py::class_<PyProposition>(m, "Proposition")
    .def("__repr__", &PyProposition::repr)
    .def_property_readonly("identifier", &PyProposition::identifier)
    // Lifetime note: returns a PyProposition, which has a null destructor (as
    // it should---no need to delete internal structures)
    .def_property_readonly("predicate", &PyProposition::predicate)
    .def_property_readonly("in_goal", &PyProposition::in_goal)
    // Lifetime note: returns py::list of PyTerms; no lifetime management needed
    // (they'll die when they die)
    .def_property_readonly("terms", &PyProposition::terms)
    SCREW_PICKLE();
  py::class_<PyLiftedAction>(m, "LiftedAction")
    .def("__repr__", &PyLiftedAction::repr)
    .def_property_readonly("name", &PyLiftedAction::name)
    // Lifetime note: returns PyVariables and strings
    .def_property_readonly("parameters_and_types",
                           &PyLiftedAction::parameters_and_types)
    // list of (predicate, argument list) pairs; obviously won't work when you
    // have foralls, etc.
    .def_property_readonly("involved_propositions",
                           &PyLiftedAction::involved_propositions)
    SCREW_PICKLE();
  py::class_<PyGroundAction>(m, "GroundAction")
    .def("__repr__", &PyGroundAction::repr)
    .def_property_readonly("identifier", &PyGroundAction::identifier)
    .def_property_readonly("lifted_action", &PyGroundAction::lifted_action)
    .def_property_readonly("arguments", &PyGroundAction::arguments)
    .def_property_readonly("involved_propositions",
                           &PyGroundAction::involved_propositions)
    SCREW_PICKLE();
  py::class_<PyProblem>(m, "Problem")
    .def_property_readonly("propositions", &PyProblem::propositions)
    .def_property_readonly("ground_actions", &PyProblem::ground_actions)
    .def_property_readonly("domain", &PyProblem::domain)
    .def_property_readonly("name", &PyProblem::name)
    .def_property_readonly("num_actions", &PyProblem::num_actions)
    .def_property_readonly("num_props", &PyProblem::num_props)
    .def("prop_truth_mask", &PyProblem::prop_truth_mask)
    .def("act_applicable_mask", &PyProblem::act_applicable_mask)
    .def("init_state", &PyProblem::init_state)
    .def("intermediate_atom_state", &PyProblem::intermediate_atom_state)
    .def("apply", &PyProblem::apply)
    .def("applicable", &PyProblem::applicable)
    .def("__repr__", &PyProblem::repr)
    SCREW_PICKLE();
  py::class_<PyDomain>(m, "Domain")
    // FIXME: if you parse two PDDL files, the types for domains will get merged
    // together. I'm suspect this is a problem with `struct Domain` (not my
    // wrapper)
    .def_property_readonly("types", &PyDomain::types)
    .def_property_readonly("predicates", &PyDomain::predicates)
    .def_property_readonly("lifted_actions", &PyDomain::lifted_actions)
    .def_property_readonly("name", &PyDomain::name)
    .def("__repr__", &PyDomain::repr)
    SCREW_PICKLE();

  // interaction functions
  m.def("get_domains", &get_domains,
        "Dictionary mapping domain names to domains");
  m.def("get_problems", &get_problems,
        "Dictionary mapping problem names to problems");
  m.def("parse_file", &parse_file,
        "Parse domains and problems from given PDDL file");

#ifdef VERSION_INFO
  m.attr("__version__") = py::str(VERSION_INFO);
#else
  m.attr("__version__") = py::str("dev");
#endif

  return m.ptr();
}
