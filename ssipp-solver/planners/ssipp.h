#ifndef PLANNERS_SSIPP_H
#define PLANNERS_SSIPP_H

#include <iostream>
#include <memory>

#include "planner_iface.h"

#include "../ext/mgpt/actions.h"
#include "../ssps/bellman.h"
#include "../ext/mgpt/hash.h"
#include "../ssps/short_sighted_ssps.h"
#include "../ssps/ssp_iface.h"

/*
 * Implementation of SSiPP: Short-Sighted Probabilistic Planner as defined in
 * http://felipe.trevizan.org/papers/trevizan14:depth.pdf
 *
 * Bibtex entry:
   @article{trevizan14:depth,
     author = {Trevizan, F. and Veloso, M.},
     title = {{D}epth-based {S}hort-sighted {S}tochastic {S}hortest {P}ath {P}roblems},
     journal = {Artificial Intelligence},
     year = {2014},
     volume = {216},
     pages = {179 - 205},
     url = {felipe.trevizan.org/papers/trevizan14:depth.pdf},
   }
 *
 */

class heuristic_t;

/*
 * ShortSightedSSPFactory is a wrapper around the different types of
 * Short-Sighted SSPs (S4Ps) to make PlannerSSiPP a little bit more clean (i.e.,
 * not have to store all the possible parameters and do the parsing). This class
 * was not really design with the purpose being reused by other methods;
 * therefore this should be a private inner class of PlannerSSiPP but it would
 * still make its definition quite messy.
 */
class ShortSightedSSPFactory {
 public:
  ShortSightedSSPFactory() : initialized_(false) { }

  // Basically parses flag and populates the appropriated member variables. The
  // deque flags are received from SSiPP. See SSiPP usage for more info.
  void init(std::deque<std::string>& flags);

  std::unique_ptr<ShortSightedSSP> newShortSightedSSP(SSPIface const& ssp,
      state_t const& s, hash_t& fringe_heuristic) const
  {
    if (s4p_method_ == ShortSightedMethod::MAX_DEPTH) {
      return ShortSightedSSP::newMaxDepth(ssp, s, fringe_heuristic,
                                                             max_depth_cfg_.t);
    }
    else if (s4p_method_== ShortSightedMethod::RANDOM_MAX_DEPTH) {
      size_t rand_t = randInIntervalInclusive_l(random_max_depth_cfg_.lb,
                                                random_max_depth_cfg_.ub);
//      std::cout << "[S4P Factory] Random t = " << rand_t << std::endl;
      return ShortSightedSSP::newMaxDepth(ssp, s, fringe_heuristic, rand_t);
    }
    else if (s4p_method_ == ShortSightedMethod::MIN_PROB_TRAJECTORY) {
      return ShortSightedSSP::newTrajectoryBased(ssp, s, fringe_heuristic,
                                                          min_prob_cfg_.min_p);
    }
    else if (s4p_method_ == ShortSightedMethod::GREEDY) {
      return ShortSightedSSP::newGreedy(ssp, s, fringe_heuristic,
                                                        greedy_cfg_.max_nodes);
    }
    else {
      std::cerr << "ERROR: Unknown ShortSightedMethod '"
                << s4p_method_ << "'. Quitting" << std::endl;
      exit(1);
    };
    return std::unique_ptr<ShortSightedSSP>(nullptr);
  }

  static void usage(std::ostream& os);

 private:
  /*
   * Typedefs
   */
  enum class ShortSightedMethod {MAX_DEPTH,
                                 RANDOM_MAX_DEPTH,
                                 MIN_PROB_TRAJECTORY,
                                 GREEDY};
  friend std::ostream& operator<<(std::ostream& os,
                                 ShortSightedSSPFactory::ShortSightedMethod m);
  /*
   * These structs are here just to organize the different Short-Sight SSPs
   * parameters.
   */
  struct MaxDepthCfg {
    size_t t;
  };

  struct RandomMaxDepthCfg {
    size_t lb;
    size_t ub;
  };

  struct MinProbTrajectoryCfg {
    Rational min_p;
  };

  struct GreedyCfg {
    size_t max_nodes;
  };

  /*
   * Member variables
   */
  bool initialized_;
  ShortSightedMethod s4p_method_;

