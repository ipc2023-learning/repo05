#include <assert.h>
#include <stack>
#include <typeinfo>

#include "actions.h"
#include "../../utils/die.h"
#include "../../utils/exceptions.h"
#include "expressions.h"
#include "formulas.h"
#include "global.h"
#include "problems.h"
#include "states.h"
#include "../../utils/utils.h"

#include "effects.h"

bool ConditionalEffect::working_ = false;
bool ProbabilisticEffect::working_ = false;

#ifdef DEBUG_FLATTENING
size_t Effect::debug_indent_ = 0;
#endif

void Effect::printCOUT() const {
  this->print(std::cout, gpt::problem->domain().predicates(),
                         gpt::problem->domain().functions(),
                         gpt::problem->terms());
}

/*******************************************************************************
 *
 * assignment
 *
 ******************************************************************************/

Assignment::Assignment( unsigned oper, const Application& application,
    const Expression& expr, bool reward_assignment) : operator_(oper),
    application_(&application), expr_(&expr), reward_assignment_(reward_assignment)
{
  Expression::register_use( application_ );
  Expression::register_use( expr_ );
  notify( this, "Assignment::Assignment(unsigned,Application&,Expression&)" );
}

Assignment::~Assignment()
{
  Expression::unregister_use( application_ );
  Expression::unregister_use( expr_ );
}

// See comments in the header file (effects.h)
bool Assignment::is_a_supported_reward_assigment() const {
  if (get_operator() != Assignment::INCREASE_OP &&
      get_operator() != Assignment::DECREASE_OP)
  {
//    std::cout << "Not increase nor decrease" << std::endl;
    return false;
  }

  if (typeid(*expr_) != typeid(Value)) {
//    std::cout << "exp not value" << std::endl;
    return false;
  }


//  std::cout << "SUPPORTED REWARD" << std::endl;
  return true;
}

bool Assignment::is_a_reward_reassigment() const {
  return (gpt::problem->domain().functions().name(application_->function()) == "reward");

  if (! reward_assignment_)
    return false;

  return true;
}


void
Assignment::affect( ValueMap& values ) const
{
  if( operator_ == ASSIGN_OP )
  {
    values[application_] = expr_->value( values );
  }
  else
  {
    ValueMap::const_iterator vi = values.find( application_ );
//    if( vi == values.end() )
//      throw Exception( "changing undefined value" );
//    else
    DIE(vi != values.end(), "changing undefined value", 132);
    if( operator_ == SCALE_UP_OP )
      values[application_] = (*vi).second * expr_->value( values );
    else if( operator_ == SCALE_DOWN_OP )
      values[application_] = (*vi).second / expr_->value( values );
    else if( operator_ == INCREASE_OP )
      values[application_] = (*vi).second + expr_->value( values );
    else
      values[application_] = (*vi).second - expr_->value( values );
  }
}

void
Assignment::affect( state_t& state ) const
{
#ifdef NO_STRICT
  WARNING("Assignment::affect is not supported.");
#else
  std::cout << IN_COLOR(BRIGHT_RED,
      "Method not supported. Try compiling with the flag -DNO_STRICT (1)")
    << std::endl;
  DIE(false, "[Assignment::affect] Unsupported effect (no implementation)", 133);
#endif
}

const Assignment&
Assignment::instantiation( const SubstitutionMap& subst,
    const problem_t& problem ) const
{
  return (*new Assignment(operator_,
                          application_->substitution(subst),
                          expr_->instantiation(subst, problem),
                          reward_assignment_));
}

bool
Assignment::operator==( const Assignment& assig ) const
{
  return (operator_ == assig.operator_ &&
          *application_ == *assig.application_ &&
          *expr_ == *assig.expr_ &&
          reward_assignment_ == assig.reward_assignment_);
}

void
Assignment::print( std::ostream& os, const FunctionTable& functions,
    const TermTable& terms, bool ignore_reward ) const
{
  if (ignore_reward &&
      functions.name(application_->function()) == "reward") {
//    std::cout << "Ignoring reward" << std::endl;
    os << "(and)";
    return;
  }
//  else {
//    std::cout << "Not ignoring reward" << std::endl;
//  }

  os << '(';
  if( operator_ == ASSIGN_OP )
    os << "assign ";
  else if( operator_ == SCALE_UP_OP )
    os << "scale-up ";
  else if( operator_ == SCALE_DOWN_OP )
    os << "scale-down ";
  else if( operator_ == INCREASE_OP )
    os << "increase ";
  else
    os << "decrease ";
  application_->print( os, functions, terms );
  os << ' ';
  expr_->print( os, functions, terms );
  os << ')';
}


/*******************************************************************************
 *
 * add effect
 *
 ******************************************************************************/

  AddEffect::AddEffect( const Atom& atom )
: atom_(&atom)
{
  Effect::register_use( this );
  StateFormula::register_use( atom_ );
  notify( this, "AddEffect::AddEffect(Atom&)" );
}

AddEffect::~AddEffect()
{
  StateFormula::unregister_use( atom_ );
}

const Effect&
AddEffect::flatten( void ) const
{
#ifdef DEBUG_FLATTENING
  print_indent();
  std::cout << "[AddEffect::flatten]:\n";
  debug_indent_ += 2;
  print_indent();
  std::cout << "in = out: ";
  this->printCOUT();
  std::cout << std::endl;
  debug_indent_ -= 2;
#endif
  Effect::register_use( this );
  return( *this );
}

void
AddEffect::state_change( AtomList& adds, AtomList& deletes,
    AssignmentList& assignments,
    const state_t& state ) const
{
  adds.push_back( atom_ );
}


void
AddEffect::translate( stripsEffect_t &s_effect,
    conditionalEffectList_t &c_effect ) const
{
  ushort_t atm = problem_t::atom_hash_get( atom() );
  s_effect.insert_add( atm );
}

