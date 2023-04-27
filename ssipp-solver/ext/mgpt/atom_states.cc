#include <assert.h>
#include <math.h>
#include <sstream>
#include <stdlib.h>

#include "global.h"
#include "domains.h"
#include "expressions.h"
#include "formulas.h"
#include "functions.h"
#include "hash.h"
#include "problems.h"
#include "states.h"
#include "../../utils/die.h"
#include "../../ssps/prob_dist_state.h"

/*******************************************************************************
 *
 * state
 *
 ******************************************************************************/

enum { CLEAR = 0x0, OPEN = 0x1, CLOSED = 0x2 };
std::unique_ptr<stateHash_t> state_t::state_hash_ = nullptr;
bool state_t::state_space_generated_ = false;
size_t state_t::size_ = 0;

bool state_t::holds(Atom const& atom) const {
  return holds(problem_t::atom_hash_get(atom));
}

bool state_t::add(Atom const& atom) {
  return add(problem_t::atom_hash_get(atom));
}

bool state_t::clear(Atom const& a) {
  return clear(problem_t::atom_hash_get(a));
}

/*****************************************************************************/


void state_t::initialize( const problem_t &problem )
{
  state_hash_ = std::make_unique<stateHash_t>( gpt::initial_hash_size );
  size_ = (unsigned)ceil( (double)problem.number_atoms() / 32 );

  if( gpt::verbosity >= 300 )
    std::cout << "[state]: size = " << size_ << std::endl;
}

void state_t::statistics( std::ostream &os )
{
  os << "[state-hash]: number states = " << state_hash_->size() << std::endl;
}

  const state_t*
state_t::get_state( const state_t &state )
{
  return( state_hash_->get( state )->state() );
}

void state_t::generate_state_space( const problem_t &problem, hash_t &hash_table,
    std::deque<hashEntry_t*> &space )
{
  // check if state space has been already generated
  if (state_space_generated_) {
    stateHash_t::const_iterator it = state_hash_->begin();
    while (it != state_hash_->end()) {
      hashEntry_t *entry = hash_table.get(*(*it++));
      space.push_back(entry);
    }
    return;
  }

  std::vector<state_t> initial_states;
  initial_states.push_back(problem.get_initial_state());
  generateStateSpaceFrom(problem, hash_table, space, initial_states, 0);
  state_space_generated_ = true;
}


void state_t::generateStateSpaceFrom(const problem_t &problem,
      hash_t &hash_table, std::deque<hashEntry_t*> &space,
      std::vector<state_t> const& initial_states,
      size_t max_depth)
{
  actionList_t::const_iterator ai;
  std::deque<std::pair<size_t, hashEntry_t*> > Q;

  std::set<hashEntry_t*,std::less<hashEntry_t*> > open;
  std::set<hashEntry_t*,std::less<hashEntry_t*> > closed;


  for (std::vector<state_t>::const_iterator ii = initial_states.begin();
      ii != initial_states.end(); ++ii)
  {
    hashEntry_t *entry = hash_table.get(*ii);
    open.insert(entry);
    Q.push_front(std::make_pair(0, entry));
  }

  // FWT: ProbDistState is not static here because this method should be execute
  // very few times per run (usually at most once).
  ProbDistState pr;

  // generate state space
  while (!Q.empty()) {
    std::pair<size_t, hashEntry_t*> front = Q.front();
    size_t cur_depth = front.first;
    hashEntry_t *tmp, *entry = front.second;
    Q.pop_front();
    //entry->set_bits(CLOSED);
    closed.insert(entry);
    space.push_front(entry);

    if (space.size() % 10000 == 0) {
      gpt::checkDeadline();
    }

    if (gpt::verbosity >= 450) {
      std::cout << "SPACE: state ";
      entry->state()->full_print(std::cout, &problem);
      std::cout << std::endl;
    }

    if (max_depth > 0 && max_depth == cur_depth)
      continue;

    if (problem.isGoal(*(entry->state())))
      continue;

    for (ai = problem.actionsT().begin(); ai != problem.actionsT().end(); ++ai)
      if ((*ai)->enabled(*entry->state())) {
        problem.expand(**ai, *entry->state(), pr);
        for (auto const& ip : pr) {
          state_t const& s_prime = ip.event();
          if (!(tmp = hash_table.find(s_prime))) {
            tmp = hash_table.insert(s_prime);
            //tmp->set_bits(OPEN);
            open.insert(tmp);
            Q.push_front(std::make_pair(cur_depth+1, tmp));
          }
          else if ((open.find(tmp) == open.end()) && (closed.find(tmp) == closed.end())) { //if(!(tmp->bits() & (OPEN|CLOSED))) {
            // tmp->set_bits(OPEN);
            open.insert(tmp);
            Q.push_front(std::make_pair(cur_depth+1, tmp));
          }
        }
      }
  }

  if (gpt::verbosity >= 450)
    std::cout << "SPACE: [no-more]" << std::endl;

  // clear bits
//  for (std::deque<hashEntry_t*>::iterator di = space.begin();
//      di != space.end(); ++di)
//    (*di)->set_bits(CLEAR);
}

