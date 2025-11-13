#pragma once

#include <string>
#include <tuple>
#include <vector>

#include "task/include/task.hpp"

namespace moskaev_v_max_value_elem_matrix {

using InType = std::vector<std::vector<int>>;  // Входные данные - сама матрица
using OutType = int;                           // максимальный элемент
using TestType = std::tuple<int, std::string>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace moskaev_v_max_value_elem_matrix