const Effect&
AddEffect::instantiation( const SubstitutionMap& subst,
    const problem_t& problem ) const
{
  const Atom* inst_atom = &atom().substitution( subst );
  if( inst_atom == atom_ )
  {
    StateFormula::unregister_use( inst_atom );
    Effect::register_use( this );
    return( *this );
  }
  else
  {
    const Effect *result = new AddEffect( *inst_atom );
    StateFormula::unregister_use( inst_atom );
    return( *result );
  }
}

bool
AddEffect::operator==( const Effect& eff ) const
{
  const AddEffect *aeff = dynamic_cast<const AddEffect*>(&eff);
  return( (aeff != NULL) && (atom() == aeff->atom()) );
}

void
AddEffect::print( std::ostream& os, const PredicateTable& predicates,
    const FunctionTable& functions,
    const TermTable& terms, bool ignore_reward ) const
{
  atom().print( os, predicates, functions, terms );
}

void
AddEffect::analyze( PredicateTable &predicates, TermTable &terms,
    std::map<const StateFormula*,const Atom*> &hash ) const
{
}

const Effect&
AddEffect::rewrite( std::map<const StateFormula*,const Atom*> &hash ) const
{
  Effect::register_use( this );
  return( *this );
}


/*******************************************************************************
 *
 * delete effect
 *
 ******************************************************************************/

  DeleteEffect::DeleteEffect( const Atom& atom )
: atom_(&atom)
{
  Effect::register_use( this );
  StateFormula::register_use( atom_ );
  notify( this, "DeleteEffect::DeleteEffect(Atom&)" );
}

DeleteEffect::~DeleteEffect()
{
  StateFormula::unregister_use( atom_ );
}

const Effect&
DeleteEffect::flatten( void ) const
{
#ifdef DEBUG_FLATTENING
  print_indent();
  std::cout << "[DeleteEffect::flatten]:\n";
  debug_indent_ += 2;
  print_indent();
  std::cout << "in = out: ";
  this->printCOUT();
  std::cout << std::endl;
  debug_indent_ -= 2;
#endif
  Effect::register_use( this );
  return( *this );
}

void
DeleteEffect::state_change( AtomList& adds, AtomList& deletes,
    AssignmentList& assignments,
    const state_t& state ) const
{
  deletes.push_back( atom_ );
}


void
DeleteEffect::translate( stripsEffect_t &s_effect,
    conditionalEffectList_t &c_effect ) const
{
  ushort_t atm = problem_t::atom_hash_get( atom() );
  s_effect.insert_del( atm );
}

const Effect&
DeleteEffect::instantiation( const SubstitutionMap& subst,
    const problem_t& problem ) const
{
  const Atom* inst_atom = &atom().substitution( subst );
  if( inst_atom == atom_ )
  {
    StateFormula::unregister_use( inst_atom );
    Effect::register_use( this );
    return( *this );
  }
  else
  {
    const Effect *result = new DeleteEffect( *inst_atom );
    StateFormula::unregister_use( inst_atom );
    return( *result );
  }
}

bool
DeleteEffect::operator==( const Effect& eff ) const
{
  const DeleteEffect *deff = dynamic_cast<const DeleteEffect*>(&eff);
  return( (deff != NULL) && (atom() == deff->atom()) );
}

void
DeleteEffect::print( std::ostream& os, const PredicateTable& predicates,
    const FunctionTable& functions,
    const TermTable& terms, bool ignore_reward ) const
{
  os << "(not ";
  atom().print( os, predicates, functions, terms );
  os << ")";
}

void
DeleteEffect::analyze( PredicateTable &predicates, TermTable &terms,
    std::map<const StateFormula*,const Atom*> &hash ) const
{
}

const Effect&
DeleteEffect::rewrite( std::map<const StateFormula*,const Atom*> &hash ) const
{
  Effect::register_use( this );
  return( *this );
}


/*******************************************************************************
 *
 * assignment effect
 *
 ******************************************************************************/

AssignmentEffect::AssignmentEffect(const Assignment& assignment)
    : assignment_(&assignment)
{
  Effect::register_use( this );
  notify( this, "AssignmentEffect::AssignmentEffect(Assignment&)" );
}

AssignmentEffect::~AssignmentEffect()
{
  delete assignment_;
}

const Effect&
AssignmentEffect::flatten( void ) const
{
  if (assignment_->is_a_supported_reward_assigment()) {
#ifdef DEBUG_FLATTENING
    print_indent();
    std::cout << "[Assignment::flatten (supported reward)]:\n";
    debug_indent_ += 2;
    print_indent();
    std::cout << "in = out: ";
    this->printCOUT();
    std::cout << std::endl;
    debug_indent_ -= 2;
#endif
    Effect::register_use(this);
    return (*this);
  } else {
#ifdef NO_STRICT
    WARNING("AssignmentEffect::flatten is not supported");
#ifdef DEBUG_FLATTENING
    print_indent();
    std::cout << "[AssignmentEffect::flatten (not supported reward)]:\n";
    debug_indent_ += 2;
    print_indent();
    std::cout << "in: ";
    this->printCOUT();
    std::cout << "out: (and)" << std::endl;
    debug_indent_ -= 2;
#endif
    return (*new ConjunctiveEffect());
#else
  std::cout << IN_COLOR(BRIGHT_RED,
      "Method not supported. Try compiling with the flag -DNO_STRICT (2)")
    << std::endl;
  DIE(false, "[Assignment::flatten] Unsupported effect (no implementation)", 133);
#ifdef DEBUG_FLATTENING
    print_indent();
    std::cout << "[AssignmentEffect::flatten (not supported reward)]:\n";
    debug_indent_ += 2;
    print_indent();
    std::cout << "in = out: ";
    this->printCOUT();
    std::cout << std::endl;
    debug_indent_ -= 2;
#endif
  return (*this);
#endif
  }
}

