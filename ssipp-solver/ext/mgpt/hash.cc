#include "global.h"
#include "actions.h"
#include "problems.h"
#include "hash.h"
#include "../../utils/die.h"
#include "../../ssps/prob_dist_state.h"
#include "../../ssps/ssp_iface.h"

#include <math.h>
#include <limits.h>
#include <values.h>
#include <sstream>



void hashEntry_t::update(double value)  {
//#ifdef DEBUG_TRACE
//  std::cout << "Update: ";
//  state()->full_print(std::cout, gpt::problem, false, true);
//  std::cout << std::endl;
//#endif

#ifdef DEBUG_HASH_UPDATE
  std::cout << "<hashEntry_t>::update: " << *state() << " from " << value_;
#endif
  if (hash_)
    hash_->increment_counter();
  value_ = GPTMIN(value,(double)gpt::dead_end_value.double_value());
#ifdef DEBUG_HASH_UPDATE
  std::cout << " to " << value_ << std::endl;
#endif
}

  static unsigned
prime( unsigned n )
{
  register unsigned i;
  static bool initialized = false;
  static unsigned p, current, size, *primes;

  if( !initialized )
  {
    p = 0;
    current = 2;
    size = gpt::initial_hash_size;
    primes = (unsigned*)calloc( size, sizeof(unsigned) );
    initialized = true;
  }

  do {
    unsigned bound = (unsigned)ceil( sqrt( (double)current ) );
    for( i = 0; (i < p) && (primes[i] < bound); ++i )
      if( current % primes[i] == 0 ) break;

    if( (i == p) || (primes[i] == bound) )
    {
      if( p == size )
      {
        size = size>>1;
        primes = (unsigned*)realloc( primes, size * sizeof(unsigned) );
      }
      primes[p++] = current;
      current += (p==1?1:2);
    }
    else
      current += 2;
  } while( primes[p-1] < n );

  return( primes[p-1] );
}


/*******************************************************************************
 *
 * hash
 *
 ******************************************************************************/

hash_t::hash_t( unsigned dimension, heuristic_t &heuristic )
{
  ext_ = false;
  size_ = 0;
  heuristic_ = &heuristic;
  dimension_ = prime( dimension );
  table_ = (hashEntry_t**)calloc( dimension_, sizeof(hashEntry_t*) );
  number_ = (unsigned*)calloc( dimension_, sizeof(unsigned) );

  if( gpt::verbosity >= 300 )
    std::cout << "[hash]: new hash: dimension = " << dimension_ << std::endl;
}

hash_t::~hash_t() {
//  std::cout << "V(s0) = " << this->find(gpt::problem->get_initial_state())->value()
//            << std::endl;
#ifdef DUMP_HASH
  for (hash_t::const_iterator hi = begin(); hi != end(); ++hi) {
    if (*hi == NULL) break;  // BUG in the hash_t::const_iterator
    /*
    std::cout << *(*hi)->state();
    if ((*hi)->bits() & 0x1) {
      std::cout << "  SOLVED!" << std::endl;
    } else {
      std::cout << " not solved" << std::endl;
    }
    */
    std::cout << "V(" << (*hi)->state()->toStringFull(gpt::problem, false, false)
              << ") = " << (*hi)->value() << std::endl;
  }   // for hi
#endif

  for( unsigned i = 0; i < dimension_; ++i )
    for( hashEntry_t *ptr = table_[i]; ptr != NULL; )
    {
      hashEntry_t *next = ptr->next_;
      delete ptr;
      ptr = next;
    }
  free( table_ );
  free( number_ );

  if( gpt::verbosity >= 300 )
    std::cout << "[hash]: deleted" << std::endl;

//  std::cout << "HASH: total qValue calls: " << total_q_calls_ << std::endl
//            << "      total update calls: " << get_update_counter() << std::endl;
}

  void
hash_t::rehash( void )
{
  unsigned tdimension = dimension_;
  dimension_ = prime( dimension_<<1 );
  hashEntry_t **ttable = table_;
  table_ = (hashEntry_t**)calloc( dimension_, sizeof(hashEntry_t*) );
  free( number_ );
  number_ = (unsigned*)calloc( dimension_, sizeof(unsigned) );

  if( gpt::verbosity >= 300 )
  {
    std::cout << "[hash]: rehash: size = " << size_
      << ", dimension = " << dimension_ << std::endl;
  }

  size_ = 0;
  hashEntry_t *ptr, *tmp;
  for( unsigned i = 0; i < tdimension; ++i )
    for( ptr = ttable[i]; ptr != NULL; ptr = tmp )
    {
      tmp = ptr->next_;
      insert( ptr );
    }

  free( ttable );
}