state_t::state_t(const atomList_t &alist) : data_(new unsigned[size_]()) {
  notify(this, "state_t::state_t(const atomList_t&)");
  for (size_t i = 0; i < alist.size(); ++i)
    add(alist.atom(i));
#ifdef CACHE_ISGOAL_CALLS
  isGoal_ = 0;
#endif
}


state_t::state_t(std::string str, bool use_atom_index)
  : data_(new unsigned[size_]())
{
#ifdef CACHE_ISGOAL_CALLS
  isGoal_ = 0;
#endif
  if (use_atom_index) {
    std::deque<std::string> active_atoms = splitString(str, " ");
    for (std::string const& id : active_atoms) {
      if (!id.empty()) {
        ushort_t atom = atoi(id.c_str());
        add(atom);
      }
    }
  }
  else {
    std::deque<std::string> active_atoms = splitString(str, ",");
    TermList term_parameters;
    for (std::string& id : active_atoms) {
      trim_string(id);
      if (id.length() == 0) { continue; }

      std::deque<std::string> parts = splitString(id, " ");
      std::pair<Predicate, bool> p =
             gpt::problem->domain().predicates().find_predicate(parts.front());
      assert(p.second);
      Predicate atom_predicate = p.first;
      // Parsing terms
      term_parameters.clear();
      parts.pop_front();
      for (std::string const& pi : parts) {
        std::pair<Object, bool> o = gpt::problem->terms().find_object(pi);
        assert(o.second);
        // TODO(fwt): add type verification
        term_parameters.push_back(o.first);
      }
      size_t n = term_parameters.size();
      assert(gpt::problem->domain().predicates().arity(atom_predicate) == n);
      this->add(Atom::make_atom(atom_predicate, term_parameters));
    }
  }
}


size_t state_t::code() const {
  size_t c = 0;
  for (state_t::const_predicate_iterator ai = predicate_begin();
      ai != predicate_end(); ++ai)
  {
    DIE((*ai) % 2 == 0, "State has odd atom!", -1);
    c = c | (1 << (*ai / 2));
  }
  return c;
}

std::string state_t::toString(bool print_braces) const {
//  return toStringFull(gpt::problem, false, print_braces);
  std::ostringstream ost;
  if (print_braces) { ost << "["; }

  bool print_sep = false;
  for (state_t::const_predicate_iterator ai = predicate_begin();
      ai != predicate_end(); ++ai)
  {
    if (print_sep) { ost << " "; }
    else { print_sep = true; }
    ost << *ai;
  }

  if (print_braces) { ost << "]"; }

  return ost.str();
}

std::string state_t::toStringFull(problem_t const* problem, bool print_idx,
    bool print_braces) const
{
  std::ostringstream ost;
  state_t::const_predicate_iterator ai;

  if (print_braces)
    ost << "[";

  for( ai = predicate_begin(); ai != predicate_end(); ++ai )
  {
    ost << " ";
    const Atom *atom = problem_t::atom_inv_hash_get( *ai );
    if (print_idx)
      ost << *ai << ":";

    if (*ai % 2)
      ost << "(not ";

    problem->print(ost, *atom);
    if (*ai % 2)
      ost << ")";
  }
  if (print_braces)
    ost << " ]";
  return ost.str();
}

void state_t::debug_print(problem_t const* problem, hash_t* h) const
{
  state_t::const_predicate_iterator ai;

  std::cout << "[";
  for( ai = predicate_begin(); ai != predicate_end(); ++ai )
  {
    std::cout << " ";
    Atom const* atom = problem_t::atom_inv_hash_get(*ai);
    if (*ai % 2) std::cout << "(not";
    problem->print(std::cout, *atom);
    if (*ai % 2) std::cout << ")";
  }
  if (h != NULL) {
    hashEntry_t const* node = h->get(*this);
    DIE(node != NULL, "Hash entry not found", 108);
    std::cout << ", " << node->value();
  }
  std::cout << " ]";
}

void state_t::printXML( std::ostream &os, bool goal ) const
{
  os << "<state>";
  if( goal ) os << "<is-goal/>";

  os << "</state>";
}

void state_t::send( std::ostream& os ) const
{
}
