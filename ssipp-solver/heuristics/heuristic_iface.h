#ifndef HEURISTICS_IFACE_H
#define HEURISTICS_IFACE_H

#include <iostream>
#include <stack>

#include "../ext/mgpt/states.h"
#include "../utils/die.h"
#include "../ext/mgpt/problems.h"
#include "../ssps/ssp_iface.h"


#ifndef DEBUG_HEUR
#define DEBUG_HEUR 0
#endif


/*******************************************************************************
 *
 * heuristic (abstract class)
 *
 * This represents a generic heuristic for an SSP, independently of its
 * encoding.
 *
 ******************************************************************************/

class heuristic_t {
 public:
  heuristic_t(std::string name) : name_(name), total_calls_(0),
      mean_cputime_(0.0), m2_cputime_(0.0)
  { }

  virtual ~heuristic_t() {
    std::cout << "[" << name_ << " heuristic]: total calls = "
              << total_calls_ << std::endl
              << "[" << name_ << " heuristic]: 95CI cputime (in secs) = ";
    printf("%0.7f -+ %0.7f\n", mean_cputime_,
            (1.96 * sqrt(m2_cputime_ / (total_calls_ * (total_calls_-1)))));
  }

  /*
   * Non-virtual wrapper to computeValue. This wrapper allow us to compute
   * on-the-fly stats about the heuristic and help debugging it.
   */
  double value(state_t const& s) {
    START_TIMING("heuristic_value");
    uint64_t before = get_cputime_usec();
    double val = computeValue(s);
    double time_spent_in_s = (get_cputime_usec() - before) / (double) 1000000;
    double delta = time_spent_in_s - mean_cputime_;
    total_calls_++;
    mean_cputime_ += delta/total_calls_;
    m2_cputime_ += delta * (time_spent_in_s - mean_cputime_);
    STOP_TIMING("heuristic_value");
    _D(DEBUG_HEUR, std::cout << "H(" << s.toStringFull(gpt::problem, true)
                            << ") = " << val << std::endl)
    return val;
  }

  std::string name() const { return name_; }

 protected:
  /*
   * Heuristic main method
   */
  virtual double computeValue(state_t const& state) = 0;

  /*
   * Protected member variables
   */
  std::string name_;

 private:
  /*
   * Private member variables
   */
  size_t total_calls_;
  double mean_cputime_;
  double m2_cputime_;
};



/*******************************************************************************
 *
 * FactoredHeuristic (abstract class)
 *
 * This represents an heuristic for an factored SSPs, i.e., it is based on the
 * encoding of states as the set of atom that are true.
 *
 ******************************************************************************/
class FactoredHeuristic : public heuristic_t {
 public:
  FactoredHeuristic(std::string name, problem_t const& problem)
    : heuristic_t(name), problem_(problem)
  { }

  virtual ~FactoredHeuristic() { }

 protected:
  /*
   * Heuristic main method
   */
  virtual double computeValue(state_t const& state) = 0;

  /*
   * Protected member variables
   */
  problem_t const& problem_;
};




/******************************************************************************
 *
 * helpful action heuristic - abstract
 *
 *****************************************************************************/

class helpfulActionHeuristic_t
{

 public:
  virtual double value ( const state_t &state,
       std::vector<const action_t*>* helpful ) = 0;

  virtual ~helpfulActionHeuristic_t() { }

};

#endif  // HEURISTICS_IFACE_H
