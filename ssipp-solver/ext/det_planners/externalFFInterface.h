#ifndef EXTERNAL_FF_INTERFACE_H
#define EXTERNAL_FF_INTERFACE_H

#include <iostream>
#include <string>
#include <vector>

#include "externalDetPlannerInterface.h"
#include "../mgpt/states.h"

class ExternalFFInterface : public ExternalDetPlannerInterface {
 public:
  ExternalFFInterface(problem_t const& problem, size_t timeout_in_secs,
                      DeterminizationType det_type)
    : ExternalDetPlannerInterface(problem, timeout_in_secs, det_type)
    { }
  ~ExternalFFInterface() { };

 protected:
  /* This method runs FF and get the strings corresponding to the plan found. */
  DetPlannerReturnType _run(state_t const& s,
                            std::vector<DetPlannerUnparsedAction>& plan);
};

#endif // EXTERNAL_FF_INTERFACE_H
