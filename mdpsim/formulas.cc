/*
 * Copyright 2003-2005 Carnegie Mellon University and Rutgers University
 * Copyright 2007 Håkan Younes
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "formulas.h"
//#include "states.h"
//#include "problems.h"
//#include "domains.h"
//#include "expressions.h"
//#include "exceptions.h"
//#include "strxml.h"
#include <stack>
#include <stdexcept>

/* ====================================================================== */
/* Truthiness (three-valued logic) */

Truthiness operator&&(Truthiness a, Truthiness b) {
  if (a == Truthiness::CONTRADICTION || b == Truthiness::CONTRADICTION) {
    return Truthiness::CONTRADICTION;
  } else if (a == Truthiness::TAUTOLOGY && b == Truthiness::TAUTOLOGY) {
    return Truthiness::TAUTOLOGY;
  }
  return Truthiness::INDETERMINATE;
}

Truthiness operator||(Truthiness a, Truthiness b) {
  if (a == Truthiness::TAUTOLOGY || b == Truthiness::TAUTOLOGY) {
    return Truthiness::TAUTOLOGY;
  } else if (a == Truthiness::CONTRADICTION && b == Truthiness::CONTRADICTION) {
    return Truthiness::CONTRADICTION;
  }
  return Truthiness::INDETERMINATE;
}

Truthiness operator!(Truthiness a) {
  switch (a) {
  case Truthiness::TAUTOLOGY:
    return Truthiness::CONTRADICTION;
  case Truthiness::CONTRADICTION:
    return Truthiness::TAUTOLOGY;
  default:
    return Truthiness::INDETERMINATE;
  }
}

// handles false < unknown < true comparison
bool operator<(Truthiness a, Truthiness b) {
  switch (b) {
  case Truthiness::CONTRADICTION:
    return false;
  case Truthiness::INDETERMINATE:
    return a == Truthiness::CONTRADICTION;
  default: // case Truthiness::TAUTOLOGY:
    return a == Truthiness::CONTRADICTION || a == Truthiness::INDETERMINATE;
  }
}

/* ====================================================================== */
/* Constant */

/*
 * A state formula with a constant truth value.
 */
struct Constant : public StateFormula {
  /* Tests if this state formula holds in the given state. */
  virtual bool holds(const TermTable& terms, const AtomSet& atoms,
                     const ValueMap& values) const {
    return value_;
  }

    /* Tests if this state formula holds in the given state. */
  bool holds_noValues(const TermTable& terms, const AtomSet& atoms) const {
    return value_;
  }

    /* Progress estimator. */
  int progress(const TermTable& terms, const AtomSet& atoms,
               const ValueMap& values) const {
    return 0;
  }

  /* Returns an instantiation of this state formula. */
  virtual const Constant& instantiation(const SubstitutionMap& subst,
                                        const TermTable& terms,
                                        const AtomSet& atoms,
                                        const ValueMap& values,
                                        bool state) const {
    return *this;
  }

  virtual void listAtoms(AtomSet& atomsS, AtomList &atomsL, bool dupes) const {}

 protected:
  /* Prints this object on the given stream. */
  virtual void print(std::ostream& os) const {
    os << (value_ ? "(and)" : "(or)");
  }

 private:
  /* Constant representing true. */
  static const Constant TRUE_;
  /* Constant representing false. */
  static const Constant FALSE_;

  /* Value of this constant. */
  bool value_;

  /* Constructs a constant state formula. */
  explicit Constant(bool value) : value_(value) { ref(this); }

  friend struct StateFormula;
};


/* Constant representing true. */
const Constant Constant::TRUE_(true);
/* Constant representing false. */
const Constant Constant::FALSE_(false);


/* ====================================================================== */
/* StateFormula */

/* A formula true in every state. */
const StateFormula& StateFormula::TRUE = Constant::TRUE_;
/* A formula false in every state. */
const StateFormula& StateFormula::FALSE = Constant::FALSE_;


/* Conjunction operator for state formulas. */
const StateFormula& operator&&(const StateFormula& f1,
                               const StateFormula& f2) {
  if (&f1 == &f2) {
    return f1;
  } /* else if (f1.contradiction()) {
    RCObject::ref(&f2);
    RCObject::destructive_deref(&f2);
    return f1;
  } else if (f2.contradiction()) {
    RCObject::ref(&f1);
    RCObject::destructive_deref(&f1);
    return f2;
  } else if (f1.tautology()) {
    return f2;
  } else if (f2.tautology()) {
    return f1;
  } */ else {
    Conjunction& conjunction = *new Conjunction();
    const Conjunction* c1 = dynamic_cast<const Conjunction*>(&f1);
    if (c1 != 0) {
      for (FormulaList::const_iterator fi = c1->conjuncts().begin();
           fi != c1->conjuncts().end(); fi++) {
        conjunction.add_conjunct(**fi);
      }
      RCObject::ref(c1);
      RCObject::destructive_deref(c1);
    } else {
      conjunction.add_conjunct(f1);
    }
    const Conjunction* c2 = dynamic_cast<const Conjunction*>(&f2);
    if (c2 != 0) {
      for (FormulaList::const_iterator fi = c2->conjuncts().begin();
           fi != c2->conjuncts().end(); fi++) {
        conjunction.add_conjunct(**fi);
      }
      RCObject::ref(c2);
      RCObject::destructive_deref(c2);
    } else {
      conjunction.add_conjunct(f2);
    }
    return conjunction;
  }
}


