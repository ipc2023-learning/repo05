#include <algorithm>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>

#include "../ext/mgpt/actions.h"
#include "../ext/det_planners/externalFFInterface.h"
#include "../ext/mgpt/hash.h"
#include "short_sighted_ssps.h"
#include "prob_dist_state.h"
#include "bellman.h"


/*
 * MAX-DEPTH NAMED CONSTRUCTOR
 */
// static
std::unique_ptr<ShortSightedSSP> ShortSightedSSP::newMaxDepth(SSPIface const& ssp,
    state_t const& s, hash_t& fringe_heuristic, size_t max_depth,
    size_t max_space_size, uint64_t max_cpu_time_usec)
{
  static ProbDistState pr;

  // S4P: Short-Sighted SSP
  std::unique_ptr<ShortSightedSSP> s4p(new ShortSightedSSP(ssp, s,
                                              fringe_heuristic, "Depth S4P"));

  size_t fringe = 0;
  size_t loopCounter = 0;
  /*
   * Invariants
   *  - Every pair (depth t, state s) in Q is such that
   *    - s is in s4p
   *    - s is not marked as fringe
   *    - t is less or equal to max_depth
   *
   * Constraint:
   *  - A node s is NOT added to Q if s is already in short_sighted_space,i.e.,
   *    it was already explored or added to Q to be expanded.
   *
   * At any point, if the short-sighted space generation is interrupted, then
   * all the nodes in Q should be marked as fringe since they are not internal
   * nodes.
   */
  std::queue<std::pair<size_t, state_t const>> Q;
  Q.push(std::make_pair(0, s));
  s4p->insertState(s);
  DIE(s4p->isDefinedFor(s), "s is not in the current S4P", -1);

  uint64_t cpu_time_start = get_cpu_and_sys_time_usec();
  while (!Q.empty()) {
    if ((max_space_size > 0 && s4p->totalStates() > max_space_size)
        || (max_cpu_time_usec > 0
            && (get_cpu_and_sys_time_usec() - cpu_time_start) > max_cpu_time_usec))
    {
      break;
    }

    gpt::incCounterAndCheckDeadlineEvery(loopCounter, 10000);


    size_t cur_depth = Q.front().first;
    state_t const& cur_s = Q.front().second;
    DIE(s4p->isDefinedFor(cur_s), "cur_s is not in the s4p", -1);


//    std::cout << "CUR: s = " << cur_s.toString() << "  --  depth = "
//              << cur_depth << std::endl;
    DIE(max_depth >= 0, "Unpredicted value of max_depth", 1);

    /*
     * ASSUMPTION: no early return in ifs since Q.pop is called after the if is
     * over
     */
    if (cur_depth == max_depth || ssp.isGoal(cur_s)) {
      s4p->setAsFringe(cur_s);
      fringe++;

//      std::cout << "Marking s " << cur_s.toString() << " as fringe because ";
//      if (cur_depth == max_depth) std::cout << "it is at max_depth";
//      else                        std::cout << "it is a goal";
//      std::cout << std::endl;
    }
    else {
      // The state is is not a fringe and should be further extended
//      stateHashEntry_t *next_entry;
      for (action_t const& a : ssp.applicableActions(cur_s)) {
//        std::cout << "  expanding a = " << a.name() << std::endl;
        ssp.expand(a, cur_s, pr);
        for (auto const& ip : pr) {
          state_t const& next_s = ip.event();
//          std::cout << "    s_prime = " << next_s.toString() << std::endl;
          bool not_explored_before = s4p->insertState(next_s);
          if (not_explored_before) {
//              std::cout << "      " << cur_s.toString() << " at level "
//                << cur_depth << " adding s" << next_s.toString()
//                << " to the queue" << std::endl;
//            DEBUG_MSG("shortSightedSpaceDebug",
//                "-> Added to short-sighted space s%s",
//                ShortSightedSpace::name_[next_s].c_str());
            Q.push(std::make_pair(cur_depth+1, next_s));
          }
//          else {
//            std::cout << "      " << next_s.toString() << " already in S4P "
//                      << "so not re-adding" << std::endl;
//          }
        }  // for each descendent of (cur_s,a)
      }  // for each applicable action a
    }  // state s is not a fringe
    Q.pop();
  }  // while ! Q.empty



  if (!Q.empty()) {
    // The generation of the short-sighted space was interrupted, so we have to
    // define the open nodes in Q as fringe (notice that every node in Q is also
    // in the short-sighted space
    if (max_space_size > 0 && s4p->totalStates() > max_space_size) {
      DEBUG_MSG("shortSightedSpaceDebug",
                "Max node reached: %u <= %u = |short-sighted-space|",
                max_space_size, s4p->totalStates());
    }
    else if (max_cpu_time_usec > 0
             && (get_cpu_and_sys_time_usec() - cpu_time_start) > max_cpu_time_usec)
    {
      DEBUG_MSG("shortSightedSpaceDebug",
                "Max time reached: %u <= %u = time cutoff",
                max_cpu_time_usec,
                (get_cpu_and_sys_time_usec() - cpu_time_start));
    }
    else {
      std::cerr << "Unpredicted case: non-empty Q without violated cutoff. "
                << "Quiting." << std::endl;
      exit(-1);
    }
    while (!Q.empty()) {
      size_t cur_depth = Q.front().first;
      state_t const& cur_s = Q.front().second;
      DEBUG_MSG("shortSightedSpaceDebug", "MAX NODE Poped: (%d,s%s)",
          cur_depth, cur_s.toStringFull(gpt::problem).c_str());
      DIE(s4p->isDefinedFor(cur_s), "cur_s is not in the s4p", -1);
      DIE(!s4p->isGoal(cur_s), "cur_s is already a fringe state", -1);
      fringe++;
      s4p->setAsFringe(cur_s);
      Q.pop();
    }
  }  // while !Q.empty (when s4p generation was interrupted)


//  std::cout << "Short-sighted space size: "
//            << short_sighted_space.size() << std::endl;
  DEBUG_MSG("shortSightedSpaceSize",
      "[MaxDepthWithBounds]: space size %d / goal set size %d",
      s4p->totalStates(), fringe);

//  DIE(s4p->satisfiesSufficientConditions(),
//      "Depth Based S4P doesn't satisfies NIPS'12 sufficient conditions", -1);

  return s4p;
}




