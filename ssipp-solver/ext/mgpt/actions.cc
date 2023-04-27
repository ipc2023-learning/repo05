#include "actions.h"
#include "../../utils/exceptions.h"
#include "problems.h"
#include "domains.h"
#include "formulas.h"
#include "states.h"
#include "../../utils/die.h"

#include <assert.h>
#include <sstream>
#include <stack>


/*******************************************************************************
 *
 * action schema
 *
 ******************************************************************************/

  ActionSchema::ActionSchema( const std::string& name )
: name_(name), precondition_(&StateFormula::TRUE), effect_(new ConjunctiveEffect())
{
  StateFormula::register_use( precondition_ );
}

ActionSchema::~ActionSchema()
{
  StateFormula::unregister_use( precondition_ );
  Effect::unregister_use( effect_ );
}

  void
ActionSchema::set_precondition( const StateFormula& precondition )
{
  if( &precondition != precondition_ )
  {
    StateFormula::unregister_use( precondition_ );
    precondition_ = &precondition;
    StateFormula::register_use( precondition_ );
  }
}

  void
ActionSchema::set_effect( const Effect& effect )
{
  if( &effect != effect_ )
  {
    Effect::unregister_use( effect_ );
    effect_ = &effect;
    Effect::register_use( effect_ );
  }
}

