#ifndef ATOM_STATES_H
#define ATOM_STATES_H

#include <cstring>
#include <deque>
#include <list>
#include <map>
#include <memory>
#include <sstream>
#include <set>
#include <unordered_map>

#include "rational.h"
#include "atom_list.h"
#include "../../utils/die.h"
#include "md4c.h"

class Action;
class Application;
class Atom;
class AtomSet;
class State;
class ValueMap;
class problem_t;

class hash_t;
class stateHash_t;
class hashEntry_t;


/*******************************************************************************
 *
 * state
 *
 ******************************************************************************/

class state_t
{
  unsigned *data_;
#ifdef CACHE_ISGOAL_CALLS
  mutable int isGoal_;
#endif

  static size_t size_;
  static std::unique_ptr<stateHash_t> state_hash_;
  static bool state_space_generated_;

 public:
  explicit state_t() : data_(new unsigned[size_]()) {
    notify(this, "state_t::state_t()");
#ifdef CACHE_ISGOAL_CALLS
    isGoal_ = 0;
#endif
  }
  state_t(const state_t& state) : data_(new unsigned[size_]()) {
    notify(this, "state_t::state_t(state_t&)");
    memcpy(data_, state.data_, size_ * sizeof(unsigned));
#ifdef CACHE_ISGOAL_CALLS
    isGoal_ = state.isGoal_;
#endif
  }


#if not defined(IGNORE_MOVE_OPS_STATE)
  state_t(state_t&& other) noexcept : data_(nullptr)  {
    notify(this, "state_t::state_t(state_t&&)" );
    data_ = other.data_;
    other.data_ = nullptr;
#ifdef CACHE_ISGOAL_CALLS
    isGoal_ = other.isGoal_;
#endif
  }
#endif


  state_t(atomList_t const& alist);
  // Generate a state given a string. If use_atom_index is TRUE, then the
  // string contains numbers (atom number) separated by spaces, for
  // example "10 1234 4320". Otherwise (use_atom_index is FALSE), then the
  // string contains the instantiated predicates separated by comma, for
  // example "at l-001-l001, broken, speed_x 0, speed_y 0, ..."
  state_t(std::string str, bool use_atom_index = false);

  ~state_t() {
    delete[] data_;
  }

  void operator=(state_t const& state) {
    if (this != &state) {
      // FWT: it should work fine with new because it is primitive type
      memcpy(data_, state.data_, size_ * sizeof(unsigned));
#ifdef CACHE_ISGOAL_CALLS
      isGoal_ = state.isGoal_;
#endif
    }
  }

#if not defined(IGNORE_MOVE_OPS_STATE)
  void operator=(state_t&& other) noexcept {

    if (data_ == other.data_) {
      std::cout << "\n\n\n[state_t] SELF MOVE ASSIGNMENT. It shouldn't happen"
        << std::endl;
      // Preventing the data from being erased when the dtor of other is
      // called. Note that this object is already pointing to the same chunk of
      // memory.
      other.data_ = nullptr;
    }
    else {
      delete[] data_;
      data_ = other.data_;
      other.data_ = nullptr;
#ifdef CACHE_ISGOAL_CALLS
      isGoal_ = state.isGoal_;
#endif
    }
  }
#endif


#ifdef CACHE_ISGOAL_CALLS
  int isGoal() const { return isGoal_; }
  void clearGoalFlag() const { isGoal_ = 0; }
  void setGoalFlag(bool is_goal) const {
    if (is_goal) isGoal_ = 1;
    else         isGoal_ = -1;
  }
#endif

  static void initialize( const problem_t &problem );
  static void finalize( void );
  static void statistics( std::ostream &os );
  static const state_t* get_state( const state_t &state );
  static void generate_state_space(const problem_t &problem,
      hash_t &hash_table, std::deque<hashEntry_t*> &space);
  // Use max_depth equals 0 to expand the space completely
  static void generateStateSpaceFrom(const problem_t &problem,
      hash_t &hash_table, std::deque<hashEntry_t*> &space,
      std::vector<state_t> const& initial_states,
      size_t max_depth);

  static size_t size( void ) { return( size_ ); }

  void make_digest( void ) { }
  bool make_check( void ) const { return( true ); }
  const unsigned* data( void ) const { return( data_ ); }
  unsigned hash_value( void ) const
  {
#if 1
    unsigned *ptr, result;
    unsigned char digest[16];
    MD4_CTX context;

    // compute MD4 digests
    MD4Init( &context );
    MD4Update( &context, (unsigned char*)data_, size_ * sizeof(unsigned) );
    MD4Final( digest, &context );

    // compact digest into unsigned (assumes sizeof(unsigned) = 4)
    ptr = (unsigned*)digest;
    result = (ptr[0] ^ ptr[1] ^ ptr[2] ^ ptr[3]);
    return( result );
#else
    unsigned value = 0;
    for( size_t i = 0; i < size_; ++i )
      value = value ^ data_[i];
    return( value );
#endif
  }
  unsigned digest( void ) const { return( hash_value() ); }