/*
 * TRAJECTORY-BASED NAMED CONSTRUCTOR
 */
//static
std::unique_ptr<ShortSightedSSP> ShortSightedSSP::newTrajectoryBased(
    SSPIface const& ssp, state_t const& s, hash_t& fringe_heuristic,
    Rational min_p, size_t max_space_size, uint64_t max_cpu_time_usec)
{
  /****************************************************************************
   *
   * Data structure used only by this method so far. Move it out if any other
   * method need them.
   */
  // Structure to store the state of explored states
  struct StateInfo {
    StateInfo() : open(false), p(0) { }
    StateInfo(bool o, Rational r) : open(o), p(r) { }
    bool open;
    Rational p;
  };
  using HashStateToStateInfo = HashMapState<StateInfo>;

  // Structure for the priority queue where the rational (first) is the
  // probability of the state (second) being reached from s in that particular
  // trajectory (note that the trajectory is irrelevant).
  using RationalAndState = std::pair<Rational, state_t>;

  // Functor to give top priority to the item with highest probability
  // (RationalAndState::first)
  struct MaxRationalFirst {
    // Returns true if p2 has higher priority than p1
    bool operator()(RationalAndState const& p1, RationalAndState const& p2) {
      return p1.first < p2.first;
    }
  };
  /***************************************************************************/

  static ProbDistState pr;

  // S4P: Short-Sighted SSP
  std::unique_ptr<ShortSightedSSP> s4p(new ShortSightedSSP(ssp, s,
                                    fringe_heuristic, "Trajectory-based S4P"));

  // Counters for the loop
  size_t fringe       = 0;  // number of fringe nodes found
  size_t loop_counter = 0;  // counter for the deadline checker

  HashStateToStateInfo explored_space;

  // The top node is the one with highest probability (Rational).
  // TODO(fwt): OPTIMIZATION: not sure if vector or deque is the best here.
  std::priority_queue<RationalAndState, std::vector<RationalAndState>,
                      MaxRationalFirst> Q;
  /*
   * Invariants for Q: every node s in Q is such that
   *    - s is also in short_sighted_space
   *    - explored_space[s] is defined
   *    - s is not marked as fringe
   *    - priority of s (i.e. the trace prob to reach s) is greater or equal
   *      to min_p
   */
  Q.push(RationalAndState(1, s));
  s4p->insertState(s);
  explored_space[s] = StateInfo(true, Rational(1));

  uint64_t cpu_time_start = get_cpu_and_sys_time_usec();
  while (!Q.empty()) {
    gpt::incCounterAndCheckDeadlineEvery(loop_counter, 10000);

    if ((max_space_size > 0 && s4p->totalStates() >= max_space_size)
        || (max_cpu_time_usec > 0
            && (get_cpu_and_sys_time_usec() - cpu_time_start) > max_cpu_time_usec))
    {
      break;
    }

    Rational const cur_trace_p = Q.top().first;
    state_t const cur_s = Q.top().second;
    Q.pop();

    DEBUG_MSG("shortSightedSpaceDebug", "Poped: (%f,%s)",
              cur_trace_p.double_value(), cur_s.toString().c_str());

    /*
     * Asserting the invariants
     */
    DIE(s4p->isDefinedFor(cur_s), "cur_s is not in the current S4P", -1);
    DIE(explored_space.find(cur_s) != explored_space.end(),
        "explored_space[cur_s] is not defined", -1);
    DIE(!s4p->isGoal(cur_s), "cur_s is marked as fringe in the S4P", -1);
    DIE(cur_trace_p >= min_p, "cur_trace_p is too low", -1);


    /*
     * Base cases with early returns
     */
    if (explored_space[cur_s].open == false) {
      // This node was already expanded before, i.e., there was a duplicated
      // entry in the priority list
      DEBUG_MSG("shortSightedSpaceDebug",
                "  Ignoring it because node is marked as close.");
      continue;
    }
    else if (ssp.isGoal(cur_s)) {
      DEBUG_MSG("shortSightedSpaceDebug",
                "  cur_s is an original goal. Adding as fringe.");
      s4p->setAsFringe(cur_s);
      fringe++;
      explored_space[cur_s].open = false;
      continue;
    }

    /*
     * At this point cur_s is NOT a goal (original or artificial) and is marked
     * as open
     */
    DIE(explored_space[cur_s].open, "Not expecting a closed node", -1);
    DIE(!ssp.isGoal(cur_s), "Not expecting an original goal", -1);

    // Since the node is open, then it never appeared before in the priority
    // queue or it reappeared with larger probability
    DIE(explored_space[cur_s].p == cur_trace_p, "Expecting the same prob", -1);


    // Since we will expand this open node, it will be marked as closed
    explored_space[cur_s].open = false;
    for (action_t const& a : ssp.applicableActions(cur_s)) {
      ssp.expand(a, cur_s, pr);
      for (auto const& ip : pr) {
        state_t const& next_s = ip.event();
        Rational const trace_next_s_p(cur_trace_p * Rational(ip.prob()));
        bool const not_explored_before = s4p->insertState(next_s);

        DEBUG_MSG("shortSightedSpaceDebug",
              "  Successor (next_s) = %s (this traj prob = %f)",
              next_s.toString().c_str(), trace_next_s_p.double_value());

        if (not_explored_before) {
          DEBUG_MSG("shortSightedSpaceDebug",
                    "    Added to short-sighted space.");

          DIE(explored_space.find(next_s) == explored_space.end(),
              "next_s not explored before but is in the explored_space hash",
              -1);

          explored_space[next_s] = StateInfo(true, trace_next_s_p);

          if (ssp.isGoal(next_s)) {
            // it's a real goal, so we're done.
            DEBUG_MSG("shortSightedSpaceDebug",
                      "    Is goal so marking as fringe and continuing.");

            // next_s was just added, so it should not be marked as a goal
            // already
            DIE(!s4p->isGoal(next_s), "next_s is already a goal state", -1);
            fringe++;
            s4p->setAsFringe(next_s);
            explored_space[next_s].open = false;  // goal states don't expand
          }  // next_s is an original goal
          else if (trace_next_s_p < min_p) {
            // next_s was not explored before and the only known trajectory to
            // it has probability < min_p, so marking it as a fringe node. Note
            // that if we later find a trajectory with prob >= min_p, then we
            // will remove next_s from the fringe.
            DEBUG_MSG("shortSightedSpaceDebug",
                      "    This first trajectory to next_s has low prob (< %f)."
                      " Marking as fringe.", min_p.double_value());

            fringe++;
            s4p->setAsFringe(next_s);
            explored_space[next_s].open = false;  // too little prob to expand
          }  // next_s is not an original goal and trajectory probability < min_p
          else {
            // next_s was not explored before and the only known trajectory to
            // it has probability >= min_p, so we add it to the queue.
            assert(trace_next_s_p >= min_p);
            Q.push(RationalAndState(trace_next_s_p, next_s));
            explored_space[next_s].open = true;

            DEBUG_MSG("shortSightedSpaceDebug",
              "    Pushing it to Q for the first time. [queue size = %u]",
              Q.size());
          }  // next_s is not an original goal and trajectory probability >= min_p
        }  // next_s was NOT explored before
        else {
          /*
           * next_s was explored before. Now it is either open (it is in the
           * queue) or closed.
           */
          DIE(!not_explored_before, "next_s should already be explored", -1);
          DIE(s4p->isDefinedFor(next_s), "next_s is not in the current S4P", -1);

          if (ssp.isGoal(next_s)) {
            // next_s was visited before, so it is already marked as goal
            DIE(s4p->isGoal(next_s), "next_s is not marked as goal", -1);
          }
          else if (trace_next_s_p >= min_p
                    && trace_next_s_p > explored_space[next_s].p)
          {
            // This node was explored before however we can reach it now using a
            // different trajectory that has more probability mass AND this new
            // probability mass is large enough
            Q.push(RationalAndState(trace_next_s_p, next_s));

            if (!explored_space[next_s].open) {
              // next_s was already expanded but it needs to be expanded again
              explored_space[next_s].open = true;
              if (s4p->isGoal(next_s)) {
                // The probability of the old traces didn't pass the min_p
                // threshold, so next_s was a fringe. Now we have to fix it.
                DIE(explored_space[next_s].p < min_p,
                    "expecting old traj prob < min_p", -1);
                fringe--;
                s4p->setAsInternal(next_s);
              }
            }
            assert(explored_space[next_s].open);
            DEBUG_MSG("shortSightedSpaceDebug",
                "    Pushing to Q again because its prob. increased. "
                "Before was %f) [queue size = %u]",
                explored_space[next_s].p.double_value(), Q.size());
            // Didn't update this before because of the DEBUG_MSG above
            explored_space[next_s].p = trace_next_s_p;
          }  // next_s is not an original goal and new trajectory has more probability mass
          else {
            assert(!ssp.isGoal(next_s));
            assert(trace_next_s_p < min_p
                    || trace_next_s_p <= explored_space[next_s].p);
            DEBUG_MSG("shortSightedSpaceDebug", "    Ignoring it because we "
                "expanded it before with larger prob (%f) "
                "OR it still has prob. less than min_p",
                explored_space[next_s].p.double_value());
          }
        }  // next_s was explored before
      }  // for each successor next_s of (s,a)
    }  // for each applicable action a
  }  // while Q is not empty

  while (!Q.empty()) {
    // The generation of the short-sighted space was interrupted, so we have to
    // define the open nodes in Q as fringe (notice that every node in Q is also
    // in the short-sighted space (invariant of Q)
    if (max_space_size > 0 && s4p->totalStates() > max_space_size) {
      DEBUG_MSG("shortSightedSpaceDebug",
                "Max node reached: %u <= %u = |short-sighted-space|",
                max_space_size, s4p->totalStates());
    }
    else if (max_cpu_time_usec > 0
             && (get_cpu_and_sys_time_usec() - cpu_time_start) > max_cpu_time_usec)
    {
      DEBUG_MSG("shortSightedSpaceDebug",
                "Max time reached: %u <= %u = time cutoff",
                max_cpu_time_usec,
                (get_cpu_and_sys_time_usec() - cpu_time_start));
    }
    else {
      std::cerr << "Unpredicted case: non-empty Q without violated cutoff. "
                << "Quiting." << std::endl;
      exit(-1);
    }
    state_t const& cur_s = Q.top().second;
    DEBUG_MSG("shortSightedSpaceDebug", "MAX NODE Poped: (%f,%s)",
                     Q.top().first.double_value(), cur_s.toString().c_str());

    DIE(s4p->isDefinedFor(cur_s), "cur_s is not in the current S4P", -1);
    if (explored_space[cur_s].open) {
      if (!ssp.hasApplicableActions(cur_s)) {
        DEBUG_MSG("shortSightedSpaceDebug",
            "  - Node is a deadend and will not be flagged as fringe: %s",
            cur_s.toString().c_str());
        DIE(s4p->isDefinedFor(cur_s), "cur_s is not in the current S4P", -1);
        explored_space[cur_s].open = false;
      }
      else {
        DEBUG_MSG("shortSightedSpaceDebug",
            "  - Node is open and will be flagged as fringe: %s",
            cur_s.toString().c_str());
        if (!s4p->isGoal(cur_s)) {
          // Although no state s in Q can be a fringe node, s might be more
          // than once in Q and moving it as a fringe here breaks this
          // invariant.
          fringe++;
          s4p->setAsFringe(cur_s);
          explored_space[cur_s].open = false;
        }
      }
    }
    else {
      DEBUG_MSG("shortSightedSpaceDebug", "  - Node is already closed: %s",
          cur_s.toString().c_str());
    }

    Q.pop();
  }  // while !Q.empty

  DEBUG_MSG("shortSightedSpaceDebug",
      "[UsingMinTraceProb]: space size %d / goal set size %d",
      s4p->totalStates(), fringe);

//  DIE(s4p->satisfiesSufficientConditions(),
//      "Traj Based S4P doesn't satisfies NIPS'12 sufficient conditions", -1);
  return s4p;
}



