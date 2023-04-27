#include "problems.h"
#include "domains.h"
#include "../../utils/exceptions.h"
#include "states.h"
#include "../../utils/die.h"
#include "effects.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <typeinfo>
#include <map>
#include <algorithm>    // std::random_shuffle

//#define REPLANNER_DEBUG

/*******************************************************************************
 *
 * problem_t
 *
 ******************************************************************************/


/*******************************************************************************
 * SSPIface implementations methods                                           */
state_t const& problem_t::s0() const {
  static state_t const s0_ = get_initial_state();
  return s0_;
}


/******************************************************************************/

problem_t::ProblemMap problem_t::problems = problem_t::ProblemMap();

ushort_t problem_t::atom_index_ = 0;
bool problem_t::no_more_atoms_ = false;
std::map<const Atom*,ushort_t> problem_t::atom_hash_;
std::map<ushort_t,const Atom*> problem_t::atom_inv_hash_;

ushort_t problem_t::fluent_index_ = 0;
std::map<const Application*,ushort_t> problem_t::fluent_hash_;
std::map<ushort_t,const Application*> problem_t::fluent_inv_hash_;

  ushort_t
problem_t::atom_hash_get( const Atom& atom, bool negated )
{
  std::map<const Atom*,ushort_t>::const_iterator it = atom_hash_.find( &atom );
  if( it == atom_hash_.end() )
  {
    StateFormula::register_use( &atom );
//    if( no_more_atoms_ ) {
//      throw Exception( "sorry, no more atoms available" );
    if (no_more_atoms_) {
      std::cout << "shit, can't add atom '";
      atom.print(std::cout, gpt::problem->domain().predicates(),
                 gpt::problem->domain().functions(), gpt::problem->terms());
      std::cout << "' to atom hash" << std::endl;
    }
    DIE(!no_more_atoms_, "no more atoms available", 143);
    //if( atom_index_ == USHORT_MAX ) throw Exception( "maximum number of atoms reached" );
    DIE(atom_index_ != USHORT_MAX, "maximum number of atoms reached", 144)
    if (atom_index_ > MAX_ATOMS) {
      std::cout << "This problem has more than " << MAX_ATOMS
                << " atoms! Update MAX_ATOMS in global.h or recompile using "
                << "-DMAX_ATOMS=X where X is the new value. Quitting"
                << std::endl;
      exit(-1);
    }
    atom_hash_.insert( std::make_pair( &atom, atom_index_ ) );
    atom_inv_hash_.insert( std::make_pair( atom_index_, &atom ) );
    atom_index_ += 2;
    return( atom_index_ - 2 + (ushort_t)negated );
  }
  else
    return( (*it).second + (ushort_t)negated );
}

  const Atom*
problem_t::atom_inv_hash_get( ushort_t atom )
{
  std::map<ushort_t,const Atom*>::const_iterator it;
  it = atom_inv_hash_.find( atom & ~0x1 );
  return( it == atom_inv_hash_.end() ? NULL : (*it).second );
}

  ushort_t
problem_t::fluent_hash_get( const Application& app )
{
  std::map<const Application*,ushort_t>::const_iterator it = fluent_hash_.find( &app );
  if( it == fluent_hash_.end() )
  {
    Expression::register_use( &app );
    fluent_hash_.insert( std::make_pair( &app, fluent_index_ ) );
    fluent_inv_hash_.insert( std::make_pair( fluent_index_, &app ) );
    ++fluent_index_;
    return( fluent_index_ - 1 );
  }
  else
    return( (*it).second );
}

  const Application*
problem_t::fluent_inv_hash_get( ushort_t fluent )
{
  std::map<ushort_t,const Application*>::const_iterator it;
  it = fluent_inv_hash_.find( fluent & ~0x1 );
  return( it == fluent_inv_hash_.end() ? NULL : (*it).second );
}

  problem_t*
problem_t::find( const std::string& name )
{
  ProblemMap::const_iterator pi = problems.find( name );
  return( (pi!=problems.end()?(*pi).second:NULL) );
}

problem_t* problem_t::first_problem() {
  ProblemMap::const_iterator pi = problems.begin();
  return (pi != problems.end() ? (*pi).second : NULL);
}

  void
problem_t::clear( void )
{
  ProblemMap::const_iterator pi = begin();
  while( pi != end() )
  {
    problem_t::unregister_use( (*pi).second );
    pi = begin();
  }
  problems.clear();
}

  problem_t *
problem_t::allocate( const std::string &name, const Domain &domain )
{
  problem_t *p = find( name );
  if( p == NULL )
    p = new problem_t( name, domain );
  else
    problem_t::register_use( p );
  return( p );
}

  problem_t *
problem_t::allocate( const std::string &name, const problem_t &problem )
{
  problem_t *p = find( name );
  if( p == NULL )
    p = new problem_t( name, problem );
  else
    problem_t::register_use( p );
  return( p );
}

  problem_t::problem_t( const std::string &name, const problem_t &problem )
: ref_count_(0), name_(name), domain_(&problem.domain()),
  terms_(TermTable(problem.domain().terms())),
  goal_(&problem.goal()), metric_(problem.metric()),
  nprec_(false), goal_atom_(false)
{
  problem_t::register_use( this );
  StateFormula::register_use( goal_ );
  problems[name] = this;
  goalT_ = problem.goalT();
  terms_ = problem.terms_;
  instantiated_hash_ = problem.instantiated_hash_;

  // copy initial atoms, fluents, effects, and actions
  for( AtomSet::const_iterator ai = problem.init_atoms().begin(); ai != problem.init_atoms().end(); ++ai )
    add_init_atom( **ai );

  for( ValueMap::const_iterator vi = problem.init_fluents().begin(); vi != problem.init_fluents().end(); ++vi )
    add_init_fluent( *(*vi).first, (*vi).second );

  for( EffectList::const_iterator ei = problem.init_effects().begin(); ei != problem.init_effects().end(); ++ei )
    add_init_effect( **ei );

  for( ActionList::const_iterator ai = problem.actions().begin(); ai != problem.actions().end(); ++ai )
  {
    Action::register_use( *ai );
    actions_.push_back( *ai );
  }

  for( actionList_t::const_iterator ai = problem.actionsT().begin(); ai != problem.actionsT().end(); ++ai )
  {
    action_t::register_use( *ai );
    actionsT().push_back( *ai );
  }
}

  problem_t::problem_t( const std::string &name, const Domain &domain )
