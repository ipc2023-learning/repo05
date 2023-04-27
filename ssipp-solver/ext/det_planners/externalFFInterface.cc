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
#include "../mgpt/global.h"
#include "../mgpt/hash.h"


/*
 * _run: Runs the external FF using states s as initial state and the domain
 * determinization. Returns true if a solution was found. In this case, plan
 * is the sequence of actions chosen by FF
 */
DetPlannerReturnType ExternalFFInterface::_run(state_t const& s,
    std::vector<DetPlannerUnparsedAction>& plan)
{
  // Generating the file with the problem
  std::string problem_path = tmpDir() + "problem.pddl";
  std::ofstream problem_file;
  problem_file.open(problem_path.c_str());
  DIE(problem_file.is_open(), "ERROR opening file", 99);
  printProblem(problem_file, s);
  problem_file.close();

  // Running FF
  std::ostringstream cmd_ost;
  if (timeoutInSecs() > 0) {
    cmd_ost << "timeout " << timeoutInSecs() << " ";
  }
  cmd_ost << gpt::ff_path
          << " -R " << gpt::seed
          << " -o " << domainFile()
          << " -f " << problem_path
          << " 2>&1";

  std::string cmd = cmd_ost.str();

#ifdef DEBUG_EXTERNAL_FF
  std::cout << "[ExternalFFInterface::_run] calling ff: " << cmd << std::endl;
#endif

  FILE* ff_out = popen(cmd.c_str(), "r");

  DIE(ff_out, "Could not open the pipe to FF.", 173);

  // Parsing the FF output
  char line[1024];
  bool plan_found = false;
//  size_t l = 0;
  while (fgets(line, 1024, ff_out)) {
//    std::cout << "|1:" << l << "|" <<  line;  l++;
    if (strstr(line, "ff: found legal plan as follows")) {
      plan_found = true;
      break;
    } else if (strstr(line,
          "goal can be simplified to TRUE. The empty plan solves it")) {
      // As the FF message says, the initial and the goal state are the same
      // (sometimes it doesn't figure this out, thus the next while loop
      // handle the case in which the step is empty). When this happens, FF
      // return 1, therefore no need to add a DIE statement here.
      pclose(ff_out);
      return SUCCESS;
    } else if (strstr(line, "goal can be simplified to FALSE. No plan will solve it")) {
      // As the FF message says, the problem has no solution given the current
      // determinization
      pclose(ff_out);
      return NO_PLAN;
    } else if (strstr(line, "Timeout: aborting command")) {
      // The FF timed out. Closing the pipe and returning
      pclose(ff_out); // Ignoring the value since it will be 1 (assuming we're
                      // using the timeout cmd)
      keepTmpFiles();
      return TIMEOUT;
    }
  }
  if (!plan_found) {
    int retcode = pclose(ff_out); // Don't forget this otherwise we will keep
                                  // several files opened and an error will be
                                  // generated
#ifdef NDEBUG
#else
    if (retcode != 0) {
      std::cout << "FF cmd: " << cmd << std::endl;
    }
#endif
    keepTmpFiles();
    DIE(retcode == 0, "Pipe did not returned 0, i.e. execution error on FF", 173);
    return NO_PLAN;
  }

  size_t action_id = 0;
//  l = 0;
  while (fgets(line, 1024, ff_out)) {
//    std::cout << "|2:" << l << "|" <<  line; l++;
    if (strstr(line, "Timeout: aborting command")) {
      // The FF timed out. Closing the pipe and returning
      pclose(ff_out); // Ignoring the value since it will be 1 (assuming we're
                      // using the timeout cmd)
      keepTmpFiles();
      return TIMEOUT;
    }

    if (action_id == 0) {
      if (!strncmp(line, "step", 4)) {
        char* delim = strstr(line, ":");
        if (delim) {
          // First action
          DetPlannerUnparsedAction a(strstr(line, ":") + 2);
          plan.push_back(a);
          action_id++;
        } else {
          // There's no first action, that is, the current state is a goal
          // state!
          break;
        }
      }
      // else // first action not found yet
    } else {
      char* delim = strstr(line, ":");
      if (delim) {
        DetPlannerUnparsedAction a(delim + 2);
        plan.push_back(a);
        action_id++;
      } else {
        break;
      }
    }
  }
  int retcode = pclose(ff_out);
  // Some times we get all the info of the pipe before it is done (there's
  // stuff still in the buffer), so the retcode is not 0. The error
  // verification is done above, so, hopefully, the retcode can be ignored
  if (retcode) { retcode++; }  // making gcc stop complaining
//  DIE(retcode == 0, "Pipe did not returned 0, i.e. execution error on FF.", 173);
  return SUCCESS;
}


