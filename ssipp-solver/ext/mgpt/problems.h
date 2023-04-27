#ifndef PROBLEMS_T_H
#define PROBLEMS_T_H

#include "global.h"
#include "actions.h"
#include "domains.h"
#include "effects.h"
#include "formulas.h"
#include "expressions.h"
#include "../../ssps/prob_dist_state.h"
#include "terms.h"
#include "types.h"

#include "../../ssps/ssp_iface.h"
#include "../../ssps/ssp_utils.h"  // for reachableStatesFrom

#include <algorithm>
#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <string>
#include <vector>

class Application;
class Atom;
class Domain;
class Expression;
class StateFormula;

class state_t;


#ifndef DEBUG_MED_RELAX
#define DEBUG_MED_RELAX 0
#endif

#ifndef DEBUG_ACTION_FLATTENING
#define DEBUG_ACTION_FLATTENING 0
#endif


/*******************************************************************************
 *
 * problem_t
 *
 ******************************************************************************/

class problem_t {
  public:
    typedef enum { MINIMIZE_EXPECTED_COST, MAXIMIZE_GOAL_PROBABILITY } metric_t;
    actionList_t actionsT_;

    static void clean_all_the_static_data() {
      problems.clear();
      fluent_hash_.clear();
      fluent_inv_hash_.clear();
      atom_hash_.clear();
      atom_inv_hash_.clear();

      atom_index_ = 0;
      no_more_atoms_ = false;
      fluent_index_ = 0;
    }
  private:
    mutable size_t ref_count_;

    class ProblemMap : public std::map<std::string,problem_t*> { };

    static ProblemMap problems;
    static ProblemMap::const_iterator begin( void ) { return( problems.begin() ); }
    static ProblemMap::const_iterator end( void ) { return( problems.end() ); }

    std::map<std::string, const action_t*> stringy_actions_; // delicious
    void restring_actions();

    static ushort_t atom_index_;
    static bool no_more_atoms_;
    static std::map<const Atom*,ushort_t> atom_hash_;
    static std::map<ushort_t,const Atom*> atom_inv_hash_;

    static ushort_t fluent_index_;
    static std::map<const Application*,ushort_t> fluent_hash_;
    static std::map<ushort_t,const Application*> fluent_inv_hash_;

    std::string name_;
    const Domain* domain_;
    TermTable terms_;
    // AtomSet is unordered (and probably faster), AtomTable is ordered (and
    // good for taking set differences)
    AtomSet init_atoms_;
    Atom::AtomTable init_atoms_static_;
    ValueMap init_fluents_;
    EffectList init_effects_;
    const StateFormula* goal_;
    const StateFormula* original_goal_;
    Rational goal_reward_;
    metric_t metric_;
    ActionList actions_;

    bool nprec_;
    bool goal_atom_;
    atomListList_t goalT_;
    std::map<const StateFormula*,const Atom*> instantiated_hash_;

    static problem_t* allocate( const std::string &name, const problem_t &problem );

    static void compute_weak_relaxation(problem_t &problem, bool verb,
        bool apply_self_loop_correction = false);
    static void compute_medium_relaxation(problem_t &problem, bool verb,
        bool apply_self_loop_correction = false);
    static void compute_strong_relaxation(problem_t &problem, bool verb,
        bool apply_self_loop_correction = false);
    static void subsets( size_t i, const conditionalEffectList_t &ceff_set,
        std::list<const conditionalEffect_t*> &tmp_set,
        std::list<std::list<const conditionalEffect_t*>*> &result );

    explicit problem_t( const std::string &name, const problem_t &problem );

    friend std::ostream& operator<<( std::ostream& os, const problem_t& p );

  public:
    problem_t( const std::string &name, const Domain &domain );
    ~problem_t();

    static void register_use( const problem_t *p )
    {
#ifdef MEM_DEBUG
      if(p) std::cerr << "[pro]: inc-ref-count " << p << " = " << p->ref_count_+1 << std::endl;
#endif
      if( p ) ++p->ref_count_;
    }
    static void unregister_use( const problem_t *p )
    {
#ifdef MEM_DEBUG
      if(p) std::cerr << "[pro]: dec-ref-count " << p << " = " << p->ref_count_-1 << std::endl;
#endif
      if( p && (--p->ref_count_ == 0) ) delete p;
    }