: ref_count_(0), name_(name), domain_(&domain), terms_(TermTable(domain.terms())),
  goal_(&StateFormula::FALSE), metric_(MINIMIZE_EXPECTED_COST),
  nprec_(false), goal_atom_(false)
{
  problem_t::register_use( this );
  StateFormula::register_use( goal_ );
  problems[name] = this;
}

problem_t::~problem_t()
{
  DIE(ref_count_ == 0, "Destructing Object that still has a ref.", 101);
  problems.erase( name() );

  StateFormula::unregister_use( goal_ );

  // no need to unregister these, since they were never registered
  init_atoms_static_.clear();

  for( AtomSet::const_iterator ai = init_atoms_.begin(); ai != init_atoms_.end(); ++ai )
    StateFormula::unregister_use( *ai );

  for( ValueMap::const_iterator vi = init_fluents_.begin(); vi != init_fluents_.end(); ++vi )
    Expression::unregister_use( (*vi).first );

  for( EffectList::const_iterator ei = init_effects_.begin(); ei != init_effects_.end(); ++ei )
    Effect::unregister_use( *ei );

  for( ActionList::const_iterator ai = actions().begin(); ai != actions().end(); ++ai )
    Action::unregister_use( *ai );

  for( actionList_t::const_iterator ai = actionsT().begin(); ai != actionsT().end(); ++ai )
    action_t::unregister_use( *ai );
}


#ifdef CACHE_ISGOAL_CALLS
bool problem_t::isGoal(state_t const& s) const {
  // TODO(fwt): Try to move this to the header, i.e., inline
  bool answer = false;
  START_TIMING("isGoal");
  int cached = s.isGoal();
  if (cached == 0) {
    answer = goal().holds(s);
    s.setGoalFlag(answer);
  }
  else {
    DIE(cached == -1 || cached == 1, "Case not predicted", -1);
    LOG_VAR_SINGLE_INC("CachedIsGoalCall", 1);
    answer = (cached > 0);
  }
#ifdef DOUBLECHECK_CACHE_ISGOAL_CALLS
  DIE(answer == goal().holds(s),
      "Cached answer and real answer differs for isGoal!", -1);
//  std::cout << "WRONG ANSWER! Correct = " << (real_answer ? "true" : "false")
//    << " returned = "  << (answer ? "true" : "false")
//    << " cache (int) = " << cached << std::endl;
#endif
  return answer;
}
#endif


  void
problem_t::add_init_atom( const Atom& atom )
{
  if( init_atoms_.find( &atom ) == init_atoms_.end() )
  {
    StateFormula::register_use( &atom );
    init_atoms_.insert( &atom );
    if (domain().predicates().static_predicate(atom.predicate())) {
      // don't bother registering again, we only need one
      init_atoms_static_.insert(&atom);
    }
  }
}

  void
problem_t::add_init_fluent( const Application& application, const Rational& value )
{
  if( init_fluents_.find( &application ) == init_fluents_.end() )
  {
    Expression::register_use( &application );
    init_fluents_.insert( std::make_pair( &application, value ) );
  }
  else
    init_fluents_[&application] = value;
}

  void
problem_t::add_init_effect( const Effect& effect )
{
  Effect::register_use( &effect );
  init_effects_.push_back( &effect );
}

  void
problem_t::set_goal( const StateFormula& goal )
{
  if( &goal != goal_ )
  {
    StateFormula::unregister_use( goal_ );
    goal_ = &goal;
    StateFormula::register_use( goal_ );
  }
}

  void
problem_t::instantiate_actions( void )
{
  original_goal_ = goal_;
  StateFormula::register_use(original_goal_);

  const StateFormula *ngoal = &goal().instantiation( SubstitutionMap(), *this );
  set_goal( *ngoal );
  StateFormula::unregister_use( ngoal );
  domain().instantiated_actions( actions_, instantiated_hash_, *this );

  // generate atoms
  std::map<const StateFormula*,const Atom*>::const_iterator hi;
  for( hi = instantiated_hash_.begin(); hi != instantiated_hash_.end(); ++hi )
  {
    (*hi).first->generate_atoms();
    (*hi).second->generate_atoms();
  }
}


