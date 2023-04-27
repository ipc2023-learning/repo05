#ifndef HEURISTICS_FACTORY_H
#define HEURISTICS_FACTORY_H

#include <iostream>
#include <stack>
#include <memory>

#include "heuristic_iface.h"
#include "../ssps/ssp_iface.h"


std::shared_ptr<heuristic_t> createHeuristic(SSPIface const& ssp,
                                             std::string const& heuristic);


#endif  // HEURISTICS_FACTORY_H
