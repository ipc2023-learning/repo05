#include <cstring>
#include <fstream>
#include <glob.h>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <string.h>

#include "../mgpt/actions.h"
#include "../../utils/die.h"
#include "../../utils/exceptions.h"
#include "externalLamaInterface.h"
#include "../mgpt/global.h"
#include "../mgpt/hash.h"
#include "../../utils/utils.h"


/*
 * _run: Runs the external Lama using states s as initial state and the domain
 * determinization. Returns true if a solution was found. In this case, plan
 * is the sequence of actions chosen by Lama
 */
DetPlannerReturnType ExternalLamaInterface::_run(state_t const& s,
    std::vector<DetPlannerUnparsedAction>& plan)
{
  // Generating the file with the problem
  std::string problem_path = tmpDir() + "/problem.pddl";
  std::ofstream problem_file;
  problem_file.open(problem_path.c_str());
  DIE(problem_file.is_open(), "ERROR opening file", 99);
  printProblem(problem_file, s);
  problem_file.close();

  plan_id_++;
  std::ostringstream output_file_ost;
  output_file_ost << tmpDir() << "/plan." << plan_id_ << ".txt";
  std::string output_file = output_file_ost.str();

  // Building
  std::string cmd(gpt::lama_path + " " + domainFile()
                                 + " " + problem_path
                                 + " " + output_file
                                + ">/dev/null 2>/dev/null");
//                                );
  if (timeoutInSecs() > 0) {
    std::ostringstream timeout_cmd;
    timeout_cmd << "my_timeout -c " << timeoutInSecs() << " ";
    cmd = timeout_cmd.str() + cmd;
  }

#ifdef DEBUG_EXTERNAL_LAMA
  size_t _epoch_before_call = get_secs_since_epoch();
  std::cout << "[ExternalLamaInterface:_run] calling lama: " << cmd
            << std::endl;
#endif

  DIE(system(NULL), "Shell not available", -1);
  // Running Lama
  int retcode = system(cmd.c_str());

#ifdef DEBUG_EXTERNAL_LAMA
  std::cout << "[ExternalLamaInterface:_run] lama took "
            << (get_secs_since_epoch() - _epoch_before_call)
            << " secs and returned code: " << retcode
            << std::endl;
#endif

  // LAMA might return more than one plan file, so let's check each one of
  // them and keep the smallest
  std::vector<std::string> plan_files = availablePlanFiles(output_file);
  if (plan_files.size() == 0) {
    if (retcode == 0) {
#ifdef DEBUG_EXTERNAL_LAMA
      std::cout << "[ExternalLamaInterface:_run] NO plan found"
                << std::endl;
#endif
      return NO_PLAN;
    }
    else {
#ifdef DEBUG_EXTERNAL_LAMA
      std::cout << "[ExternalLamaInterface:_run] timeout"
                << std::endl;
#endif
      return TIMEOUT;
    }
  }
  else {
#ifdef DEBUG_EXTERNAL_LAMA
    std::cout << "[ExternalLamaInterface:_run]: found "
              << plan_files.size() << " plans" << std::endl;
#endif
    std::vector<DetPlannerUnparsedAction> local_plan;
    for (size_t i = 0; i < plan_files.size(); ++i) {
      local_plan.clear();
      parsePlanFile(plan_files[i], local_plan);
      if (plan.size() == 0 || plan.size() > local_plan.size()) {
        plan.clear();
        plan = local_plan;
      }
    }
    if (plan.size() == 0)
    { return NO_PLAN; }
    return SUCCESS;
  }
}


std::vector<std::string> ExternalLamaInterface::availablePlanFiles(
    std::string const& plan_prefix) const
{
  std::string pat(plan_prefix + ".*");
  glob_t glob_result;
  glob(pat.c_str(), GLOB_TILDE, NULL, &glob_result);
  std::vector<std::string> ret;
  for (size_t i=0; i < glob_result.gl_pathc; ++i) {
    ret.push_back(std::string(glob_result.gl_pathv[i]));
  }
  globfree(&glob_result);
  return ret;
}


void ExternalLamaInterface::parsePlanFile(std::string const& plan_file,
    std::vector<DetPlannerUnparsedAction>& plan) const
{
  std::ifstream file(plan_file.c_str());
  if (file.is_open()) {
    std::string line;
    while(file.good()) {
      getline(file, line);
      if (line.empty())
      { continue; }
      plan.push_back(DetPlannerUnparsedAction(line.c_str()));
    }
    file.close();
  }
  else {
    std::cerr << "Error opening the plan file '" << plan_file
              << "'." << std::endl;
    exit(-1);
  }
}
