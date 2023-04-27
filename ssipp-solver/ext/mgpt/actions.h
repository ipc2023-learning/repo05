#ifndef ACTIONS_H
#define ACTIONS_H

#include "global.h"
#include "effects.h"
#include "formulas.h"
#include "../../ssps/prob_dist_state.h"
#include "rational.h"
#include "terms.h"
#include "../../utils/utils.h"
#include "atom_list.h"

#include <assert.h>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <unordered_map>

#ifndef DEBUG_SELFLOOP
#define DEBUG_SELFLOOP 0
#endif

#ifndef DEBUG_DIST_ATOM
#define DEBUG_DIST_ATOM 0
#endif

class PredicateTable;
class FunctionTable;
class ValueMap;
class StateFormula;
class AtomSet;
class Effect;

class action_t;
class state_t;
class problem_t;
class conditionalEffect_t;
class probabilisticEffect_t;

class Action;
class ActionList;

typedef std::vector<const action_t*> actionList_t;


typedef std::unordered_map<atom_t, Rational> HashAtomtToRational;


/*******************************************************************************
 *
 * action schema
 *
 ******************************************************************************/

class ActionSchema
{
  std::string name_;
  VariableList parameters_;
  const StateFormula* precondition_;
  const Effect* effect_;

  public:
  ActionSchema( const std::string& name );
  ~ActionSchema();

  void add_parameter( Variable parameter ) { parameters_.push_back( parameter ); }
  void set_precondition( const StateFormula& precondition );
  void set_effect( const Effect& effect );
  const std::string& name( void ) const { return( name_ ); }
  size_t arity( void ) const { return( parameters_.size() ); }
  Variable parameter( size_t i ) const { return( parameters_[i] ); }
  const StateFormula& precondition( void ) const { return( *precondition_ ); }
  const Effect& effect( void ) const { return( *effect_ ); }
  void instantiations( ActionList& actions, const problem_t& problem ) const;
  const Action& instantiation( const SubstitutionMap& subst,
      const problem_t& problem,
      const StateFormula& precond ) const;
  void print( std::ostream& os, const PredicateTable& predicates,
      const FunctionTable& functions, const TermTable& terms ) const;

  void analyze( PredicateTable &predicates, TermTable &terms,
      std::map<const StateFormula*,const Atom*> &hash ) const;
  const ActionSchema& rewrite( std::map<const StateFormula*,const Atom*> &hash ) const;
};

class ActionSchemaMap : public std::map<std::string, const ActionSchema*> { };


/*******************************************************************************
 *
 * action
 *
 ******************************************************************************/

class Action
{
  mutable size_t ref_count_;
  std::string name_;
  ObjectList arguments_;
  const StateFormula *precondition_;
  const Effect* effect_;

  Action() : ref_count_(0) { }

  public:
  Action( const std::string& name );
  ~Action();

  static void register_use( const Action* a )
  {
#ifdef MEM_DEBUG
    if(a) std::cerr << "[ACT]: inc-ref-count " << a << " = " << a->ref_count_+1 << std::endl;
#endif
    if( a != NULL ) ++a->ref_count_;
  }
  static void unregister_use( const Action* a )
  {
#ifdef MEM_DEBUG
    if(a) std::cerr << "[ACT]: dec-ref-count " << a << " = " << a->ref_count_-1 << std::endl;
#endif
    if( a && (--a->ref_count_ == 0) ) delete a;
  }

  void add_argument( Object argument ) { arguments_.push_back( argument ); }
  void set_precondition( const StateFormula& precondition );
  void set_effect( const Effect& effect );
  const std::string& name( void ) const { return( name_ ); }
  size_t arity( void ) const { return( arguments_.size() ); }
  Object argument( size_t i ) const { return( arguments_[i] ); }
  const StateFormula& precondition( void ) const { return( *precondition_ ); }
  const Effect& effect( void ) const { return( *effect_ ); }

