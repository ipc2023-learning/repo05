#ifndef REQUIREMENTS_H
#define REQUIREMENTS_H

class Requirements
{
  public:
    bool strips;
    bool typing;
    bool negative_preconditions;
    bool disjunctive_preconditions;
    bool equality;
    bool existential_preconditions;
    bool universal_preconditions;
    bool conditional_effects;
    bool fluents;
    bool probabilistic_effects;
    bool rewards;

    Requirements();
    void quantified_preconditions( void );
    void adl( void );
    std::string asPDDL(bool ignore_prob = false, bool ignore_rewards = false) const;
};

#endif // REQUIREMENTS_H