void
AssignmentEffect::state_change( AtomList& adds, AtomList& deletes,
    AssignmentList& assignments,
    const state_t& state ) const
{
  assignments.push_back( assignment_ );
}


void AssignmentEffect::translate(stripsEffect_t &s_effect,
    conditionalEffectList_t &c_effect) const
{
  // Make sure that the operation is increase or decrease
  DIE(assignment_->is_a_supported_reward_assigment(),
      "Unsupported effect (no implementation)", 133);


  int func_idx = assignment().application().function();
  if (assignment().is_a_reward_reassigment()) {
    assert(func_idx == ACTION_COST);
//    std::cout << "reward idx = " << assignment().application().function()
//              << std::endl;
  }
  else {
//    std::cout << "[AssignmentEffect::translate]: extra cost function '"
//              << gpt::problem->domain().functions().name(func_idx)
//              << "' idx = " << func_idx << std::endl;
  }

  // Grab the value in which the effect change the
  Value const* value_eff = dynamic_cast<Value const*>(assignment_->expression());
  DIE(value_eff != NULL, "Unsupported effect (no implementation)", 133);
  if (assignment_->get_operator() == Assignment::INCREASE_OP) {
    // Increasing the reward means negative cost
    s_effect.increase_cost_by(value_eff->value(), func_idx);
  }
  else if (assignment_->get_operator() == Assignment::DECREASE_OP) {
    // Decreasing the reward means cost
    s_effect.increase_cost_by(-1 * value_eff->value(), func_idx);
  }
  else {
    DIE(assignment_->get_operator() == Assignment::INCREASE_OP ||
        assignment_->get_operator() == Assignment::DECREASE_OP,
        "Unsupported effect (no implementation)", 133);
  }
}

const Effect&
AssignmentEffect::instantiation( const SubstitutionMap& subst,
    const problem_t& problem ) const
{
  return( *new AssignmentEffect( assignment().instantiation( subst, problem ) ) );
}

bool
AssignmentEffect::operator==( const Effect& eff ) const
{
  const AssignmentEffect *assig = dynamic_cast<const AssignmentEffect*>(&eff);
  return( (assig != NULL) && (assignment() == assig->assignment()) );
}

void
AssignmentEffect::print( std::ostream& os,
    const PredicateTable& predicates,
    const FunctionTable& functions,
    const TermTable& terms, bool ignore_reward ) const
{
  assignment().print( os, functions, terms, ignore_reward );
}

void
AssignmentEffect::analyze( PredicateTable &predicates, TermTable &terms,
    std::map<const StateFormula*,const Atom*> &hash ) const
{
}

const Effect&
AssignmentEffect::rewrite( std::map<const StateFormula*,const Atom*> &hash ) const
{
  Effect::register_use( this );
  return( *this );
}


/*******************************************************************************
 *
 * conjunctive effect
 *
 ******************************************************************************/

ConjunctiveEffect::ConjunctiveEffect()
{
  Effect::register_use( this );
  notify( this, "ConjunctiveEffect::ConjunctiveEffect()" );
}

ConjunctiveEffect::~ConjunctiveEffect()
{
  for( EffectList::const_iterator ei = conjuncts_.begin(); ei != conjuncts_.end(); ++ei )
    Effect::unregister_use( *ei );
}

  void
ConjunctiveEffect::add_conjunct( const Effect& conjunct )
{
  const ConjunctiveEffect* conj_effect = dynamic_cast<const ConjunctiveEffect*>(&conjunct);
  if( conj_effect != NULL )
  {
    for( EffectList::const_iterator ei = conj_effect->conjuncts_.begin();
        ei != conj_effect->conjuncts_.end(); ++ei )
    {
      Effect::register_use( *ei );
      conjuncts_.push_back( *ei );
    }
    Effect::unregister_use( &conjunct );
  }
  else
  {
    conjuncts_.push_back( &conjunct );
  }
}

const Effect& ConjunctiveEffect::flatten(void) const
{
  if (size() == 0) {
    Effect::register_use(this);
#ifdef DEBUG_FLATTENING
    print_indent();
    std::cout << "[ConjEff::flatten]:\n";
    debug_indent_ += 2;
    print_indent();
    std::cout << "in = out: ";
    this->printCOUT();
    std::cout << std::endl;
    debug_indent_ -= 2;
#endif
    return *this;
  }
  else {

#ifdef DEBUG_FLATTENING
    print_indent();
    std::cout << "[ConjEff::flatten]:\n";
    debug_indent_ += 2;
    print_indent();
    std::cout << "in: ";
    this->printCOUT();
    std::cout << std::endl;
#endif

    const ProbabilisticEffect *prob_eff = new ProbabilisticEffect;
    const ProbabilisticEffect *tmp;

    for (size_t i = 0; i < size(); ++i) {
      const Effect *eff = &conjunct(i).flatten();
      const ProbabilisticEffect *peff =
                dynamic_cast<const ProbabilisticEffect*>(eff);

      if (peff == NULL) {
        ProbabilisticEffect *peff = new ProbabilisticEffect;
        peff->add_outcome(Rational(1), *eff);
        tmp = &prob_eff->cross_product(*peff);
        Effect::unregister_use(peff);

      } else {
        tmp = &prob_eff->cross_product(*peff);
        Effect::unregister_use(eff);
      }
      Effect::unregister_use(prob_eff);
      prob_eff = tmp;
#ifdef DEBUG_FLATTENING
      print_indent();
      std::cout << "partial output: ";
      prob_eff->printCOUT();
      std::cout << std::endl;
#endif

    }

    DIE(prob_eff->size() > 0, "Prob effect with 0 (or less) effects", 111);

    Effect const* rv = prob_eff;
    if (prob_eff->size() == 1) {
      rv = &prob_eff->effect(0);
      Effect::register_use(rv);
      Effect::unregister_use(prob_eff);
    }
#ifdef DEBUG_FLATTENING
    print_indent();
    std::cout << "out: ";
    rv->printCOUT();
    std::cout << std::endl;
    debug_indent_ -= 2;
#endif
    return *rv;
  }
}