  const Action& flatten( const problem_t &problem ) const;
  action_t& translate( const problem_t &problem ) const;

  DEPRECATED bool enabled( const state_t& state ) const;
  DEPRECATED void affect( state_t& state ) const;
  Rational cost(const state_t& state) const { return Rational(1); }

  void print_full( std::ostream& os, const PredicateTable& predicates,
      const FunctionTable& functions,
      const TermTable& terms ) const;
  void print( std::ostream& os, const TermTable& terms ) const;
  void printXML( std::ostream& os, const TermTable& terms ) const;
};

class ActionList : public std::vector<const Action*> { };


/*******************************************************************************
 *
 * conditional effect list
 *
 ******************************************************************************/

class conditionalEffectList_t
{
  size_t size_;
  const conditionalEffect_t **data_, **data_ptr_;

  public:
  conditionalEffectList_t() : size_(0), data_(0), data_ptr_(0) { }
  ~conditionalEffectList_t() { if( data_ ) free( data_ ); }

  Rational cost(state_t const& s, size_t cost_idx = ACTION_COST) const;
  Rational cost_lowerbound() const;
  Rational cost_upperbound() const;

  size_t size( void ) const { return( data_ptr_ - data_ ); }
  const conditionalEffect_t& effect( size_t i ) const { return( *data_[i] ); }
  bool find(const conditionalEffect_t &eff) const;
  void insert( const conditionalEffect_t *effect );
  bool affect(state_t const& state, state_t& s_prime, bool nprec = false ) const;
  void clear() { data_ptr_ = data_; }
  void print( std::ostream &os ) const;
  bool operator==( const conditionalEffectList_t &clist ) const;
  conditionalEffectList_t& operator=( const conditionalEffectList_t &effect );
  bool is_reward_consistent_with(conditionalEffectList_t const& clist) const;
};


/*******************************************************************************
 *
 * probabilistic effect list
 *
 ******************************************************************************/

class probabilisticEffectList_t
{
  size_t size_;
  const probabilisticEffect_t **data_, **data_ptr_;

  public:
  probabilisticEffectList_t() : size_(0), data_(0), data_ptr_(0) { }
  ~probabilisticEffectList_t() { if(data_) free(data_); }

  // Return the cost of the action IF it is the same cost independently of the
  // probabilistic outcome and state in which the action is being applied.
  // Otherwise, dies with and assertion
  Rational cost(size_t cost_idx = ACTION_COST) const;
  Rational cost(state_t const& s, size_t cost_idx = ACTION_COST) const;

  Rational cost_lowerbound() const;
  Rational cost_upperbound() const;
  void shift_cost_by(Rational c);

  size_t size() const { return(data_ptr_ - data_); }
  const probabilisticEffect_t& effect(size_t i) const { return(*data_[i]); }
  bool find(const probabilisticEffect_t &eff) const;
  bool insert(probabilisticEffect_t const* effect);
  bool affect(state_t const& state, state_t& s_prime, bool nprec = false) const;
  void clear() { data_ptr_ = data_; }
  void print(std::ostream& os) const;
  probabilisticEffectList_t& operator=(probabilisticEffectList_t const& effect);
};


/*******************************************************************************
 *
 * effect (abstract class)
 *
 ******************************************************************************/

class effect_t
{
 public:
  effect_t() { }
  virtual ~effect_t() { }
  virtual bool affect(state_t const& state, state_t& s_prime, bool nprec = false) const = 0;
  virtual void collect_prec_atoms( atomList_t &atoms ) const = 0;
  virtual void collect_add_atoms( atomList_t &atoms ) const = 0;
  virtual void collect_del_atoms( atomList_t &atoms ) const = 0;
  virtual void print( std::ostream &os ) const = 0;
};


/*******************************************************************************
 *
 * strips effect
 *
 ******************************************************************************/

