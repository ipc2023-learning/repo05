#ifndef HASH_H
#define HASH_H

#include "global.h"
#include "../../heuristics/heuristic_iface.h"
#include "states.h"
#include "md4c.h"

class state_t;
class problem_t;
class hash_t;

#ifndef DEBUG_TIED_ACTIONS
#define DEBUG_TIED_ACTIONS 0
#endif

/*******************************************************************************
 *
 * hash entry
 *
 ******************************************************************************/

class hashEntry_t
{
  hash_t* hash_;
  double value_;
  unsigned bits_;
  const state_t *state_;
  hashEntry_t *next_, *prev_;
  friend class hash_t;

  public:
  hashEntry_t() : hash_(0), value_(0), bits_(0), state_(NULL), next_(0), prev_(0) { }
  hashEntry_t(hash_t* hash, const state_t &state, double value = 0)
    : hash_(hash), value_(value), bits_(0), next_(0), prev_(0)
  {
    notify( this, "hashEntry_t::hashEntry_t(state_t&,double)" );
    state_ = state_t::get_state( state );
  }
  virtual ~hashEntry_t() { }

  void update( double value );

  double value( void ) const { return( value_ ); }
  unsigned bits( void ) const { return( bits_ ); }
  void set_bits( unsigned bits ) { bits_ = bits; }
  const state_t* state( void ) const { return( state_ ); }
  const hashEntry_t* next( void ) const { return( next_ ); }
};


/*******************************************************************************
 *
 * hash entry (extended)
 *
 ******************************************************************************/

class hashEntryX_t : public hashEntry_t
{
  ushort_t data_[2];
  hashEntryX_t *elink_;

  public:
  hashEntryX_t() : hashEntry_t() { data_[0] = USHORT_MAX; data_[1] = USHORT_MAX; }
  hashEntryX_t(hash_t* hash, const state_t &state, double value = 0 )
    : hashEntry_t(hash, state,value)
  {
    data_[0] = USHORT_MAX;
    data_[1] = USHORT_MAX;
  }
  virtual ~hashEntryX_t() { }

  unsigned low( void ) const { return( data_[0] ); }
  void set_low( unsigned low ) { data_[0] = low; }
  unsigned idx( void ) const { return( data_[1] ); }
  void set_idx( unsigned idx ) { data_[1] = idx; }
  hashEntryX_t* elink( void ) { return( elink_ ); }
  void set_elink( hashEntryX_t *elink ) { elink_ = elink; }
};


/*******************************************************************************
 *
 * hash
 *
 ******************************************************************************/

class hash_t
{
  bool ext_;
  unsigned size_;
  unsigned dimension_;
  unsigned *number_;
  mutable heuristic_t *heuristic_;
  hashEntry_t **table_;
  size_t update_counter_;
  uint64_t total_q_calls_;

  void rehash( void );

 public:
  hash_t()
    : ext_(false), size_(0), dimension_(0), number_(0), heuristic_(0), table_(0),
      update_counter_(0), total_q_calls_(0) { }
  hash_t( unsigned dimension, heuristic_t &heuristic );
  virtual ~hash_t();

  // Replaces the current heuristic H_cur by h and return the pointer to H_cur
  heuristic_t* replaceHeuristic(heuristic_t& h) {
    heuristic_t* old_h = heuristic_;
    heuristic_ = &h;
    return old_h;
  }

  heuristic_t* getHeuristic() const { return heuristic_; }

  size_t get_update_counter() const { return update_counter_; }
  void reset_update_counter() { } //update_counter_ = 0;
  void increment_counter() { update_counter_++; }

  void set_extended( void ) { ext_ = true; }
  unsigned hash_value(state_t const& s) const {
    return s.hash_value() % dimension_;
  }
  double heuristic(state_t const& s) const { return heuristic_->value(s);  }
  hashEntry_t* find( const state_t &state ) const
  {
    unsigned idx = hash_value( state );
    for( hashEntry_t *ptr = table_[idx]; ptr != NULL; ptr = ptr->next_ )
      if( *(ptr->state_) == state ) return( ptr );
    return( NULL );
  }
  hashEntry_t* insert(const state_t &state) {
    double value = 0.0;
      value = heuristic(state);
    return insert(state, value);
  }