void problem_t::flatten() {
  _D(DEBUG_ACTION_FLATTENING,
      std::cout << "[problem_t::flatten] Starting...\n";
  );

  FunctionTable const& ft = domain().functions();
  if (ft.size() == 0) {
    std::cout << "ERROR! This problem has no functions at all. It should have "
              << "at least a reward function [" << __FILE__ << ":" << __LINE__
              << "]. Quitting." << std::endl;
    exit(-1);
  }
  if (ft.name(0) != "reward") {
    std::cout << "ERROR! This problem has no 'reward' function or it is not the "
              << "function of index 0. [" << __FILE__ << ":" << __LINE__ << "]. "
              << "Quitting." << std::endl;
    exit(-1);
  }

  if (ft.size() > 1) {
    std::cout << "[problem_t::flatten] Extra functions in the problem: ";
    for (size_t i = 1; i < ft.size(); ++i) {
      std::cout << ft.name(i) << " (idx = " << i << "), ";
    }
    std::cout << std::endl;
  }


  // goal
  const StateFormula &g = goal().flatten();
  g.translate(goalT_);
  StateFormula::unregister_use(&g);

  Rational cost_lowerbound(1);
  Rational cost_upperbound(1);

  size_t t_simplified = 0;
  size_t t_ignored = 0;
  size_t t_actions = 0;
  size_t t_early_simplified = 0;

  // actions
  for (ActionList::const_iterator it = actions().begin();
        it != actions().end(); ++it)
  {
    t_actions++;
    _D(DEBUG_ACTION_FLATTENING, std::cout << "\n\n1.PLAIN:\n";
        print_full(std::cout, *(*it));
        std::cout << std::endl;
    );

    /*
     * An action has an implicit precondition when its top most effect is a
     * conditional effect. Therefore, if the conditional is met, then the
     * action has effects; otherwise the action has no effect (i.e., a
     * self-loop). The code bellow tries to detect it (without rearranging
     * the effect) and simplifies the action by moving the conditional to the
     * precondition and thus ignoring the self-loop effect (what should never
     * be chosen in an SSP setting). This can really speed up the search since
     * less actions will be applicable in several states (all states if the
     * original action does not have a precondition).
     */
#ifdef KEEP_IMPLICIT_PREC
    static bool __keep_implicit_prec_msg_shown = false;
    if (!__keep_implicit_prec_msg_shown) {
      std::cout << "\n\n[problem_t::flatten] "
                << "NOT simplifying implicit preconditions.\n" << std::endl;
      __keep_implicit_prec_msg_shown = true;
    }
#else
    ConditionalEffect const* c_eff =
                    dynamic_cast<ConditionalEffect const*>(&((*it)->effect()));
    if (c_eff) {
      Action const* a_const = *it;
      StateFormula const* new_prec = 0;
      if (a_const->precondition().tautology()) {
        // The action has no precondition, thus we can reuse the conditional
        // precondition
        new_prec = &(c_eff->condition());
      }
      else {
        // Build a conjunction between the precondition and the conditional
        Conjunction* conj =  new Conjunction;
        conj->add_conjunct(a_const->precondition());
        StateFormula::register_use(&(a_const->precondition()));
        conj->add_conjunct(c_eff->condition());
        StateFormula::register_use(&(c_eff->condition()));
        new_prec = conj;
      }
      Action* a_non_const = const_cast<Action*>(*it);

      StateFormula::register_use(new_prec);
      a_non_const->set_precondition(*new_prec);
      Effect::register_use(&c_eff->effect());
      a_non_const->set_effect(c_eff->effect());

      t_early_simplified++;

      _D(DEBUG_ACTION_FLATTENING, std::cout
              << "1a: SIMPLIFIED IMPLICIT PRECONDITION:" << std::endl;
            print_full(std::cout, *(*it));
            std::cout << std::endl;
      );
    }
#endif

    const Action *flat = &(*it)->flatten(*this);

    _D(DEBUG_ACTION_FLATTENING, std::cout << "2.FLAT:" << std::endl;
        print_full(std::cout, *flat);
        std::cout << std::endl;
    );

    action_t *action = &flat->translate(*this);
    if (action->empty()) {
      _D(DEBUG_ACTION_FLATTENING, std::cout << "2a.FLAT: empty action... "
                                     << "Ignoring it" << std::endl;
      );
      t_ignored++;
      //delete action
      continue;
    }

    bool simplified = action->simplify_confiditonal();
    if (simplified)
      t_simplified++;

    _D(DEBUG_ACTION_FLATTENING,
      if (simplified) {
        std::cout << "3.TRANSLATED AND SIMPLIFIED:" << std::endl;
      }
      else {
        std::cout << "3.TRANSLATED:" << std::endl;
      }
      action->print_full(std::cout);
      std::cout << std::endl;
    );

    actionsT().push_back(action);
    Action::unregister_use(flat);

    probabilisticAction_t* pact =
        dynamic_cast<probabilisticAction_t*>(action);
    if (pact) {
      bool consistent = pact->is_reward_consistent();
      _D(DEBUG_ACTION_FLATTENING, std::cout
                << "4.IS THE ACTION REWARD CONSISTENCY: "
                << (consistent ? "yes" : "NO!")
                << std::endl;
      );

      DIE(consistent, "Reward inconsistency in the probabilistic action", 163);
    } else {
      _D(DEBUG_ACTION_FLATTENING, std::cout << "4.CHECKING REWARD CONSISTENCY:"
                << " yes by definition (not prob action)"
                << std::endl;
      );
    }

    if (gpt::use_action_cost && gpt::normalize_action_cost) {
      Rational cur_cost_lowerbound = action->cost_lowerbound();
      Rational cur_cost_upperbound = action->cost_upperbound();

      _D(DEBUG_ACTION_FLATTENING, std::cout << "Action cost bound: ["
                << cur_cost_lowerbound << ", " << cur_cost_upperbound
                << "]" << std::endl;
      );

      if (it == actions().begin()) {
        cost_lowerbound = cur_cost_lowerbound;
        cost_upperbound = cur_cost_upperbound;
      } else {
        if (cur_cost_lowerbound < cost_lowerbound)
          cost_lowerbound = cur_cost_lowerbound;
        if (cur_cost_upperbound > cost_upperbound)
          cost_upperbound = cur_cost_upperbound;
      }
    }
  }

  if (gpt::randomize_actionsT_order) {
    std::cout << "[problem_t::flatten] Randomizing actionsT vector ordering"
              << std::endl;
    std::random_shuffle(actionsT().begin(), actionsT().end(), rand0toN_l);
  }

//  size_t iit = 0;
//  for (actionList_t::iterator it = actionsT().begin();
//       it != actionsT().end(); ++it, ++iit)
//  {
//    std::cout << iit << ": " << (*it)->name() << std::endl;
//  }

  if (gpt::use_action_cost && gpt::normalize_action_cost) {
    /*
     * If we are considering the cost of the actions and the decided to
     * normalize their costs, i.e. make all the actions cost be >= 0, then we
     * might have extra work: shift their cost
     *
     */
    _D(DEBUG_ACTION_FLATTENING, std::cout << "4.5. OVERALL COST ACTION BOUND: "
              << "[" << cost_lowerbound << ", "
              << cost_upperbound << "]"
              << std::endl;
    );

    if (cost_lowerbound < 1) {
      gpt::cost_shift = Rational(1) - cost_lowerbound;
      for (actionList_t::iterator it = actionsT().begin();
            it != actionsT().end(); ++it)
      {
        _D(DEBUG_ACTION_FLATTENING, std::cout
                  << "5. COST SHIFT: "  << (*it)->name()
                  << " cost lower bound: " << (*it)->cost_lowerbound()
                  << " -> " << ((*it)->cost_lowerbound() + gpt::cost_shift)
                  << std::endl;
        );
        const_cast<action_t*>((*it))->shift_cost_by(gpt::cost_shift);
        DIE((*it)->cost_lowerbound() > 0, "Non-positive cost of action", 164);
      }
    }
  }


  if (gpt::use_action_cost && gpt::use_state_cost) {
    _D(DEBUG_ACTION_FLATTENING, std::cout << "Goal reward: " << goal_reward();
        if (goal_reward() > 0)
          std::cout << " > 0 therefore all states will have a cost of "
                    << (-1 * goal_reward()) << " while the goal states "
                    << " will have cost 0" << std::endl;
        std::cout << std::endl;
    );
  }

  // builds up an action name -> action pointer map
  restring_actions();

  std::cout << "Action parsing/translating/flattening summary:" << std::endl
            << "  Total possible actions  : " << t_actions << std::endl
            << "  Total ignored (empty) a.: " << t_ignored << std::endl
            << "  Total considered actions: " << (t_actions - t_ignored) << std::endl
            << "  Total early simplified a: " << t_early_simplified << std::endl
            << "  Total simplified actions: " << t_simplified << std::endl
            << std::endl;

  _D(DEBUG_ACTION_FLATTENING, std::cout << "[problem_t::flatten] DONE"
                                        << std::endl;);
}