void
ConjunctiveEffect::state_change( AtomList& adds, AtomList& deletes,
    AssignmentList& assignments,
    const state_t& state ) const
{
  for( EffectList::const_iterator ei = conjuncts_.begin(); ei != conjuncts_.end(); ++ei )
    (*ei)->state_change( adds, deletes, assignments, state );
}


void
ConjunctiveEffect::translate( stripsEffect_t &s_effect,
    conditionalEffectList_t &c_effect ) const
{
  for( size_t i = 0; i < size(); ++i )
    conjunct( i ).translate( s_effect, c_effect );
}

const Effect&
ConjunctiveEffect::instantiation( const SubstitutionMap& subst,
    const problem_t& problem ) const
{
  ConjunctiveEffect& inst_effect = *new ConjunctiveEffect();
  for( EffectList::const_iterator ei = conjuncts_.begin(); ei != conjuncts_.end(); ++ei )
    inst_effect.add_conjunct( (*ei)->instantiation( subst, problem ) );
  return( inst_effect );
}

bool
ConjunctiveEffect::operator==( const Effect& eff ) const
{
  const ConjunctiveEffect *ceff = dynamic_cast<const ConjunctiveEffect*>(&eff);
  if( (ceff != NULL) && (size() == ceff->size()) )
  {
    for( size_t i = 0; i < size(); ++i )
      if( !(conjunct( i ) == ceff->conjunct( i )) )
        return( false );
    return( true );
  }
  return( true );
}

void
ConjunctiveEffect::print( std::ostream& os,
    const PredicateTable& predicates,
    const FunctionTable& functions,
    const TermTable& terms, bool ignore_reward ) const
{
  if( size() == 1 )
    conjunct(0).print( os, predicates, functions, terms, ignore_reward );
  else
  {
    os << "(and";
    for( EffectList::const_iterator ei = conjuncts_.begin();
        ei != conjuncts_.end(); ++ei )
    {
      os << ' ';
      (*ei)->print( os, predicates, functions, terms, ignore_reward );
    }
    os << ")";
  }
}

void
ConjunctiveEffect::analyze( PredicateTable &predicates, TermTable &terms,
    std::map<const StateFormula*,const Atom*> &hash ) const
{
  for( EffectList::const_iterator ei = conjuncts_.begin(); ei != conjuncts_.end(); ++ei )
    (*ei)->analyze( predicates, terms, hash );
}

const Effect&
ConjunctiveEffect::rewrite( std::map<const StateFormula*,const Atom*> &hash ) const
{
  ConjunctiveEffect *conj = new ConjunctiveEffect;
  for( EffectList::const_iterator ei = conjuncts_.begin(); ei != conjuncts_.end(); ++ei )
    conj->add_conjunct( (*ei)->rewrite( hash ) );
  return( *conj );
}


/*******************************************************************************
 *
 * conditional effect
 *
 ******************************************************************************/

  const Effect&
ConditionalEffect::make( const StateFormula& condition, const Effect& effect )
{
  if( condition.tautology() )
  {
    StateFormula::unregister_use( &condition );
    return( effect );
  }
  else if( condition.contradiction() )
  {
    StateFormula::unregister_use( &condition );
    Effect::unregister_use( &effect );
    return( *new ConjunctiveEffect() );
  }
  else
  {
    const Effect *eff = new ConditionalEffect( condition, effect );
    StateFormula::unregister_use( &condition );
    Effect::unregister_use( &effect );
    return( *eff );
  }
}

ConditionalEffect::ConditionalEffect( const StateFormula& condition,
    const Effect& effect )
: condition_(&condition), effect_(&effect)
{
  Effect::register_use( this );
  StateFormula::register_use( condition_ );
  Effect::register_use( effect_ );
  notify( this, "ConditionalEffect::ConditionalEffect(StateFormula&,Effect&)" );
}

ConditionalEffect::~ConditionalEffect()
{
  StateFormula::unregister_use( condition_ );
  Effect::unregister_use( effect_ );
}

const Effect&
ConditionalEffect::flatten( void ) const
{
#ifdef DEBUG_FLATTENING
    print_indent();
    std::cout << "[Conditional::flatten]:\n";
    debug_indent_ += 2;
    print_indent();
    std::cout << "in: ";
    this->printCOUT();
    std::cout << std::endl;
#endif
  const Effect *effect = &effect_->flatten();
  const ProbabilisticEffect *prob_effect = dynamic_cast<const ProbabilisticEffect*>(effect);
  const ConditionalEffect *cond_effect = dynamic_cast<const ConditionalEffect*>(effect);
  Effect const* rv = NULL;

  if (effect == this) {
    Effect::unregister_use(effect);
    Effect::register_use(this);
    rv = this;
  }
  else if (prob_effect != NULL) {
    ProbabilisticEffect *result = new ProbabilisticEffect;
    for( size_t i = 0; i < prob_effect->size(); ++i )
    {
      StateFormula::register_use( &condition() );
      Effect::register_use( &prob_effect->effect( i ) );
      const Effect *ceff = &make( condition(), prob_effect->effect( i ) );
      result->add_outcome( prob_effect->probability(i), ceff->flatten() );
      Effect::unregister_use( ceff );
    }
    Effect::unregister_use(effect);
    rv = result;
  }
  else if (cond_effect != NULL) {
    Conjunction *cond = new Conjunction;
    StateFormula::register_use(&condition());
    cond->add_conjunct(condition());
    StateFormula::register_use(&cond_effect->condition());
    cond->add_conjunct(cond_effect->condition());

    Effect::register_use(&cond_effect->effect());
    rv = &make(*cond, cond_effect->effect());
    Effect::unregister_use(effect);
  }
  else {
    StateFormula::register_use(&condition());
    rv = &make(condition(), *effect);
  }
#ifdef DEBUG_FLATTENING
  print_indent();
  std::cout << "out: ";
  rv->printCOUT();
  std::cout << std::endl;
  debug_indent_ -= 2;
#endif
  return *rv;
}