  /*
   * Configuration variables
   */
  MaxDepthCfg max_depth_cfg_;
  RandomMaxDepthCfg random_max_depth_cfg_;
  MinProbTrajectoryCfg  min_prob_cfg_;
  GreedyCfg greedy_cfg_;
};



class PlannerSSiPP : public HeuristicPlanner {
 public:
  PlannerSSiPP(SSPIface const& ssp, heuristic_t& heur, double epsilon,
      std::string flags_str)
    : HeuristicPlanner(), ssp_(ssp),
      internal_v_(new hash_t(gpt::initial_hash_size, heur)), v_(*internal_v_),
      epsilon_(epsilon), short_sighted_epsilon_(-1), cur_s4p_(nullptr),
      opt_planner_class_(OptAlg::LRTDP), opt_planner_(nullptr)
  {
    parseParameters(flags_str);
  }

  PlannerSSiPP(SSPIface const& ssp, hash_t& v, double epsilon,
      std::string flags_str)
    : HeuristicPlanner(), ssp_(ssp), internal_v_(nullptr), v_(v), epsilon_(epsilon),
      short_sighted_epsilon_(-1), cur_s4p_(nullptr),
      opt_planner_class_(OptAlg::LRTDP), opt_planner_(nullptr)
  {
    parseParameters(flags_str);
  }

  /*
   * Planner Interface
   */
  action_t const* decideAction(state_t const& s) override;

  action_t const* decideAction(state_t const& s) const override {
    return Bellman::constGreedyAction(s, v_, ssp_);
  }

  /*
   * Ad-hoc training method: simulate rounds until either:
   *  - Deadline is reached
   *  - 50 consecutive rounds reach the goal and delta V(s0) <= epsilon
   */
  void trainForUsecs(uint64_t) override;

  void initRound() override {
    // Deleting the optimal planner and short-sighted SSP if they exist
    opt_planner_.reset(nullptr);
    cur_s4p_.reset(nullptr);
  }

  void endRound() override { }

  void resetRoundStatistics() override { }

  void statistics(std::ostream& os, int level) const override { }

  /*
   * Heuristic Planner Interface
   */
  double value(state_t const& s) const override { return v_.value(s); }


  /*
   * SSiPP Methods
   */
  // Show the option of ssipp
  void usage(std::ostream& os) const;


 private:
  /*
   * SSiPP Methods
   */
  void parseParameters(std::string const& flags_str);

  /*
   * Enums
   */
  enum class OptAlg {LRTDP, VI};
  friend std::ostream& operator<<(std::ostream& os, PlannerSSiPP::OptAlg a);

  /*
   * Member variables
   */
  SSPIface const& ssp_;
  std::unique_ptr<hash_t> internal_v_;
  hash_t& v_;
  double epsilon_;
  double short_sighted_epsilon_;
  std::unique_ptr<ShortSightedSSP> cur_s4p_;
  OptAlg opt_planner_class_;
  std::unique_ptr<OptimalPlanner> opt_planner_;
  // All the parameters and parsing for the different short-sighted SSPs are
  // handled by the factory.
  ShortSightedSSPFactory s4p_factory_;
};


inline std::ostream& operator<<(std::ostream& os, PlannerSSiPP::OptAlg a) {
  using OptAlg = PlannerSSiPP::OptAlg;
  switch (a) {
    case OptAlg::LRTDP  : os << "LRTDP";      break;
    case OptAlg::VI     : os << "VI";         break;
    default             : os.setstate(std::ios_base::failbit);
  }
  return os;
}


inline std::ostream& operator<<(std::ostream& os,
                                ShortSightedSSPFactory::ShortSightedMethod m) {
  using S4P = ShortSightedSSPFactory::ShortSightedMethod;
  switch (m) {
    case S4P::MAX_DEPTH           : os << "max-depth";        break;
    case S4P::RANDOM_MAX_DEPTH    : os << "random-max-depth"; break;
    case S4P::MIN_PROB_TRAJECTORY : os << "min-prob-traj"; break;
    case S4P::GREEDY              : os << "greedy"; break;
    default                       : os.setstate(std::ios_base::failbit);
  }
  return os;
}
#endif // PLANNERS_SSIPP_H
