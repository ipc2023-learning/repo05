#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string.h>

#include "../mgpt/actions.h"
#include "../../utils/die.h"
#include "../../utils/exceptions.h"
#include "externalFFInterface.h"
#include "externalLamaInterface.h"
#include "../mgpt/global.h"
#include "../mgpt/hash.h"


// static
ExternalDetPlannerInterface* ExternalDetPlannerInterface::createInterface(
    problem_t const& problem, size_t timeout_in_secs,
    DeterminizationType det_type, std::string det_planner)
{
  if (det_planner == "ff" || det_planner == "FF") {
    return new ExternalFFInterface(problem, timeout_in_secs, det_type);
  }
  else if (det_planner == "lama" || det_planner == "LAMA") {
    return new ExternalLamaInterface(problem, timeout_in_secs, det_type);
  }
  else {
    std::cerr << "Deterministic planner '" << det_planner << "' is unknown..."
              << std::endl;
    exit(-1);
  }
}



/*
 * Constructor
 */
ExternalDetPlannerInterface::ExternalDetPlannerInterface(problem_t const& problem,
    size_t timeout_in_secs, DeterminizationType det_type)
  : problem_(problem), delete_files_(true), total_calls_(0),
    timeout_in_secs_(timeout_in_secs)
{
  std::string template_str1 = std::string(gpt::tmp_dir) +
                              "/externalDetPlannerInterface_XXXXXX";
  char* template_str2 = strdup(template_str1.c_str());
  // FIXME(fwt): mkdtemp can return NULL...
  tmp_dir_ = std::string(mkdtemp(template_str2)) +  "/";
  free(template_str2);

  domain_file_ = tmp_dir_ + "domain.pddl";
  std::ofstream domain;
  domain.open(domain_file_.c_str());
  DIE(domain.is_open(), "ERROR opening file", 99);
  printDomainDeterminization(domain, det_type);
  domain.close();
}


/*
 * Destructor
 */
ExternalDetPlannerInterface::~ExternalDetPlannerInterface() {
#ifdef EXTERNAL_FF_KEEP_FILES
#else
  if (delete_files_) {
    std::ostringstream cmd;
    cmd << "rm -rf " << tmp_dir_;
    int rv = system(cmd.str().c_str());
    if (!rv) {
      std::cerr << "Unable to delete tmp dir (= '" << tmp_dir_ << "') of "
        << "the external det planner." << std::endl;
    }
  }
#endif
}


/******************************************************************************
 *
 * Domain determinization methods
 *
 ******************************************************************************/

/*
 * printActionSchemaHeaderAsPPDDL
 */
void ExternalDetPlannerInterface::printActionSchemaHeaderAsPPDDL(std::ostream& os,
    ActionSchema const& action_schema, std::string const& name) const {

  problem_t const& p = *gpt::problem;
  os << "  (:action " << name  << std::endl;

  if (action_schema.arity() > 0) {
    os << "   :parameters (";
    for (size_t vi = 0; vi < action_schema.arity(); ++vi) {
      os << " ";
      p.domain().terms().print_term(os, action_schema.parameter(vi));
      os << " - ";
      p.domain().types().print_type(os,
          p.domain().terms().type(action_schema.parameter(vi)));
    }
    os << ")" << std::endl;
  }
  os << "   :precondition ";
  action_schema.precondition().print(os, p.domain().predicates(),
      p.domain().functions(), p.domain().terms());
  os << std::endl;
}


/*
 * printDomainDeterminization
 */
