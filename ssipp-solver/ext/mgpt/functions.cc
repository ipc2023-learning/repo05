#include "functions.h"

FunctionTable::~FunctionTable()
{
  for( std::vector<TypeList*>::const_iterator pi = parameters_.begin(); pi != parameters_.end(); ++pi )
    delete *pi;
}

  Function
FunctionTable::add_function( const std::string& name )
{
  // FWT: Checking if the function already exists. This fixes the problems of
  // having duplicated goal-probability functions (legacy function that was
  // removed) and is also a 'hack-ish' solution for the enforcement of a domain
  // always having a reward function and it is the first function. See Domain
  // constructor
  for (int i = 0; i < (int) names_.size(); ++i) {
    if (names_[i] == name) {
      return i;
    }
  }

  // The function doesn't exist, so creating a new one one
  Function function = last_function() + 1;
  names_.push_back(name);
  functions_.insert(std::make_pair(name, function));
  parameters_.push_back(new TypeList());
  static_functions_.insert(function);
  return function;
}

std::pair<Function,bool>
FunctionTable::find_function( const std::string& name ) const
{
  std::map<std::string,Function>::const_iterator fi = functions_.find( name );
  if( fi != functions_.end() )
    return( std::make_pair( (*fi).second, true ) );
  else
    return( std::make_pair( 0,false ) );
}

void
FunctionTable::print_function( std::ostream& os, Function function ) const
{
  os << names_[function];
}