/* Disjunction operator for state formulas. */
const StateFormula& operator||(const StateFormula& f1,
                               const StateFormula& f2) {
  if (&f1 == &f2) {
    return f1;
  } /* else if (f1.tautology()) {
    RCObject::ref(&f2);
    RCObject::destructive_deref(&f2);
    return f1;
  } else if (f2.tautology()) {
    RCObject::ref(&f1);
    RCObject::destructive_deref(&f1);
    return f2;
  } else if (f1.contradiction()) {
    return f2;
  } else if (f2.contradiction()) {
    return f1;
  } */ else {
    Disjunction& disjunction = *new Disjunction();
    const Disjunction* d1 = dynamic_cast<const Disjunction*>(&f1);
    if (d1 != 0) {
      for (FormulaList::const_iterator fi = d1->disjuncts().begin();
           fi != d1->disjuncts().end(); fi++) {
        disjunction.add_disjunct(**fi);
      }
      RCObject::ref(d1);
      RCObject::destructive_deref(d1);
    } else {
      disjunction.add_disjunct(f1);
    }
    const Disjunction* d2 = dynamic_cast<const Disjunction*>(&f2);
    if (d2 != 0) {
      for (FormulaList::const_iterator fi = d2->disjuncts().begin();
           fi != d2->disjuncts().end(); fi++) {
        disjunction.add_disjunct(**fi);
      }
      RCObject::ref(d2);
      RCObject::destructive_deref(d2);
    } else {
      disjunction.add_disjunct(f2);
    }
    return disjunction;
  }
}


/* Output operator for state formulas. */
std::ostream& operator<<(std::ostream& os, const StateFormula& f) {
  f.print(os);
  return os;
}


/* ====================================================================== */
/* TruthyWrapper */


const StateFormula &TruthyWrapper::make(const StateFormula &wrapped_, Truthiness truthiness_) {
  if (truthiness_ == Truthiness::INDETERMINATE) {
    return wrapped_;
  }
  return *new TruthyWrapper(wrapped_, truthiness_);
}


TruthyWrapper::TruthyWrapper(const StateFormula &wrapped, Truthiness truthiness_)
  : truthiness_(truthiness_), wrapped_(&wrapped)  {
  if (truthiness_ != Truthiness::TAUTOLOGY
      && truthiness_ != Truthiness::CONTRADICTION) {
    throw std::domain_error("TruthyWrapper can only handle true/false");
  }
  ref(wrapped_);
}


TruthyWrapper::~TruthyWrapper() {
  destructive_deref(wrapped_);
}


/* Tests if this state formula holds in the given state. */
bool TruthyWrapper::holds(const TermTable& terms,
                     const AtomSet& atoms, const ValueMap& values) const {
  return wrapped_->holds(terms, atoms, values);
}


/* Tests if this state formula holds in the given state. */
bool TruthyWrapper::holds_noValues(const TermTable& terms,
		     const AtomSet& atoms) const {
  return wrapped_->holds_noValues(terms, atoms);
}


/* Progress estimator. */
int TruthyWrapper::progress(const TermTable& terms,
		 const AtomSet& atoms, const ValueMap& values) const {
    return wrapped_->progress(terms, atoms, values);
}

/* Returns an instantiation of this state formula. */
const StateFormula& TruthyWrapper::instantiation(const SubstitutionMap& subst,
                                            const TermTable& terms,
                                            const AtomSet& atoms,
                                            const ValueMap& values,
                                            bool state) const {
  // the wrapped thing will build another TruthyWrapper if it needs it
  return wrapped_->instantiation(subst, terms, atoms, values, state);
}


/* Prints this object on the given stream. */
void TruthyWrapper::print(std::ostream& os) const {
  if (wrapped_ != nullptr) {
    os << *wrapped_;
  } else {
    os << wrapped_;
  }
}


/* Enumerate all atoms appearing in this state formula */
void TruthyWrapper::listAtoms(AtomSet& atomsS, AtomList& atomsL, bool dupes)
  const {
  wrapped_->listAtoms(atomsS, atomsL, dupes);
}


/* ====================================================================== */
/* Constant */

/* ====================================================================== */
/* Atom */

/* Table of atoms. */
Atom::AtomTable Atom::atoms;


/* Comparison function. */
bool Atom::AtomLess::operator()(const Atom* a1, const Atom* a2) const {
  if (a1->predicate() < a2->predicate()) {
    return true;
  } else if (a2->predicate() < a1->predicate()) {
    return false;
  } else {
    return a1->terms() < a2->terms();
  }
}


/* Returns an atom with the given predicate and terms. */
const Atom& Atom::make(Predicate predicate, const TermList& terms) {
  Atom* atom = new Atom(predicate);
  bool ground = true;
  for (TermList::const_iterator ti = terms.begin(); ti != terms.end(); ti++) {
    atom->add_term(*ti);
    if (ground && (*ti).variable()) {
      ground = false;
    }
  }
  if (!ground) {
    return *atom;
  } else {
    std::pair<AtomTable::const_iterator, bool> result = atoms.insert(atom);
    if (!result.second) {
      delete atom;
      return **result.first;
    } else {
      return *atom;
    }
  }
}


/* Deletes this atom. */
Atom::~Atom() {
  AtomTable::const_iterator ai = atoms.find(this);
  if (*ai == this) {
    atoms.erase(ai);
  }
}


/* Tests if this state formula holds in the given state. */
bool Atom::holds(const TermTable& terms,
                 const AtomSet& atoms, const ValueMap& values) const {
  return atoms.find(this) != atoms.end();
}