void ExternalDetPlannerInterface::printDomainDeterminization(std::ostream& os,
    DeterminizationType det_type)
{
  problem_t const& p = *gpt::problem;
  Domain const& d = p.domain();

//  std::cout << p << std::endl << std::endl << std::endl;

  /*** DEFINE ***/
  os << "(define (domain " << d.name() << ")" << std::endl;

  /*** REQUIREMENTS ***/
//  std::string req_str = d.requirements.asPDDL(true, true);
//  if (req_str != "")
//    os << "  " << req_str << std::endl;
  os << "  (:requirements :adl)" << std::endl;

  /*** TYPES ***/
  os << "  (:types";

  // ASSUMPTION(fwt): I'm assuming that the first type is always
  // object (the base type in the pddl that doesn't need to be declared)
  for (Type i = d.types().first_type() + 1;
      i <= d.types().last_type(); ++i)
  {
    os << " ";
    d.types().print_type(os, i);
    // subtypes... Ignoring for now
    for (Type j = d.types().first_type() + 1; j <= d.types().last_type(); ++j) {
      DIE(i == j || ! d.types().subtype(i, j), "Subtypes detected on printAsPPDDL", 171);
    }
  }
  os << ")" << std::endl; //types


  /*** CONSTANTS ***/
  if (d.terms().first_object() <= d.terms().last_object()) {
    os << "  (:constants";
    for (Object i = d.terms().first_object();
        i <= d.terms().last_object(); ++i)
    {
      os << "  ";
      d.terms().print_term(os, i);
      os << " - ";
      d.types().print_type(os, d.terms().type(i));
      if (d.terms().first_object() < d.terms().last_object())
        os << std::endl;
    }
    os << ")" << std::endl; // constants
  }

  /*** PREDICATES ***/
  os << "  (:predicates " << std::endl;
  for (Predicate i = d.predicates().first_predicate();
      i <= d.predicates().last_predicate(); ++i)
  {
    os << "    (";
    d.predicates().print_predicate(os, i);
    size_t arity = d.predicates().arity(i);
    for (size_t j = 0; j < arity; ++j) {
      os << " ?v" << j << " - ";
      d.types().print_type(os, d.predicates().parameter(i, j));
    }
    os << ")" << std::endl;
//    if( d.predicates().static_predicate( i ) )
//      os << " <static>";
  }
  os << "  )" << std::endl; // predicates


  /*** FUNCTIONS ***/
  // Ignoring them..
  for (Function i = d.functions().first_function();
      i <= d.functions().last_function(); ++i)
  {
    DIE(d.functions().name(i) == "reward",
        "Unexpected domain function in printAsPPDDL", 172);
  }

  /*** ACTIONS ***/

  gpt::external_ff_ignore_forall_w_prob_effects = true;
  for (ActionSchemaMap::const_iterator ai = d.actions().begin();
      ai != d.actions().end(); ++ai)
  {
    Effect const& flat_eff = ai->second->effect().flatten();
    ProbabilisticEffect const* flat_p_eff =
      dynamic_cast<ProbabilisticEffect const*>(&flat_eff);
    if (! flat_p_eff) {
      // This is not a probabilistic action
      printActionSchemaHeaderAsPPDDL(os, *(ai->second), ai->second->name());
      os << "   :effect ";
      flat_eff.print(os, d.predicates(), d.functions(),
          d.terms(), true);
      os << std::endl;
      os << "  )" << std::endl; // action
      Effect::register_use(&flat_eff);

      name_to_effect_[ai->second->name()] = DetActionInfo(1.0,
          &flat_eff, ai->second);

    } else {
      // PROBABILISTIC ACTION!
      std::set<size_t> most_likely_effs;
      // Finding the set of most likely effects
      if (det_type == MOST_LIKELY_OUTCOMES) {
        Rational max_p = 0;
        Rational p_noop = 1; // probability that nothing happens
        for (size_t ei = 0; ei < flat_p_eff->size(); ++ei) {
          Rational p_i = flat_p_eff->probability(ei);
          p_noop = p_noop - p_i;
          if (p_i > max_p) {
            most_likely_effs.clear();
            most_likely_effs.insert(ei);
            max_p = p_i;
          } else if (p_i == max_p) {
            most_likely_effs.insert(ei);
          }
        }
        if (p_noop > max_p) {
          // The NO-OP effect has more probability than any other effect,
          // therefore this action will be ignored
          most_likely_effs.clear();
        }
      }

      for (size_t ei = 0; ei < flat_p_eff->size(); ++ei) {

        if (det_type == MOST_LIKELY_OUTCOMES &&
            most_likely_effs.find(ei) == most_likely_effs.end())
        {
          // Ignoring this effect if the most likely outcome determinization
          // was chosen.
          continue;
        }

        std::ostringstream name_os, effect_os;

        // Getting the effect and checking if it's not empty
        flat_p_eff->effect(ei).print(effect_os, d.predicates(),
            d.functions(), d.terms(), true);
        std::string effect_str = effect_os.str();
        if (effect_str.find("(") != std::string::npos) {
          name_os << ai->second->name() << "_" << ei;
          std::string name = name_os.str();
          printActionSchemaHeaderAsPPDDL(os, *(ai->second), name);
          os << "   :effect " << effect_str << std::endl;
          os << "  )" << std::endl; // action

          Effect::register_use(&flat_p_eff->effect(ei));
          name_to_effect_[name] =
            DetActionInfo(flat_p_eff->probability(ei).double_value(),
              &flat_p_eff->effect(ei), ai->second);
        }
      }
    }
  }
  gpt::external_ff_ignore_forall_w_prob_effects = false;

  os << ")" << std::endl; //define
}