void
ConditionalEffect::state_change( AtomList& adds, AtomList& deletes,
    AssignmentList& assignments,
    const state_t& state ) const
{
  if( condition().holds( state ) )
    effect().state_change( adds, deletes, assignments, state );
}


void
ConditionalEffect::translate( stripsEffect_t &s_effect,
    conditionalEffectList_t &c_effect ) const
{
//  if( working_ )
//    throw Exception( "ConditionalEffect::translate: error: already working" );
  DIE(! working_, "BUG_3: translations of conditional effects.", 203); //BUG_3

  working_ = true;
  conditionalEffect_t *ceffect = new conditionalEffect_t;

  // translate condition
  condition().translate( ceffect->precondition() );

  // translate effect
  effect().translate( ceffect->s_effect(), c_effect );

  // insert into list
  c_effect.insert( ceffect );
  working_ = false;
}

const Effect&
ConditionalEffect::instantiation( const SubstitutionMap& subst,
    const problem_t& problem ) const
{
  const StateFormula *cond = &condition().instantiation( subst, problem );
  const Effect *eff = &effect().instantiation( subst, problem );
  const Effect *result = &make( *cond, *eff );
  return( *result );
}

bool
ConditionalEffect::operator==( const Effect& eff ) const
{
  const ConditionalEffect *ceff = dynamic_cast<const ConditionalEffect*>(&eff);
  return( (ceff != NULL) && (condition() == ceff->condition()) && (effect() == ceff->effect()) );
}

void
ConditionalEffect::print( std::ostream& os,
    const PredicateTable& predicates,
    const FunctionTable& functions,
    const TermTable& terms, bool ignore_reward ) const
{
  os << "(when ";
  condition().print( os, predicates, functions, terms );
  os << ' ';
  effect().print( os, predicates, functions, terms, ignore_reward );
  os << ")";
}

void
ConditionalEffect::analyze( PredicateTable &predicates, TermTable &terms,
    std::map<const StateFormula*,const Atom*> &hash ) const
{
  condition().analyze( predicates, terms, hash );
  effect().analyze( predicates, terms, hash );
}

const Effect&
ConditionalEffect::rewrite( std::map<const StateFormula*,const Atom*> &hash ) const
{
  return( ConditionalEffect::make( condition().rewrite( hash ), effect().rewrite( hash ) ) );
}


/*******************************************************************************
 *
 * probabilistic effect
 *
 ******************************************************************************/

ProbabilisticEffect::ProbabilisticEffect() {
  Effect::register_use(this);
  sum_up_to_one_ = false;
  notify(this, "ProbabilisticEffect::ProbabilisticEffect()");
}

ProbabilisticEffect::~ProbabilisticEffect()
{
  for( EffectList::const_iterator ei = effects_.begin(); ei != effects_.end(); ++ei )
    Effect::unregister_use( *ei );
}