/* Tests if this state formula holds in the given state. */
bool Atom::holds_noValues(const TermTable& terms,
		 const AtomSet& atoms) const {
    return atoms.find(this) != atoms.end();
}

/* Progress estimator. */
int Atom::progress(const TermTable& terms,
		 const AtomSet& atoms, const ValueMap& values) const {
    return (atoms.find(this) != atoms.end()) ? 1 : 0;
}


/* Returns this atom subject to the given substitution. */
const Atom& Atom::substitution(const SubstitutionMap& subst) const {
  TermList inst_terms;
  bool substituted = false;
  for (TermList::const_iterator ti = terms().begin();
       ti != terms().end(); ti++) {
    SubstitutionMap::const_iterator si =
      (*ti).variable() ? subst.find((*ti).as_variable()) : subst.end();
    if (si != subst.end()) {
      inst_terms.push_back((*si).second);
      substituted = true;
    } else {
      inst_terms.push_back(*ti);
    }
  }
  if (substituted) {
    return make(predicate(), inst_terms);
  } else {
    return *this;
  }
}


/* Returns an instantiation of this state formula. */
const StateFormula& Atom::instantiation(const SubstitutionMap& subst,
                                        const TermTable& terms,
                                        const AtomSet& atoms,
                                        const ValueMap& values,
                                        bool state) const {
  if (this->terms().empty()) {
    if (state || PredicateTable::static_predicate(predicate())) {
      Truthiness new_truth;
      if (atoms.find(this) != atoms.end()) {
        new_truth = Truthiness::TAUTOLOGY;
      } else {
        new_truth = Truthiness::CONTRADICTION;
      }
      return TruthyWrapper::make(*this, new_truth);
    }
    return *this;
  } else {
    TermList inst_terms;
    bool substituted = false;
    size_t objects = 0;
    for (TermList::const_iterator ti = this->terms().begin();
         ti != this->terms().end(); ti++) {
      SubstitutionMap::const_iterator si =
        (*ti).variable() ? subst.find((*ti).as_variable()) : subst.end();
      if (si != subst.end()) {
        inst_terms.push_back((*si).second);
        substituted = true;
        objects++;
      } else {
        inst_terms.push_back(*ti);
        if ((*ti).object()) {
          objects++;
        }
      }
    }
    if (substituted) {
      const Atom& inst_atom = make(predicate(), inst_terms);
      if ((state || PredicateTable::static_predicate(predicate()))
          && objects == inst_terms.size()) {
        Truthiness new_truth;
        if (atoms.find(&inst_atom) != atoms.end()) {
          new_truth = Truthiness::TAUTOLOGY;
        } else {
          new_truth = Truthiness::CONTRADICTION;
        }
        return TruthyWrapper::make(inst_atom, new_truth);
      }
      return inst_atom;
    }
    return *this;
  }
}

void Atom::listAtoms(AtomSet& atomsS, AtomList &atomsL, bool dupes) const {
  auto rv = atomsS.insert(this);
  if (rv.second || dupes)
    atomsL.push_back(this);
}

/* Prints this object on the given stream. */
void Atom::print(std::ostream& os) const {
  os << '(' << predicate();
  for (TermList::const_iterator ti = terms().begin();
       ti != terms().end(); ti++) {
    os << ' ' << *ti;
  }
  os << ')';
}


/* ==================================================================== */
/* Equality between terms.                                              */
/* Can be verified statically, so no holds* or progress* methods needed */

/* Returns an equality of the two terms. */
const StateFormula& Equality::make(const Term& term1, const Term& term2) {
  if (term1 == term2) {
    return TRUE;
  } else if (term1.object() && term2.object() && term1 != term2) {
    return FALSE;
  } else {
    return *new Equality(term1, term2);
  }
}


/* Tests if this state formula holds in the given state. */
bool Equality::holds(const TermTable& terms,
                     const AtomSet& atoms, const ValueMap& values) const {
  throw std::logic_error("Equality::holds not implemented");
}

/* Tests if this state formula holds in the given state. */
bool Equality::holds_noValues(const TermTable& terms,
		     const AtomSet& atoms) const {
    throw std::logic_error("Equality::holds erroneously called");
}

/* Progress estimator. */
int Equality::progress(const TermTable& terms,
		 const AtomSet& atoms, const ValueMap& values) const {
    throw std::logic_error("Equality::progress erroneously called");
}

/* Returns an instantiation of this state formula. */
const StateFormula& Equality::instantiation(const SubstitutionMap& subst,
                                            const TermTable& terms,
                                            const AtomSet& atoms,
                                            const ValueMap& values,
                                            bool state) const {
  Term inst_term1 = term1();
  if (inst_term1.variable()) {
    const SubstitutionMap::const_iterator si =
      subst.find(inst_term1.as_variable());
    if (si != subst.end()) {
      inst_term1 = (*si).second;
    }
  }
  Term inst_term2 = term2();
  if (inst_term2.variable()) {
    const SubstitutionMap::const_iterator si =
      subst.find(inst_term2.as_variable());
    if (si != subst.end()) {
      inst_term2 = (*si).second;
    }
  }
  if (inst_term1 == term1() && inst_term2 == term2()) {
    return *this;
  } else {
    return make(inst_term1, inst_term2);
  }
}


/* Prints this object on the given stream. */
void Equality::print(std::ostream& os) const {
  os << "(= " << term1() << ' ' << term2() << ")";
}


void Equality::listAtoms(AtomSet &, AtomList &, bool) const {}


/* ====================================================================== */
/* Comparison */

