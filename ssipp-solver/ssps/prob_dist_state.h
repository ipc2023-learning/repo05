#ifndef PROB_DIST_STATE
#define PROB_DIST_STATE

#include <vector>
#include "../utils/prob_dist.h"

class state_t;
struct hashState;

// TODO(fwt): adding this here to avoid adding global.h to the header. Improve
// this by splitting the global.h between useful general stuff and global
// planner stuff.
#define STRINGIFY(s) XSTRINGIFY(s)
#define XSTRINGIFY(s) #s

// FWT: Changing the value of PROB_DIST_ARRAY_SIZE also affects the decision of
// when to expand probabilistic actions using an intermediary hash-based
// ProbDist. See comments in probabilisticAction_t::expand (actions.h)
#if not defined PROB_DIST_ARRAY_SIZE
#define PROB_DIST_ARRAY_SIZE 128
#endif

typedef ProbDistIface<state_t> ProbDistStateIface;

// FWT: typedefs for all the different implementations of ProbDistIface for
// state_t. This should be used in special cases in which a given implementation
// is sure to have an advantage over others
typedef ProbDistStlArray<state_t, PROB_DIST_ARRAY_SIZE>   ProbDistStateStlArray;
typedef ProbDistAllocArray<state_t, PROB_DIST_ARRAY_SIZE> ProbDistStateAllocArray;
typedef ProbDistVector<state_t>                           ProbDistStateVector;
typedef ProbDistMap<state_t>                              ProbDistStateMap;
typedef ProbDistHash<state_t, hashState>                  ProbDistStateHash;

// Default type of ProbDistState
typedef ProbDistStateStlArray ProbDistState;

// Default type for a vector of probability distribution of states. Notice that
// ProbDistStlArray is not "MOVE" safe since its move semantics is actually a
// copy semantics (there is no pointer to the beginning to the array).
// Therefore, we need to use any other implementation of ProbDist and
// ProbDistStateAllocArray is the closest in terms of efficiency that offers a
// proper move semantics.
//
// Good usage of VectorProbDistState:
// * Declare using prealloaction:
//      VectorProbDistState foo(100);
// * Increase its size by:
//      foo.emplace_back();
typedef std::vector<ProbDistStateVector> VectorProbDistState;

#endif  // PROB_DIST_STATE
