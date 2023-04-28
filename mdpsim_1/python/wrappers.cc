#include "pymdpsim.h"

// PyPredicate

PyPredicate::PyPredicate(const Domain *domain, const string name)
  : domain(domain), name_(name) {}

const string PyPredicate::name() const {
  return name_;
}

const string PyPredicate::repr() const {
  string out;
  out.append("Predicate(<").append(name_).append(">)");
  return name_;
}

const py::list PyPredicate::arg_types() {
  auto predicate = domain->predicates().find_predicate(name_);
  if (predicate == nullptr) {
    throw std::runtime_error("could not look up predicate by name");
  }

  const TypeList &params = PredicateTable::parameters(*predicate);
  py::list rv;
  for (auto i = params.begin(); i != params.end(); i++) {
    Type t = *i;
    rv.append(domain->types().find_name(t));
  }

  return rv;
}

// PyProposition

PyProposition::PyProposition(const Atom *atom, const Domain *domain, bool is_goal_)
  : atom(atom), domain(domain), is_goal_(is_goal_), has_goal_info_(true) {}

PyProposition::PyProposition(const Atom *atom, const Domain *domain)
  : atom(atom), domain(domain), is_goal_(false), has_goal_info_(false) {}

PyPredicate *PyProposition::predicate() {
  return new PyPredicate(domain, PredicateTable::name(atom->predicate()));
}

bool PyProposition::in_goal() {
  if (!has_goal_info_) {
    throw py::value_error("This proposition doesn't have an associated goal "
                          "flag (may be lifted)");
  }
  return is_goal_;
}

py::list PyProposition::terms() {
  py::list rv;
  TermList term_list = atom->terms();
  for (auto i = term_list.cbegin(); i != term_list.cend(); i++) {
    const Term term = *i;
    if (term.variable()) {
      const Variable var = term.as_variable();
      auto pyvar = PyVariable(var, domain);
      rv.append(pyvar);
    } else {
      const Object obj = term.as_object();
      auto pyobj = PyPDDLObject(obj, domain);
      rv.append(pyobj);
    }
  }
  return rv;
}

const string PyProposition::identifier() {
  stringstream out;
  out << *atom;
  return out.str();
}

const string PyProposition::repr() {
  stringstream out;
  // outputs something like Proposition(pred a1 a2 a3)
  out << "Proposition" << *atom;
  return out.str();
}

// PyTerm

PyTerm::PyTerm(const Term term, const Domain *domain)
  : term(term), domain(domain), index(term.index()) {}

long PyTerm::hash() const {
  return 0x1131eL ^ (long)index;
}

bool PyTerm::eq(PyTerm *other) const {
  int other_index = other->index;
  return other != nullptr && index == other_index;
}

const string PyTerm::type() const {
  const Type &t = TermTable::type(term);
  return domain->types().find_name(t);
}

const string PyTerm::name() const {
  stringstream out;
  out << term;
  return out.str();
}

// PyPDDLObject (subclass of PyTerm)

PyPDDLObject::PyPDDLObject(const Object object, const Domain *domain)
  : PyTerm(object, domain),
    object(object) {}

const string PyPDDLObject::repr() const {
  stringstream out;
  out << "Object(" << name() << " : " << type() << ")";
  return out.str();
}

// PyVariable (subclass of PyTerm)

PyVariable::PyVariable(const Variable variable, const Domain *domain)
  : PyTerm(variable, domain), variable(variable) {}

const string PyVariable::repr() const {
  stringstream out;
  out << "Variable(<" << (Term)variable << " (" << variable.index() << ")>)";
  return out.str();
}

// PyLiftedAction

PyLiftedAction::PyLiftedAction(const ActionSchema *schema, const Domain *domain)
  : schema(schema), domain(domain) {}

const string PyLiftedAction::name() const{
  return schema->name();
}

const string PyLiftedAction::repr() const {
  string out;
  out.append("LiftedAction(<").append(schema->name()).append(">)");
  return out;
}

const py::list PyLiftedAction::parameters_and_types() {
  py::list rv;
  auto parameters = schema->parameters();
  for (auto i = parameters.cbegin(); i != parameters.cend(); i++) {
    auto var = *i;
    auto type = TermTable::type(var);
    auto name = domain->types().find_name(type);

    rv.append(py::make_tuple(PyVariable(var, domain), name));
  }
  return rv;
}