/* Constructs a comparison. */
Comparison::Comparison(const Expression& expr1, const Expression& expr2)
  : expr1_(&expr1), expr2_(&expr2) {
  ref(expr1_);
  ref(expr2_);
}


/* Deletes this comparison. */
Comparison::~Comparison() {
  destructive_deref(expr1_);
  destructive_deref(expr2_);
}


void Comparison::listAtoms(AtomSet &, AtomList &, bool) const {}


/* ====================================================================== */
/* LessThan */

/* Returns a less-than comparison of the two expressions. */
const StateFormula& LessThan::make(const Expression& expr1,
                                   const Expression& expr2) {
  const Value* v1 = dynamic_cast<const Value*>(&expr1);
  if (v1 != 0) {
    const Value* v2 = dynamic_cast<const Value*>(&expr2);
    if (v2 != 0) {
      const StateFormula& value = (v1->value() < v2->value()) ? TRUE : FALSE;
      ref(v1);
      ref(v2);
      destructive_deref(v1);
      destructive_deref(v2);
      return value;
    }
  }
  return *new LessThan(expr1, expr2);
}


/* Tests if this state formula holds in the given state. */
bool LessThan::holds(const TermTable& terms,
                     const AtomSet& atoms, const ValueMap& values) const {
  return expr1().value(values) < expr2().value(values);
}


/* Tests if this state formula holds in the given state. */
 bool LessThan::holds_noValues(const TermTable& terms,
		     const AtomSet& atoms) const {
     return true;
 }

/* Returns an instantiation of this state formula. */
const StateFormula& LessThan::instantiation(const SubstitutionMap& subst,
                                            const TermTable& terms,
                                            const AtomSet& atoms,
                                            const ValueMap& values,
                                            bool state) const {
  const Expression& inst_expr1 = expr1().instantiation(subst, values);
  const Expression& inst_expr2 = expr2().instantiation(subst, values);
  if (&inst_expr1 == &expr1() && &inst_expr2 == &expr2()) {
    return *this;
  } else {
    return make(inst_expr1, inst_expr2);
  }
}

/* Progress estimator. */
int LessThan::progress(const TermTable& terms,
		 const AtomSet& atoms, const ValueMap& values) const {
       return (expr1().value(values) < expr2().value(values)) ? 1 : 0;
}

/* Prints this object on the given stream. */
void LessThan::print(std::ostream& os) const {
  os << "(< " << expr1() << ' ' << expr2() << ")";
}


/* ====================================================================== */
/* LessThanOrEqualTo */

/* Returns a less-than-or-equal-to comparison of the two expressions. */
const StateFormula& LessThanOrEqualTo::make(const Expression& expr1,
                                            const Expression& expr2) {
  const Value* v1 = dynamic_cast<const Value*>(&expr1);
  if (v1 != 0) {
    const Value* v2 = dynamic_cast<const Value*>(&expr2);
    if (v2 != 0) {
      const StateFormula& value = (v1->value() <= v2->value()) ? TRUE : FALSE;
      ref(v1);
      ref(v2);
      destructive_deref(v1);
      destructive_deref(v2);
      return value;
    }
  }
  return *new LessThanOrEqualTo(expr1, expr2);
}


/* Tests if this state formula holds in the given state. */
bool LessThanOrEqualTo::holds(const TermTable& terms,
                              const AtomSet& atoms,
                              const ValueMap& values) const {
  return expr1().value(values) <= expr2().value(values);
}

/* Tests if this state formula holds in the given state. */
bool LessThanOrEqualTo::holds_noValues(const TermTable& terms,
		     const AtomSet& atoms) const {
     return true;
 }


/* Returns an instantiation of this state formula. */
const StateFormula&
LessThanOrEqualTo::instantiation(const SubstitutionMap& subst,
                                 const TermTable& terms,
                                 const AtomSet& atoms,
                                 const ValueMap& values,
                                 bool state) const {
  const Expression& inst_expr1 = expr1().instantiation(subst, values);
  const Expression& inst_expr2 = expr2().instantiation(subst, values);
  if (&inst_expr1 == &expr1() && &inst_expr2 == &expr2()) {
    return *this;
  } else {
    return make(inst_expr1, inst_expr2);
  }
}

/* Progress estimator. */
 int LessThanOrEqualTo::progress(const TermTable& terms,
		 const AtomSet& atoms, const ValueMap& values) const {
     return (expr1().value(values) <= expr2().value(values)) ? 1 : 0;
}

/* Prints this object on the given stream. */
void LessThanOrEqualTo::print(std::ostream& os) const {
  os << "(<= " << expr1() << ' ' << expr2() << ")";
}


/* ====================================================================== */
/* EqualTo */

/* Returns a equal-to comparison of the two expressions. */
const StateFormula& EqualTo::make(const Expression& expr1,
                                  const Expression& expr2) {
  const Value* v1 = dynamic_cast<const Value*>(&expr1);
  if (v1 != 0) {
    const Value* v2 = dynamic_cast<const Value*>(&expr2);
    if (v2 != 0) {
      const StateFormula& value = (v1->value() == v2->value()) ? TRUE : FALSE;
      ref(v1);
      ref(v2);
      destructive_deref(v1);
      destructive_deref(v2);
      return value;
    }
  }
  return *new EqualTo(expr1, expr2);
}


/* Tests if this state formula holds in the given state. */
bool EqualTo::holds(const TermTable& terms,
                    const AtomSet& atoms, const ValueMap& values) const {
  return expr1().value(values) == expr2().value(values);
}


/* Tests if this state formula holds in the given state. */
 bool EqualTo::holds_noValues(const TermTable& terms,
		     const AtomSet& atoms) const {
     return true;
 }

