#include <cerrno>
#include <ctime>
#include <fstream>
#include <iostream>
#include <signal.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>

#include "../../utils/exceptions.h"
#include "global.h"
#include "../../utils/utils.h"



// SignalManager: Registring callback function
SignalManager::SignalManager() {
#ifndef SSIPP_NO_SIGNAL_MANAGER
  // SIGURS1: dump the current log
  signal(SIGUSR1, SignalManager::sigUsr1Handler);
  // SIGTERM: dump the log and then quit
  signal(SIGTERM, SignalManager::sigTermHandler);
#endif
}

// static
void SignalManager::sigUsr1Handler(int signum) {
  std::cout << "Caught signal " << signum << std::endl;
  if (signum != 10) {
    exit(signum);
  }
//  std::cout << "Saving log..." << std::endl;
}

// static
void SignalManager::sigTermHandler(int signum) {
  std::cout << "Caught signal " << signum << std::endl;
  if (signum != 15) {
    exit(signum);
  }
  std::cout << "Saving the log and quitting..." << std::endl;
  exit(signum);
}



namespace gpt
{
  uint64_t train_for_usecs = 0;
  uint64_t parsing_cpu_time = 0;
#ifdef USE_CACHE_PROB_OP_ADDS_ATOM
  size_t total_saved_prob_op_adds_atom_calls = 0;
  size_t total_prob_op_adds_atom_calls = 0;
#endif
  bool randomize_actionsT_order = true;
  double given_value_for_vStar_s0 = 0;
  uint64_t max_cpu_sys_time_usec = 0;
  uint64_t max_rss_kb = 0;
  std::shared_ptr<Deadline> _deadline_ = nullptr;
  StopCriterion stopCriterion = NONE;
  SignalManager signalManager;
  std::string ff_path = "./ff_mod";
  std::string lama_path = "./lama";
  std::string legend_file = "";
  std::set<std::string> debug_signals = parseDebugSignals();
  double ignore_effects_with_prob_less_than = 0.0;
  std::shared_ptr<PlannerFFReplan> followable_ffreplan;
  std::string followable_ffreplan_param = "";
  bool external_ff_ignore_forall_w_prob_effects = false;
  bool suppress_round_info = false;
  bool default_hp = true;
  std::string algorithm = "lrtdp";
  bool domain_analysis = false;
  Rational dead_end_value = Rational(500); //UINT_MAX;
  unsigned bound = 0;
  unsigned cutoff = 0;
  double epsilon = 0.0001;
  bool hash_all = true;
  std::string heuristic = "smartZero";
  size_t initial_hash_size = 204800;
  unsigned max_database_size = 32;
  bool noise = false;
  double noise_level = 0;
  unsigned seed = 0;
  int simulations = 0;
  unsigned verbosity = 0;
  unsigned warning_level = 0;
  double heuristic_weight = 1;
  size_t xtra = 0;
  bool print_turn_details = false;
  std::string execution_simulator = "local";
  uint32_t total_execution_rounds = 30;
  int max_time_in_secs = 0;
  uint64_t start_time = 0;
  problem_t const* problem = NULL;
  Simulator const* simulator = NULL;
  bool show_applied_policy = false;
  bool show_computed_policy = false;
  uint32_t max_turn = 10000;
  Rational cost_shift = Rational(0);

  /*
   * The default is to ignore state cost and consider only action costs. To:
   *  - also consider state cost (i.e., action and state), use -C full
   *  - ignore all the costs (both action and state), use -C ignore
   */
  bool use_action_cost = true;
  bool use_state_cost = false;
  bool normalize_action_cost = false;

  std::string tmp_dir = "/tmp/";
};


#if MEM_DEBUG

  void *
operator new( size_t size )
{
  void *result = malloc( size );
  fprintf( stderr, "new %p %d\n", result, size );
  return( result );
}

  void *
operator new[]( size_t size )
{
  void *result = malloc( size );
  fprintf( stderr, "new[] %p %d\n", result, size );
  return( result );
}

  void
operator delete( void *ptr )
{
  if( ptr )
  {
    fprintf( stderr, "del %p\n", ptr );
    free( ptr );
  }
}

  void
operator delete[]( void *ptr )
{
  if( ptr )
  {
    fprintf( stderr, "del[] %p\n", ptr );
    free( ptr &);
  }
}

#endif // MEM_DEBUG

char const* current_file;
extern int yyparse();
extern FILE* yyin;

bool readPDDLFile(char const* name) {
  yyin = fopen(name, "r");
  if (yyin == NULL) {
    std::cout << "parser:" << name << ": " << strerror(errno) << std::endl;
    return (false);
  }
  else {
    current_file = name;
    bool success;
    try {
      success = (yyparse () == 0);
    }
    catch (Exception exception) {
      fclose(yyin);
      std::cout << exception << std::endl;
      return false;
    }
    fclose(yyin);
    return success;
  }
}
