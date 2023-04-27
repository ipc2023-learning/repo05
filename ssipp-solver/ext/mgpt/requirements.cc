#include <iostream>
#include <sstream>
#include "requirements.h"

  Requirements::Requirements()
: strips(true), typing(false), negative_preconditions(false),
  disjunctive_preconditions(false), equality(false),
  existential_preconditions(false), universal_preconditions(false),
  conditional_effects(false), fluents(false), probabilistic_effects(false),
  rewards(false)
{
}

  void
Requirements::quantified_preconditions( void )
{
  existential_preconditions = true;
  universal_preconditions = true;
}

  void
Requirements::adl( void )
{
  strips = true;
  typing = true;
  negative_preconditions = true;
  disjunctive_preconditions = true;
  equality = true;
  quantified_preconditions();
  conditional_effects = true;
}

std::string Requirements::asPDDL(bool ignore_prob, bool ignore_rewards) const {
  std::ostringstream ost;
  ost << "(:requirements";
  if (strips && typing && negative_preconditions && disjunctive_preconditions
      && equality && existential_preconditions && universal_preconditions
      && conditional_effects)
    ost << " :adl";
  else {
    if (strips) ost << " :strips";
    if (typing) ost << " :typing";
    if (negative_preconditions) ost << " :negative_preconditions";
    if (disjunctive_preconditions) ost << " :disjunctive_preconditions";
    if (equality) ost << " :equality";
    if (existential_preconditions) ost << " :existential_preconditions";
    if (universal_preconditions) ost << " :universal_preconditions";
    if (conditional_effects) ost << " :conditional_effects";
    if (fluents) ost << " :fluents";
  }

  if (probabilistic_effects && !ignore_prob)
    ost << " :probabilistic-effects";
  if (rewards && !ignore_rewards)
    ost << " :rewards";

  ost << ")";
  std::string s = ost.str();
  if (s == "(:requirements)")
    return "";
  else
    return s;
}