/*******************************************************************************
 *
 * Weak Relaxation:remove probabilistic operators
 *
 ******************************************************************************/
const problem_t&
problem_t::weak_relaxation( void ) const
{
  // create problem
  std::ostringstream ost;
  ost << name() << "-weak-relaxation";
  problem_t *result = problem_t::allocate( ost.str(), *this );

  // compute relaxation and return
  problem_t::compute_weak_relaxation( *result, true );
  return( *result );
}

void problem_t::compute_weak_relaxation(problem_t &problem, bool verb,
    bool apply_self_loop_correction)
{
  actionList_t::const_iterator it;
  deterministicAction_t *dact;
  const probabilisticAction_t *pact;

  if (gpt::verbosity >= 200)
    std::cout << "[relaxation]: weak-relaxation" << std::endl;

  if (problem.actionsT().empty())
    problem.flatten();

  /*
   * remove disjunctive goal
   */
  if (problem.goalT().size() > 1) {
    // create goal atom
    problem.goal_atom_ = true;
    ushort_t goal_atom = problem_t::atom_get_new();

    // create goal action
    dact = new deterministicAction_t("(reach-goal)",
        "<action><name>reach-goal</name></action>" );
    dact->insert_precondition(problem.goalT());
    dact->insert_add(goal_atom);
    problem.actionsT().push_back( dact );

    // clear and set new goal
    for (size_t i = 0; i < problem.goalT().size(); ++i)
      delete &problem.goalT().atom_list(i);

    problem.goalT().clear();

    atomList_t *glist = new atomList_t;
    glist->insert(goal_atom);
    problem.goalT().insert(glist);
  }

  /*
   * remove probabilistic actions
   */
  actionList_t tmp_list;
  for (it = problem.actionsT().begin(); it != problem.actionsT().end(); ++it) {
    pact = dynamic_cast<const probabilisticAction_t*>(*it);
    if (pact != NULL) {
      for (size_t i = 0; i < pact->size(); ++i) {
        if (!pact->effect(i).empty()) {
          if ((*it)->precondition().size() > 1) {
          // This action has disjunctive preconditions
            for (size_t j = 0; j < (*it)->precondition().size(); ++j) {
              std::ostringstream ost;
              ost << pact->name() << "-prob-" << 1+i << "-prec-" << 1+j;
              dact = new deterministicAction_t(ost.str(), pact->nameXML());
              dact->insert_precondition(pact->precondition().atom_list(j));
              dact->set_effect(pact->effect(i));
              DIE(dact->precondition().size() == 1,
                  "Unexpected size of precondition", 106);
              if (apply_self_loop_correction) {
                dact->set_self_loop_correction(pact->probability(i).inverse());
              }
              tmp_list.push_back(dact);
              if (verb && (gpt::verbosity >= 500)) {
                dact->print_full(std::cout);
                std::cout << std::endl;
              }
            }
          }
          else {
          // This action does not have disjunctive preconditions
            std::ostringstream ost;
            ost << pact->name() << "-prob-" << 1+i;
            dact = new deterministicAction_t(ost.str(), pact->nameXML());
            dact->insert_precondition(pact->precondition());
            dact->set_effect(pact->effect(i));
            DIE(dact->precondition().size() == 1,
                "Unexpected size of precondition", 106);
            if (apply_self_loop_correction) {
              dact->set_self_loop_correction(pact->probability(i).inverse());
            }
            tmp_list.push_back(dact);
            if (verb && (gpt::verbosity >= 500)) {
              dact->print_full( std::cout );
              std::cout << std::endl;
            }
          }
        }  // pact->effect(i) is not empty
      }  // for each effect of pact
    }  // action (*it) is probabilistic
    else if (!((const deterministicAction_t*)(*it))->effect().empty()) {
      if ((*it)->precondition().size() > 1) {
      // action (*it) has disjuntive preconditions
        for (size_t j = 0; j < (*it)->precondition().size(); ++j) {
          std::ostringstream ost;
          ost << (*it)->name() << "-prec-" << 1+j;
          dact = new deterministicAction_t(ost.str(), (*it)->nameXML());
          dact->insert_precondition((*it)->precondition().atom_list(j));
          dact->set_effect(((const deterministicAction_t*)(*it))->effect());
          DIE(dact->precondition().size() == 1,
              "Unexpected size of precondition", 106);
          tmp_list.push_back(dact);
          if(verb && (gpt::verbosity >= 500)) {
            dact->print_full(std::cout);
            std::cout << std::endl;
          }
        }
      }
      else {
      // (*it) is deterministic and does not have disjuntive preconditions, so
      // we just need to clone it
        dact = (deterministicAction_t*)(*it)->clone();
        DIE(dact->precondition().size() == 1,
            "Unexpected size of precondition", 106);
        tmp_list.push_back(dact);
        if (verb && (gpt::verbosity >= 500)) {
          dact->print_full(std::cout);
          std::cout << std::endl;
        }
      }
    }  // (*it) is a deterministic action

  }

  problem.actionsT().clear();
  problem.actionsT().insert(problem.actionsT().begin(), tmp_list.begin(),
      tmp_list.end());
  std::cout << "[weak relaxation] number of actions: "
            << problem.actionsT().size() << std::endl;
}

