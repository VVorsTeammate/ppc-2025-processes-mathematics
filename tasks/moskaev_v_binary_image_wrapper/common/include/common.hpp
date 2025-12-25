// common.hpp
#pragma once

#include <tuple>
#include <utility>
#include <vector>

#include "task/include/task.hpp"

namespace moskaev_v_binary_image_wrapper {

using InType = std::vector<int>;
using OutType = std::vector<std::vector<std::pair<int, int>>>;
using TestType = std::tuple<int, std::string>;
using BaseTask = ppc::task::Task<InType, OutType>;

struct Point {
  int x;
  int y;

  Point() : x(0), y(0) {}
  Point(int x, int y) : x(x), y(y) {}

  bool operator==(const Point &other) const {
    return x == other.x && y == other.y;
  }

  bool operator!=(const Point &other) const {
    return !(*this == other);
  }
};

}  // namespace moskaev_v_binary_image_wrapper