/* Returns an instantiation of this state formula. */
const StateFormula& EqualTo::instantiation(const SubstitutionMap& subst,
					   const TermTable& terms,
					   const AtomSet& atoms,
					   const ValueMap& values,
					   bool state) const {
  const Expression& inst_expr1 = expr1().instantiation(subst, values);
  const Expression& inst_expr2 = expr2().instantiation(subst, values);
  if (&inst_expr1 == &expr1() && &inst_expr2 == &expr2()) {
    return *this;
  } else {
    return make(inst_expr1, inst_expr2);
  }
}

/* Progress estimator. */
 int EqualTo::progress(const TermTable& terms,
		 const AtomSet& atoms, const ValueMap& values) const {
     return (expr1().value(values) == expr2().value(values)) ? 1 : 0;
 }

/* Prints this object on the given stream. */
void EqualTo::print(std::ostream& os) const {
  os << "(= " << expr1() << ' ' << expr2() << ")";
}


/* ====================================================================== */
/* GreaterThanOrEqualTo */

/* Returns a greater-than-or-equal-to comparison of the two expressions. */
const StateFormula& GreaterThanOrEqualTo::make(const Expression& expr1,
                                               const Expression& expr2) {
  const Value* v1 = dynamic_cast<const Value*>(&expr1);
  if (v1 != 0) {
    const Value* v2 = dynamic_cast<const Value*>(&expr2);
    if (v2 != 0) {
      const StateFormula& value = (v1->value() >= v2->value()) ? TRUE : FALSE;
      ref(v1);
      ref(v2);
      destructive_deref(v1);
      destructive_deref(v2);
      return value;
    }
  }
  return *new GreaterThanOrEqualTo(expr1, expr2);
}


/* Tests if this state formula holds in the given state. */
bool GreaterThanOrEqualTo::holds(const TermTable& terms,
                                 const AtomSet& atoms,
                                 const ValueMap& values) const {
  return expr1().value(values) >= expr2().value(values);
}

/* Tests if this state formula holds in the given state. */
 bool GreaterThanOrEqualTo::holds_noValues(const TermTable& terms,
		     const AtomSet& atoms) const {
     return true;
 }


/* Returns an instantiation of this state formula. */
const StateFormula&
GreaterThanOrEqualTo::instantiation(const SubstitutionMap& subst,
                                    const TermTable& terms,
                                    const AtomSet& atoms,
                                    const ValueMap& values,
                                    bool state) const {
  const Expression& inst_expr1 = expr1().instantiation(subst, values);
  const Expression& inst_expr2 = expr2().instantiation(subst, values);
  if (&inst_expr1 == &expr1() && &inst_expr2 == &expr2()) {
    return *this;
  } else {
    return make(inst_expr1, inst_expr2);
  }
}

/* Progress estimator. */
 int GreaterThanOrEqualTo::progress(const TermTable& terms,
		 const AtomSet& atoms, const ValueMap& values) const {
     return (expr1().value(values) >= expr2().value(values)) ? 1 : 0;
}


/* Prints this object on the given stream. */
void GreaterThanOrEqualTo::print(std::ostream& os) const {
  os << "(>= " << expr1() << ' ' << expr2() << ")";
}


/* ====================================================================== */
/* GreaterThan */

/* Returns a greater-than comparison of the two expressions. */
const StateFormula& GreaterThan::make(const Expression& expr1,
                                      const Expression& expr2) {
  const Value* v1 = dynamic_cast<const Value*>(&expr1);
  if (v1 != 0) {
    const Value* v2 = dynamic_cast<const Value*>(&expr2);
    if (v2 != 0) {
      const StateFormula& value = (v1->value() > v2->value()) ? TRUE : FALSE;
      ref(v1);
      ref(v2);
      destructive_deref(v1);
      destructive_deref(v2);
      return value;
    }
  }
  return *new GreaterThan(expr1, expr2);
}


/* Tests if this state formula holds in the given state. */
bool GreaterThan::holds(const TermTable& terms,
                        const AtomSet& atoms, const ValueMap& values) const {
  return expr1().value(values) > expr2().value(values);
}

/* Tests if this state formula holds in the given state. */
 bool GreaterThan::holds_noValues(const TermTable& terms,
		     const AtomSet& atoms) const {
     return true;
 }


/* Returns an instantiation of this state formula. */
const StateFormula& GreaterThan::instantiation(const SubstitutionMap& subst,
                                               const TermTable& terms,
                                               const AtomSet& atoms,
                                               const ValueMap& values,
                                               bool state) const {
  const Expression& inst_expr1 = expr1().instantiation(subst, values);
  const Expression& inst_expr2 = expr2().instantiation(subst, values);
  if (&inst_expr1 == &expr1() && &inst_expr2 == &expr2()) {
    return *this;
  } else {
    return make(inst_expr1, inst_expr2);
  }
}

/* Progress estimator. */
 int GreaterThan::progress(const TermTable& terms,
		 const AtomSet& atoms, const ValueMap& values) const {
     return (expr1().value(values) > expr2().value(values)) ? 1 : 0;
 }

/* Prints this object on the given stream. */
// void GreaterThan::randomState(State* s,
//                            const Problem& problem) const {
//     std::cerr<<"GreaterThan::randomState - Too bad we end up here..."<<std::endl;
// }

/* Prints this object on the given stream. */
void GreaterThan::print(std::ostream& os) const {
  os << "(> " << expr1() << ' ' << expr2() << ")";
}


/* ====================================================================== */
/* Negation */