/*******************************************************************************
 *
 * Medium Relaxation: transform operators to STRIPS
 *
 ******************************************************************************/
const problem_t&
problem_t::medium_relaxation( void ) const
{
  // create problem
  std::ostringstream ost;
  ost << name() << "-medium-relaxation";
  problem_t *result = problem_t::allocate( ost.str(), *this );

  // compute relaxation and return
  problem_t::compute_medium_relaxation( *result, true );
  return( *result );
}

void problem_t::compute_medium_relaxation(problem_t& problem, bool verb,
    bool apply_self_loop_correction)
{
  deterministicAction_t *dact;
  actionList_t::const_iterator ai;
  std::list<const conditionalEffect_t*> tmp_set;
  std::list<const conditionalEffect_t*>::const_iterator ci;
  std::list<std::list<const conditionalEffect_t*>*> ssets;
  std::list<std::list<const conditionalEffect_t*>*>::const_iterator si;

  // start from a weak relaxation
  problem_t::compute_weak_relaxation(problem, false,
                                     apply_self_loop_correction);

  if (gpt::verbosity >= 200)
    std::cout << "[relaxation]: medium-relaxation" << std::endl;

  /*
   * remove conditional effects (might explode exponentially)
   */
  actionList_t tmp_list;
  for (ai = problem.actionsT().begin(); ai != problem.actionsT().end(); ++ai) {
    const deterministicAction_t *cdact = (const deterministicAction_t*)(*ai);
    if (cdact->effect().c_effect().size() == 0) {
      // No conditional effects
      dact = (deterministicAction_t*)cdact->clone();
      DIE(((const deterministicAction_t*)dact)->precondition().size() == 1,
          "Unexpected size of precondition", 106);
      tmp_list.push_back(dact);
    }
    else {
      // This action has conditional effects
      _D(DEBUG_MED_RELAX, std::cout
          << "===========================================" << std::endl
          << "The following action has cond effects and is being "
          << "processed now" << std::endl;
        cdact->print_full();
        std::cout << "\nThe cost of this action is: " << cdact->cost(ACTION_COST) << std::endl;
      )
      size_t i;
      tmp_set.clear();
      ssets.clear();
      // Removing the disjunctions from the conditions of conditional effects
      // by cloning them
      conditionalEffectList_t flat_list = removeDisjunctionsFromConditions(
          cdact->effect().c_effect());
//      problem_t::subsets(0, cdact->effect().c_effect(), tmp_set, ssets);
      problem_t::subsets(0, flat_list, tmp_set, ssets);
//      size_t total_added = 0;
      for (i = 0, si = ssets.begin(); si != ssets.end(); ++si, ++i) {
        std::ostringstream ost;
        ost << cdact->name() << "-c-" << 1+i;
        dact = new deterministicAction_t(ost.str(), cdact->nameXML());
        dact->insert_precondition(cdact->precondition());
        dact->insert_effect(cdact->effect().s_effect());
        dact->set_self_loop_correction(cdact->self_loop_correction());
        // process subset of conditional effects
        for (ci = (*si)->begin(); ci != (*si)->end(); ++ci) {
          dact->insert_effect((*ci)->s_effect());
          DIE((*ci)->precondition().size() == 1,
              "disjunctive preconditions in conditional effects not supported",
              145);
          for (size_t j = 0; j < (*ci)->precondition().atom_list(0).size(); ++j)
          {
            dact->precondition().atom_list(0).insert(
                                    (*ci)->precondition().atom_list(0).atom(j));
          }
        }
        // This should not be necessary anymore because insert_effect now copies
        // the "increases" and "decreases" over all cost functions (rewards
        // included)/
        //  dact->effect().set_cost(cdact->effect().cost());

        DIE(((const deterministicAction_t*)dact)->precondition().size() == 1,
            "Unexpected size of precondition", 106);
        if (!dact->precondition().atom_list(0).contradiction()
            && !dact->effect().empty())
        {
          _D(DEBUG_MED_RELAX, std::cout << "------------------" << std::endl
                << "Adding the following non-conditional action" << std::endl;
            dact->print_full();
            std::cout << std::endl;
          )
          tmp_list.push_back(dact);
//          total_added++;
        }
        else {
#if 0
          std::cout << "removing action " << dact->name();
          if( dact->precondition().atom_list( 0 ).contradiction() )
          {
            std::cout << " since false precondition = ";
            dact->precondition().print( std::cout );
          }
          if( dact->effect().empty() )
          {
            std::cout << " since empty effect";
          }
          std::cout << std::endl;
#endif
          action_t::unregister_use(dact);
        }
        delete (*si);
      }  // for si
//      std::cout << "[medium relax]: " << cdact->name() << " -> "
//                << total_added << " actions\n";

      _D(DEBUG_MED_RELAX, std::cout
          << "===========================================" << std::endl;
      )
    }  // case in which the action *ai has conditional effects
    action_t::unregister_use(*ai);
  }  // for each action *ai

  /*
   * check if there is a negative precondition
   */
  for (ai = tmp_list.begin(); !problem.nprec_ && (ai != tmp_list.end()); ++ai)
  {
    for (size_t i = 0; i < (*ai)->precondition().atom_list( 0 ).size(); ++i) {
      if ((*ai)->precondition().atom_list(0).atom(i) % 2) {
        problem.nprec_ = true;
        break;
      }
    }
  }

  if (!problem.nprec_) {
//    std::cout << std::endl << "Checking if goal has negative atoms: goal = ";
//    problem.goal().print(std::cout, problem.domain().predicates(),
//                         problem.domain().functions(), problem.terms());
    // Checking if the goal has negative atoms
    for (size_t id = 0; !problem.nprec_ && id < problem.goalT().size(); ++id) {
      atomList_t const& goal_conj_id = problem.goalT().atom_list(id);
      for (size_t ic = 0; ic < goal_conj_id.size(); ++ic) {
        if (goal_conj_id.atom(ic) % 2) {
          problem.nprec_ = true;
//          std::cout << " It HAS negative atoms!";
          break;
        }
      }
    }
//    std::cout << std::endl;
  }

  /*
   * complete operators with negative atoms and insert into result
   */
  problem.actionsT().clear();
  for (ai = tmp_list.begin(); ai != tmp_list.end(); ++ai) {
    if (problem.nprec_) {
      dact = (deterministicAction_t*)(*ai);
      for (ushort_t i = 0; i < number_atoms(); i += 2) {
        if (dact->effect().s_effect().add_list().find(i)) {
          dact->insert_del(i % 2 ? i-1 : i+1);
        }
        if (dact->effect().s_effect().del_list().find(i)) {
          dact->insert_add(i % 2 ? i-1 : i+1);
        }
      }
    }
    problem.actionsT().push_back(*ai);
    if (verb && (gpt::verbosity >= 500)) {
      (*ai)->print_full(std::cout);
      std::cout << std::endl;
    }
  }
  std::cout << "[medium relaxation] number of actions: "
            << problem.actionsT().size() << std::endl;
}