  hashEntry_t* insert( const state_t &state, double value)
  {
    hashEntry_t *entry;
    //xxxxxx if( 4*size_ > 3*dimension_ ) rehash();
    if( !ext_ )
      entry = new hashEntry_t(this, state, value);
    else
      entry = new hashEntryX_t(this, state, value);
    insert( entry );
    return( entry );
  }
  void insert( hashEntry_t *entry )
  {
    unsigned idx = hash_value( *entry->state() );
    entry->next_ = table_[idx];
    entry->prev_ = NULL;
    if( table_[idx] )
      table_[idx]->prev_ = entry;
    table_[idx] = entry;
    ++number_[idx];
    ++size_;
  }
  hashEntry_t* get( const state_t &state )
  {
    hashEntry_t *result = find( state );
    return( !result ? insert( state ) : result );
  }

  double value(state_t const& s) const {
    hashEntry_t const* entry = find(s);
    if (!entry) return heuristic(s);
    else        return entry->value();
  }

  double value(state_t const& s) {
    hashEntry_t const* entry = (gpt::hash_all ? get(s) : find(s));
    if (!entry) {
      // If gpt::hash_all is used, then a new entry if created in the line
      // before and this part of the if will never happen
      return heuristic(s);
    }
    else
      return entry->value();
  }

  void update(state_t const& s, double value) {
    hashEntry_t* entry = (gpt::hash_all ? get(s) : find(s));
    if (entry) {
      entry->update(value);
    }
    else {
      insert(s, value);
    }
  }

  unsigned dimension( void ) const { return( dimension_ ); }
  unsigned size( void ) const { return( size_ ); }
  unsigned diameter( void ) const;
  void print(std::ostream &os, SSPIface const& ssp) const;

 public: // iterator
  class const_iterator;
  friend class hash_t::const_iterator;

  class const_iterator
  {
    const hash_t *hash_;
    const hashEntry_t *ptr_;
    size_t idx_;

    protected:
    const_iterator( const hash_t *h, int pos ) : hash_(h), ptr_(0), idx_(0)
    {
      if( pos == 0 )
      {
        for( ; idx_ < hash_->dimension_; ++idx_ )
          if( (ptr_ = hash_->table_[idx_]) )
            break;
      }
    }

    public:
    const_iterator() : hash_(0), ptr_(0), idx_(0) { }
    const hashEntry_t* operator*( void ) const { return( ptr_ ); }
    const_iterator operator++( void ) // pre increment
    {
      if( ptr_ != NULL )
      {
        if( !(ptr_ = ptr_->next()) )
        {
          for( ++idx_; idx_ < hash_->dimension_; ++idx_ )
            if( (ptr_ = hash_->table_[idx_]) )
              break;
        }
      }
      return( *this );
    }
    const_iterator operator++( int ) // post increment
    {
      const_iterator it( *this ); // use default copy const.
      ++(*this);
      return( it );
    }
    bool operator==( const const_iterator &it ) const
    {
      return( (ptr_ == it.ptr_) && (idx_ == it.idx_) );
    }
    bool operator!=( const const_iterator &it ) const
    {
      return( (ptr_ != it.ptr_) || (idx_ != it.idx_) );
    }
    friend class hash_t;
  };
  const const_iterator begin( void ) const
  {
    return( const_iterator( this, 0 ) );
  }
  const const_iterator end( void ) const
  {
    return( const_iterator( this, 1 ) );
  }
  void dump( std::ostream &os ) const;
};


/*******************************************************************************
 *
 * state hash entry
 *
 ******************************************************************************/

class stateHashEntry_t
{
  const state_t *state_;
  stateHashEntry_t *next_, *prev_;
  friend class stateHash_t;