class stripsEffect_t : public effect_t
{
 private:
  VecRationals cost_;
  atomList_t add_list_;
  atomList_t del_list_;
  static bool reward_warning_printed_;

 public:
  stripsEffect_t();
  virtual ~stripsEffect_t() { }

  atomList_t& add_list( void ) { return( add_list_ ); }
  const atomList_t& add_list( void ) const { return( add_list_ ); }
  atomList_t& del_list( void ) { return( del_list_ ); }
  const atomList_t& del_list( void ) const { return( del_list_ ); }
  bool empty() const {
    if (add_list().size() > 0 || del_list().size() > 0) {
      return false;
    }
    for (Rational const& c : cost_) {
      if (c != 0) {
        return false;
      }
    }
    return true;
  }
  void insert_add( ushort_t atom ) { add_list_.insert( atom ); }
  void insert_del( ushort_t atom ) { del_list_.insert( atom ); }

  static void print_reward_warning() {
    if (!reward_warning_printed_) {
      std::cout << std::endl
          << "[COST/ REWARD WARNING]: The actions in given problem manipulates its COST/REWARD."
          << std::endl
          << "                        Bear in mind that an action cost is 1 + the manipulation."
          << std::endl
          << "                        Therefore if an action A increases the reward in 1, then"
          << std::endl
          << "                        the cost of A will be 0 (= 1 + (-1))."
          << std::endl << std::endl;
      reward_warning_printed_ = true;
    }
  }

  void set_cost(Rational c, size_t cost_idx = ACTION_COST) {
    assert(cost_idx < cost_.size());
    if (gpt::use_action_cost) {
      print_reward_warning();
      cost_[cost_idx] = c;
    }
  }

  void increase_cost_by(Rational c, size_t cost_idx = ACTION_COST) {
    assert(cost_idx < cost_.size());
    if (gpt::use_action_cost) {
      print_reward_warning();
      cost_[cost_idx] += c;
    }
  }


  Rational cost(size_t cost_idx = ACTION_COST) const {
    assert(cost_idx < cost_.size());
    return cost_[cost_idx];
  }

  virtual bool affect(state_t const& state, state_t& s_prime, bool nprec = false ) const;

  virtual void collect_prec_atoms( atomList_t &atoms ) const;
  virtual void collect_add_atoms( atomList_t &atoms ) const;
  virtual void collect_del_atoms( atomList_t &atoms ) const;
  virtual void print( std::ostream &os ) const;
  bool operator==( const stripsEffect_t &effect ) const;
  stripsEffect_t& operator=( const stripsEffect_t &effect );
};


/*******************************************************************************
 *
 * conditional effect
 *
 ******************************************************************************/

class conditionalEffect_t : public effect_t
{
  atomListList_t prec_list_;
  stripsEffect_t s_effect_;

  public:
  conditionalEffect_t()
  {
    notify( this, "conditionalEffect_t::conditionalEffect_t()" );
  }
  virtual ~conditionalEffect_t() { }

  atomListList_t& precondition( void ) { return( prec_list_ ); }
  const atomListList_t& precondition( void ) const { return( prec_list_ ); }
  stripsEffect_t& s_effect( void ) { return( s_effect_ ); }
  const stripsEffect_t& s_effect( void ) const { return( s_effect_ ); }
  bool empty( void ) const { return( s_effect().empty() ); }

  Rational cost(state_t const& s, size_t cost_idx = ACTION_COST) const {
    if (precondition().holds(s))
      return s_effect_.cost(cost_idx);
    else
      return Rational(0);
  }

  virtual Rational cost_lowerbound() const { return s_effect_.cost(); };
  virtual Rational cost_upperbound() const { return s_effect_.cost(); };

  virtual bool affect(state_t const& state, state_t& s_prime, bool nprec = false ) const;

