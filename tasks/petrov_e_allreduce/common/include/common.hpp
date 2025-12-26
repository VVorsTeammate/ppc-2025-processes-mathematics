#pragma once

#include <tuple>
#include <vector>

#include "task/include/task.hpp"

namespace petrov_e_allreduce {

using InType = std::tuple<int, int, std::vector<double>>;
using OutType = std::vector<double>;
using TestType = std::tuple<int, int, std::vector<double>, std::vector<double>>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace petrov_e_allreduce