/*
 * printProblem
 */
void ExternalDetPlannerInterface::printProblem(std::ostream& os,
    state_t const& initial_state) const
{
  problem_t const& p = *gpt::problem;
  os << "(define (problem " << p.name() << ")" << std::endl;
  os << "  (:domain " << p.domain().name() << ")" << std::endl;

  /*** OBJECTS ***/
  if (p.terms().first_object() <= p.terms().last_object()) {
    os << "  (:objects";
    for (Object i = p.terms().first_object();
        i <= p.terms().last_object(); ++i)
    {
      os << "  ";
      p.terms().print_term(os, i);
      os << " - ";
      p.domain().types().print_type(os, p.terms().type(i));
      if (p.terms().first_object() < p.terms().last_object())
        os << std::endl;
    }
    os << ")" << std::endl;
  }

  /*** INITIAL STATE ***/
  os << "  (:init";
  // Static predicates
  for (AtomSet::const_iterator ai = p.init_atoms().begin();
      ai != p.init_atoms().end(); ++ai)
  {
    if (p.domain().predicates().static_predicate((*ai)->predicate())) {
      os << " ";
      (*ai)->print(os, p.domain().predicates(), p.domain().functions(),
                   p.terms());
    }
  }
  // Non-static predicates
  initial_state.full_print(os, gpt::problem, false, false);
  os << ")" << std::endl; // initial state

  /*** GOAL STATE ***/
  os << "  (:goal ";
  p.original_goal().print(os, p.domain().predicates(), p.domain().functions(), p.terms());
  os << ")" << std::endl;

  os << ")" << std::endl; // define
}


/*
 * parseAction
 */
DetPlannerParsedAction ExternalDetPlannerInterface::parseAction(
    DetPlannerUnparsedAction a) const
{
  DetPlannerParsedAction parsed_a;
  char* action = strdup(a.c_str());
  char* pch = strtok(action, " ()\r\n");
  parsed_a.name() = std::string(pch);
  pch = strtok(NULL, " ()\n\r");
  while (pch != NULL) {
    parsed_a.pushBackArgument(std::string(pch));
    pch = strtok (NULL, " ()\n\r");
  }
  free(action);
  return parsed_a;
}


/*
 * planProbability
 */
double ExternalDetPlannerInterface::planProbability(
    std::vector<DetPlannerUnparsedAction>& plan) const
{
  double p = 1.0;
  for (std::vector<DetPlannerUnparsedAction>::iterator ip = plan.begin();
      ip != plan.end(); ++ip)
  {
    DetPlannerParsedAction a = parseAction(*ip);
    ActionNameToInfo::const_iterator it = name_to_effect_.find(a.name());
    DIE(it != name_to_effect_.end(),
        "Action mapping not found in DetPlanner interface", 174);
    p = p * it->second.p;
  }
  return p;
}


/*
 * _lengthAndLikelihoodPlanFrom
 */
DetPlannerReturnType ExternalDetPlannerInterface::_lengthAndLikelihoodPlanFrom(state_t const& s,
    size_t& length, double* likelihood)
{
  std::vector<DetPlannerUnparsedAction> plan;
  DetPlannerReturnType retcode = run(s, plan);
  if (retcode == SUCCESS) {
#ifdef DEBUG_EXTERNAL_DET_PLANNER
    std::cout << "H_FF(";
    s.full_print(std::cout, &problem_);
    std::cout << ") = " << plan.size()
              << "  {Prob = " << planProbability(plan) << "}" << std::endl;
    for (size_t i = 0; i < plan.size(); ++i) {
      DetPlannerParsedAction a = parseAction(plan[i]);
      std::cout << "[" << i << "] " << a.name() << "[#args " << a.arity() << "](";
      for (size_t ip = 0; ip < a.arity(); ++ip)
        std::cout << a.parameter(ip) << ",";
      std::cout << ")" << std::endl;
    }
    std::cout << std::endl;
    // Testing the executionTrace
    std::vector<std::pair<double, state_t> > state_trace;
    std::vector<std::string> actionXML_trace;
    std::vector<action_t const*> actionT_trace;
    executionTrace(s, plan, state_trace, &actionXML_trace, &actionT_trace);
#endif

    length = plan.size();
    if (likelihood)
      (*likelihood) = planProbability(plan);
    return SUCCESS;
  }
  return retcode;
}