  virtual void collect_prec_atoms( atomList_t &atoms ) const;
  virtual void collect_add_atoms( atomList_t &atoms ) const;
  virtual void collect_del_atoms( atomList_t &atoms ) const;
  virtual void print( std::ostream &os ) const;
  bool operator==( const conditionalEffect_t &effect ) const;
  conditionalEffect_t& operator=( const conditionalEffect_t &effect );
};


/*******************************************************************************
 *
 * deterministic effect = a strips effect and list of conditional effects
 *
 ******************************************************************************/

class deterministicEffect_t : public effect_t
{
  stripsEffect_t s_effect_;
  conditionalEffectList_t c_effect_;

 public:
  deterministicEffect_t() { }
  ~deterministicEffect_t();

  stripsEffect_t& s_effect( void ) { return( s_effect_ ); }
  const stripsEffect_t& s_effect( void ) const { return( s_effect_ ); }
  conditionalEffectList_t& c_effect( void ) { return( c_effect_ ); }
  const conditionalEffectList_t& c_effect( void ) const { return( c_effect_ ); }
  bool empty( void ) const;
  void insert_add( ushort_t atom ) { s_effect_.insert_add( atom ); }
  void insert_del( ushort_t atom ) { s_effect_.insert_del( atom ); }
  void insert_effect( const stripsEffect_t &seff );
  void delete_del_list( void ) { s_effect().del_list().clear(); }

  Rational cost(state_t const& s, size_t cost_idx = ACTION_COST) const {
    return s_effect_.cost(cost_idx) + c_effect_.cost(s, cost_idx);
  }


  Rational cost(size_t cost_idx = ACTION_COST) const {
    // TODO(fwt): If it's flatten this should be OK. Added for compatibility
    // with H_add code
    return s_effect_.cost(cost_idx);
  }

  virtual Rational cost_lowerbound() const {
    return (s_effect_.cost() + c_effect_.cost_lowerbound());
  }

  virtual Rational cost_upperbound() const {
    return (s_effect_.cost() + c_effect_.cost_upperbound());
  }

  virtual void set_cost(Rational c) { s_effect_.set_cost(c); }
  virtual void shift_cost_by(Rational c) { s_effect_.increase_cost_by(c); }

  virtual bool affect(state_t const& state, state_t& s_prime, bool nprec = false) const;
  virtual void collect_prec_atoms( atomList_t &atoms ) const;
  virtual void collect_add_atoms( atomList_t &atoms ) const;
  virtual void collect_del_atoms( atomList_t &atoms ) const;
  virtual void print( std::ostream &os ) const;
  bool operator==( const deterministicEffect_t &effect ) const;
  deterministicEffect_t& operator=( const deterministicEffect_t &effect );

  void probability_of_adding_atoms(state_t const& s,
      HashAtomtToRational& prob) const;
};


/*******************************************************************************
 *
 * probabilistic effect
 *
 ******************************************************************************/

class probabilisticEffect_t : public deterministicEffect_t
{
  Rational probability_;

  public:
  probabilisticEffect_t( Rational p ) : probability_(p)
  {
    notify( this, "probabilisticEffect_t::probabilisticEffect_t(Rational)" );
  }
  ~probabilisticEffect_t() { }

  Rational probability( void ) const { return( probability_ ); }
  void increase_probability( Rational p ) { probability_ = probability_ + p; }
  void change_probability_to( Rational p ) { probability_ = p; }

  virtual bool affect(state_t const& state, state_t& s_prime, bool nprec = false ) const;
  virtual void print( std::ostream &os ) const;
  bool operator==( const probabilisticEffect_t &effect ) const;
  probabilisticEffect_t& operator=( const probabilisticEffect_t &effect );

  virtual bool is_reward_consistent_with(probabilisticEffect_t const& peff) const;
};


/*******************************************************************************
 *
 * action (abstract class)
 *
 ******************************************************************************/

class action_t
{
  mutable size_t ref_count_;
  const char *name_, *nameXML_;
  atomListList_t precondition_;

  action_t const* original_action_; // Added this for the replanners
  effect_t const* original_effect_;