    static problem_t* allocate( const std::string &name, const Domain &domain );
    static problem_t* find( const std::string &name );
    static problem_t* first_problem();
    static void clear( void );
    static ushort_t number_atoms( void ) { return( atom_index_ ); }
    static ushort_t number_fluents( void ) { return( fluent_index_ ); }
    static ushort_t atom_hash_get( const Atom &atom, bool negated = false );
    static ushort_t atom_get_new( void )
    {
      ushort_t atm = atom_index_;
      atom_index_ += 2;
      return( atm );
    }
    static void no_more_atoms( void ) { no_more_atoms_ = true; }
    static ushort_t fluent_hash_get( const Application &app );
    static const Atom* atom_inv_hash_get( ushort_t atom );
    static const Application* fluent_inv_hash_get( ushort_t fluent );

    const Domain& domain( void ) const { return( *domain_ ); }
    TermTable& terms( void ) { return( terms_ ); }
    const TermTable& terms( void ) const { return( terms_ ); }

    void add_init_atom( const Atom& atom );
    void add_init_fluent( const Application &application, const Rational &value );
    void add_init_effect( const Effect &effect );
    void set_goal( const StateFormula &goal );
    void set_goal_reward(Rational const r) { goal_reward_ = r; }
    void set_metric( metric_t metric ) { metric_ = metric; }

    void instantiate_actions( void );
    void flatten( void );

    const action_t *find_action(const std::string &name) const;

   public:
    const problem_t& weak_relaxation( void ) const;
    const problem_t& medium_relaxation( void ) const;
    const problem_t& strong_relaxation(
        bool apply_self_loop_correction = false) const;
    const problem_t& self_loop_relaxation() const {
        return strong_relaxation(true);
    }

    // Fills the provided object list with objects (including constants
    // declared in the domain) that are compatible with the given type.
    void compatible_objects( ObjectList &objects, Type type ) const;

    const AtomSet& init_atoms() const { return( init_atoms_ ); }
    const Atom::AtomTable& init_atoms_static() const {return init_atoms_static_;}
    const ValueMap& init_fluents() const { return( init_fluents_ ); }
    const EffectList& init_effects() const { return( init_effects_ ); }
    const StateFormula& goal() const { return( *goal_ ); }
    const StateFormula& original_goal() const { return( *original_goal_ ); }

    bool isDeadend(state_t const& s) const {
      for (actionList_t::const_iterator ai = actionsT().begin();
           ai != actionsT().end(); ++ai)
        { if ((*ai)->enabled(s)) { return false; } }
      return true;
    }
    Rational goal_reward() const { return goal_reward_; }

    const metric_t metric( void ) const { return( metric_ ); }
    const ActionList& actions( void ) const { return( actions_ ); }

    bool nprec( void ) const { return( nprec_ ); }
    bool goal_atom( void ) const { return( goal_atom_ ); }
    const atomListList_t& goalT( void ) const { return( goalT_ ); }
    atomListList_t& goalT( void ) { return( goalT_ ); }
    const actionList_t& actionsT( void ) const { return( actionsT_ ); }
    actionList_t& actionsT( void ) { return( actionsT_ ); }
    void complete_state( state_t &state ) const;

    DEPRECATED void enabled_actions( ActionList& actions, const state_t& state ) const;
    DEPRECATED bool has_enabled_actions(const state_t& state ) const;

    void print( std::ostream &os, const StateFormula &formula ) const;
    void print( std::ostream &os, const Application &app ) const;
    void print( std::ostream &os, const Action &action ) const;
    void print_full( std::ostream &os, const Action &action ) const;

    state_t const get_initial_state() const;
    // Same as get_initial_state() but it caches the initial state and then
    // returns the cached version.
    state_t const& s0() const;
    // constructs an intermediate state from a spec like "predA objX objY, predB
    // objZ, ..."
    state_t const get_intermediate_state(const std::string &props);

#ifdef CACHE_ISGOAL_CALLS
    bool isGoal(state_t const& s) const;
#else
    bool isGoal(state_t const& s) const { return goal().holds(s); }
#endif
    std::string const& name() const { return name_; }

    void expand(action_t const& a, state_t const& s, ProbDistStateIface& pr) const;

    Rational terminalCost(state_t const& s) const {
      assert(isGoal(s));
      Rational c(0);
      if (gpt::use_action_cost && gpt::use_state_cost) {
        c = goal_reward();
      }
      assert(c >= Rational(0));
      return c;
    }
};

std::ostream& operator<<( std::ostream& os, const problem_t& p );


conditionalEffectList_t removeDisjunctionsFromConditions(
    conditionalEffectList_t const& ceff_set);
#endif // PROBLEMS_T_H
