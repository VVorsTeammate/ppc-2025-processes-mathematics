#include <gtest/gtest.h>

#include <cstddef>
#include <fstream>
#include <string>
#include <tuple>
#include <vector>

#include "orehov_n_topology_star/common/include/common.hpp"
#include "orehov_n_topology_star/mpi/include/ops_mpi.hpp"
#include "orehov_n_topology_star/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"
#include "util/include/util.hpp"

namespace orehov_n_topology_star {

class OrehovNTopologyStarPerfTests : public ppc::util::BaseRunPerfTests<InType, OutType> {
  InType input_data_;

  void SetUp() override {
    std::string abs_path = ppc::util::GetAbsoluteTaskPath(PPC_ID_orehov_n_topology_star, "performance_data.txt");
    std::ifstream file(abs_path);
    if (!file.is_open()) {
      throw "file not open\n";
    }

    int source = 0;
    int dest = 0;
    int size = 0;
    file >> source >> dest >> size;
    std::vector<int> data(static_cast<std::size_t>(size * 5));
    for (int i = 0; i < size; i++) {
      file >> data[i];
    }
    for (int i = size; i < size * 2; i++) {
      data[i] = data[i % size];
      data[i + size] = data[i % size];
      data[i + (size * 2)] = data[i % size];
      data[i + (size * 3)] = data[i % size];
    }
    size *= 5;

    file.close();

    input_data_ = std::make_tuple(source, dest, size, data);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    std::vector<int> source_data = std::get<3>(input_data_);
    int size = std::get<2>(input_data_);

    for (int i = 0; i < size; i++) {
      if (source_data[i] != output_data[i]) {
        return false;
      }
    }
    return true;
  }

  InType GetTestInputData() final {
    return input_data_;
  }
};

TEST_P(OrehovNTopologyStarPerfTests, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks = ppc::util::MakeAllPerfTasks<InType, OrehovNTopologyStarMPI, OrehovNTopologyStarSEQ>(
    PPC_SETTINGS_orehov_n_topology_star);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = OrehovNTopologyStarPerfTests::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, OrehovNTopologyStarPerfTests, kGtestValues, kPerfTestName);

}  // namespace orehov_n_topology_star