// list of (probably lifted) propositions; obviously won't work when you have
// foralls, etc.
const py::list PyLiftedAction::involved_propositions() {
  py::list rv;
  AtomSet ignore_set;
  AtomList all_atoms;

  // get atoms from condition, then effect; final arg allows dupes
  schema->precondition().listAtoms(ignore_set, all_atoms, true);
  schema->effect().listAtoms(ignore_set, all_atoms, true);

  for (auto i = all_atoms.cbegin(); i != all_atoms.cend(); i++) {
    rv.append(PyProposition(*i, domain));
  }

  return rv;
}

// PyGroundAction

PyGroundAction::PyGroundAction(const Action *action, const Problem *problem)
  : action(action), problem(problem) {}

const PyLiftedAction PyGroundAction::lifted_action() const {
  auto schema = get_schema();
  return PyLiftedAction(schema, &problem->domain());
}

const ActionSchema *PyGroundAction::get_schema() const {
  // need to use name to look up schema
  ActionSchemaMap schema_table = problem->domain().actions();
  auto schema_name = action->name();
  return schema_table[schema_name];
}

const py::list PyGroundAction::arguments() const {
  py::list rv;
  auto obj_list = action->arguments();
  for (auto i = obj_list.cbegin(); i != obj_list.cend(); i++) {
    rv.append(PyPDDLObject(*i, &problem->domain()));
  }
  return rv;
}

const py::list PyGroundAction::involved_propositions() const {
  py::list rv;
  AtomSet ignore_set;
  AtomList all_atoms;

  action->precondition().listAtoms(ignore_set, all_atoms, true);
  action->effect().listAtoms(ignore_set, all_atoms, true);

  PyProblem pprob = PyProblem(problem);
  for (auto i = all_atoms.cbegin(); i != all_atoms.cend(); i++) {
    rv.append(PyProposition(*i, &problem->domain(),
                                  pprob.is_goal_atom(*i)));
  }

  return rv;
}

const string PyGroundAction::identifier() const {
  stringstream out;
  out << *action;
  return out.str();
}

const string PyGroundAction::repr() const {
  stringstream out;
  out << "Action" << *action;
  return out.str();
}

// PyProblem

PyProblem::PyProblem(const PyProblem &other)
  : problem(other.problem), cached_init_state(*problem) {
  init_maps();
};

PyProblem::PyProblem(const Problem *problem)
  : problem(problem), cached_init_state(*problem) {
  init_maps();
}

const string PyProblem::name() const {
  return problem->name();
}

const string PyProblem::repr() const {
  string out;
  out.append("Problem(<").append(problem->name()).append(">)");
  return out;
}

const py::list PyProblem::propositions() const {
  py::list rv;
  for (auto i = atom_vec.cbegin(); i != atom_vec.cend(); i++) {
    rv.append(PyProposition(*i, &problem->domain(), is_goal_atom(*i)));
  }
  return rv;
}

const py::list PyProblem::ground_actions() const {
  py::list rv;
  for (auto i = action_vec.cbegin(); i != action_vec.cend(); i++) {
    rv.append(PyGroundAction(*i, problem));
  }
  return rv;
}

bool PyProblem::is_goal_atom(const Atom *at) const {
  auto it = goal_atoms.find(at);
  return it != goal_atoms.end();
}

const PyDomain PyProblem::domain() const {
  return PyDomain(&problem->domain());
}

void PyProblem::init_maps() {
  build_maps(problem, atom_vec, action_vec);
  AtomList tmp;
  problem->goal().listAtoms(goal_atoms, tmp);
}

size_t PyProblem::num_actions() const {
  return action_vec.size();
}

size_t PyProblem::num_props() const {
  return atom_vec.size();
}

bool PyProblem::applicable(StatePtr state,
                           const PyGroundAction &action) const {
  return action.action->enabled(problem->terms(), state->atoms(), state->values());
}

StatePtr PyProblem::apply(StatePtr state,
                          const PyGroundAction &py_act) const {
  // apply action
  unique_ptr<State> next_state;
  if (!applicable(state, py_act)) {
    stringstream out;
    out << "Can't apply action '" << py_act.repr() << "' to state '" << state << "'";
    throw py::value_error(out.str());
  }
  // yes, ->next() really returns a reference to a newly-allocated object
  // also, it makes it bloody impossible to get back out goal reward, grrr
  State &heap_state = state->next(*(py_act.action));
  return StatePtr(&heap_state);
}

