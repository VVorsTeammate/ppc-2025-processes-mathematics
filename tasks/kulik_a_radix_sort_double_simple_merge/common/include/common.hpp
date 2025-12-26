#pragma once

#include <string>
#include <vector>

#include "task/include/task.hpp"

namespace kulik_a_radix_sort_double_simple_merge {

using InType = std::vector<double>;
using OutType = std::vector<double>;
using TestType = std::string;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace kulik_a_radix_sort_double_simple_merge
