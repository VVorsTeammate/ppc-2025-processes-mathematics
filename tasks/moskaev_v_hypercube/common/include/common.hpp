#pragma once

#include <string>
#include <tuple>
#include <vector>

#include "task/include/task.hpp"

namespace moskaev_v_hypercube {

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
  bool topology_verified;
  bool communication_ok;
  bool computation_ok;
  int total_tests_passed;
  int max_hops_required;

  HypercubeTestResult()
      : topology_verified(false),
        communication_ok(false),
        computation_ok(false),
        total_tests_passed(0),
        max_hops_required(0) {}
};

using InType = HypercubeTestData;
using OutType = HypercubeTestResult;
using TestType = std::tuple<HypercubeTestData, std::string>;

using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace moskaev_v_hypercube