StatePtr PyProblem::init_state() const{
  // State retains a problem pointer, but problems don't get destroyed until
  // shutdown, so that's no biggie.
  return std::make_shared<State>(*problem);
}

StatePtr PyProblem::intermediate_atom_state(const string &props_true) const {
  auto props_begin = sregex_iterator(props_true.begin(), props_true.end(), prop_re);
  auto props_end = std::sregex_iterator();
  AtomSet true_atoms;
  for (auto prop_it = props_begin; prop_it != props_end; ++prop_it) {
    // get a particular prop string (just string like "p o1 o2 o3")
    auto match = *prop_it;
    if (match.size() != 2) {
      throw std::runtime_error("match not 2 items long---intermediate_state() bug!");
    }

    // now we need to split the joined tokens on spaces
    string joined_toks = match[1];
    auto tok_it = sregex_iterator(joined_toks.begin(), joined_toks.end(), tok_re);
    auto toks_end = std::sregex_iterator();
    if (tok_it == toks_end) {
      throw std::runtime_error("somehow got no tokens (?!)---intermediate_state() bug!");
    }
    // extract predicate first
    const string pred_name = tok_it->str();
    ++tok_it;
    // now terms
    std::vector<string> term_names;
    for (; tok_it != toks_end; ++tok_it) {
      term_names.emplace_back(tok_it->str());
    }

    // finally, construct atom
    const Atom *new_atom = getAtom(*problem, pred_name, term_names);
    if (!new_atom) {
      std::stringstream err;
      err << "could not construct atom from predicate '" << pred_name
          << "' and " << term_names.size() << " terms";
      for (const auto &s : term_names) {
        err << " '" << s << "'";
      }
      throw py::value_error(err.str());
    }
    true_atoms.insert(new_atom);
  }

  return std::make_shared<State>(*problem, true_atoms, cached_init_state.values());
}

py::list PyProblem::prop_truth_mask(const State &state) const {
  py::list rv;
  for (auto it = atom_vec.cbegin(); it != atom_vec.cend(); it++) {
    bool holds = (*it)->holds(problem->terms(),
                              state.atoms(),
                              state.values());
    bool in_goal = goal_atoms.find(*it) != goal_atoms.end();
    auto prop = PyProposition(*it, &problem->domain(), in_goal);
    rv.append(py::make_tuple(prop, holds));
  }
  assert(rv.size() == num_props());
  return rv;
}

py::list PyProblem::act_applicable_mask(const State &state) const {
  // tuples: true for enabled action, false for disabled action
  py::list rv;
  for (auto it = action_vec.cbegin(); it != action_vec.cend(); it++) {
    auto enabled = (*it)->enabled(problem->terms(), state.atoms(),
                                  state.values());
    auto act = PyGroundAction(*it, problem);
    rv.append(py::make_tuple(act, enabled));
  }
  assert(rv.size() == num_actions());
  return rv;
}

// PyDomain

PyDomain::PyDomain(const PyDomain &other) : domain(other.domain) {}

PyDomain::PyDomain(const Domain *domain) : domain(domain) {}

const string PyDomain::name() const {
  return domain->name();
}

const string PyDomain::repr() const {
  string out;
  out.append("Domain(<").append(domain->name()).append(">)");
  return out;
}

py::list PyDomain::types() {
  py::list rv;
  for (auto i : domain->types().names()) {
    rv.append(i);
  }
  return rv;
}

py::list PyDomain::predicates() {
  py::list rv;
  auto pred_map = domain->predicates().predicates();
  for (auto i = pred_map.cbegin(); i != pred_map.cend(); i++) {
    rv.append(new PyPredicate(domain, i->first));
  }
  return rv;
}

py::list PyDomain::lifted_actions() {
  py::list rv;
  // as_map is string (name) => ActionSchema* (actual spec)
  auto as_map = domain->actions();
  for (auto i = as_map.cbegin(); i != as_map.cend(); i++) {
    rv.append(PyLiftedAction(i->second, domain));
  }
  return rv;
}