conditionalEffectList_t removeDisjunctionsFromConditions(
    conditionalEffectList_t const& ceff_set)
{
  conditionalEffectList_t rv;
  for (size_t ie = 0; ie < ceff_set.size(); ++ie) {
    if (ceff_set.effect(ie).precondition().size() == 1) {
      rv.insert(&ceff_set.effect(ie));
    }
    else {
      for (size_t ic = 0; ic < ceff_set.effect(ie).precondition().size(); ++ic)
      {
        conditionalEffect_t* new_effect = new conditionalEffect_t;
        (*new_effect) = ceff_set.effect(ie);
        atomList_t& selected_prec = new_effect->precondition().atom_list(ic);
        new_effect->precondition().clear();
        new_effect->precondition().insert(&selected_prec);
        rv.insert(new_effect);
//        std::cout << "Original conditionalEffect" << std::endl;
//        ceff_set.effect(ie).print(std::cout);
//        std::cout << "\nNew " << ic << "-th conditionalEffect" << std::endl;
//        new_effect->print(std::cout);
//        std::cout << std::endl;
      }
    }
  }
  return rv;
}

  void
problem_t::subsets( size_t i, const conditionalEffectList_t &ceff_set,
    std::list<const conditionalEffect_t*> &tmp_set,
    std::list<std::list<const conditionalEffect_t*>*> &result )
{
  if( i == ceff_set.size() )
  {
    std::list<const conditionalEffect_t*> *set = new std::list<const conditionalEffect_t*>;
    set->insert( set->begin(), tmp_set.begin(), tmp_set.end() );
    result.push_back( set );
  }
  else
  {
    tmp_set.push_back( &ceff_set.effect( i ) );
    problem_t::subsets( i+1, ceff_set, tmp_set, result );
    tmp_set.pop_back();
    problem_t::subsets( i+1, ceff_set, tmp_set, result );
  }
}

/*******************************************************************************
 *
 * Strong Relaxation: transform operators to STRIPS and remove delete lists
 *
 ******************************************************************************/
problem_t const& problem_t::strong_relaxation(
    bool apply_self_loop_correction) const
{
  // create problem
  std::ostringstream ost;
  ost << name() << "-strong-relaxation";
  if (apply_self_loop_correction)
    ost << "-self-loop";
  problem_t *result = problem_t::allocate(ost.str(), *this);

  // compute relaxation and return
  problem_t::compute_strong_relaxation(*result, true,
                                                  apply_self_loop_correction);
  return( *result );
}