 protected:
  action_t( const std::string &name, const std::string &nameXML );

 public:
  action_t() : ref_count_(0) { }
  virtual ~action_t();

  static void register_use( const action_t *a )
  {
#ifdef MEM_DEBUG
    if(a) std::cerr << "[act]: inc-ref-count " << a << " = " << a->ref_count_+1 << std::endl;
#endif
    if( a != NULL ) ++a->ref_count_;
  }
  static void unregister_use( const action_t *a )
  {
#ifdef MEM_DEBUG
    if(a) std::cerr << "[act]: dec-ref-count " << a << " = " << a->ref_count_-1 << std::endl;
#endif
    if( a && (--a->ref_count_ == 0) ) delete a;
  }

  atomListList_t& precondition( void ) { return( precondition_ ); }
  const atomListList_t& precondition( void ) const { return( precondition_ ); }
  const char* name( void ) const { return( name_ ); }
  const char* nameXML( void ) const { return( nameXML_ ); }


  void print( std::ostream &os ) const { os << name(); }
  void printXML( std::ostream &os ) const { os << nameXML(); }
  bool enabled( const state_t& state, bool nprec = false ) const
  {
    return precondition().holds(state, nprec);
  }


  void insert_precondition( const atomList_t &alist );
  void insert_precondition( const atomListList_t &alist );
  virtual bool simplify_confiditonal() = 0;

  virtual bool empty( void ) const = 0;
  virtual bool affect( state_t& state, bool nprec = false ) const = 0;

  virtual void expand(state_t const& state, ProbDistStateIface& pr,
      bool nprec = false) const = 0;

  virtual void print_full( std::ostream &os ) const = 0;
  virtual void print_full() const { print_full(std::cout); }
  virtual action_t* clone( void ) const = 0;
  virtual void collect_prec_atoms( atomList_t &atoms ) const = 0;
  virtual void collect_add_atoms( atomList_t &atoms ) const = 0;
  virtual void collect_del_atoms( atomList_t &atoms ) const = 0;


  void set_original_action(action_t const* a) { original_action_ = a; register_use(a); }
  action_t const* get_original_action() const { return original_action_; }

  virtual void replanning_split(double threshold, actionList_t& result) const = 0;

  virtual Rational cost(state_t const& s) const { return cost(s, ACTION_COST); }
  virtual Rational cost(state_t const& state, size_t cost_idx) const = 0;

  virtual Rational cost(size_t cost_idx) const = 0;
  virtual Rational cost() const { return cost(ACTION_COST); }

  virtual Rational cost_lowerbound() const = 0;
  virtual Rational cost_upperbound() const = 0;
  virtual void shift_cost_by(Rational c) = 0;

  virtual void probability_of_adding_atoms(state_t const& s,
      HashAtomtToRational& prob) const = 0;


  /* to store actions in original problem */
  void set_original_effect(const effect_t *e ) { original_effect_ = e; }

  const action_t* original_action() const;
  const action_t* original_np_action() const;
  const effect_t* original_effect() const { return original_effect_; }

  /* some small helper methods */
  bool deletes_atom(ushort_t atom) const;
  bool adds_atom(ushort_t atom) const;

};


/*******************************************************************************
 *
 * deterministic action
 *
 ******************************************************************************/

class deterministicAction_t : public action_t
{
  deterministicEffect_t effect_;
  Rational self_loop_correction_;

 public:
  deterministicAction_t( const std::string &name, const std::string &nameXML );
  virtual ~deterministicAction_t();

  deterministicEffect_t& effect( void ) { return( effect_ ); }
  const deterministicEffect_t& effect( void ) const { return( effect_ ); }
  void set_effect( const deterministicEffect_t &eff ) { effect() = eff; }
  void set_self_loop_correction(Rational const& r) {
    self_loop_correction_ = r;
  }
  Rational self_loop_correction() const { return self_loop_correction_; }
  void insert_add( ushort_t atom ) { effect().insert_add( atom ); }
  void insert_del( ushort_t atom ) { effect().insert_del( atom ); }
  void insert_effect( const stripsEffect_t &seff ) { effect().insert_effect( seff ); }
  void delete_del_list( void ) { effect().delete_del_list(); }