  public:
  stateHashEntry_t() : state_(NULL), next_(0), prev_(0) { }
  stateHashEntry_t( const state_t &state ) : next_(0), prev_(0)
  {
    notify( this, "stateHashEntry_t::stateHashEntry_t(state_t&)" );
    state_ = new state_t( state );
  }
  ~stateHashEntry_t() { delete state_; }
  const state_t* state( void ) const { return( state_ ); }
  const stateHashEntry_t* next( void ) const { return( next_ ); }
};


/*******************************************************************************
 *
 * state hash
 *
 ******************************************************************************/

class stateHash_t
{
  protected:
    unsigned size_;
    unsigned dimension_;
    unsigned *number_;
    stateHashEntry_t **table_;

    void rehash( void );

  public:
    stateHash_t() : size_(0), dimension_(0), number_(0), table_(0) { }
    stateHash_t( unsigned dimension );
    ~stateHash_t();

    unsigned hash_value(state_t const& s) const {
      return s.hash_value() % dimension_;
    }
    stateHashEntry_t* find( const state_t &state ) const
    {
      unsigned idx = hash_value( state );
      for( stateHashEntry_t *ptr = table_[idx]; ptr != NULL; ptr = ptr->next_ )
        if( *(ptr->state_) == state ) return( ptr );
      return( NULL );
    }
    stateHashEntry_t* insert( const state_t &state )
    {
      //xxxxxx if( 4*size_ > 3*dimension_ ) rehash();
      stateHashEntry_t *entry = new stateHashEntry_t( state );
      insert( entry );
      return( entry );
    }
    void insert( stateHashEntry_t *entry )
    {
      unsigned idx = hash_value( *entry->state() );
      entry->next_ = table_[idx];
      entry->prev_ = NULL;
      if( table_[idx] )
        table_[idx]->prev_ = entry;
      table_[idx] = entry;
      ++number_[idx];
      ++size_;
    }
    stateHashEntry_t* get( const state_t &state )
    {
      stateHashEntry_t *result = find( state );
      return( !result ? insert( state ) : result );
    }

    unsigned dimension( void ) const { return( dimension_ ); }
    unsigned size( void ) const { return( size_ ); }
    unsigned diameter( void ) const;
    void print( std::ostream &os, const problem_t &problem ) const;
    void dump( std::ostream &os ) const;

  public: // iterator
    class const_iterator;
    friend class stateHash_t::const_iterator;

    class const_iterator
    {
      const stateHash_t *hash_;
      const stateHashEntry_t *ptr_;
      size_t idx_;

      protected:
      const_iterator(stateHash_t const* shash, int pos)
        : hash_(shash), ptr_(0), idx_(0)
      {
        if( pos == 0 )
        {
          for( ; idx_ < hash_->dimension_; ++idx_ )
            if( (ptr_ = hash_->table_[idx_]) )
              break;
        }
        else
        {
          idx_ = hash_->dimension_;
          ptr_ = 0;
        }
      }

      public:
      const_iterator() : hash_(0), ptr_(0), idx_(0) { }
      const state_t* operator*( void ) const { return( ptr_->state() ); }
      const_iterator operator++( void ) // pre increment
      {
        if( ptr_ != NULL )
        {
          if( !(ptr_ = ptr_->next()) )
          {
            for( ++idx_; idx_ < hash_->dimension_; ++idx_ )
              if( (ptr_ = hash_->table_[idx_]) )
                break;
          }
        }
        return( *this );
      }
      const_iterator operator++( int ) // post increment
      {
        const_iterator it( *this ); // use default copy const.
        ++(*this);
        return( it );
      }
      bool operator==( const const_iterator &it ) const
      {
        return( (ptr_ == it.ptr_) && (idx_ == it.idx_) );
      }
      bool operator!=( const const_iterator &it ) const
      {
        return( (ptr_ != it.ptr_) || (idx_ != it.idx_) );
      }
      friend class stateHash_t;
    };
    const const_iterator begin( void ) const
    {
      return( const_iterator( this, 0 ) );
    }
    const const_iterator end( void ) const
    {
      return( const_iterator( this, 1 ) );
    }
};

#endif // HASH_H