/* Returns the negation of the given state formula. */
const StateFormula& Negation::make(const StateFormula& formula) {
  const StateFormula &rv = *new Negation(formula);
  if (formula.tautology()) {
    return TruthyWrapper::make(rv, Truthiness::CONTRADICTION);
  } else if (formula.contradiction()) {
    return TruthyWrapper::make(rv, Truthiness::TAUTOLOGY);
  }
  return rv;
}


/* Constructs a negated state formula. */
Negation::Negation(const StateFormula& negand)
  : negand_(&negand) {
  ref(negand_);
}


/* Deletes this negation. */
Negation::~Negation() {
  destructive_deref(negand_);
}


/* Tests if this state formula holds in the given state. */
bool Negation::holds(const TermTable& terms,
                     const AtomSet& atoms, const ValueMap& values) const {
  return !negand().holds(terms, atoms, values);
}

/* Tests if this state formula holds in the given state. */
bool Negation::holds_noValues(const TermTable& terms,
		     const AtomSet& atoms) const {
    /* XXX(sam, 2023-07-07): holds_noValues() for negative conditions is really
     * dicey: for atoms etc., holds_noValues should simply return true iff that
     * atom has been instantiated, regardless of its value, thanks to the closed
     * world assumption. Evaluating negation is much harder, since we need to
     * check whether it was ever the case that the negated condition was not true
     * in the past. This cannot be accomplished just by looking at the atom set.
     *
     * For now I'm side-stepping this by always returning true, which is an
     * over-approximation of the thing we really want. This may or may not be okay.
     *
     * return !negand().holds_noValues(terms, atoms);
     */
    return true;
}

/* Progress estimator. */
int Negation::progress(const TermTable& terms,
		 const AtomSet& atoms, const ValueMap& values) const {
    return -negand().progress(terms, atoms, values);
}

/* Returns an instantiation of this state formula. */
const StateFormula& Negation::instantiation(const SubstitutionMap& subst,
                                            const TermTable& terms,
                                            const AtomSet& atoms,
                                            const ValueMap& values,
                                            bool state) const {
  const StateFormula& inst_negand =
    negand().instantiation(subst, terms, atoms, values, state);
  if (&inst_negand == &negand()) {
    return *this;
  } else {
    return make(inst_negand);
  }
}


/* Prints this object on the given stream. */
void Negation::print(std::ostream& os) const {
  os << "(not " << negand() << ")";
}


/* Enumerate all atoms appearing in this state formula */
void Negation::listAtoms(AtomSet& atomsS, AtomList& atomsL, bool dupes)
  const {
  negand_->listAtoms(atomsS, atomsL, dupes);
}

/* ====================================================================== */
/* Conjunction. */

/* Deletes this conjunction. */
Conjunction::~Conjunction() {
  for (FormulaList::const_iterator fi = conjuncts().begin();
       fi != conjuncts().end(); fi++) {
    destructive_deref(*fi);
  }
}


/* Adds a conjunct to this conjunction. */
void Conjunction::add_conjunct(const StateFormula& conjunct) {
  conjuncts_.push_back(&conjunct);
  ref(&conjunct);
}


/* Tests if this state formula holds in the given state. */
bool Conjunction::holds(const TermTable& terms,
                        const AtomSet& atoms, const ValueMap& values) const {
  for (FormulaList::const_iterator fi = conjuncts().begin();
       fi != conjuncts().end(); fi++) {
    if (!(*fi)->holds(terms, atoms, values)) {
      return false;
    }
  }
  return true;
}

/* Tests if this state formula holds in the given state. */
bool Conjunction::holds_noValues(const TermTable& terms,
			const AtomSet& atoms) const {
  for (FormulaList::const_iterator fi = conjuncts_.begin();
       fi != conjuncts_.end(); fi++) {
      if (!(*fi)->holds_noValues(terms, atoms)) {
      return false;
    }
  }
  return true;
}

/* Progress estimator. */
int Conjunction::progress(const TermTable& terms,
		 const AtomSet& atoms, const ValueMap& values) const {
    int tmp = 0;
  for (FormulaList::const_iterator fi = conjuncts_.begin();
       fi != conjuncts_.end(); fi++) {
      tmp += (*fi)->progress(terms,atoms,values);
  }
  return tmp;
}


/* Returns an instantiation of this state formula. */
const StateFormula& Conjunction::instantiation(const SubstitutionMap& subst,
                                               const TermTable& terms,
                                               const AtomSet& atoms,
                                               const ValueMap& values,
                                               bool state) const {
  Conjunction &inst_conj = *(new Conjunction());
  for (FormulaList::const_iterator fi = conjuncts().begin();
       fi != conjuncts().end(); fi++) {
    inst_conj.add_conjunct((*fi)->instantiation(subst, terms, atoms, values, state));
  }
  return inst_conj;
}


/* Whether state formula is statically true, statically false, or something else */
Truthiness Conjunction::truthiness() const {
  Truthiness truthexity = Truthiness::TAUTOLOGY;
  for (auto fi = conjuncts_.cbegin(); fi != conjuncts_.cend()
         && truthexity != Truthiness::CONTRADICTION; fi++) {
    truthexity = truthexity && (*fi)->truthiness();
  }
  return truthexity;
}


/* Prints this object on the given stream. */
void Conjunction::print(std::ostream& os) const {
  os << "(and";
  for (FormulaList::const_iterator fi = conjuncts().begin();
       fi != conjuncts().end(); fi++) {
    os << ' ' << **fi;
  }
  os << ")";
}


