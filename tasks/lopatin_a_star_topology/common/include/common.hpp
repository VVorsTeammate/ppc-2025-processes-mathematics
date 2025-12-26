#pragma once

#include <string>
#include <tuple>
#include <vector>

#include "task/include/task.hpp"

namespace lopatin_a_star_topology {

using InType = std::tuple<int, int, int, std::vector<double>>;  // std::tuple<source rank, dest rank, data size, data>
using OutType = std::vector<double>;                            // received data
using TestType = std::string;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace lopatin_a_star_topology