/*
 * executionTrace: fills state_trace, actionXML_trace and actionT_trace with
 * the execution trace that FF generates using the current determinization.
 * This trace is such that action_trace[i] is executed at state_trace[i-1]
 * resulting in state_trace[i]. That is:
 *    initial_state -> action_trace[0] -> state_trace[0] -> action_trace[1]
 *      -> state_trace[1] -> action_trace[2] -> ... -> action_trace[end]
 *        -> state_trace[end] = goal
 * If likelihood_cutoff is greater than 0, then the trace is stopped when the
 * plan likelihood is less than likelihood_cutoff
 */
void ExternalDetPlannerInterface::executionTrace(state_t const& initial_state,
      std::vector<DetPlannerUnparsedAction> const& plan,
      std::vector<std::pair<double, state_t> >& state_trace,
      std::vector<std::string>* actionXML_trace,
      std::vector<action_t const*>* actionT_trace,
      double likelihood_cutoff) const
{
  state_t next_s(initial_state);
  double cur_likelihood = 1.0;

#ifdef DEBUG_EXTERNAL_DET_PLANNER
  std::cout << "Plan trace:\n";
  std::cout << "  " << state_trace.size() << " ";
  next_s.full_print(std::cout, &problem_);
  std::cout << std::endl;
#endif

  for (std::vector<DetPlannerUnparsedAction>::const_iterator ip = plan.begin();
      ip != plan.end(); ++ip)
  {
    DetPlannerParsedAction parsed_a = parseAction(*ip);

//    std::cout << cur_likelihood << " -> ";
    /*
     * Getting the ActionSchema and Effect that represents the chosen action
     * by FF
     */
    ActionNameToInfo::const_iterator it = name_to_effect_.find(parsed_a.name());
    DIE(it != name_to_effect_.end(),
        "Action mapping not found in DetPlanner interface", 174);
    Effect const* eff = it->second.det_eff;
    ActionSchema const* action_schema = it->second.action_schema;
    cur_likelihood *= it->second.p;

    if (cur_likelihood < likelihood_cutoff) {
//      std::cout << " IGNORING {action_t::name = " << action_schema->name()
//        << "} BECAUSE cur_likelihood after applying it is "
//        << cur_likelihood << std::endl;
      break;
    }

    /*** Vars to obtain the actionXML and actionT ***/
    std::string actionXML;
    std::ostringstream ostXML;

    if (actionXML_trace || actionT_trace) {
      ostXML << "<action><name>" << action_schema->name() << "</name>";
    }

#ifdef DEBUG_EXTERNAL_DET_PLANNER
    std::cout << "\t\t" << action_schema->name() << "(";
#endif

    /*
     * Building the substitution map to instantiate the effect applied by FF
     */
    SubstitutionMap subst;
    for (size_t ia = 0; ia < action_schema->arity(); ++ia) {
      std::pair<Object,bool> o = problem_.terms().find_object(
                                                       parsed_a.parameter(ia));
      DIE(o.second, "Action mapping not found in DetPlanner interface", 174);

#ifdef DEBUG_EXTERNAL_DET_PLANNER
      problem_.domain().terms().print_term(std::cout,
                                           action_schema->parameter(ia));
      std::cout << "=" << parsed_a.parameter(ia)
                << (o.second ? "" : "????") << ",";
#endif

      subst[action_schema->parameter(ia)] = o.first;

      if (actionXML_trace || actionT_trace) {
        ostXML << "<term>";
        problem_.terms().print_term(ostXML, o.first);
        ostXML << "</term>";
      }
    }
    if (actionXML_trace || actionT_trace) {
      ostXML << "</action>";
      actionXML = ostXML.str();
    }
#ifdef DEBUG_EXTERNAL_DET_PLANNER
    std::cout << ")" << std::endl;
#endif

    /*** Adding the XML action if it was requested ***/
    if (actionXML_trace)
      actionXML_trace->push_back(actionXML);

    /*** Finding the action_t that is equivalent to the instantiated action
         schema if it was requested. This can be costly, so, if the action
         will only be sent to the server as XML, consider using only the
         actionXML_trace ***/
    if (actionT_trace) {
      action_t const* selected_action = NULL;
      for (size_t ia = 0; ia < problem_.actionsT().size(); ++ia)
        if (problem_.actionsT()[ia]->enabled(next_s) &&
            strcasecmp(problem_.actionsT()[ia]->nameXML(),
                       actionXML.c_str()) == 0)
        {
          selected_action = problem_.actionsT()[ia];
          break;
        }
#ifdef DEBUG_EXTERNAL_DET_PLANNER
      if (!selected_action) {
        action_t const* selected_but_not_enabled = NULL;
        for (size_t ia = 0; ia < problem_.actionsT().size(); ++ia)
          if (strcasecmp(problem_.actionsT()[ia]->nameXML(),
                         actionXML.c_str()) == 0)
          {
            selected_but_not_enabled = problem_.actionsT()[ia];
            break;
          }
        if (selected_but_not_enabled) {
          std::cout << std::endl << std::endl
                    << "*******        BUG        **************" << std::endl
                    << "*** the action_t we are looking "
                    << selected_but_not_enabled->name() << " is considered as "
                    << "not enabled!!!!" << std::endl
                    << "*** This has a huge potential to be a BUG in the "
                    << "domain description..." << std::endl
                    << "****************************************" << std::endl;
        }
      }
#endif
      DIE(selected_action, "Action mapping not found in FF interface", 174);
      actionT_trace->push_back(selected_action);
#ifdef DEBUG_EXTERNAL_DET_PLANNER
      std::cout << "\t\taction_t::name = " << selected_action->name()
                << std::endl;
#endif
    }

    /*
     * Instantiating the effect
     */
    Effect const* inst_eff = &eff->instantiation(subst, problem_);

#ifdef DEBUG_EXTERNAL_DET_PLANNER
    std::cout << "\t\tEff chosen by Det Planner = ";
    inst_eff->print(std::cout, problem_.domain().predicates(),
                    problem_.domain().functions(), problem_.terms(), true);
    std::cout << std::endl
              << "\t\tP(chosen eff) = " << it->second.p << "  --  "
              << "P(plan so far) = " << cur_likelihood << std::endl;
#endif

    /*
     * Computing the next state: i.e. the given effect is deterministic
     * therefore there's only one possible next state
     */
    assert(dynamic_cast<ProbabilisticEffect const*>(inst_eff) == nullptr);
    AtomList adds;
    AtomList dels;
    AssignmentList assig;
    inst_eff->state_change(adds, dels, assig, next_s);

    for (AtomList::const_iterator ai = dels.begin(); ai != dels.end(); ++ai) {
      next_s.clear(**ai);
    }
    for (AtomList::const_iterator ai = adds.begin(); ai != adds.end(); ++ai) {
      next_s.add(**ai);
    }
    for (AssignmentList::const_iterator ai = assig.begin();
        ai != assig.end(); ++ai)
    {
      if (!(*ai)->is_a_reward_reassigment()) {
        std::cout << "ERROR: Unexpected assignment effect! Quiting"
                  << std::endl;
        assert(false);
        exit(-1);
      }
    }
    next_s.make_digest();
    state_trace.push_back(std::make_pair(cur_likelihood, next_s));

#ifdef DEBUG_EXTERNAL_DET_PLANNER
    std::cout << "  " << state_trace.size() << " ";
    next_s.full_print(std::cout, &problem_);
    std::cout << std::endl;
#endif
  }
}


