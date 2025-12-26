#pragma once

#include <string>
#include <tuple>
#include <vector>

#include "task/include/task.hpp"

namespace orehov_n_topology_star {

using InType = std::tuple<int, int, int, std::vector<int>>;
using OutType = std::vector<int>;
using TestType = std::string;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace orehov_n_topology_star