void
ActionSchema::instantiations( ActionList& actions, const problem_t& problem ) const
{
  size_t n = arity();
  if( n == 0 )
  {
    SubstitutionMap subst;
    const StateFormula& precond = precondition().instantiation( subst, problem );
    if( !precond.contradiction() )
      actions.push_back( &instantiation( subst, problem, precond ) );
    else
      StateFormula::unregister_use( &precond );
  }
  else
  {
    SubstitutionMap args;
    std::vector<ObjectList> arguments( n, ObjectList() );
    std::vector<ObjectList::const_iterator> next_arg;
    for( size_t i = 0; i < n; ++i )
    {
      problem.compatible_objects( arguments[i],
          problem.domain().terms().type( parameter(i) ) );
      if( arguments[i].empty() ) return;
      next_arg.push_back( arguments[i].begin() );
    }

    std::stack<const StateFormula*> preconds;
    preconds.push( precondition_ );
    StateFormula::register_use( preconds.top() );
    for( size_t i = 0; i < n; )
    {
      SubstitutionMap pargs;
      args.insert( std::make_pair( parameter(i), *next_arg[i] ) );
      pargs.insert( std::make_pair( parameter(i), *next_arg[i] ) );
      const StateFormula& precond = preconds.top()->instantiation( pargs, problem );
      preconds.push( &precond );

      if( (i + 1 == n) || precond.contradiction() )
      {
        if( !precond.contradiction() )
        {
          StateFormula::register_use( preconds.top() );
          actions.push_back( &instantiation( args, problem, precond ) );
        }

        for( int j = i; j >= 0; --j )
        {
          StateFormula::unregister_use( preconds.top() );
          preconds.pop();
          args.erase( parameter( j ) );
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
        ++i;
    }

    while( !preconds.empty() )
    {
      StateFormula::unregister_use( preconds.top() );
      preconds.pop();
    }
  }
}

const Action&
ActionSchema::instantiation( const SubstitutionMap& subst, const problem_t& problem,
    const StateFormula& precond ) const
{
  Action *action = new Action( name() );

  for( size_t i = 0; i < arity(); ++i )
  {
    SubstitutionMap::const_iterator si = subst.find( parameter( i ) );
    action->add_argument( (*si).second );
  }

  action->set_precondition( precond );
  StateFormula::unregister_use( &precond );
  const Effect *eff = &effect().instantiation( subst, problem );
  action->set_effect( *eff );
  Effect::unregister_use( eff );

  return( *action );
}

void
ActionSchema::print( std::ostream& os, const PredicateTable& predicates,
    const FunctionTable& functions,
    const TermTable& terms ) const
{
  os << "  " << name();
  os << std::endl << "    parameters:";
  for( VariableList::const_iterator vi = parameters_.begin(); vi != parameters_.end(); ++vi )
  {
    os << ' ';
    terms.print_term( os, *vi );
  }
  os << std::endl << "    precondition: ";
  precondition().print( os, predicates, functions, terms );
  os << std::endl << "    effect: ";
  effect().print( os, predicates, functions, terms );
}

void
ActionSchema::analyze( PredicateTable &predicates, TermTable &terms,
    std::map<const StateFormula*,const Atom*> &hash ) const
{
  if( gpt::verbosity >= 350 )
    std::cout << "analyzing schema for action `" << name() << "'" << std::endl;
  precondition().analyze( predicates, terms, hash );
  effect().analyze( predicates, terms, hash );
}

const ActionSchema&
ActionSchema::rewrite( std::map<const StateFormula*,const Atom*> &hash ) const
{
  ActionSchema *action = new ActionSchema( name() );
  for( size_t i = 0; i < arity(); ++i )
    action->add_parameter( parameter( i ) );
  action->set_precondition( precondition().rewrite( hash ) );
  action->set_effect( effect().rewrite( hash ) );
  return( *action );
}


/*******************************************************************************
 *
 * action
 *
 ******************************************************************************/

  Action::Action( const std::string& name )
: ref_count_(0), name_(name), precondition_(&StateFormula::TRUE),
  effect_(new ConjunctiveEffect())
{
  notify( this, "Action::Action(std::string&)" );
  Action::register_use( this );
  StateFormula::register_use( precondition_ );
}

Action::~Action()
{
  DIE(ref_count_ == 0, "Destructing Object that still has a ref.", 101);
  StateFormula::unregister_use( precondition_ );
  Effect::unregister_use( effect_ );
}

  void
Action::set_precondition( const StateFormula& precondition )
{
  if( &precondition != precondition_ )
  {
    StateFormula::unregister_use( precondition_ );
    precondition_ = &precondition;
    StateFormula::register_use( precondition_ );
  }
}

  void
Action::set_effect( const Effect& effect )
{
  if( &effect != effect_ )
  {
    Effect::unregister_use( effect_ );
    effect_ = &effect;
    Effect::register_use( effect_ );
  }
}

const Action&
Action::flatten( const problem_t &problem ) const
{
  Action *action = new Action( name() );

  for( ObjectList::const_iterator oi = arguments_.begin(); oi != arguments_.end(); )
    action->add_argument( *oi++ );

  const StateFormula *prec = &precondition().flatten( false );
  action->set_precondition( *prec );
  StateFormula::unregister_use( prec );

  const Effect *eff = &effect().flatten();
  action->set_effect( *eff );
  Effect::unregister_use( eff );

  return( *action );
}

action_t& Action::translate(const problem_t &problem) const {
  // generate names
  std::ostringstream ost, ostXML;
  ost << "(" << name();
  ostXML << "<action><name>" << name() << "</name>";
  for( ObjectList::const_iterator oi = arguments_.begin(); oi != arguments_.end(); ++oi )
  {
    ost << ' ';
    problem.terms().print_term( ost, *oi );
    ostXML << "<term>";
    problem.terms().print_term( ostXML, *oi );
    ostXML << "</term>";
  }
  ost << ")";
  ostXML << "</action>";

  const ProbabilisticEffect *peff =
    dynamic_cast<const ProbabilisticEffect*>(&effect());

  if (peff != NULL) {
    probabilisticAction_t* action = new probabilisticAction_t(ost.str(),
                                                              ostXML.str());
    precondition().translate(action->precondition());
    peff->translate(action->effect());

    if (action->effect().size() > 1)
      return (*action);
    else {
      DIE(action->probability(0) == 1, "Prob action with one effect that doesn't have p=1", 102);
      deterministicAction_t *daction =
        new deterministicAction_t(ost.str(), ostXML.str());
      daction->precondition() = action->precondition();
      daction->effect() = action->effect(0);
      action_t::unregister_use(action);
      return (*daction);
    }
  } else {
    deterministicAction_t *action = new deterministicAction_t(ost.str(),
                                                              ostXML.str());
    precondition().translate(action->precondition());
    effect().translate(action->effect().s_effect(), action->effect_.c_effect());
    return (*action);
  }
}

bool Action::enabled(const state_t& state) const {
  if (!precondition().holds(state))
    return false;
  return true;
}

void
Action::affect( state_t& state ) const
{
  AtomList adds;
  AtomList deletes;
  AssignmentList assig;
  effect().state_change( adds, deletes, assig, state );

  for( AtomList::const_iterator ai = deletes.begin(); ai != deletes.end(); ++ai )
    state.clear( **ai );

  for( AtomList::const_iterator ai = adds.begin(); ai != adds.end(); ++ai )
    state.add( **ai );

  for( AssignmentList::const_iterator ai = assig.begin(); ai != assig.end(); ++ai )
    (*ai)->affect( state );

  state.make_digest();
  DIE(state.make_check(), "Fail state.make_check()", 103);
}


void
Action::print_full( std::ostream& os, const PredicateTable& predicates,
    const FunctionTable& functions,
    const TermTable& terms ) const
{
  os << "(action (" << name();
  for( ObjectList::const_iterator oi = arguments_.begin(); oi != arguments_.end(); ++oi )
  {
    os << ' ';
    terms.print_term( os, *oi );
  }
  os << ")" << std::endl << "        (prec ";
  precondition().print( os, predicates, functions, terms );
  os << ")" << std::endl << "        (eff ";
  effect().print( os, predicates, functions, terms );
  os << "))";
}

void
Action::print( std::ostream& os, const TermTable& terms ) const
{
  os << '(' << name();
  for( ObjectList::const_iterator oi = arguments_.begin(); oi != arguments_.end(); ++oi )
  {
    os << ' ';
    terms.print_term( os, *oi );
  }
  os << ')';
}

void
Action::printXML( std::ostream& os, const TermTable& terms ) const
{
  os << "<action><name>" << name() << "</name>";
  for( ObjectList::const_iterator oi = arguments_.begin(); oi != arguments_.end(); ++oi )
  {
    os << "<term>";
    terms.print_term( os, *oi );
    os << "</term>";
  }
  os << "</action>";
}


/*******************************************************************************
 *
 * conditional effect list
 *
 ******************************************************************************/

bool
conditionalEffectList_t::affect(state_t const& state, state_t& s_prime, bool nprec ) const
{
  bool rv = false;
  for( size_t i = 0; i < size(); ++i )
    rv = effect(i).affect(state, s_prime, nprec ) || rv;
  return( rv );
}

void
conditionalEffectList_t::print( std::ostream &os ) const
{
  for( size_t i = 0; i < size(); ++i )
  {
    effect( i ).print( os );
    if( i + 1 < size() ) os << " ";
  }
}

bool
conditionalEffectList_t::operator==( const conditionalEffectList_t &clist ) const
{
  for( size_t i = 0; i < size(); ++i )
    if( !clist.find( effect( i ) ) )
      return( false );
  for( size_t i = 0; i < clist.size(); ++i )
    if( !find( clist.effect( i ) ) )
      return( false );
  return( true );
}

  conditionalEffectList_t&
conditionalEffectList_t::operator=( const conditionalEffectList_t &clist )
{
  clear();
  for( size_t i = 0; i < clist.size(); ++i )
  {
    conditionalEffect_t *ceffect = new conditionalEffect_t;
    *ceffect = clist.effect( i );
    insert( ceffect );
  }
  return( *this );
}

Rational conditionalEffectList_t::cost(state_t const& s, size_t cost_idx) const
{
  Rational total_cost(0);
  for (size_t i = 0; i < size(); ++i)
    total_cost = total_cost + effect(i).cost(s, cost_idx);
  return total_cost;
}

Rational conditionalEffectList_t::cost_lowerbound() const {
  if (size() == 0) return Rational(0);

  Rational lowerbound = effect(0).cost_lowerbound();
  for (size_t i = 1; i < size(); ++i) {
    Rational i_lb = effect(i).cost_lowerbound();
    if (i_lb < lowerbound)
      lowerbound = i_lb;
  }
  return lowerbound;
}


Rational conditionalEffectList_t::cost_upperbound() const {
  if (size() == 0) return Rational(0);

  Rational upperbound = effect(0).cost_upperbound();
  for (size_t i = 1; i < size(); ++i) {
    Rational i_ub = effect(i).cost_upperbound();
    if (i_ub > upperbound)
      upperbound = i_ub;
  }
  return upperbound;
}


bool one_way_conditionalEffectList_reward_consistent_test(
    conditionalEffectList_t const& cl1,
    conditionalEffectList_t const& cl2)
{
  for (size_t i = 0; i < cl1.size(); ++i) {
    bool at_least_one_match = false;
    for (size_t j = 0; j < cl2.size(); ++j) {
      // Making sure that every precondition match also matches in the reward
      // change
      if (cl1.effect(i).precondition() == cl2.effect(j).precondition()) {
        at_least_one_match = true;
        if (cl1.effect(i).s_effect().cost() != cl2.effect(j).s_effect().cost())
          return false;
      }
    }
    // if no match was found, then the effect should not change the reward
    if (!at_least_one_match && cl1.effect(i).s_effect().cost() != Rational(0))
      return false;
  }
  return true;
}

bool conditionalEffectList_t::is_reward_consistent_with(
    conditionalEffectList_t const& clist) const
{
  return one_way_conditionalEffectList_reward_consistent_test(*this, clist) &&
          one_way_conditionalEffectList_reward_consistent_test(clist, *this);
}


/*******************************************************************************
 *
 * probabilistic effect list
 *
 ******************************************************************************/

bool
probabilisticEffectList_t::affect(state_t const& state, state_t& s_prime,
    bool nprec ) const
{
  double r = drand48();
  double sum = 0;
  for( size_t i = 0; i < size(); ++i )
  {
    sum += effect( i ).probability().double_value();
    if( r < sum )
      return( effect( i ).affect( state, s_prime, nprec ) );
  }
  return( false );
}

void
probabilisticEffectList_t::print( std::ostream &os ) const
{
  os << "(probabilistic";
  for( size_t i = 0; i < size(); ++i )
  {
    os << " ";
    effect( i ).print( os );
  }
  os << ")";
}

  probabilisticEffectList_t&
probabilisticEffectList_t::operator=( const probabilisticEffectList_t &plist )
{
  clear();
  for( size_t i = 0; i < plist.size(); ++i )
  {
    probabilisticEffect_t *peffect =
      new probabilisticEffect_t( plist.effect( i ).probability() );
    *peffect = plist.effect( i );
    insert( peffect );
  }
  return( *this );
}



Rational probabilisticEffectList_t::cost(state_t const& state, size_t cost_idx)
  const
{
  DIE(size() > 0, "Unexpected size of probabilisticEffectList_t", 162);
  // ASSUMPTION(fwt): I'm assuming that the reward/cost is consistent within the
  // probabilistic effects (the meaning of this is checked by the for bellow).
  // Notice that we try to enforce this in the stage 4 of problem::flatten
  // using the method is_reward_consistent of the probabilistic actions
#ifdef DOUBLE_CHECK_REWARD_CONSISTENCY_PROB_ACTION
  Rational cost_eff_0 = effect(0).cost(state, cost_idx);
  for (size_t i=1; i < size(); ++i)
    DIE(cost_eff_0 == effect(i).cost(state, cost_idx), // 163
        "Reward inconsistency in the probabilistic action", 163);
#endif
  Rational expected_cost = 0.0;
  for (size_t i = 0; i < size(); ++i) {
    expected_cost += effect(i).probability() * effect(i).cost(state, cost_idx);
  }
  return expected_cost;
}


Rational probabilisticEffectList_t::cost_lowerbound() const {
  DIE(size() > 0, "Unexpected size of probabilisticEffectList_t", 162);
  // ASSUMPTION(fwt): Same as probabilisticEffectList_t::cost(state_t const&
  // state) See comments above
  return effect(0).cost_lowerbound();
}

Rational probabilisticEffectList_t::cost_upperbound() const {
  DIE(size() > 0, "Unexpected size of probabilisticEffectList_t", 162);
  // ASSUMPTION(fwt): Same as probabilisticEffectList_t::cost(state_t const&
  // state) See comments above
  return effect(0).cost_upperbound();
}

void probabilisticEffectList_t::shift_cost_by(Rational c) {
  for (size_t i=0; i < size(); ++i)
    // HACK(fwt): It's a nightmare to make an effect(i) function that returns a
    // non-const probabilisticEffect_t
    const_cast<probabilisticEffect_t&>(effect(i)).shift_cost_by(c);
}


/*******************************************************************************
 *
 * strips effect
 *
 ******************************************************************************/

bool stripsEffect_t::reward_warning_printed_ = false;

stripsEffect_t::stripsEffect_t() : cost_(gpt::problem->domain().functions().size(),
                                         Rational(0))
{  }


bool
stripsEffect_t::affect(state_t const& state, state_t& s_prime, bool nprec ) const
{
  bool rv = false;
  // NOTE: Add effects are considered first, then delete effects. Therefore,
  // if an atom is in both add and delete lists, then it won't be appear in
  // the resulting state
  for( size_t i = 0; i < add_list().size(); ++i )
    rv = s_prime.add( add_list().atom( i ) ) || rv;
  for( size_t i = 0; i < del_list().size(); ++i )
    rv = s_prime.clear( del_list().atom( i ) ) || rv;
  return( rv );
}

void
stripsEffect_t::collect_prec_atoms( atomList_t &atoms ) const
{
}

void
stripsEffect_t::collect_add_atoms( atomList_t &atoms ) const
{
  atoms.insert( add_list() );
}

void
stripsEffect_t::collect_del_atoms( atomList_t &atoms ) const
{
  atoms.insert( del_list() );
}

void stripsEffect_t::print(std::ostream& os) const {
  bool print_space = false;
  if (add_list().size() > 0) {
    os << "(add ";
    add_list().print( os );
    os << ")";
    print_space = true;
  }

  if (del_list().size() > 0) {
    os << (print_space ? " " : "") << "(del ";
    del_list().print( os );
    os << ")";
    print_space = true;
  }

  FunctionTable const& ft = gpt::problem->domain().functions();
  for (size_t i = 0; i < ft.size(); ++i) {
    std::string const& f_name = ft.name(i);
//    if (f_name == "goal-probability") {
//      continue;
//    }

    os << (print_space ? " " : "") << "(";
    if (f_name == "reward") {
      // Cost is always 1 - reward and reward <= 0
      os << "cost += " << (1 - cost_[i]) << ")";
    }
    else {
      os << f_name << " += " << cost_[i] << ")";
    }
  }
}

bool stripsEffect_t::operator==( const stripsEffect_t &effect ) const {
  return (add_list() == effect.add_list()
            && del_list() == effect.del_list()
            && cost_ == effect.cost_);
}

stripsEffect_t& stripsEffect_t::operator=(const stripsEffect_t &effect) {
  add_list() = effect.add_list();
  del_list() = effect.del_list();
  cost_ = effect.cost_;
  return (*this);
}


/*******************************************************************************
 *
 * conditional effect
 *
 ******************************************************************************/

bool
conditionalEffect_t::affect(state_t const& state, state_t& s_prime, bool nprec ) const
{
  return( precondition().holds( state, nprec ) && s_effect().affect(state, s_prime, nprec ) );
}

void
conditionalEffect_t::collect_prec_atoms( atomList_t &atoms ) const
{
  for( size_t i = 0; i < precondition().size(); ++i )
    atoms.insert( precondition().atom_list( i ) );
  s_effect().collect_prec_atoms( atoms );
}

void
conditionalEffect_t::collect_add_atoms( atomList_t &atoms ) const
{
  s_effect().collect_add_atoms( atoms );
}

void
conditionalEffect_t::collect_del_atoms( atomList_t &atoms ) const
{
  s_effect().collect_del_atoms( atoms );
}

void
conditionalEffect_t::print( std::ostream &os ) const
{
  os << "(when (";
  precondition().print( os );
  os << ") ";
  s_effect().print( os );
  os << ")";
}

bool
conditionalEffect_t::operator==( const conditionalEffect_t &effect ) const
{
  return( (precondition() == effect.precondition()) &&
      (s_effect() == effect.s_effect()) );
}

  conditionalEffect_t&
conditionalEffect_t::operator=( const conditionalEffect_t &effect )
{
  precondition() = effect.precondition();
  s_effect() = effect.s_effect();
  return( *this );
}


/*******************************************************************************
 *
 * deterministic effect
 *
 ******************************************************************************/

deterministicEffect_t::~deterministicEffect_t()
{
  for( size_t i = 0; i < c_effect().size(); ++i )
    delete &c_effect().effect( i );
}

bool
deterministicEffect_t::empty( void ) const
{
  if( !s_effect().empty() ) return( false );
  for( size_t i = 0; i < c_effect().size(); ++i )
    if( !c_effect().effect( i ).empty() )
      return( false );
  return( true );
}

  void
deterministicEffect_t::insert_effect( const stripsEffect_t &seff )
{
  for( size_t i = 0; i < seff.add_list().size(); ++i )
    s_effect().add_list().insert( seff.add_list().atom( i ) );
  for( size_t i = 0; i < seff.del_list().size(); ++i )
    s_effect().del_list().insert( seff.del_list().atom( i ) );
  s_effect().increase_cost_by(seff.cost(ACTION_COST), ACTION_COST);
}

bool
deterministicEffect_t::affect(state_t const& state, state_t& s_prime, bool nprec ) const
{
  bool rv = false;
  rv = s_effect().affect( state, s_prime, nprec ) || rv;
  rv = c_effect().affect( state, s_prime, nprec ) || rv;
  return( rv );
}

void
deterministicEffect_t::collect_prec_atoms( atomList_t &atoms ) const
{
  s_effect().collect_prec_atoms( atoms );
  for( size_t i = 0; i < c_effect().size(); ++i )
    c_effect().effect( i ).collect_prec_atoms( atoms );
}

void
deterministicEffect_t::collect_add_atoms( atomList_t &atoms ) const
{
  s_effect().collect_add_atoms( atoms );
  for( size_t i = 0; i < c_effect().size(); ++i )
    c_effect().effect( i ).collect_add_atoms( atoms );
}

void
deterministicEffect_t::collect_del_atoms( atomList_t &atoms ) const
{
  s_effect().collect_del_atoms( atoms );
  for( size_t i = 0; i < c_effect().size(); ++i )
    c_effect().effect( i ).collect_del_atoms( atoms );
}

void
deterministicEffect_t::print( std::ostream &os ) const
{
  s_effect().print( os );
  if( c_effect().size() > 0 )
  {
    if( (s_effect().add_list().size() > 0) ||
        (s_effect().del_list().size() > 0) )
      os << " ";
    c_effect().print( os );
  }
}

bool
deterministicEffect_t::operator==( const deterministicEffect_t &effect ) const
{
  return( (s_effect() == effect.s_effect()) &&
      (c_effect() == effect.c_effect()) );
}

  deterministicEffect_t&
deterministicEffect_t::operator=( const deterministicEffect_t &effect )
{
  s_effect() = effect.s_effect();
  c_effect() = effect.c_effect();
  return( *this );
}


void deterministicEffect_t::probability_of_adding_atoms(
    state_t const& s, HashAtomtToRational& prob) const
{
  bool nprec = gpt::problem->nprec();

  atomList_t add;

  // Collecting the add atoms from the strips effect
  s_effect().collect_add_atoms(add);

  // Collecting the add atoms from the conditional effect that are applicable
  conditionalEffectList_t const& c_eff_list = c_effect();
  for (size_t i = 0; i < c_eff_list.size(); ++i) {
    if (c_eff_list.effect(i).precondition().holds(s, nprec)) {
      c_eff_list.effect(i).collect_add_atoms(add);
    }
  }

  // Adding the collected atoms to the probability list
  for (size_t i = 0; i < add.size(); ++i) {
    HashAtomtToRational::iterator it = prob.find(add.atom(i));
    FANCY_DIE_IF(it != prob.end() && prob[add.atom(i)] != 1, 171,
        "prob has atom '%u' and its probability is %f",
        add.atom(i), prob[add.atom(i)].double_value());
    prob[add.atom(i)] = 1;
  }
}



/*******************************************************************************
 *
 * probabilistic effect
 *
 ******************************************************************************/

bool
probabilisticEffect_t::affect(state_t const& state, state_t& s_prime, bool nprec ) const
{
  return( deterministicEffect_t::affect( state, s_prime, nprec ) );
}

void
probabilisticEffect_t::print( std::ostream &os ) const
{
  os << "(" << probability() << " ";
  deterministicEffect_t::print( os );
  os << ")";
}

bool
probabilisticEffect_t::operator==( const probabilisticEffect_t &effect ) const
{
  return( (s_effect() == effect.s_effect()) &&
      (c_effect() == effect.c_effect()) );
}

  probabilisticEffect_t&
probabilisticEffect_t::operator=( const probabilisticEffect_t &effect )
{
  probability_ = effect.probability();
  s_effect() = effect.s_effect();
  c_effect() = effect.c_effect();
  return( *this );
}

bool probabilisticEffect_t::is_reward_consistent_with(
    probabilisticEffect_t const& peff) const
{
  if (this->s_effect().cost() != peff.s_effect().cost())
    return false;
  if (! this->c_effect().is_reward_consistent_with(peff.c_effect()))
    return false;
  return true;
}


/*******************************************************************************
 *
 * action
 *
 ******************************************************************************/

action_t::action_t(const std::string &name, const std::string &nameXML) : ref_count_(0)
{
  action_t::register_use(this);
  name_ = strdup(name.c_str());
  nameXML_ = strdup(nameXML.c_str());
  original_action_ = NULL;
}

action_t::~action_t()
{
  if (original_action_) {
    unregister_use(original_action_);
  }
  DIE(ref_count_ == 0, "Destructing Object that still has a ref.", 101);
  free( (void*)name_ );
  free( (void*)nameXML_ );
}

  void
action_t::insert_precondition( const atomList_t &alist )
{
  atomList_t *al = new atomList_t;
  *al = alist;
  precondition().insert( al );
}

  void
action_t::insert_precondition( const atomListList_t &alist )
{
  for( size_t i = 0; i < alist.size(); ++i )
    insert_precondition( alist.atom_list( i ) );
}


/*******************************************************************************
 *
 * deterministic action
 *
 ******************************************************************************/

deterministicAction_t::deterministicAction_t(std::string const& name,
    std::string const& nameXML) : action_t(name,nameXML),
    self_loop_correction_(1)
{
  notify(this,
     "deterministicAction_t::deterministicAction_t(std::string&,std::string&)");
}

deterministicAction_t::~deterministicAction_t()
{
}

bool deterministicAction_t::affect(state_t& state, bool nprec) const {
  state_t s_prime = state;
  bool rv = effect().affect(state, s_prime, nprec);
  s_prime.make_digest();
  state = s_prime;
  return rv;
}


void deterministicAction_t::expand(state_t const& s, ProbDistStateIface& pr,
      bool nprec) const
{
  // TODO(fwt): make this more efficient
  pr.clear();
  state_t s_prime(s);
  affect(s_prime, nprec);
  pr.insert(s_prime, 1.0);
}


void
deterministicAction_t::print_full( std::ostream &os ) const
{
  os << "(action " << name() << std::endl
    << "        (prec ";
  precondition().print( os );
  os << ")" << std::endl
    << "        (eff ";
  effect().print( os );
  _D(DEBUG_SELFLOOP, if (self_loop_correction() != 1) {
      os << std::endl << "        self-loop correction = "
         << self_loop_correction();
    });
  os << "))";
}

action_t* deterministicAction_t::clone() const {
  deterministicAction_t *result = new deterministicAction_t(name(), nameXML());
  result->precondition() = precondition();
  result->effect() = effect();
  result->self_loop_correction_ = self_loop_correction_;
  return result;
}

void
deterministicAction_t::collect_prec_atoms( atomList_t &atoms ) const
{
  for( size_t i = 0; i < precondition().size(); ++i )
    atoms.insert( precondition().atom_list( i ) );
  effect().collect_prec_atoms( atoms );
}

void
deterministicAction_t::collect_add_atoms( atomList_t &atoms ) const
{
  effect().collect_add_atoms( atoms );
}

void
deterministicAction_t::collect_del_atoms( atomList_t &atoms ) const
{
  effect().collect_del_atoms( atoms );
}

bool deterministicAction_t::simplify_confiditonal() {
  if (effect().s_effect().empty() && effect().c_effect().size() == 1) {
    // This deterministic action as only one conditional effect, therefore it
    // will be changed to a deterministic action with preconditions instead of
    // using conditional
    const conditionalEffect_t& c_eff = effect().c_effect().effect(0);

    if (precondition().size() == 1 && precondition().atom_list(0).size() == 0)
      // The action as no precondition. The atom_list(0) of size() == 0 is
      // just marking the fact that the precondition is empty
      precondition().clear();

    insert_precondition(c_eff.precondition());

    effect().s_effect() = c_eff.s_effect();
    effect().c_effect().clear();
    //delete c_eff;
    return true;
  }
  return false;
}


Rational deterministicAction_t::cost(size_t cost_idx) const {
  if (gpt::use_action_cost) {
    if (cost_idx == ACTION_COST) {
      // Cost is always 1 - reward and reward <= 0
//      assert(effect_.cost(cost_idx) <= 0);
      return self_loop_correction_ * (Rational(1) - effect_.cost(cost_idx));
    }
    else {
      return self_loop_correction_ * effect_.cost(cost_idx);
    }
  } else {
    return self_loop_correction_;
  }
}


Rational deterministicAction_t::cost(state_t const& s, size_t cost_idx) const {
  if (gpt::use_action_cost) {
    if (cost_idx == ACTION_COST) {
      // Cost is always 1 - reward and reward <= 0
//      assert(effect_.cost(cost_idx) <= 0);
      return self_loop_correction_ * (Rational(1) - effect_.cost(s, cost_idx));
    }
    else {
      return self_loop_correction_ * effect_.cost(s, cost_idx);
    }
  } else {
    return self_loop_correction_;
  }
}


const action_t* action_t::original_action() const {
  // TODO(fwt): merge with get_original_action()
  // this assumes that NULL will never be returned
  // get_original_action returns NULL if the action hasn't changed
  const action_t *o = this;
  while(o->original_action_) {
    o = o->original_action_;
  }
  return o;
}



/* get highest ancestor that is not probabilistic */
const action_t* action_t::original_np_action() const {

  const action_t *o = this;
  while(o->original_action_) {
    const probabilisticAction_t* pact =
      dynamic_cast<const probabilisticAction_t*>( o->original_action_ );
    if(pact == NULL) {
      o = o->original_action_;
    }
    else {
      break;
    }
  }
  /*
  std::cout << "-----------------------------" << std::endl;
  std::cout << "action being returned from original_np()" <<std::endl;
  print_full(std::cout); std::cout << std::endl;
  std::cout << "-----------------------------" << std::endl;*/
  return o;
}



bool action_t::deletes_atom( ushort_t atom ) const {
  atomList_t alist;
  collect_del_atoms( alist );
  return (alist.find(atom));
}

bool action_t::adds_atom( ushort_t atom ) const {
  atomList_t alist;
  collect_add_atoms(alist);
  return (alist.find(atom));
}


/*******************************************************************************
 *
 * probabilistic action
 *
 ******************************************************************************/

  probabilisticAction_t::probabilisticAction_t( const std::string &name, const std::string &nameXML )
: action_t(name,nameXML)
{
  notify( this, "probabilisticAction_t::probabilisticAction_t(std::string&,std::string&)" );
}

probabilisticAction_t::~probabilisticAction_t()
{
  for( size_t i = 0; i < size(); ++i )
    delete &effect( i );
}

bool
probabilisticAction_t::empty( void ) const
{
  for( size_t i = 0; i < size(); ++i )
    if( !effect( i ).empty() )
      return( false );
  return( true );
}

bool probabilisticAction_t::affect(state_t& state, bool nprec) const {
  state_t s_prime = state;
  bool rv = effect().affect(state, s_prime, nprec);
  s_prime.make_digest();
  state = s_prime;
  return rv;
}


/*
 * See explanations in the header file
 */
void probabilisticAction_t::expand_hash_based(state_t const& s,
    ProbDistStateIface& pr, bool nprec) const
{
  static bool msg = true;
  if (msg) {
    msg = false;
    std::cout << "\nUsing the HASH expansion in probAct_t::expand"
      << std::endl;
  }

  static ProbDistStateHash pr_hash;
  pr_hash.clear();
  expand_directly(s, pr_hash, nprec);

  // Fulfilling the original pr with the summarized and equivalent ProbDist
  pr.clear();
  for (auto const& ip : pr_hash) {
    pr.insert(ip.event(), ip.prob());
  }
}


/*
 * See explanations in the header file
 */
void probabilisticAction_t::expand_directly(state_t const& s,
    ProbDistStateIface& pr, bool nprec) const
{
  pr.clear();
  for (size_t i = 0; i < effect().size(); ++i) {
    // TODO(fwt): make this more efficient
    state_t s_prime = s;
    effect(i).affect(s, s_prime, nprec);
    s_prime.make_digest();
    pr.insert(s_prime, probability(i).double_value());
  }
}


void
probabilisticAction_t::print_full( std::ostream &os ) const
{
  os << "(action " << name() << std::endl
    << "        (prec ";
  precondition().print( os );
  os << ")" << std::endl
    << "        (eff ";
  effect().print( os );
  os << "))";
}

action_t*
probabilisticAction_t::clone( void ) const
{
  probabilisticAction_t *result = new probabilisticAction_t( name(), nameXML() );
  result->precondition() = precondition();
  result->effect() = effect();
  return( result );
}

void
probabilisticAction_t::collect_prec_atoms( atomList_t &atoms ) const
{
  for( size_t i = 0; i < precondition().size(); ++i )
    atoms.insert( precondition().atom_list( i ) );

  for( size_t i = 0; i < effect().size(); ++i )
    effect( i ).collect_prec_atoms( atoms );
}

void
probabilisticAction_t::collect_add_atoms( atomList_t &atoms ) const
{
  for( size_t i = 0; i < effect().size(); ++i )
    effect( i ).collect_add_atoms( atoms );
}

void
probabilisticAction_t::collect_del_atoms( atomList_t &atoms ) const
{
  for( size_t i = 0; i < effect().size(); ++i )
    effect( i ).collect_del_atoms( atoms );
}


void
probabilisticAction_t::replanning_split(double threshold, actionList_t& result) const
{
  probabilisticEffectList_t remaining_effects;
  size_t subaction_id = 0;
  for (size_t i = 0; i < size(); ++i)
  {
    if (probability(i).double_value() <= threshold) {
      if (effect(i).empty()) {
#ifdef REPLANNER_DEBUG
        std::cout << "Ignoring the following effect since it's empty:\n";
        effect(i).print(std::cout);
        std::cout << std::endl;
#endif
        continue;
      }

      // Making a new deterministic effect that contains only the current
      // effect
      deterministicEffect_t *deffect = new deterministicEffect_t();
      deffect->s_effect() = effect(i).s_effect();
      deffect->c_effect() = effect(i).c_effect();


      // Making a new deterministic action using this new deterministc effect
      std::ostringstream newName;
      newName << name() << "_" << subaction_id;
      deterministicAction_t *daction = new deterministicAction_t(newName.str(),
          nameXML());
      daction->precondition() = precondition();
      daction->effect() = *deffect;
      daction->set_original_action(this);

#ifdef REPLANNER_DEBUG
      std::cout << "Detached action:\n";
      daction->print_full(std::cout);
      std::cout << std::endl;
#endif
      result.push_back(daction);
      subaction_id++;
    } else {
      // Keeping this effect as the remaining ones
      remaining_effects.insert(&effect(i));
    }
  }

  if (subaction_id == 0) { // Nothing happened to this action, so keeping it
#ifdef REPLANNER_DEBUG
    std::cout << "Keeping the original action:\n";
    print_full(std::cout);
    std::cout << std::endl;
#endif
    action_t* my_clone = clone();
    my_clone->set_original_action(this->get_original_action());
    result.push_back(my_clone);

  } else { // This action was modified. At least one effect was detached
    if (remaining_effects.size() == 1) { // The remaining action should be deterministic
      if (remaining_effects.effect(0).empty()) {
#ifdef REPLANNER_DEBUG
        std::cout << "Ignoring the remaining effect since it's empty:\n";
        remaining_effects.effect(0).print(std::cout);
        std::cout << std::endl;
#endif
      } else {

        deterministicEffect_t *deffect = new deterministicEffect_t();
        deffect->s_effect() = remaining_effects.effect(0).s_effect();
        deffect->c_effect() = remaining_effects.effect(0).c_effect();

        // Making a new deterministic action using this new deterministc effect
        std::ostringstream newName;
        newName << name() << "_R";
        deterministicAction_t *daction = new deterministicAction_t(newName.str(),
                                                                   nameXML());
        daction->precondition() = precondition();
        daction->effect() = *deffect;
        daction->set_original_action(this);

#ifdef REPLANNER_DEBUG
        std::cout << "Remaining (deterministic) action:\n";
        daction->print_full(std::cout);
        std::cout << std::endl;
#endif
        result.push_back(daction);
      }
    } else if (remaining_effects.size() > 1) { // There's more than one remaining effect
      // assert(remaining_effects.size() > 1);
      // Re-normalizing the remaining effect
      Rational sum_p_remaining = 0;
      for (size_t i = 0; i < remaining_effects.size(); ++i)
        sum_p_remaining = sum_p_remaining + remaining_effects.effect(i).probability();

      for (size_t i = 0; i < remaining_effects.size(); ++i)
        (const_cast<probabilisticEffect_t*>(&(remaining_effects.effect(i))))->
          change_probability_to(
              remaining_effects.effect(i).probability() / sum_p_remaining);


      // Making a new deterministic action using this new deterministc effect
      std::ostringstream newName;
      newName << name() << "_R";
      probabilisticAction_t *paction = new probabilisticAction_t(newName.str(),
                                                                 nameXML());
      paction->precondition() = precondition();
      paction->effect() = remaining_effects;
      paction->set_original_action(this);

#ifdef REPLANNER_DEBUG
      std::cout << "Remaining (probabilistic) action:\n";
      paction->print_full(std::cout);
      std::cout << std::endl;
#endif
      result.push_back(paction);
    }
  }
}


bool probabilisticAction_t::is_reward_consistent() const {
  if (size() == 0)
    return true;

  for (size_t i = 1; i < size(); ++i)
    if (! effect(i).is_reward_consistent_with(effect(0)))
      return false;

  return true;
}

Rational probabilisticAction_t::cost(size_t cost_idx) const {
  if (gpt::use_action_cost) {
    if (cost_idx == ACTION_COST) {
      // Cost is always 1 - reward and reward <= 0
//      assert(effect_list_.cost(cost_idx) <= 0);
      return Rational(1) - effect_list_.cost(cost_idx);
    }
    else {
      return effect_list_.cost(cost_idx);
    }
  } else {
    return Rational(1);
  }
}


Rational probabilisticAction_t::cost(state_t const& s, size_t cost_idx) const {
  if (gpt::use_action_cost) {
    if (cost_idx == ACTION_COST) {
      // Cost is always 1 - reward and reward <= 0
//      assert(effect_list_.cost(cost_idx) <= 0);
      return Rational(1) - effect_list_.cost(s, cost_idx);
    }
    else {
      return effect_list_.cost(s, cost_idx);
    }
  } else {
    return Rational(1);
  }
}


void probabilisticAction_t::probability_of_adding_atoms(state_t const& s,
    HashAtomtToRational& prob) const
{
  HashAtomtToRational cur_eff;

  // Collecting the add atoms from each probabilistic effect
  for (size_t i = 0; i < size(); ++i) {
    cur_eff.clear();
    effect(i).probability_of_adding_atoms(s, cur_eff);
    for (HashAtomtToRational::iterator it = cur_eff.begin();
         it != cur_eff.end(); ++it)
    {
      prob[it->first] += probability(i) * it->second;
    }
  }
}

ConditionAndRational::ConditionAndRational(atomListList_t const* c, Rational r)
    : condition_(c), r_(r) { }


void ProbDistOfAddingAnAtom::increaseProbabilityOfConditionBy(
    atom_t atom, atomListList_t const& condition, Rational const& increment)
{
  VectorOfConditionAndRational& vector_of_conds = conditional_prob_[atom];
  for (VectorOfConditionAndRational::iterator ic = vector_of_conds.begin();
       ic != vector_of_conds.end(); ++ic)
  {
    if (*(ic->condition()) == condition) {
      ic->r_ += increment;
      return;
    }
  }
  // Every condition in vector_of_conds is different from the given condition
  ConditionAndRational cr(&condition, increment);
  vector_of_conds.push_back(cr);
}


void ProbDistOfAddingAnAtom::process_det_effect(
    deterministicEffect_t const& det_eff, Rational prob,
    bool consider_negative_atoms)
{
  // Adding non-conditonal effects
  atomList_t const& add_list = det_eff.s_effect().add_list();
  for (size_t i = 0; i < add_list.size(); ++i) {
    atom_t atom = add_list.atom(i);
    potential_support_.insert(atom);
    non_conditional_prob_[atom] += prob;
    _D(DEBUG_DIST_ATOM, std::cout << "  adding non-cond atom "
        << atomToString(atom) << " with prob " << prob << std::endl)
  }

  if (consider_negative_atoms) {
    atomList_t const& del_list = det_eff.s_effect().del_list();
    for (size_t i = 0; i < del_list.size(); ++i) {
      atom_t atom = del_list.atom(i);
      ++atom; // Negative atom
      potential_support_.insert(atom);
      non_conditional_prob_[atom] += prob;
      _D(DEBUG_DIST_ATOM, std::cout << "  adding non-cond (del) atom "
          << atomToString(atom) << " with prob " << prob << std::endl)
    }
  }

  // Adding conditional effects
  conditionalEffectList_t const& cond_eff = det_eff.c_effect();
  if (cond_eff.size() > 0) {
    has_cond_effects_ = true;
    for (size_t ie = 0; ie < cond_eff.size(); ++ie) {
    // for each conditional effect ie
      atomListList_t const& prec_list = cond_eff.effect(ie).precondition();

      // Processing each disjunction indexed by id of the precondition of
      // the effect
      for (size_t id = 0; id < prec_list.size(); ++id) {
        atomList_t const& cur_disjunction = prec_list.atom_list(id);

        // Adding the atoms of this condition in the list of atoms that this
        // action depends on
        for (size_t ic = 0; ic < cur_disjunction.size(); ++ic) {
          depends_on_.insert(cur_disjunction.atom(ic));
          atoms_in_cond_triggers_.insert(cur_disjunction.atom(ic));
        }
      }

      // Adding the following test to the atoms added by this conditional
      // effect:
      //    if prec_list holds then increase the probability of the atom being
      //    added by prob
      // Notice that prec_list represents a disjunction of conjunctions.
      //
      // This rule is for the positive atoms
      atomList_t const& cond_add_list =
                                   cond_eff.effect(ie).s_effect().add_list();
      for (size_t j = 0; j < cond_add_list.size(); ++j)
      {
        atom_t atom = cond_add_list.atom(j);
        potential_support_.insert(atom);
        increaseProbabilityOfConditionBy(atom, prec_list, prob);
        _D(DEBUG_DIST_ATOM, std::cout << "  adding COND atom "
            << atomToString(atom) << " with prob " << prob << std::endl)
      }

      if (consider_negative_atoms) {
        // This rule is for the negative atoms
        atomList_t const& cond_del_list =
                                    cond_eff.effect(ie).s_effect().del_list();
        for (size_t j = 0; j < cond_del_list.size(); ++j)
        {
          atom_t atom = cond_del_list.atom(j);
          ++atom; // Negative atom
          potential_support_.insert(atom);
          increaseProbabilityOfConditionBy(atom, prec_list, prob);
          _D(DEBUG_DIST_ATOM, std::cout << "  adding COND (neg) atom "
              << atomToString(atom) << " with prob " << prob << std::endl)
        }
      }
    }
  }
}

void ProbDistOfAddingAnAtom::process_action(action_t const* a,
    bool consider_negative_atoms)
{
  _D(DEBUG_DIST_ATOM, std::cout << "Processing action " << a->name()
              << " to find prob of adding atom" << std::endl;
      a->print_full();
      std::cout << std::endl;)
  FANCY_DIE_IF(a == NULL, 171, "Action ptr is NULL!!!", a);

  // Adding the preconditions (disjunctions of conjunctions) of the action a
  // in the list of atoms that a depends on
  atomListList_t const& preconditions = a->precondition();
  for (size_t i = 0; i < preconditions.size(); ++i) {
    atomList_t const& cur_disjunction = preconditions.atom_list(i);
    for (size_t j = 0; j < cur_disjunction.size(); ++j) {
      depends_on_.insert(cur_disjunction.atom(j));
    }
  }

  deterministicAction_t const* det_a =
        dynamic_cast<deterministicAction_t const*>(a);
  if (det_a) {
    process_det_effect(det_a->effect(), 1, consider_negative_atoms);
  }
  else {
    probabilisticAction_t const* prob_a =
        dynamic_cast<probabilisticAction_t const*>(a);
    FANCY_DIE_IF(prob_a == NULL, 171, "Action '%s' is neither det nor prob!",
        a->name());
    probabilisticEffectList_t const& prob_effects = prob_a->effect();
    for (size_t i = 0; i < prob_effects.size(); ++i) {
      probabilisticEffect_t const& p_eff = prob_effects.effect(i);
      process_det_effect(p_eff, p_eff.probability(), consider_negative_atoms);
    }
  }

  _D(DEBUG_DIST_ATOM, std::cout << "Action processed. Here is a dump of "
      << "the result:" << std::endl;
    dump();)
}

void ProbDistOfAddingAnAtom::dump() {
  std::cout << "  has_cond_effects_ = "
    << (has_cond_effects_ ? "true" : "false") << std::endl;

  std::cout << "  depends on atoms:";
  for (SetAtom_t::const_iterator id = depends_on_.begin();
       id != depends_on_.end(); ++id)
  {
    std::cout << " " << atomToString(*id);
  }
  std::cout << std::endl
            << "  Probability table: (atom ; probability)" << std::endl;

  for (SetAtom_t::const_iterator is = potential_support_.begin();
       is != potential_support_.end(); ++is)
  {
    Rational max_prob = non_conditional_prob_[*is];
    std::cout << "    " << atomToString(*is)
              << " ; " << non_conditional_prob_[*is];
    HashAtomtToVectorOfConditionAndRational::const_iterator ic =
                                                  conditional_prob_.find(*is);
    if (ic == conditional_prob_.end()) {
      std::cout << " (no conditionals for this atom)";
    }
    else {
      VectorOfConditionAndRational const& conds = ic->second;
      std::cout << " (+ the following " << conds.size()
                << " conditional cases)";
      for (size_t i = 0; i < conds.size(); ++i) {
        max_prob += conds[i].probability();
        std::cout << std::endl << "            + "
          << conds[i].probability() << "  (if ";
        for (size_t ic = 0; ic < conds[i].condition()->size(); ++ic) {
          // for each conjunction
          atomList_t const& conj = conds[i].condition()->atom_list(ic);
          if (conds[i].condition()->size() > 1) { std::cout << "("; }
          for (size_t ia = 0; ia < conj.size(); ++ia) {
            std::cout << atomToString(conj.atom(ia));
            if (ia + 1 < conj.size()) { std::cout << " "; }
          }
          if (conds[i].condition()->size() > 1) {
            std::cout << ")";
            if (ic + 1 < conds[i].condition()->size()) { std::cout << " or "; }
          }
        }
        std::cout << " holds)";
      }
      std::cout << std::endl << "         --> Max prob = " << max_prob;
    }
    std::cout << std::endl;
  }
}