/*
* GREEDY NAMED CONSTRUCTOR
*/
//static
std::unique_ptr<ShortSightedSSP> ShortSightedSSP::newGreedy(
   SSPIface const& ssp, state_t const& s, hash_t& v, size_t max_space_size,
   uint64_t max_cpu_time_usec)
{
  assert(max_space_size > 0);

  static ProbDistState pr;

  // S4P: Short-Sighted SSP
  std::unique_ptr<ShortSightedSSP> s4p(new ShortSightedSSP(ssp, s,
                                        v, "Greedy-based S4P"));

  // Counters for the loop
  size_t fringe       = 0;  // number of fringe nodes found
  size_t loop_counter = 0;  // counter for the deadline checker

  auto min_v_first = [&v](state_t const& s1, state_t const& s2) -> bool {
                        return v.value(s2) < v.value(s1);
                    };

  // The top node is the one with highest probability (Rational).
  std::priority_queue<state_t, std::vector<state_t>, decltype(min_v_first)>
                                                               Q(min_v_first);

  /*
   * Invariants for Q: every node s in Q is such that
   *    - s is also in short_sighted_space
   *    - s is not marked as fringe
   */
  Q.push(s);
  s4p->insertState(s);

  uint64_t cpu_time_start = get_cpu_and_sys_time_usec();
  while (!Q.empty()) {
    gpt::incCounterAndCheckDeadlineEvery(loop_counter, 10000);

    if (s4p->totalStates() >= max_space_size
        || (max_cpu_time_usec > 0
            && (get_cpu_and_sys_time_usec() - cpu_time_start) > max_cpu_time_usec))
    {
      break;
    }

    state_t const cur_s = Q.top();
    Q.pop();

    DEBUG_MSG("shortSightedSpaceDebug", "Poped: (%f,%s) -- |s4p| = %d",
              v.value(s), cur_s.toString().c_str(), s4p->totalStates());

    /*
     * Asserting the invariants
     */
    DIE(s4p->isDefinedFor(cur_s), "cur_s is not in the current S4P", -1);
    DIE(!s4p->isGoal(cur_s), "cur_s is marked as fringe in the S4P", -1);


    if (ssp.isGoal(cur_s)) {
      DEBUG_MSG("shortSightedSpaceDebug",
                "  cur_s is an original goal. Adding as fringe.");
      s4p->setAsFringe(cur_s);
      fringe++;
    }
    else {
      for (action_t const& a : ssp.applicableActions(cur_s)) {
        ssp.expand(a, cur_s, pr);
        for (auto const& ip : pr) {
          state_t const& next_s = ip.event();
          DEBUG_MSG("shortSightedSpaceDebug",
              "  Successor (next_s) = %s (V = %f)",
              next_s.toString().c_str(), v.value(next_s));

          if (s4p->isDefinedFor(next_s)) {
            DEBUG_MSG("shortSightedSpaceDebug",
                "    Was already processed. Continuing.")
            continue;
          }

          // next_s was not in the s4p before
          s4p->insertState(next_s);

          if (ssp.isGoal(next_s)) {
            DEBUG_MSG("shortSightedSpaceDebug",
                "    Is goal so marking as fringe and continuing.");
            // it's a real goal, so we're done.
            fringe++;
            s4p->setAsFringe(next_s);
          }  // next_s is an original goal
          else {
            Q.push(next_s);
            DEBUG_MSG("shortSightedSpaceDebug",
                  "    Pushing it to Q for the first time. [queue size = %u]",
                  Q.size());
          }  // next_s is not an original goal and trajectory probability >= min_p
        }  // for each successor next_s of (s,a)
      }  // for each applicable action a
    }  // cur_s is not an original goal
  }  // while Q is not empty

  if (!Q.empty()) {
    // The generation of the short-sighted space was interrupted, so we have to
    // define the open nodes in Q as fringe (notice that every node in Q is also
    // in the short-sighted space (invariant of Q)
    if (s4p->totalStates() >= max_space_size) {
     DEBUG_MSG("shortSightedSpaceDebug",
                "Max node reached: %u <= %u = |short-sighted-space|",
                max_space_size, s4p->totalStates());
    }
    else if (max_cpu_time_usec > 0
             && (get_cpu_and_sys_time_usec() - cpu_time_start) > max_cpu_time_usec)
    {
      DEBUG_MSG("shortSightedSpaceDebug",
                "Max time reached: %u <= %u = time cutoff",
                max_cpu_time_usec,
                (get_cpu_and_sys_time_usec() - cpu_time_start));
    }
    else {
      std::cerr << "Unpredicted case: non-empty Q without violated cutoff. "
                << "Quiting." << std::endl;
      assert(false);
      exit(-1);
    }

    while (!Q.empty()) {
      state_t const& cur_s = Q.top();
      DEBUG_MSG("shortSightedSpaceDebug", "MAX NODE Poped: (%f,%s)",
                                        v.value(cur_s), cur_s.toString().c_str());

      DIE(s4p->isDefinedFor(cur_s), "cur_s is not in the current S4P", -1);
      if (!ssp.hasApplicableActions(cur_s)) {
        DEBUG_MSG("shortSightedSpaceDebug",
            "  - Node is a deadend and will not be flagged as fringe: %s",
            cur_s.toString().c_str());
        DIE(s4p->isDefinedFor(cur_s), "cur_s is not in the current S4P", -1);
      }
      else {
        DEBUG_MSG("shortSightedSpaceDebug",
            "  - Node is not explored and will be flagged as fringe: %s",
            cur_s.toString().c_str());
        assert(!s4p->isGoal(cur_s));
        // Although no state s in Q can be a fringe node, s might be more
        // than once in Q and moving it as a fringe here breaks this
        // invariant.
        fringe++;
        s4p->setAsFringe(cur_s);
      }
      Q.pop();
    }  // while !Q.empty
  }  // if !Q.empty

  DEBUG_MSG("shortSightedSpaceDebug",
      "[UsingMinTraceProb]: space size %d / goal set size %d",
      s4p->totalStates(), fringe);

//  DIE(s4p->satisfiesSufficientConditions(),
//      "Greedy S4P doesn't satisfies NIPS'12 sufficient conditions", -1);
  return s4p;
}


