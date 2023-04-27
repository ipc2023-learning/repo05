#ifndef EXTERNAL_DET_PLANNER_INTERFACE_H
#define EXTERNAL_DET_PLANNER_INTERFACE_H

#include <iostream>
#include <string>
#include <stdlib.h>
#include <vector>
#include <list>
#include <unordered_map>

#include "../mgpt/actions.h"
#include "../mgpt/effects.h"
#include "../mgpt/global.h"
#include "../../ssps/policy.h"
#include "../mgpt/states.h"


// Action name and a vector with its parameters
class DetPlannerParsedAction : public std::pair<std::string, std::vector<std::string> > {
 public:
  std::string const& name() const { return first; }
  std::string& name() { return first; }
  size_t arity() const { return second.size(); }
  std::string const& parameter(size_t i) const { return second[i]; }
  std::string& parameter(size_t i) { return second[i]; }
  void pushBackArgument(std::string arg) { second.push_back(arg); }
};


// Lower case string
class DetPlannerUnparsedAction : public std::string {
 public:
  // This constructor makes sure that this string will always be lower case
  DetPlannerUnparsedAction(char const* c) {
    std::string s(c);
    for (size_t i = 0; i < s.length(); ++i)
      s[i] = std::tolower(s[i]);
    this->assign(s);
  }
};


// Compare two strings ignoring the case
struct eqcasestr {
  bool operator()(std::string s1, std::string s2) const {
//    std::cout << "=== strcasecmp('" << s1 << "', '" << s2 << "') = "
//      << strcasecmp(s1.c_str(), s2.c_str()) << std::endl;
//    std::cout.flush();
    return (strcasecmp(s1.c_str(), s2.c_str()) == 0);
  }
};


// Hashing function for strings ignoring their case
struct hash_case_str  {
  std::hash<std::string> h;
  size_t operator()(std::string const& s) const {
    std::string t(s);
    for (size_t i=0; i < t.length(); ++i)
  		t[i] = std::tolower(t[i]);
    return h(t);
  };
};


// Structure to keep track of the probability, effect and original action
// schema of each new deterministic action
class DetActionInfo {
 public:
  double p;
  Effect const* det_eff;
  ActionSchema const* action_schema;

  DetActionInfo() : p(double(-1)), det_eff(NULL), action_schema(NULL) { }
  DetActionInfo(double q, Effect const* e, ActionSchema const* a)
    : p(q), det_eff(e), action_schema(a) { }
};


// Hash from strings (ignoring the case) representing the name of the new
// deterministic action (for example, pick-up_1) to DetActionInfo
typedef std::unordered_map<std::string, DetActionInfo,
                                    hash_case_str, eqcasestr> ActionNameToInfo;


enum DetPlannerReturnType {SUCCESS = 0, NO_PLAN, TIMEOUT};


class ExternalDetPlannerInterface {
 public:
  ExternalDetPlannerInterface(problem_t const& problem, size_t timeout_in_secs,
      DeterminizationType det_type);
  virtual ~ExternalDetPlannerInterface();

  static ExternalDetPlannerInterface* createInterface(problem_t const& problem,
      size_t timeout_in_secs, DeterminizationType det_type,
      std::string det_planner);

  DetPlannerReturnType lengthPlanFrom(state_t const& s, size_t& length)
   { return _lengthAndLikelihoodPlanFrom(s, length); }

  DetPlannerReturnType lengthAndLikelihoodPlanFrom(state_t const& s, size_t& length,
     double& likelihood)
   { return _lengthAndLikelihoodPlanFrom(s, length, &likelihood); }

