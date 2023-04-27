#ifndef EXTERNAL_LAMA_INTERFACE_H
#define EXTERNAL_LAMA_INTERFACE_H

#include <iostream>
#include <string>
#include <vector>
#include <sys/stat.h>

#include "externalDetPlannerInterface.h"
#include "../mgpt/states.h"

class ExternalLamaInterface : public ExternalDetPlannerInterface {
 public:
  ExternalLamaInterface(problem_t const& problem, size_t timeout_in_secs,
                      DeterminizationType det_type)
    : ExternalDetPlannerInterface(problem, timeout_in_secs, det_type),
      plan_id_(0)
    {
      struct stat buf;
      if (stat(gpt::lama_path.c_str(), &buf) == -1) {
        std::cerr << "Lama was not found on '" << gpt::lama_path
                  << "'" << std::endl;
        exit(-1);
      }
    }
  ~ExternalLamaInterface() { };

 protected:
  /* This method runs Lama and get the strings corresponding to the plan found. */
  DetPlannerReturnType _run(state_t const& s,
                            std::vector<DetPlannerUnparsedAction>& plan);
 private:

  // Given the plan prefix passed to LAMA, it returns the name of all the
  // plan files containing that prefix
  std::vector<std::string> availablePlanFiles(
      std::string const& plan_prefix) const;

  // Given the path of a plan file, parses it and put the content in the plan
  // vector
  void parsePlanFile(std::string const& plan_file,
                     std::vector<DetPlannerUnparsedAction>& plan) const;

  size_t plan_id_;
};

#endif // EXTERNAL_LAMA_INTERFACE_H