  virtual Rational cost(size_t cost_idx) const;
  virtual Rational cost(state_t const& state, size_t cost_idx) const;

  virtual Rational cost_lowerbound() const {
    if (gpt::use_action_cost) {
      // This version of the cost is not ready and should be avoided. The usage
      // is only for internal test. For a real version using costs of actions
      // and costs of STATES (this is really important). See the reward_enabled
      // branch
      return Rational(1) + effect_.cost_lowerbound();
    } else {
      return Rational(1);
    }
  }
  virtual Rational cost_upperbound() const {
    if (gpt::use_action_cost) {
      return Rational(1) + effect_.cost_upperbound();
    } else {
      return Rational(1);
    }
  }
  virtual void shift_cost_by(Rational c) { effect_.shift_cost_by(c); }

  virtual bool simplify_confiditonal();

  virtual bool empty( void ) const { return( effect().empty() ); }
  virtual bool affect( state_t& state, bool nprec = false ) const;

  virtual void expand(state_t const& state, ProbDistStateIface& pr,
      bool nprec = false) const;

  virtual void print_full( std::ostream &os ) const;
  virtual void print_full() const { print_full(std::cout); }
  virtual action_t* clone( void ) const;
  virtual void collect_prec_atoms( atomList_t &atoms ) const;
  virtual void collect_add_atoms( atomList_t &atoms ) const;
  virtual void collect_del_atoms( atomList_t &atoms ) const;

  friend action_t& Action::translate( const problem_t &problem ) const;

  virtual void replanning_split(double threshold, actionList_t& result) const
  {

#ifdef REPLANNER_DEBUG
    print_full(std::cout);
    if (effect().empty())
      std::cout << " --- !!!! IGNORING SINCE IT'S EMPTY";
    std::cout << std::endl;
#endif
    if (! effect().empty()) {
      action_t* my_clone = clone();
      my_clone->set_original_action(this->get_original_action());
      result.push_back(my_clone);
    }
  }

  virtual void probability_of_adding_atoms(state_t const& s,
      HashAtomtToRational& prob) const
  {
    effect().probability_of_adding_atoms(s, prob);
  }
};


/*******************************************************************************
 *
 * probabilistic action
 *
 ******************************************************************************/

class probabilisticAction_t : public action_t
{
  probabilisticEffectList_t effect_list_;

  public:
  probabilisticAction_t( const std::string &name, const std::string &nameXML );
  virtual ~probabilisticAction_t();

  probabilisticEffectList_t& effect( void ) { return( effect_list_ ); }
  const probabilisticEffectList_t& effect( void ) const { return( effect_list_ ); }
  bool insert_effect( const probabilisticEffect_t *eff )
  {
    return( effect().insert( eff ) );
  }
  size_t size( void ) const { return( effect().size() ); }
  const probabilisticEffect_t& effect( size_t i ) const { return( effect().effect( i ) ); }
  Rational probability( size_t i ) const { return( effect().effect( i ).probability() ); }

  // Return the cost of the action IF it is the same cost independently of the
  // probabilistic outcome and state in which the action is being applied.
  // Otherwise, dies with and assertion
  virtual Rational cost(size_t cost_idx) const;
  virtual Rational cost(state_t const& state, size_t cost_idx) const;


  virtual Rational cost_lowerbound() const {
    if (gpt::use_action_cost) {
      // This version of the cost is not ready and should be avoided. The usage
      // is only for internal test. For a real version using costs of actions
      // and costs of STATES (this is really important). See the reward_enabled
      // branch
      return Rational(1) + effect_list_.cost_lowerbound();
    } else {
      return Rational(1);
    }
  }
  virtual Rational cost_upperbound() const {
    if (gpt::use_action_cost) {
      return Rational(1) + effect_list_.cost_upperbound();
    } else {
      return Rational(1);
    }
  }

