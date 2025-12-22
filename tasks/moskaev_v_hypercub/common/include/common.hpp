#pragma once

#include <string>
#include <tuple>

#include "task/include/task.hpp"

namespace moskaev_v_hypercub {

struct HypercubeTestData {
  int test_size;
  int seed;
  bool test_communication;
  bool test_computation;

  HypercubeTestData() : test_size(0), seed(0), test_communication(true), test_computation(true) {}

  HypercubeTestData(int size, int s, bool comm = true, bool comp = true)
      : test_size(size), seed(s), test_communication(comm), test_computation(comp) {}
};

struct HypercubeTestResult {
  bool topology_verified{false};
  bool communication_ok{false};
  bool computation_ok{false};
  int total_tests_passed{0};
  int max_hops_required{0};

  HypercubeTestResult() = default;  // Удалить старый конструктор
};

using InType = HypercubeTestData;
using OutType = HypercubeTestResult;
using TestType = std::tuple<HypercubeTestData, std::string>;

using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace moskaev_v_hypercub