ProbabilisticEffect const& ProbabilisticEffect::cross_product(
    ProbabilisticEffect const& prob_effect) const
{
  ProbabilisticEffect *result = new ProbabilisticEffect;

  Rational total_prob_cross_prod = 0;
  Rational total_prob_of_this_peff = 0;
  Rational total_prob_of_arg_peff = 0;

#ifdef DEBUG_CROSSPROD
  std::cout << "[ProbEff::cross_product INPUT]:\n  ";
  this->printCOUT();
  std::cout << " {sum_up = " << (sum_up_to_one_ ? "T" : "f") << "}\n X\n  ";
  prob_effect.printCOUT();
  std::cout << " {sum_up = " << (prob_effect.sum_up_to_one_ ? "T" : "f") << "}" << std::endl;
#endif

  for (size_t i = 0; i < size(); ++i) {
    for (size_t j = 0; j < prob_effect.size(); ++j) {
      Rational p = probability(i) * prob_effect.probability(j);
      //      std::cout << p << " = " << probability(i) << " * "  << prob_effect.probability(j) << std::endl;
      DIE(p > 0, "BUG_2: Negative probability", 202); //BUG_2
      if (p.double_value() < gpt::ignore_effects_with_prob_less_than) {
#ifdef DEBUG_IGNORED_EFFECTS
        std::cout << "Ignoring action with p = " << p << std::endl;
#endif
        continue;
      }

      // this var will represent the conjunction of the i-th effect of this
      // effect and the j-th effect of prob_effect
      ConjunctiveEffect *conjunction = new ConjunctiveEffect;
      Effect::register_use(&effect(i));
      conjunction->add_conjunct(effect(i));
      Effect::register_use(&prob_effect.effect(j));
      conjunction->add_conjunct(prob_effect.effect(j));
      result->add_outcome(p, *conjunction);

#ifdef DEBUG_CROSSPROD
      std::cout << "    [" << probability(i) << " ";
      effect(i).printCOUT();
      std::cout << "]\n    X\n    [" << prob_effect.probability(j) << " ";
      prob_effect.effect(j).printCOUT();
      std::cout << "]\n    = [" << p << " ";
      conjunction->printCOUT();
      std::cout << "]" << std::endl;
#endif
      if (i == 0) {
        // Making sure we add the probabilities just once (could be any value
        // of i)
        total_prob_of_arg_peff += prob_effect.probability(j);
      }

      total_prob_cross_prod += probability(i) * prob_effect.probability(j);
    }  // for each effect indexed by j of prob_effect
    total_prob_of_this_peff += probability(i);
  }  // for each effect indexed by i of this effect

  // Checking for

  DIE(total_prob_cross_prod >= 0, "Negative probability (Rational overflow)", 112);
  DIE(total_prob_of_this_peff >= 0, "Negative probability (Rational overflow)", 112);
  DIE(total_prob_of_arg_peff >= 0, "Negative probability (Rational overflow)", 112);

  /*
   * Checking if any of the probabilistic effects were incomplete, i.e., if
   * they have and implicit probability of nothing happening
   */
  if (!sum_up_to_one_ && total_prob_of_this_peff < 1) {
    for (size_t j = 0; j < prob_effect.size(); ++j) {
      Effect::register_use(&prob_effect.effect(j));
      Rational p = (1 - total_prob_of_this_peff) * prob_effect.probability(j);
      DIE(p > 0, "BUG_2: Negative probability", 202); //BUG_2
      result->add_outcome(p , prob_effect.effect(j));
      total_prob_cross_prod += p;
#ifdef DEBUG_CROSSPROD
      std::cout << "    Completing (this_peff < 1) [" << p << " ";
      prob_effect.effect(j).printCOUT();
      std::cout << "]" << std::endl;
#endif
    }
  }

  if (!prob_effect.sum_up_to_one_ && total_prob_of_arg_peff < 1) {
    for (size_t i = 0; i < size(); ++i) {
      Effect::register_use(&effect(i));
      Rational p = (1 - total_prob_of_arg_peff) * probability(i);
      DIE(p > 0, "BUG_2: Negative probability", 202); //BUG_2
      result->add_outcome(p, effect(i));
      total_prob_cross_prod += p;
#ifdef DEBUG_CROSSPROD
      std::cout << "    Completing (arg_peff < 1) [" << p << " ";
      effect(i).printCOUT();
      std::cout << "]" << std::endl;
#endif
    }
  }

  /*
   * If both effects are incomplete, then we will add a null effect with the
   * missing probability mass.
   *
   * FWT: the original code was done such that this step is performed during
   * the translation to avoid unnecessary large intermediary distributions
   * during the cross products. However, even in small examples, this approach
   * is not numerically stable and result in wrong actions. Therefore, I'm
   * performing this earlier to make sure it will be done properly.
   */
  if (!sum_up_to_one_ && total_prob_of_this_peff < 1
        && !prob_effect.sum_up_to_one_ && total_prob_of_arg_peff < 1)
  {
    ConjunctiveEffect *conjunction = new ConjunctiveEffect;
    result->add_outcome(1 - total_prob_cross_prod, *conjunction);
#ifdef DEBUG_CROSSPROD
      std::cout << "    Completing (arg_peff < 1 and arg_this < 1) ["
                << (1 - total_prob_cross_prod) << " (and)]" << std::endl;
#endif
  }

  result->sum_up_to_one_ = true;

#ifdef DEBUG_CROSSPROD
  std::cout << "[ProbEff::cross_product OUTPUT]:  ";
  result->printCOUT();
  std::cout << " {sum_up = " << (result->sum_up_to_one_ ? "T" : "f") << "}" << std::endl;
#endif


  return *result;
}

  bool
ProbabilisticEffect::add_outcome( const Rational& p, const Effect& effect)
{
  if (p.double_value() < gpt::ignore_effects_with_prob_less_than) {
#ifdef DEBUG_IGNORED_EFFECTS
    std::cout << "Ignoring action with p = " << p << std::endl;
#endif
    return true;
  }

  const ProbabilisticEffect* prob_effect =
    dynamic_cast<const ProbabilisticEffect*>(&effect);
  if( prob_effect != NULL )
  {
    for( size_t i = 0; i < prob_effect->size(); ++i )
    {
      Rational p_prime = p * prob_effect->probability(i);
      if (p_prime.double_value() < gpt::ignore_effects_with_prob_less_than) {
#ifdef DEBUG_IGNORED_EFFECTS
        std::cout << "Ignoring action with p = "
                  << p_prime << std::endl;
#endif
        continue;
      }
      Effect::register_use( &prob_effect->effect( i ) );
      if( !add_outcome(p_prime, prob_effect->effect(i) ) )
        return( false );
    }
    Effect::unregister_use( &effect );
  }
  else if( p != 0 )
  {
    DIE(p > 0, "BUG_2: Negative probability", 202); //BUG_2
    effects_.push_back( &effect );
    w_.push_back(p);
  }
  return( true );
}

Rational ProbabilisticEffect::probability(size_t i) const {
  Rational p = w_[i];
  DIE(p > 0, "BUG_2: Negative probability", 202); //BUG_2
  return p;
}