  virtual bool simplify_confiditonal() { return false; }

  virtual void shift_cost_by(Rational c) { effect_list_.shift_cost_by(c); }

  virtual bool empty( void ) const;
  virtual bool affect( state_t& state, bool nprec = false ) const;

  /*
   * FWT: In the current action design, the same event could be added more than
   * once to the ProbDistIface (mainly due to 2 or more different effects that
   * result in the same state s'). Therefore, some ProbDistState might be huge
   * but the actual support (i.e., number of unique events) is small. One
   * example of this case is the sysAdmin problems. Thus, if the number of
   * effects of the action is too large, we use a hash based ProbDist to
   * collapse them into a smaller representation of the same ProbDist. The
   * problem is that for small supports, the hash based ProbDist is inefficient
   * and it is necessary to find the correct trade off here. For now, the
   * PROB_DIST_ARRAY_SIZE is used because a fixed size array is the default
   * ProbDistState and if the number of effects is greater than
   * PROB_DIST_ARRAY_SIZE then the expansion will not fit the
   * ProbDistStateStlArray neither the ProbDistStateAllocArray.
   */
  virtual void expand(state_t const& s, ProbDistStateIface& pr,
    bool nprec = false) const
  {
    if (effect().size() > PROB_DIST_ARRAY_SIZE)
      expand_hash_based(s, pr, nprec);
    else
      expand_directly(s, pr, nprec);
  }

  virtual void expand_hash_based(state_t const& s, ProbDistStateIface& pr,
    bool nprec = false) const;

  virtual void expand_directly(state_t const& s, ProbDistStateIface& pr,
    bool nprec = false) const;

  virtual void print_full( std::ostream &os ) const;
  virtual void print_full() const { print_full(std::cout); }
  virtual action_t* clone( void ) const;
  virtual void collect_prec_atoms( atomList_t &atoms ) const;
  virtual void collect_add_atoms( atomList_t &atoms ) const;
  virtual void collect_del_atoms( atomList_t &atoms ) const;

  virtual void replanning_split(double threshold, actionList_t& result) const;
  virtual bool is_reward_consistent() const;

  virtual void probability_of_adding_atoms(state_t const& s,
      HashAtomtToRational& prob) const;
};


class ConditionAndRational {
 public:
  ConditionAndRational() : condition_(0), r_(0) { }
  ConditionAndRational(atomListList_t const* c, Rational r);
  virtual ~ConditionAndRational() { }
  Rational run_condition(state_t const& s, bool nprec) const {
    if (condition_->holds(s, nprec)) {
      return r_;
    }
    else {
      return 0;
    }
  }
  Rational run_condition(atomList_t const& atm_list, bool nprec) const {
    if (condition_->holds(atm_list, nprec)) {
      return r_;
    }
    else {
      return 0;
    }
  }

  atomListList_t const* condition() const { return condition_; }
  Rational const& probability() const { return r_; }
 private:
  // Conditions is a list of list of atoms, representing a disjunction (top
  // level of list) of conjunctions (each atomList_t)
  atomListList_t const* condition_;
  Rational r_;

  friend class ProbDistOfAddingAnAtom;
};

typedef std::vector<ConditionAndRational> VectorOfConditionAndRational;
typedef std::unordered_map<atom_t, VectorOfConditionAndRational>
                                        HashAtomtToVectorOfConditionAndRational;
typedef std::set<atom_t> SetAtom_t;

/*******************************************************************************
 *
 * ProbDistOfAddingAnAtom
 *
 ******************************************************************************/