// Checks the 3 sufficient conditions from NIPS'12 paper (definition 5).
bool ShortSightedSSP::satisfiesSufficientConditions() const {
  size_t total_violations = 0;
  std::cout << "\n[S4P SUF. CONDs]: start checking." << std::endl;

  // Condition 1: All original goals in the s4p are also goals in the s4p.
  for (auto const& kv : ss_hash_) {
    state_t const& s = kv.first;
    if (ssp_.isGoal(s) && !isGoal(s)) {
      total_violations++;
      std::cout << "[S4P SUF. COND. 1]: state " << s.toString()
                << " is an original goal but not an s4p goal." << std::endl;
    }
  }

  // Condition 2: s4p initial state is not an artificial goal.
  if (isGoal(s0_) && !ssp_.isGoal(s0_)) {
    total_violations++;
    std::cout << "[S4P SUF. COND. 2]: s4p.s0 = " << s0_.toString()
              << " is an s4p goal but not an original goal." << std::endl;
  }

  ProbDistState pr;
  // Condition 3: all successor states from a non-goal state of the s4p must be
  // in the s4p state space.
  for (auto const& kv : ss_hash_) {
    if (kv.second != StateType::INTERNAL) {
      assert(isGoal(kv.first));
      continue;
    }

    state_t const& s = kv.first;
    for (action_t const& a : applicableActions(s)) {
      expand(a, s, pr);
      for (auto ip : pr) {
        state_t const& s_prime = ip.event();
        if (!isDefinedFor(s_prime)) {
          total_violations++;
          std::cout << "[S4P SUF. COND. 3]: state s' = " << s_prime.toString()
                    << " is not the s4p state space [it is a successor of s = "
                    << s.toString() << " and a = " << a.name() << "]"
                    << std::endl;
        }
      }  // for all the successors of (s,a)
    }  // for all a \in A(s)
  }  // for all s \in S' \cup G'

  if (total_violations > 0) {
    std::cout << "[S4P SUF. CONDs]: Total violations = " << total_violations
              << std::endl;
  }
  else {
    std::cout << "[S4P SUF. CONDs]: NO violation." << std::endl;
  }

  return total_violations == 0;
}