void Conjunction::listAtoms(AtomSet &atomsS, AtomList &atomsL, bool dupes)
  const {
  for (auto i = conjuncts_.cbegin(); i != conjuncts_.cend(); i++) {
    (*i)->listAtoms(atomsS, atomsL, dupes);
  }
}


/* ====================================================================== */
/* Disjunction. */

/* Deletes this disjunction. */
Disjunction::~Disjunction() {
  for (FormulaList::const_iterator fi = disjuncts().begin();
       fi != disjuncts().end(); fi++) {
    destructive_deref(*fi);
  }
}


/* Adds a disjunct to this disjunction. */
void Disjunction::add_disjunct(const StateFormula& disjunct) {
  disjuncts_.push_back(&disjunct);
  ref(&disjunct);
}


/* Tests if this state formula holds in the given state. */
bool Disjunction::holds(const TermTable& terms,
                        const AtomSet& atoms, const ValueMap& values) const {
  for (FormulaList::const_iterator fi = disjuncts().begin();
       fi != disjuncts().end(); fi++) {
    if ((*fi)->holds(terms, atoms, values)) {
      return true;
    }
  }
  return false;
}

/* Tests if this state formula holds in the given state. */
bool Disjunction::holds_noValues(const TermTable& terms,
			const AtomSet& atoms) const {
  for (FormulaList::const_iterator fi = disjuncts_.begin();
       fi != disjuncts_.end(); fi++) {
      if ((*fi)->holds_noValues(terms, atoms)) {
	  return true;
      }
  }
  return false;
}

/* Progress estimator. */
int Disjunction::progress(const TermTable& terms,
		 const AtomSet& atoms, const ValueMap& values) const {
    int tmp = 0;
  for (FormulaList::const_iterator fi = disjuncts_.begin();
       fi != disjuncts_.end(); fi++) {
      tmp += ((*fi)->progress(terms, atoms, values));
  }
  return tmp;
}

/* Returns an instantiation of this formula. */
const StateFormula& Disjunction::instantiation(const SubstitutionMap& subst,
                                               const TermTable& terms,
                                               const AtomSet& atoms,
                                               const ValueMap& values,
                                               bool state) const {
  Disjunction &inst_disj = *(new Disjunction());
  for (FormulaList::const_iterator fi = disjuncts().begin();
       fi != disjuncts().end(); fi++) {
    inst_disj.add_disjunct((*fi)->instantiation(subst, terms, atoms, values, state));
  }
  return inst_disj;
}


/* Whether state formula is statically true, statically false, or something else */
Truthiness Disjunction::truthiness() const {
  Truthiness truthicacy = Truthiness::CONTRADICTION;
  for (auto fi = disjuncts_.cbegin(); fi != disjuncts_.cend()
         && truthicacy != Truthiness::TAUTOLOGY; fi++) {
    truthicacy = truthicacy || (*fi)->truthiness();
  }
  return truthicacy;
}


/* Prints this object on the given stream. */
void Disjunction::print(std::ostream& os) const {
  os << "(or";
  for (FormulaList::const_iterator fi = disjuncts().begin();
       fi != disjuncts().end(); fi++) {
    os << ' ' << **fi;
  }
  os << ")";
}


void Disjunction::listAtoms(AtomSet &atomsS, AtomList &atomsL, bool dupes)
  const {
  for (auto i = disjuncts_.cbegin(); i != disjuncts_.cend(); i++) {
    (*i)->listAtoms(atomsS, atomsL, dupes);
  }
}


/* ====================================================================== */
/* Quantification */

/* Constructs a quantification. */
Quantification::Quantification(const VariableList& parameters,
                               const StateFormula& body)
  : parameters_(parameters), body_(&body) {
  ref(body_);
}


/* Deletes this quantification. */
Quantification::~Quantification() {
  destructive_deref(body_);
}


void Quantification::listAtoms(AtomSet &atomsS, AtomList &atomsL, bool dupes)
  const {
  body_->listAtoms(atomsS, atomsL, dupes);
}


/* ====================================================================== */
/* Exists */

/* Returns an existentially quantified formula. */
const StateFormula& Exists::make(const VariableList& parameters,
                                 const StateFormula& body) {
  if (parameters.empty() || body.tautology() || body.contradiction()) {
    return body;
  } else {
    return *new Exists(parameters, body);
  }
}


/* Tests if this state formula holds in the given state. */
bool Exists::holds(const TermTable& terms,
                   const AtomSet& atoms, const ValueMap& values) const {
  const StateFormula& inst_exists =
    instantiation(SubstitutionMap(), terms, atoms, values, true);
  return inst_exists.tautology();
}


/* Tests if this state formula holds in the given state. */
bool Exists::holds_noValues(const TermTable& terms,
		   const AtomSet& atoms) const {
  throw std::logic_error("Exists::holds erroneously called");
}

/* Progress estimator. */
int Exists::progress(const TermTable& terms,
		 const AtomSet& atoms, const ValueMap& values) const {
  throw std::logic_error("Exists::progress erroneously called");
}