  /*
   * Fills state_trace and action_trace with the execution trace that FF
   * generates using the current determinization. This trace is such that
   * action_trace[i] is executed at state_trace[i-1].second resulting in
   * state_trace[i].second and the likelihood of reaching
   * state_trace[i].second from the initial_state following action_trace[0:i]
   * is state_trace[i].first. The execution trace of following the
   * action_trace is:
   *    initial_state -> action_trace[0] -> state_trace[0].second
   *      -> action_trace[1] -> state_trace[1].second -> action_trace[2]
   *      -> ... -> action_trace[end] -> state_trace[end].second = goal
   * If likelihood_cutoff is greater than 0, then the trace is stopped when the
   * plan likelihood is less than likelihood_cutoff
   */
  DetPlannerReturnType planFrom(state_t const& s,
     std::vector<std::pair<double, state_t> >& state_trace,
     std::vector<action_t const*>* action_trace,
     double likelihood_cutoff = 0.0);

  // This method writes in pi the actions prescribed by FF. If pi(s) is
  // already defined, it will be overwrited
  DetPlannerReturnType partialPolicyFrom(state_t const& s, DetPolicyIface& pi,
     double likelihood_cutoff = 0.0);

 protected:
  /*
   * Domain Determinization Methods
   */

  // Prints the name, parameters and preconditions of the given action schema
  void printActionSchemaHeaderAsPPDDL(std::ostream& os,
      ActionSchema const& action_schema, std::string const& name) const;
  // Different types of determinization
  void printDomainDeterminization(std::ostream& os, DeterminizationType det_type);
  void printAllOutcomeDetDomain(std::ostream& os)
    { printDomainDeterminization(os, ALL_OUTCOMES); }
  void printMostLikelyDetDomain(std::ostream& os)
    { printDomainDeterminization(os, MOST_LIKELY_OUTCOMES); }
  // Print the problem, i.e., objects, initial state and goal.
  void printProblem(std::ostream& os, state_t const& initial_state) const;



  /* This method wraps the method bellow (_run) and collects statistics */
  DetPlannerReturnType run(state_t const& s,
      std::vector<DetPlannerUnparsedAction>& plan)
  {
    total_calls_++;
//    START_TIMING("det_planner_exec");
    DetPlannerReturnType r = _run(s, plan);
//    STOP_TIMING("det_planner_exec");
    return r;
  }

  /* This method runs the deterministic planner and get the strings corresponding
   * to the plan found */
  virtual DetPlannerReturnType _run(state_t const& s,
                              std::vector<DetPlannerUnparsedAction>& plan) = 0;

  /* Simple method to parse the string representing an action into name and
   * arguments. This method assumes that the strings are in the form:
   * (action-name p_1 p_2 p_3 ... p_k)
   * where p_i is the ith parameter of the action.
   */
  DetPlannerParsedAction parseAction(DetPlannerUnparsedAction a) const;

  /*** Methods to process the unparsed plan from FF into something meaningful*/
  double planProbability(std::vector<DetPlannerUnparsedAction>& plan) const;

  DetPlannerReturnType _lengthAndLikelihoodPlanFrom(state_t const& s,
      size_t& length, double* likelihood = NULL);

  void executionTrace(state_t const& initial_state,
      std::vector<DetPlannerUnparsedAction> const& plan,
      std::vector<std::pair<double, state_t> >& state_trace,
      std::vector<std::string>* actionXML_trace,
      std::vector<action_t const*>* actionT_trace,
      double likelihood_cutoff = 0.0) const;

  void keepTmpFiles() { delete_files_ = false; }

//  problem_t const& problem() const { return problem_; }
  size_t timeoutInSecs() const { return timeout_in_secs_; }
  std::string tmpDir() const { return tmp_dir_; }
  std::string domainFile() const { return domain_file_; }
//  ActionNameToInfo& nameToEffect() { return name_to_effect_; }

 private:
  problem_t const& problem_;
  bool delete_files_;
  size_t total_calls_;
  size_t timeout_in_secs_;
  std::string tmp_dir_;
  std::string domain_file_;
  ActionNameToInfo name_to_effect_;
};

#endif // EXTERNAL_DET_PLANNER_INTERFACE_H
