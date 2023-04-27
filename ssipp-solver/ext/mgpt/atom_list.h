#ifndef ATOM_LIST_H
#define ATOM_LIST_H

#include <iostream>
#include <list>

#include "../../utils/die.h"
#include "global.h"

#ifndef DEBUG_ATOMLIST
#define DEBUG_ATOMLIST 0
#endif

class state_t;

/*******************************************************************************
 *
 * atom list
 *
 ******************************************************************************/

class atomList_t {
  size_t size_;        // Number of ushort allocated
  ushort_t *data_;     // Pointer to the first item of the array (of size size_)
  ushort_t *data_ptr_; // Pointer to the first unused space in the data_ array?

  public:
  atomList_t() : size_(0), data_(0), data_ptr_(0)
  {
    notify( this, "atomList_t::atomList_t()" );
//    _D(DEBUG_ATOMLIST, std::cout << "atomList_t empty ctor for "
//                                 << this << std::endl;);

  }
  atomList_t( const atomList_t &alist );
  atomList_t( const ushort_t *array, size_t sz );
  ~atomList_t() { if( data_ ) free( data_ ); }

  unsigned hash_value( void ) const;
  bool equal( const ushort_t *array, size_t sz ) const;
  void intersect( const atomList_t &alist );
  bool empty_intersection( const atomList_t &alist ) const;
  size_t intersection_size( const atomList_t &alist ) const;

  // Number of atoms in the list. Note that size() <= size_ since the array
  // for storing atoms might not be fully used
  size_t size() const { return data_ptr_ - data_; }

  ushort_t atom( size_t i ) const { return( data_[i] ); }
  bool find( ushort_t atm ) const;
  void insert( ushort_t atm );
  void insert( const atomList_t &alist );
  void remove( ushort_t atm );
  void remove( const atomList_t &alist );
  bool holds( ushort_t atm, bool nprec = false ) const
  {
    if( nprec )
      return( find( atm ) );
    else
      return( atm%2 ? !find(atm) : find(atm) );
  }
  bool holds( const state_t &state, bool nprec = false ) const;
  bool holds( const atomList_t &alist, bool nprec = false ) const;
  bool contradiction( void ) const
  {
    for( size_t i = 0; i+1 < size(); ++i )
      if( (data_[i] % 2 == 0) && (data_[i]+1 == data_[i+1]) ) return( true );
    return( false );
  }
  void clear( void ) { data_ptr_ = data_; }
  void print(std::ostream &os, bool print_names = false) const;
  bool operator==( const atomList_t &alist ) const;
  atomList_t& operator=( const atomList_t &effect );

  AtomBitset toAtomBitset() const {
    AtomBitset abs;
    for (size_t i = 0; i < size(); ++i) {
      abs[atom(i)] = true;
    }
    return abs;
  }
};

  inline std::ostream&
operator<<( std::ostream &os, const atomList_t& alist )
{
  alist.print( os );
  return( os );
}

inline bool
atomList_t::find( ushort_t atm ) const
{
  for( size_t i = 0; i < size(); ++i )
    if( atom( i ) == atm ) return( true );
  return( false );
}

inline void atomList_t::insert(ushort_t atm) {
  size_t i;
  for (i = 0; i < size() && atom(i) < atm; ++i);

  if (i == size() || atom(i) > atm) {
    if (i == size()) {
      if (!data_ || data_ptr_ == &data_[size_]) {
        size_ = (!data_ ? 1 : size_ << 1);
        ushort_t* ndata_ = (ushort_t*) realloc(data_, size_*sizeof(ushort_t));
        data_ptr_ = (!data_ ? ndata_ : &ndata_[size()]);
        data_ = ndata_;
      }
      *data_ptr_ = atm;
      ++data_ptr_;
    }
    else {
      DIE(data_ != NULL, "Expecting non-null data_", 130);
      if (data_ptr_ == &data_[size_]) {
        size_ = size_ << 1;
        ushort_t *ndata_ = (ushort_t*)realloc(data_, size_ * sizeof(ushort_t));
        data_ptr_ = (!data_ ? ndata_ : &ndata_[size()]);
        data_ = ndata_;
      }
      // Shifting everyone greater than atm one position up
      for (int j = (int)size(); j > (int)i; --j) {
        data_[j] = data_[j-1];
      }
      data_[i] = atm;
      ++data_ptr_;
    }
  }
}

inline void atomList_t::remove(ushort_t atm) {
  size_t i;
  for( i = 0; (i < size()) && (atom( i ) != atm); ++i );
  if( i < size() )
  {
    for( size_t j = i; j < size(); ++j )
      data_[j] = data_[j+1];
    --data_ptr_;
  }
}


std::string atomToString(atom_t a);

/*******************************************************************************
 *
 * atom list list
 *
 ******************************************************************************/

class atomListList_t {
  size_t size_;
  atomList_t **data_, **data_ptr_;

  public:
  atomListList_t() : size_(0), data_(0), data_ptr_(0) { }
  ~atomListList_t() { if( data_ ) free( data_ ); }

  size_t size( void ) const { return( data_ptr_ - data_ ); }
  atomList_t& atom_list( size_t i ) { return( *data_[i] ); }
  const atomList_t& atom_list( size_t i ) const { return( *data_[i] ); }
  bool find( const atomList_t &alist ) const;
  void insert( atomList_t *alist );
  bool holds( const state_t &state, bool nprec = false ) const;
  bool holds(atomList_t const& atm_list, bool nprec = false) const;
  void clear( void ) { data_ptr_ = data_; }
  void print( std::ostream &os ) const;
  bool operator==( const atomListList_t &alist ) const;
  atomListList_t& operator=( const atomListList_t &alist );
};

void __powerset(std::set<atom_t> const& input_set,
              std::set<atom_t>::iterator it,
              std::list<atom_t>& tmp_list,
              std::vector<std::set<atom_t> >& result);

inline void powerset(std::set<atom_t> const& input_set,
    std::vector<std::set<atom_t> >& result)
{
  std::list<atom_t> tmp_list;
  __powerset(input_set, input_set.begin(), tmp_list, result);
}


#endif  // ATOM_LIST_H
