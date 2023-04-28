#ifndef _PYMDPSIM_H
#define _PYMDPSIM_H

#include <iostream>
#include <string>
#include <cstdlib>
#include <stdexcept>
#include <cassert>
#include <vector>
#include <map>
#include <memory>
#include <regex>
#include <sstream>
#include <utility>
#include <type_traits>

#include "pybind11/pybind11.h"
#include "pybind11/stl.h"

#include "states.h"
#include "problems.h"
#include "domains.h"

using std::string;
using std::vector;
using std::map;
using std::pair;
using std::unique_ptr;
using std::make_unique;
using std::shared_ptr;
using std::make_shared;
using std::make_pair;
using std::stringstream;
using std::regex;
using std::smatch;
using std::sregex_iterator;
namespace py = pybind11;

typedef std::shared_ptr<State> StatePtr;

class PyDomain;
class PyProblem;

void build_maps(const Problem *, AtomList &, vector<const Action*> &);

class PyTerm {
 public:
  explicit PyTerm(const Term term, const Domain *domain);
  virtual ~PyTerm() {};
  bool is_variable() const {return term.variable();}
  bool is_object() const {return term.object();}
  long hash() const;
  bool eq(PyTerm *other) const;
  virtual const string repr() const = 0;
  const string type() const;
  const string name() const;

 private:
  const Term term;
  const Domain *domain;
  const int index;
};

class PyPDDLObject : public PyTerm {
public:
  PyPDDLObject(const Object object, const Domain *domain);
  const string repr() const override;

private:
  const Object object;
};

class PyVariable : public PyTerm {
 public:
  explicit PyVariable(const Variable variable, const Domain *domain);
  const string repr() const override;

 private:
  const Variable variable;
};

class PyPredicate {
public:
  PyPredicate(const Domain *domain, const string name);
  const string name() const;
  const string repr() const;
  const py::list arg_types();

private:
  const Domain *domain;
  const string name_;
};

class PyProposition {
public:
  PyProposition(const Atom *atom, const Domain *domain, bool is_goal_);
  PyProposition(const Atom *atom, const Domain *domain);
  PyPredicate *predicate();
  bool in_goal();
  // objects and/or variables involved in this proposition
  py::list terms();
  const string identifier();
  const string repr();
  const bool is_ground();

private:
  const Atom *atom;
  const Domain *domain;
  bool is_goal_;
  bool has_goal_info_;
};

class PyLiftedAction {
public:
  PyLiftedAction(const ActionSchema *schema, const Domain *domain);
  const string name() const;
  const string repr() const;
  const py::list parameters_and_types();
  // all effect and precondition propositions, including duplicates
  const py::list involved_propositions();

private:
  const ActionSchema *schema;
  const Domain *domain;
};

class PyGroundAction {
public:
  PyGroundAction(const Action *action, const Problem *problem);
  const PyLiftedAction lifted_action() const;
  const py::list arguments() const;
  const py::list involved_propositions() const;
  const string identifier() const;
  const string repr() const;

private:
  const Action *action;
  const Problem *problem;

  const ActionSchema *get_schema() const;

  friend PyProblem;
};

class PyProblem {
public:
  PyProblem(const PyProblem &other);
  PyProblem(const Problem *problem);
  const string name() const;
  const string repr() const;
  const py::list propositions() const;
  const py::list ground_actions() const;
  bool is_goal_atom(const Atom *at) const;
  const PyDomain domain() const;
  // new stuff:
  py::list prop_truth_mask(const State &state) const;
  py::list act_applicable_mask(const State &state) const;
  StatePtr init_state() const;
  // constructs an intermediate state, caring ONLY about atoms
  StatePtr intermediate_atom_state(const string &props_true) const;
  StatePtr apply(StatePtr state, const PyGroundAction &action) const;
  bool applicable(StatePtr state, const PyGroundAction &action) const;
  size_t num_actions() const;
  size_t num_props() const;
  // Note that you can use problem->goal().progress(terms, atoms, values) to get
  // FPG-style progress measures. Not implemented now for lack of need.

private:
  const Problem *problem;
  AtomSet goal_atoms;
  AtomList atom_vec;
  vector<const Action*> action_vec;

  // for getting initial fluent values
  const State cached_init_state;

  // "modified ES262 syntax" (makes it easy to test =D)
  const regex prop_re = regex("\\s*((?:[^,\\s]+\\s*)+)(?:,|$)");
  const regex tok_re = regex("[^\\s]+");

  void init_maps();
};

class PyDomain {
 public:
  PyDomain(const PyDomain &other);
  PyDomain(const Domain *domain);
  const string name() const;
  const string repr() const;
  py::list types();
  py::list predicates();
  py::list lifted_actions();

 private:
  const Domain *domain;
};

#endif // _PYMDPSIM_H