class ProbDistOfAddingAnAtom {
 public:
  ProbDistOfAddingAnAtom() : has_cond_effects_(false) { }
  virtual ~ProbDistOfAddingAnAtom() { }

  void process_action(action_t const* a, bool consider_negative_atoms);

  SetAtom_t const& potential_support() { return potential_support_; }

  Rational prob_add_atom(atom_t atom, state_t const& s, bool nprec) {
    Rational prob = non_conditional_prob_[atom];
    if (has_cond_effects_) {
      HashAtomtToVectorOfConditionAndRational::const_iterator it =
                                                  conditional_prob_.find(atom);
      if (it != conditional_prob_.end()) {
        VectorOfConditionAndRational const& conds = it->second;
        for (size_t i = 0; i < conds.size(); ++i) {
          prob += conds[i].run_condition(s, nprec);
        }
      }
    }
    return prob;
  }

  // Returns a list of all atoms that this action depends on. This list
  // contains the preconditions and the conditions of the conditional effects
  SetAtom_t const& depends_on() { return depends_on_; }

  void dump();

 public:
  bool has_cond_effects_;
  HashAtomtToRational non_conditional_prob_;
  SetAtom_t potential_support_;
  SetAtom_t depends_on_;
  HashAtomtToVectorOfConditionAndRational conditional_prob_;
  SetAtom_t atoms_in_cond_triggers_;

  void process_det_effect(deterministicEffect_t const& det_eff, Rational prob,
      bool consider_negative_atoms);


  void increaseProbabilityOfConditionBy(atom_t atom,
      atomListList_t const& condition, Rational const& increment);
};




/*******************************************************************************
 *
 * misc. inline functions
 *
 ******************************************************************************/

inline bool
conditionalEffectList_t::find( const conditionalEffect_t &eff ) const
{
  for( size_t i = 0; i < size(); ++i )
    if( effect( i ) == eff ) return( true );
  return( false );
}

  inline void
conditionalEffectList_t::insert( const conditionalEffect_t *effect )
{
  if( !find( *effect ) )
  {
    if( !data_ || (data_ptr_ == &data_[size_]) )
    {
      size_ = (!data_ ? 1 : size_ << 1);
      const conditionalEffect_t **ndata_ =
        (const conditionalEffect_t**)
        realloc( data_, size_ * sizeof(conditionalEffect_t*) );
      data_ptr_ = (!data_ ? ndata_ : &ndata_[data_ptr_ - data_]);
      data_ = ndata_;
    }
    *data_ptr_++ = effect;
  }
}

inline bool
probabilisticEffectList_t::find( const probabilisticEffect_t &eff ) const
{
  for( size_t i = 0; i < size(); ++i )
    if( effect( i ) == eff ) return( true );
  return( false );
}

  inline bool
probabilisticEffectList_t::insert( const probabilisticEffect_t *eff )
{
  size_t i;
  for( i = 0; i < size(); ++i )
    if( effect( i ) == *eff ) break;

  if( i == size() )
  {
    if( !data_ || (data_ptr_ == &data_[size_]) )
    {
      size_ = (!data_ ? 1 : size_ << 1);
      const probabilisticEffect_t **ndata_ =
        (const probabilisticEffect_t**)
        realloc( data_, size_ * sizeof(const probabilisticEffect_t*) );
      data_ptr_ = (!data_ ? ndata_ : &ndata_[data_ptr_ - data_]);
      data_ = ndata_;
    }
    *data_ptr_++ = eff;
    return( true );
  }
  else
  {
    // this is a violation of the const abstraction
    ((probabilisticEffect_t*)data_[i])->increase_probability(eff->probability());
    return( false );
  }
}

inline Rational probabilisticEffectList_t::cost(size_t cost_idx) const {
  Rational expected_cost = 0.0;
  for (size_t i = 0; i < size(); ++i) {
    expected_cost += effect(i).probability() * effect(i).cost(cost_idx);
  }
  return expected_cost;
}

#endif // ACTIONS_H