unsigned
hash_t::diameter( void ) const
{
  unsigned result = 0;
  for( unsigned i = 0; i < dimension_; ++i )
    result = (number_[i] > result ? number_[i] : result);
  return( result );
}

void hash_t::print( std::ostream &os, SSPIface const& ssp) const {
  os << "<hash>: table begin" << std::endl;
  for( unsigned i = 0; i < dimension_; ++i )
  {
    if( table_[i] ) os << "  bucket[" << i << "] = { ";
    for( hashEntry_t *ptr = table_[i]; ptr != NULL; ptr = ptr->next_ )
    {
      os << "(" << ptr << ":" << ptr->bits() << ":";
//      if( gpt::verbosity >= 450 )
        ptr->state()->full_print(os, gpt::problem);
//      else
//        os << ptr->state()->hash_value() << ":";
      os << ":" << ptr->value() << ") ";
    }
    if( table_[i] ) os << "}" << std::endl;
  }
  os << "<hash>: table end" << std::endl;
}

void hash_t::dump(std::ostream &os) const
{
  // this was going to be different to hash_t::print… but now it's not?
  // IDK, I was originally looking at stateHash_t::print when I wrote this
  // ::dump function; I didn't realise that hash_t::dump was already pretty
  // full-featured.
  os << "<hash>: table begin for " << gpt::problem->name() << std::endl;
  for( unsigned i = 0; i < dimension_; ++i ) {
    if( table_[i] ) {
      os << "  bucket[" << i << "] = { " << std::endl;
    }
    for( hashEntry_t *ptr = table_[i]; ptr != NULL; ptr = ptr->next_ ) {
      os << "    @" << ptr << ": ";
      ptr->state()->full_print(os, gpt::problem);
      os << std::endl;
      os << "      -->" << ptr->state()->hash_value() << std::endl;
    }
    if( table_[i] ) {
      os << "}" << std::endl;
    }
  }
  os << "<hash>: table end" << std::endl;
}



/*******************************************************************************
 *
 * state hash
 *
 ******************************************************************************/

stateHash_t::stateHash_t( unsigned dimension )
{
  size_ = 0;
  dimension_ = prime( dimension );
  table_ = (stateHashEntry_t**)calloc( dimension_, sizeof(stateHashEntry_t*) );
  number_ = (unsigned*)calloc( dimension_, sizeof(unsigned) );

  if( gpt::verbosity >= 300 )
    std::cout << "[state-hash]: new hash: dimension = " << dimension_ << std::endl;
}

stateHash_t::~stateHash_t()
{
  for( unsigned i = 0; i < dimension_; ++i )
    for( stateHashEntry_t *ptr = table_[i]; ptr != NULL; )
    {
      stateHashEntry_t *next = ptr->next_;
      delete ptr;
      ptr = next;
    }
  free( table_ );
  free( number_ );

  if( gpt::verbosity >= 300 )
    std::cout << "[state-hash]: deleted" << std::endl;
}

  void
stateHash_t::rehash( void )
{
  unsigned tdimension = dimension_;
  dimension_ = prime( dimension_<<1 );
  stateHashEntry_t **ttable = table_;
  table_ = (stateHashEntry_t**)calloc( dimension_, sizeof(stateHashEntry_t*) );
  free( number_ );
  number_ = (unsigned*)calloc( dimension_, sizeof(unsigned) );

  if( gpt::verbosity >= 300 )
  {
    std::cout << "[state-hash]: rehash: size = " << size_
      << ", dimension = " << dimension_ << std::endl;
  }

  size_ = 0;
  stateHashEntry_t *ptr, *tmp;
  for( unsigned i = 0; i < tdimension; ++i )
    for( ptr = ttable[i]; ptr != NULL; ptr = tmp )
    {
      tmp = ptr->next_;
      insert( ptr );
    }

  free( ttable );
}

unsigned
stateHash_t::diameter( void ) const
{
  unsigned result = 0;
  for( unsigned i = 0; i < dimension_; ++i )
    result = (number_[i] > result ? number_[i] : result);
  return( result );
}

void
stateHash_t::print( std::ostream &os, const problem_t &problem ) const
{
  os << "<state-hash>: table begin" << std::endl;
  for( unsigned i = 0; i < dimension_; ++i )
  {
    if( table_[i] ) os << "  bucket[" << i << "] = { ";
    for( stateHashEntry_t *ptr = table_[i]; ptr != NULL; ptr = ptr->next_ )
      os << "(" << ptr << ":" << ptr->state()->hash_value() << ") ";
    if( table_[i] ) os << "}" << std::endl;
  }
  os << "<state-hash>: table end" << std::endl;
}

void stateHash_t::dump(std::ostream &os) const {}
