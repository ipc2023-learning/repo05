#ifndef PLANNER_FACTORY_H
#define PLANNER_FACTORY_H

#include <iostream>

#include "planner_iface.h"

Planner* createPlanner(SSPIface const& ssp, std::string const& name,
                       heuristic_t& heur);

#endif // PLANNER_H