void problem_t::compute_strong_relaxation(problem_t &problem, bool verb,
    bool apply_self_loop_correction)
{
  actionList_t::const_iterator ai;

  // start from a medium relaxation
  problem_t::compute_medium_relaxation(problem, false,
      apply_self_loop_correction);

  if (gpt::verbosity >= 200)
    std::cout << "[relaxation]: strong-relaxation"
      << (apply_self_loop_correction ? " self-loop" : "" ) << std::endl;

  /*
   * remove delete lists
   */
  actionList_t tmp_list;
  for (ai = problem.actionsT().begin(); ai != problem.actionsT().end(); ++ai) {
    deterministicAction_t *dact = (deterministicAction_t*)(*ai);
    dact->effect().s_effect().del_list().clear();
    if (!dact->effect().empty()) {
      tmp_list.push_back(*ai);
      if (verb && (gpt::verbosity >= 500)) {
        dact->print_full(std::cout);
        std::cout << std::endl;
      }
    }
    else {
      action_t::unregister_use(*ai);
    }
  }

  problem.actionsT().clear();
  problem.actionsT().insert(problem.actionsT().begin(), tmp_list.begin(),
                            tmp_list.end());
  std::cout << "[strong relaxation] number of actions: "
            << problem.actionsT().size() << std::endl;
}

// Fills the provided object list with objects (including constants
// declared in the domain) that are compatible with the given type.
void
problem_t::compatible_objects( ObjectList& objects, Type type ) const
{
  domain().compatible_constants( objects, type );
  Object last = terms().last_object();
  for( Object i = terms().first_object(); i <= last; ++i )
    if( domain().types().subtype( terms().type( i ), type ) )
      objects.push_back( i );
}

void
problem_t::complete_state( state_t &state ) const
{
  // insert negative atoms
  if( nprec() )
  {
    for( ushort_t atom = 0; atom < problem_t::number_atoms(); atom += 2 )
      if( !state.holds( atom ) )
        state.add( atom + 1 );
  }

  // insert internal predicates
  std::map<const StateFormula*,const Atom*>::const_iterator hi;
  for( hi = instantiated_hash_.begin(); hi != instantiated_hash_.end(); ++hi )
  {
    ushort_t atom = problem_t::atom_hash_get( *(*hi).second );
    if( (*hi).first->holds( state ) )
    {
      state.clear( 1+atom );
      state.add( atom );
    }
    else
    {
      state.clear( atom );
      state.add( 1+atom );
    }
  }
}

void
problem_t::enabled_actions( ActionList& actions, const state_t& state ) const
{
  NOT_IMPLEMENTED;
//  for( ActionList::const_iterator ai = actions_.begin(); ai != actions_.end(); ++ai )
//    if( (*ai)->enabled( state ) ) actions.push_back( *ai );
}

bool problem_t::has_enabled_actions(const state_t& state ) const {
  NOT_IMPLEMENTED;
//  for (ActionList::const_iterator ai = actions_.begin();
//      ai != actions_.end(); ++ai)
//    if ((*ai)->enabled(state)) return true;
  return false;
}



void
problem_t::print( std::ostream& os, const StateFormula &formula ) const
{
  formula.print( os, domain_->predicates(), domain_->functions(), terms() );
}

void
problem_t::print( std::ostream& os, const Application &app ) const
{
  app.print( os, domain_->functions(), terms() );
}

void
problem_t::print( std::ostream& os, const Action &action ) const
{
  action.print( os, terms() );
}

void
problem_t::print_full( std::ostream& os, const Action &action ) const
{
  action.print_full( os, domain_->predicates(), domain_->functions(), terms() );
}


void problem_t::expand(action_t const& a, state_t const& s,
    ProbDistStateIface& pr) const
{

  static ProbDistState non_completed_state_pr;
  non_completed_state_pr.clear();
  a.expand(s, non_completed_state_pr);  // FWT: maybe pass the problem nprec?

  // DO NOT REMOVE this call of pr.clear() since problem_t::expand guarantees
  // that pr will be cleared before being populated.
  pr.clear();
  state_t tmp_s;
  for (auto const& it : non_completed_state_pr) {
    tmp_s = it.event();
    complete_state(tmp_s);
    pr.insert(tmp_s, it.prob());
  }
}


state_t const problem_t::get_initial_state() const {
  state_t s0;
  for (AtomSet::const_iterator ai = init_atoms().begin();
          ai != init_atoms().end(); ++ai)
    if (!domain().predicates().static_predicate((*ai)->predicate()))
      s0.add(**ai);

  complete_state(s0);
  s0.make_digest();

  if (init_effects().size() > 1) {
    // FWT: DESIGN DECISION: We assume that the initial state is unique. This
    // is not a big deal since we can convert an SSP with an initial state
    // probability distribution to an equivalent SSP with only on initial state
    // (and an extra initialize action with the Pr_s0 transition).
    std::cout << "\n\n\nERROR! This problem has more than 1 initial effect. "
      << "This case was not predicted and/or removed from the code. See "
      << __FILE__ << ":" << __LINE__ << "for more details" << std::endl
      << "The effect in question is: ";
    for (size_t i = 0; i != init_effects().size(); ++i) {
      std::cout << "\t" << i << ": ";
      init_effects()[i]->print(std::cout, domain().predicates(),
                               domain().functions(), terms());
      std::cout << std::endl;
    }
    std::cout << std::endl << "Exiting..." << std::endl;
    exit(174);
  }
  else if (init_effects().size() == 1) {
    // This could be just the initialization of the reward, in which case there
    // is nothing to be expanded here. See AssignmentEffect::expand
    Effect const* init_eff = init_effects()[0];
    AssignmentEffect const* initialize_reward =
                               dynamic_cast<AssignmentEffect const*>(init_eff);
    if (! initialize_reward
        || ! initialize_reward->assignment().is_a_reward_reassigment())
    {
      // This is NOT a reward initialization, therefore it breaks the assumption
      // that initial states are unique
      std::cout << "\n\n\nERROR! This problem has 1 initial effect and it is "
        << "NOT a reward initialization effect. This case was not predicted "
        << "and/or removed from the code. See " << __FILE__ << ":"
        << __LINE__ << "for more details" << std::endl
        << "The effect in question is: ";
      init_eff->print(std::cout, domain().predicates(),
                         domain().functions(),
                         terms());
      std::cout << std::endl << "Exiting..." << std::endl;
      exit(173);
    }
  }
  //  else init_effects().size() == 0 and nothing needs to be done.
  return s0;
}

