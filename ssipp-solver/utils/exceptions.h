#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>
#include <iostream>
#include <string>

/*
 * Exception used when a deadline is reached. The deadline is user defined and
 * can be cpu time or any other metric. The only constraint is that one and
 * only one deadline is active at any given time.
 */
class DeadlineReachedException : public std::runtime_error {
 public:
  DeadlineReachedException(std::string const& s) : std::runtime_error(s) { }
};

/*
 * Exception to signal that the planner gave up from finding an action to be
 * applied. One reason for such exception is that the planner has an open
 * policy and a state s in which pi(s) is not defined is reached. In the
 * context of a replanner, this exception should be catch and trigger
 * replanning.
 */
class PlannerGaveUpException : public std::runtime_error {
 public:
  PlannerGaveUpException()
    : std::runtime_error("Planner gave up from choosing an action") { }
};


/*
 * INHERITED EXCEPTIONS AND MOSTLY DISABLED
 */
class Exception : public std::runtime_error {
 public:
  Exception(std::string const& msg)
    : std::runtime_error(msg), msg_(msg) { }
  ~Exception() throw() { }
  const char* what() const throw() { return msg_.c_str(); }
 private:
  friend std::ostream& operator<<(std::ostream& os, const Exception& e);
  std::string msg_;
};

std::ostream& operator<< (std::ostream& os, Exception const& e);

#endif  // EXCEPTIONS_H