Effect const& ProbabilisticEffect::flatten() const {

  if (size() == 0) {
#ifdef DEBUG_FLATTENING
    print_indent();
    std::cout << "[Prob::flatten]:\n";
    debug_indent_ += 2;
    print_indent();
    std::cout << "in = out: ";
    this->printCOUT();
    std::cout << std::endl;
    debug_indent_ -= 2;
#endif
    Effect::register_use(this);
    return *this;
  }
  else {
#ifdef DEBUG_FLATTENING
    print_indent();
    std::cout << "[Prob::flatten]:\n";
    debug_indent_ += 2;
    print_indent();
    std::cout << "in: ";
    this->printCOUT();
    std::cout << std::endl;
#endif

    ProbabilisticEffect *result = new ProbabilisticEffect;
    for (size_t i = 0; i < size(); ++i) {
      const Effect *eff = &effect(i).flatten();
      const ProbabilisticEffect *peff = dynamic_cast<const ProbabilisticEffect*>(eff);
      if (peff != NULL) {
        for (size_t j = 0; j < peff->size(); ++j) {
          Rational p_prime = probability(i) * peff->probability(j);
          if (p_prime.double_value() < gpt::ignore_effects_with_prob_less_than) {
#ifdef DEBUG_IGNORED_EFFECTS
            std::cout << "Ignoring action with p = " << p_prime << std::endl;
#endif
            continue;
          }
          Effect::register_use(&peff->effect(j));
          result->add_outcome(p_prime, peff->effect(j));
        }
        Effect::unregister_use(peff);
      }
      else {
        result->add_outcome(probability(i), *eff);
      }
    }  // for i in size()

    Effect const* rv = result;
    if (result->size() == 1 && result->probability(0) == 1) {
      rv = &result->effect(0);
      Effect::register_use(rv);
      Effect::unregister_use(result);
    }
#ifdef DEBUG_FLATTENING
    print_indent();
    std::cout << "out: ";
    rv->printCOUT();
    std::cout << std::endl;
    debug_indent_ -= 2;
#endif

    return *rv;
  }
}

void ProbabilisticEffect::state_change(AtomList& adds, AtomList& deletes,
    AssignmentList& assignments, state_t const& state) const
{
  switch (size()) {
   case 0: break;
   case 1: effect(0).state_change(adds, deletes, assignments, state);
           break;
   default:
    double rnd = rand0to1_d();
    for (size_t i = 0; i < size(); ++i) {
      rnd -= probability(i).double_value();
      if (rnd <= 0) {
        effect(i).state_change(adds, deletes, assignments, state);
        break;
      }
    }
    // If this part of the code is reached, then this effect has an implicit
    // no-op effect with probability 1 - sum(probability(i) and this no-op
    // effect what "selected" in the random draw and nothing changed.
  }
}



// This method populates plist with its own probabilistic effects
void ProbabilisticEffect::translate(probabilisticEffectList_t &plist) const {
//  if( working_ )
//    throw Exception( "ProbabilisticEffect::translate: error: already working" );
  DIE(! working_, "ProbabilisticEffect::translate: error: already working", 134);
  working_ = true;
  Rational sum = 0;
  for (size_t i = 0; i < size(); ++i) {
    probabilisticEffect_t* peffect = new probabilisticEffect_t(probability(i));
    effect(i).translate(peffect->s_effect(), peffect->c_effect());
    if (!plist.insert(peffect)) {
      delete peffect;
    }
    sum = sum + probability(i);
  }

  // If the probability does not add up to one, then we complete the plist
  // with [(1 - sum), null effect], i.e., with probability (1-sum) nothing
  // happens
  if (!sum_up_to_one_ && sum < 1) {
    DIE(sum < 1, "BUG_2: Negative probability", 202); //BUG_2
    probabilisticEffect_t *peffect = new probabilisticEffect_t(1 - sum);
    if (!plist.insert(peffect)) delete peffect;
  }
  working_ = false;
}


void
ProbabilisticEffect::translate( stripsEffect_t &s_effect,
    conditionalEffectList_t &c_effect ) const
{
  //throw Exception( "ProbabilisticEffect::translate: erroneously called" );
  DIE(false, "Method erroneously called", 135);
}

const Effect&
ProbabilisticEffect::instantiation( const SubstitutionMap& subst, const problem_t& problem ) const
{
  ProbabilisticEffect& inst_effect = *new ProbabilisticEffect();
  for( size_t i = 0; i < size(); ++i )
    inst_effect.add_outcome( probability( i ), effect( i ).instantiation( subst, problem ) );
  return( inst_effect );
}

bool
ProbabilisticEffect::operator==( const Effect& eff ) const
{
  const ProbabilisticEffect *peff = dynamic_cast<const ProbabilisticEffect*>(&eff);
  if( (peff != NULL) && (size() == peff->size()) )
  {
    for( size_t i = 0; i < size(); ++i )
      if( !(effect( i ) == peff->effect( i )) )
        return( false );
    return( true );
  }
  return( false );
}

void
ProbabilisticEffect::print( std::ostream& os,
    const PredicateTable& predicates,
    const FunctionTable& functions,
    const TermTable& terms, bool ignore_reward ) const
{
  if (size() == 0)
    os << "(and)";
/* Simple optimization for the case of only one probabilistic effect */
/*  else if(weight_sum_ == weights_.back())
  {
    os << "(probabilistic 1 ";
    effect(0).print( os, predicates, functions, terms, ignore_reward );
    os << ")";
  }
*/
  else
  {
    os << "(probabilistic";
    size_t n = size();
    for (size_t i = 0; i < n; ++i) {
      os << ' ' << probability(i) << ' ';
      effect(i).print(os, predicates, functions, terms, ignore_reward);
    }
    os << ")";
  }
}

void
ProbabilisticEffect::analyze( PredicateTable &predicates, TermTable &terms,
    std::map<const StateFormula*,const Atom*> &hash ) const
{
  for( size_t i = 0; i < size(); ++i )
    effect( i ).analyze( predicates, terms, hash );
}

const Effect&
ProbabilisticEffect::rewrite( std::map<const StateFormula*,const Atom*> &hash ) const
{
  ProbabilisticEffect *prob = new ProbabilisticEffect;
  for( size_t i = 0; i < size(); ++i )
    prob->add_outcome( probability( i ), effect( i ).rewrite( hash ) );
  return( *prob );
}