/* Returns an instantiation of this formula. */
const StateFormula& Exists::instantiation(const SubstitutionMap& subst,
                                          const TermTable& terms,
                                          const AtomSet& atoms,
                                          const ValueMap& values,
                                          bool state) const {
  int n = parameters().size();
  std::vector<const ObjectList*> arguments(n);
  std::vector<ObjectList::const_iterator> next_arg;
  for (int i = 0; i < n; i++) {
    Type t = TermTable::type(parameters()[i]);
    arguments[i] = &terms.compatible_objects(t);
    if (arguments[i]->empty()) {
      return FALSE;
    }
    next_arg.push_back(arguments[i]->begin());
  }
  const StateFormula* inst_exists = &FALSE;
  std::stack<const StateFormula*> disjuncts;
  disjuncts.push(&body().instantiation(subst, terms, atoms, values, state));
  ref(disjuncts.top());
  for (int i = 0; i < n; ) {
    SubstitutionMap pargs;
    pargs.insert(std::make_pair(parameters()[i], *next_arg[i]));
    const StateFormula& disjunct =
      disjuncts.top()->instantiation(pargs, terms, atoms, values, state);
    disjuncts.push(&disjunct);
    if (i + 1 == n) {
      inst_exists = &(*inst_exists || disjunct);
      if (inst_exists->tautology()) {
        disjuncts.pop();
        while (!disjuncts.empty()) {
          destructive_deref(disjuncts.top());
          disjuncts.pop();
        }
        return *inst_exists;
      }
      for (int j = i; j >= 0; j--) {
        if (j < i) {
          destructive_deref(disjuncts.top());
        }
        disjuncts.pop();
        next_arg[j]++;
        if (next_arg[j] == arguments[j]->end()) {
          if (j == 0) {
            i = n;
            break;
          } else {
            next_arg[j] = arguments[j]->begin();
          }
        } else {
          i = j;
          break;
        }
      }
    } else {
      ref(disjuncts.top());
      i++;
    }
  }
  while (!disjuncts.empty()) {
    destructive_deref(disjuncts.top());
    disjuncts.pop();
  }
  return *inst_exists;
}


/* Prints this object on the given stream. */
void Exists::print(std::ostream& os) const {
  VariableList::const_iterator vi = parameters().begin();
  os << "(exists (" << *vi;
  for (vi++; vi != parameters().end(); vi++) {
    os << ' ' << *vi;
  }
  os << ") " << body() << ")";
}


/* ====================================================================== */
/* Forall */

/* Returns a universally quantified formula. */
const StateFormula& Forall::make(const VariableList& parameters,
                                 const StateFormula& body) {
  if (parameters.empty() || body.tautology() || body.contradiction()) {
    return body;
  } else {
    return *new Forall(parameters, body);
  }
}


/* Tests if this state formula holds in the given state. */
bool Forall::holds(const TermTable& terms,
                   const AtomSet& atoms, const ValueMap& values) const {
  const StateFormula& inst_forall =
    instantiation(SubstitutionMap(), terms, atoms, values, true);
  return inst_forall.tautology();
}

/* Tests if this state formula holds in the given state. */
bool Forall::holds_noValues(const TermTable& terms,
		   const AtomSet& atoms) const {
    throw std::logic_error("Forall::holds erroneously called");
}

/* Progress estimator. */
int Forall::progress(const TermTable& terms,
		 const AtomSet& atoms, const ValueMap& values) const {
    //throw std::logic_error("Forall::progress erroneously called");
  const StateFormula& inst_forall =
    instantiation(SubstitutionMap(), terms, atoms, values, true);
  return inst_forall.progress(terms, atoms, values);
}

/* Returns an instantiation of this formula. */
const StateFormula& Forall::instantiation(const SubstitutionMap& subst,
                                          const TermTable& terms,
                                          const AtomSet& atoms,
                                          const ValueMap& values,
                                          bool state) const {
  int n = parameters().size();
  std::vector<const ObjectList*> arguments(n);
  std::vector<ObjectList::const_iterator> next_arg;
  for (int i = 0; i < n; i++) {
    Type t = TermTable::type(parameters()[i]);
    arguments[i] = &terms.compatible_objects(t);
    if (arguments[i]->empty()) {
      return TRUE;
    }
    next_arg.push_back(arguments[i]->begin());
  }
  const StateFormula* inst_forall = &TRUE;
  std::stack<const StateFormula*> conjuncts;
  conjuncts.push(&body().instantiation(subst, terms, atoms, values, state));
  ref(conjuncts.top());
  for (int i = 0; i < n; ) {
    SubstitutionMap pargs;
    pargs.insert(std::make_pair(parameters()[i], *next_arg[i]));
    const StateFormula& conjunct =
      conjuncts.top()->instantiation(pargs, terms, atoms, values, state);
    conjuncts.push(&conjunct);
    if (i + 1 == n) {
      inst_forall = &(*inst_forall && conjunct);
      if (inst_forall->contradiction()) {
        conjuncts.pop();
        while (!conjuncts.empty()) {
          destructive_deref(conjuncts.top());
          conjuncts.pop();
        }
        return *inst_forall;
      }
      for (int j = i; j >= 0; j--) {
        if (j < i) {
          destructive_deref(conjuncts.top());
        }
        conjuncts.pop();
        next_arg[j]++;
        if (next_arg[j] == arguments[j]->end()) {
          if (j == 0) {
            i = n;
            break;
          } else {
            next_arg[j] = arguments[j]->begin();
          }
        } else {
          i = j;
          break;
        }
      }
    } else {
      ref(conjuncts.top());
      i++;
    }
  }
  while (!conjuncts.empty()) {
    destructive_deref(conjuncts.top());
    conjuncts.pop();
  }
  return *inst_forall;
}


/* Prints this object on the given stream. */
void Forall::print(std::ostream& os) const {
  VariableList::const_iterator vi = parameters().begin();
  os << "(forall (" << *vi;
  for (vi++; vi != parameters().end(); vi++) {
    os << ' ' << *vi;
  }
  os << ") " << body() << ")";
}