state_t const problem_t::get_intermediate_state(const std::string &props) {
  // copied from state_t::state_t(string, bool)
  // didn't bother using above method because I want to handle statics right,
  // which requires access to a problem
  state_t rv;
  std::deque<std::string> active_atoms = splitString(props, ",");
  TermList term_parameters;
  Atom::AtomTable statics;
  for (std::string& id : active_atoms) {
    trim_string(id);
    if (id.length() == 0) { continue; }

    std::deque<std::string> parts = splitString(id, " ");
    std::pair<Predicate, bool> p =
      domain().predicates().find_predicate(parts.front());
    assert(p.second);
    Predicate atom_predicate = p.first;
    // Parsing terms
    term_parameters.clear();
    parts.pop_front();
    for (std::string const& pi : parts) {
      std::pair<Object, bool> o = terms().find_object(pi);
      FANCY_DIE_IF(!o.second, 180,
                   "couldn't get term pi='%s', where props='%s' (length=%i)",
                   pi.c_str(), props.c_str(), props.length());
      // TODO(fwt): add type verification
      term_parameters.push_back(o.first);
    }
    size_t n = term_parameters.size();
    assert(domain().predicates().arity(atom_predicate) == n);
    // yes, make_atom returns a bloody reference to a newly allocated object
    const Atom *new_atom = &Atom::make_atom(atom_predicate, term_parameters);
    if (domain().predicates().static_predicate(atom_predicate)) {
      // XXX: this doesn't actually work because statics are not in the table
      // for some reason (?)
      statics.insert(new_atom);
    } else {
      // this doesn't actually hold on to the atom, so we can delete atom
      // straight after
      rv.add(*new_atom);
      StateFormula::unregister_use(new_atom);
      new_atom = nullptr;
    }
  }

  // check that statics make sense
  AtomList diff_statics;
  std::set_symmetric_difference(
    statics.begin(), statics.end(),
    init_atoms_static_.begin(), init_atoms_static_.end(),
    std::back_inserter(diff_statics), Atom::AtomLess());
  if (!diff_statics.empty()) {
    // uh, oops
    std::cerr << "[error] " << diff_statics.size()
              << " elements vary between passed state and original: ";
    for (const auto it : diff_statics) {
      gpt::problem->print(std::cerr, *it);
      std::cerr << " (" << &(*it) << ") ";
      std::cerr << " ";
    }
    std::cerr << std::endl;

    std::cerr << "[error] original statics (" << init_atoms_static_.size() << " of them): ";
    for (const auto it : init_atoms_static_) {
      gpt::problem->print(std::cerr, *it);
      std::cerr << " (" << &(*it) << ") ";
    }
    std::cerr << std::endl;

    std::cerr << "[error] passed statics (" << statics.size() << " of them): ";
    for (const auto it : statics) {
      gpt::problem->print(std::cerr, *it);
      std::cerr << " (" << &(*it) << ") ";
    }
    std::cerr << std::endl;

    // just do this to get line/func info
    FANCY_DIE_IF(true, 181, "got deaded");
  }

  // get rid of atoms in statics (leaks otherwise)
  for (const auto atom_p : statics) {
    StateFormula::unregister_use(atom_p);
  }
  statics.clear();

  // stuff we always have to do
  complete_state(rv);
  rv.make_digest();
  return rv;
}

void problem_t::restring_actions() {
  stringy_actions_.clear();
  for (auto action : actionsT()) {
    std::string name = action->name();
    auto pair = std::make_pair(std::string(name), action);
    stringy_actions_.emplace(pair);
  }
}

const action_t *problem_t::find_action(const std::string &name) const {
  auto it = stringy_actions_.find(name);
  if (it == stringy_actions_.end()) {
    return nullptr;
  } else {
    return it->second;
  }
}

  std::ostream&
operator<<( std::ostream& os, const problem_t& p )
{
  os << "name: " << p.name();
  os << std::endl << "domain: " << p.domain().name();
  os << std::endl << "objects:";
  for( Object i = p.terms().first_object(); i <= p.terms().last_object(); ++i )
  {
    os << std::endl << "  ";
    p.terms().print_term( os, i );
    os << " - ";
    p.domain().types().print_type( os, p.terms().type( i ) );
  }
  os << std::endl << "init:";
  for( AtomSet::const_iterator ai = p.init_atoms_.begin(); ai != p.init_atoms_.end(); ++ai )
  {
    os << std::endl << "  ";
    (*ai)->print( os, p.domain().predicates(), p.domain().functions(), p.terms() );
  }
  for( ValueMap::const_iterator vi = p.init_fluents_.begin(); vi != p.init_fluents_.end(); ++vi )
  {
    os << std::endl << "  (= ";
    (*vi).first->print( os, p.domain().functions(), p.terms() );
    os << ' ' << (*vi).second << ")";
  }
  for( EffectList::const_iterator ei = p.init_effects_.begin(); ei != p.init_effects_.end(); ++ei )
  {
    os << std::endl << "  ";
    (*ei)->print( os, p.domain().predicates(), p.domain().functions(), p.terms() );
  }
  os << std::endl << "goal: ";
  p.goal().print( os, p.domain().predicates(), p.domain().functions(), p.terms() );
  os << std::endl << "metric: ";
  if( p.metric() == problem_t::MINIMIZE_EXPECTED_COST )
    os << "minimize-expected-cost";
  os << std::endl << "actions:";
  for( ActionList::const_iterator ai = p.actions_.begin(); ai != p.actions_.end(); ++ai )
  {
    os << std::endl << "  ";
    (*ai)->print( os, p.terms() );
  }
  return( os );
}