  bool operator==(state_t const& state) const {
    return (
            // Commented because static member (size_) comparison always true:
            // size_ == state.size_ &&
            // FWT: it should work fine with new because it is primitive type
            ! memcmp(data_, state.data_, size_ * sizeof(unsigned)));
  }
  bool operator!=(state_t const& s) const { return !operator==(s); }
  bool holds( ushort_t atom ) const
  {
#if 0
    register size_t i = atom % 32;
    register unsigned data = data_[atom>>5];
    for( size_t j = 0; j < i; data = (data>>1), ++j );
    return( data % 2 );
#else
    register size_t i = atom >> 5;  // i = (int) (atom / 32) with no rounding
    register size_t j = atom % 32;
    // data[0] contains atoms 0 to 31: |31|30|....|1|0|
    // ...
    // data[i] contains atoms i*32 to i*32+31:
    //                                |i*32+31|i*32+30|....|i*32+1|i*32|
    return( data_[i] & (0x1<<j) );
#endif
  }
  bool holds(Atom const& atom) const; /*{
    return holds(problem_t::atom_hash_get(atom));
  }*/
  bool add( ushort_t atom )
  {
    register size_t i = atom >> 5;
    register size_t j = atom % 32;
    bool rv = !(data_[i] & (0x1<<j));
    data_[i] = data_[i] | (0x1<<j);
#ifdef CACHE_ISGOAL_CALLS
    clearGoalFlag();
#endif
    return( rv );
  }
  bool add(Atom const& atom);
  bool clear( ushort_t atom )
  {
    register size_t i = atom >> 5;
    register size_t j = atom % 32;
    bool rv = (data_[i] & (0x1<<j));
    data_[i] = data_[i] & ~(0x1<<j);
#ifdef CACHE_ISGOAL_CALLS
    clearGoalFlag();
#endif
    return( rv );
  }
  bool clear(Atom const& a);
  DEPRECATED const state_t& next( const Action *action ) const
  {
    NOT_IMPLEMENTED;
    state_t* nstate = new state_t( *this );
//    if( action != NULL ) {
//      action->affect( *nstate );
//#ifdef CACHE_ISGOAL_CALLS
//      nstate->clearGoalFlag();
//#endif
//    }
//    nstate->make_digest();
    return( *nstate );
  }
  double cost( const Action *action ) const { return( 1 ); }

  std::string toString(bool print_braces = true) const;
  std::string toStringFull(const problem_t *problem,
      bool print_idx = true, bool print_braces = true) const;
  // Returns a unique and deterministic number associated with the given state
  size_t code() const;
  std::string uniqueName() const {
    std::ostringstream ost;
    ost << code();
    return ost.str();
  }

  void print(std::ostream& os) const { os << toString(); }
  void printCOUT() const { std::cout << toString(); }
  void full_print(std::ostream &os, const problem_t *problem,
                  bool print_idx = true, bool print_braces = true) const {
    os << toStringFull(problem, print_idx, print_braces);
  }
  void debug_print(const problem_t *problem, hash_t* h) const;
  void printXML( std::ostream &os, bool goal ) const;
  void send( std::ostream& os ) const;

  public: // iterators
  class const_predicate_iterator
  {
    const state_t *state_;
    size_t idx_;

    protected:
    const_predicate_iterator( const state_t &st, size_t idx = 0 ) : state_(&st), idx_(idx) { }
    void first( void )
    {
      for( idx_ = 0; (idx_ < state_->size()) && !state_->data()[idx_]; ++idx_ );
      if( idx_ < state_->size() )
      {
        register unsigned data = state_->data()[idx_];
        for( idx_ = (idx_<<5); data % 2 == 0; data = (data>>1), ++idx_ );
      }
      else
      {
        idx_ = idx_ << 5;
      }
    }
    size_t increase( void ) const
    {
      register unsigned data = 0;
      register size_t i = idx_ >> 5;
      register size_t j = idx_ % 32;

      if( j == 31 )
      {
        j = 0;
        for( ++i; (i < state_->size()) && !state_->data()[i]; ++i );
        if( i < state_->size() ) data = state_->data()[i];
      }
      else
      {
        data = (state_->data()[i]) >> (j+1);
        if( data != 0 )
        {
          ++j;
        }
        else
        {
          j = 0;
          for( ++i; (i < state_->size()) && !state_->data()[i]; ++i );
          if( i < state_->size() ) data = state_->data()[i];
        }
      }

      if( i < state_->size() )
      {
        i = (i << 5) + j;
        for( ; data % 2 == 0; data = (data>>1), ++i );
      }
      else
      {
        i = i << 5;
      }

      return( i );
    }

    public:
    const_predicate_iterator() : state_(0), idx_(0) { }
    ushort_t operator*( void ) const { return( idx_ ); }
    const_predicate_iterator operator++( void ) // pre increment
    {
      idx_ = increase();
      return( *this );
    }
    const_predicate_iterator operator++( int ) // post increment
    {
      const_predicate_iterator it( *this ); // use default copy const.
      idx_ = increase();
      return( it );
    }
    bool operator==( const const_predicate_iterator &it ) const
    {
      return( (state_ == it.state_) && (idx_ == it.idx_) );
    }
    bool operator!=( const const_predicate_iterator &it ) const
    {
      return( (state_ != it.state_) || (idx_ != it.idx_) );
    }

    friend class state_t;
  };
  const const_predicate_iterator predicate_begin( void ) const
  {
    const_predicate_iterator it( *this );
    it.first();
    return( it );
  }
  const const_predicate_iterator predicate_end( void ) const
  {
    return( const_predicate_iterator( *this, (size()<<5) ) );
  }
};

  inline std::ostream&
operator<<( std::ostream &os, const state_t &state )
{
  state.print( os );
  return( os );
}

struct hashState {
  size_t operator()(state_t const& s) const { return s.hash_value(); }
};

#endif // ATOM_STATES_H