/*
 * planFrom
 */
DetPlannerReturnType ExternalDetPlannerInterface::planFrom(state_t const& s,
    std::vector<std::pair<double, state_t> > & state_trace,
    std::vector<action_t const*>* action_trace,
    double likelihood_cutoff)
{
  std::vector<DetPlannerUnparsedAction> plan;
  DetPlannerReturnType retcode = run(s, plan);
  if (retcode == SUCCESS) {
    executionTrace(s, plan, state_trace, NULL, action_trace, likelihood_cutoff);
    return SUCCESS;
  }
  return retcode;
}


/*
 * partialPolicyFrom
 */
DetPlannerReturnType ExternalDetPlannerInterface::partialPolicyFrom(state_t const& s,
    DetPolicyIface& pi, double likelihood_cutoff)
{
  std::vector<std::pair<double, state_t> > state_trace;
  std::vector<action_t const*> action_trace;

  DetPlannerReturnType retcode = planFrom(s, state_trace, &action_trace,
                                          likelihood_cutoff);
  if (retcode == SUCCESS) {
    state_t cur_state = s;
//    std::cout << "likelihood cuteoff: " << likelihood_cutoff << std::endl;
    for (size_t t = 0; t < action_trace.size(); ++t) {
//      std::cout << action_trace[t]->name() << std::endl;
      pi.set(cur_state, action_trace[t]);
      cur_state = state_trace[t].second;
    }
    return SUCCESS;
  }
  return retcode;
}