/*******************************************************************************
 *
 * quantified effect
 *
 ******************************************************************************/

  QuantifiedEffect::QuantifiedEffect( const Effect& effect )
: effect_(&effect)
{
  Effect::register_use( this );
  Effect::register_use( effect_ );
  notify( this, "QuantifiedEffect::QuantifiedEffect(Effect&)" );
}

QuantifiedEffect::~QuantifiedEffect()
{
  Effect::unregister_use( effect_ );
}

Effect const& QuantifiedEffect::flatten() const {
#ifdef DEBUG_FLATTENING
  print_indent();
  std::cout << "[Quantified::flatten]:\n";
  debug_indent_ += 2;
  print_indent();
  std::cout << "in = out: ";
  this->printCOUT();
  std::cout << std::endl;
  debug_indent_ -= 2;
#endif

#ifdef NO_STRICT
  WARNING("QuantifiedEffect::flatten is not supported.");
  return *this;
#else
  std::cout << IN_COLOR(BRIGHT_RED,
      "Method not supported. Try compiling with the flag -DNO_STRICT (4)")
    << std::endl;
  DIE(false, "[QuantifiedEffect::flatten] Method erroneously called", 135);
  return (*this);
#endif
}

void
QuantifiedEffect::state_change( AtomList& adds, AtomList& deletes,
    AssignmentList& assignments,
    const state_t& state ) const
{
  effect().state_change( adds, deletes, assignments, state );
}


void
QuantifiedEffect::translate( stripsEffect_t &s_effect,
    conditionalEffectList_t &c_effect ) const
{
#ifdef NO_STRICT
  WARNING("QuantifiedEffect::translate should not be called");
#else
  std::cout << IN_COLOR(BRIGHT_RED,
      "Method not supported. Try compiling with the flag -DNO_STRICT (5)")
    << std::endl;
  DIE(false, "[QuantifiedEffect::translate] Method erroneously called", 135);
#endif
}

const Effect&
QuantifiedEffect::instantiation( const SubstitutionMap& subst,
    const problem_t& problem ) const
{
  int n = arity();
  if( n == 0 )
    return( effect().instantiation( subst, problem ) );
  else
  {
    SubstitutionMap args( subst );
    std::vector<ObjectList> arguments( n, ObjectList() );
    std::vector<ObjectList::const_iterator> next_arg;
    for( int i = 0; i < n; ++i )
    {
      problem.compatible_objects( arguments[i], problem.terms().type( parameter(i) ) );
      if( arguments[i].empty() ) return( *new ConjunctiveEffect() );
      next_arg.push_back( arguments[i].begin() );
    }

    ConjunctiveEffect* conj = new ConjunctiveEffect();
    std::stack<const Effect*> conjuncts;
    conjuncts.push( &effect().instantiation( args, problem ) );
    //Effect::register_use( conjuncts.top() );
    for( int i = 0; i < n; )
    {
      SubstitutionMap pargs;
      pargs.insert( std::make_pair( parameter(i), *next_arg[i] ) );
      const Effect& conjunct = conjuncts.top()->instantiation( pargs, problem );
      conjuncts.push( &conjunct );
      if( i + 1 == n )
      {
        conj->add_conjunct( conjunct );
        for( int j = i; j >= 0; --j )
        {
          if( j < i ) Effect::unregister_use( conjuncts.top() );
          conjuncts.pop();
          ++next_arg[j];
          if( next_arg[j] == arguments[j].end() )
          {
            if( j == 0 )
            {
              i = n;
              break;
            }
            else
              next_arg[j] = arguments[j].begin();
          }
          else
          {
            i = j;
            break;
          }
        }
      }
      else
      {
        //Effect::register_use( conjuncts.top() );
        ++i;
      }
    }

    while( !conjuncts.empty() )
    {
      Effect::unregister_use( conjuncts.top() );
      conjuncts.pop();
    }
    return( *conj );
  }
}

bool
QuantifiedEffect::operator==( const Effect& eff ) const
{
  return( false );
}

void QuantifiedEffect::print(std::ostream& os,
    const PredicateTable& predicates,
    const FunctionTable& functions,
    const TermTable& terms, bool ignore_reward) const
{
  if (parameters_.empty()) {
    effect().print(os, predicates, functions, terms, ignore_reward);
  }
  else {
    if (gpt::external_ff_ignore_forall_w_prob_effects) {
      const Effect* eff = &effect().flatten();
      const ProbabilisticEffect* peff =
                dynamic_cast<const ProbabilisticEffect*>(eff);
      if (peff != NULL) {
        Effect::unregister_use(eff);
        WARNING("Applying HACK for externalFF when a forall with probabilistic"
                " effect is found!");
        return;
      }
      Effect::unregister_use(eff);
      // If it is not a probabilistic effect, then we can print it because FF
      // will be able to handle it.
    }

    os << "(forall (";
    for (VariableList::const_iterator vi = parameters_.begin();
         vi != parameters_.end(); ++vi)
    {
      terms.print_term(os, *vi);
      os << " - ";
      gpt::problem->domain().types().print_type(os, terms.type(*vi));
      if (vi != parameters_.end())
        os << " ";
    }
    os << ") ";
    effect().print(os, predicates, functions, terms, ignore_reward);
    os << ")";
  }
}

void
QuantifiedEffect::analyze( PredicateTable &predicates, TermTable &terms,
    std::map<const StateFormula*,const Atom*> &hash ) const
{
  effect().analyze( predicates, terms, hash );
}

const Effect&
QuantifiedEffect::rewrite( std::map<const StateFormula*,const Atom*> &hash ) const
{
  QuantifiedEffect *q = new QuantifiedEffect( effect().rewrite( hash ) );
  Effect::unregister_use( &effect() );
  q->parameters_ = parameters_;
  return( *q );
}
