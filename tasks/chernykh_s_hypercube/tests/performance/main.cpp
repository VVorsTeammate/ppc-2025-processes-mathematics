#include <gtest/gtest.h>

#include "chernykh_s_hypercube/common/include/common.hpp"
#include "chernykh_s_hypercube/mpi/include/ops_mpi.hpp"
#include "chernykh_s_hypercube/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace chernykh_s_hypercube {

class ChernykhSRunFuncTestsHypercube : public ppc::util::BaseRunPerfTests<InType, OutType> {
 private:
  InType input_data_;

  void SetUp() override {
    input_data_ = {0, 1};
  }

  bool CheckTestOutputData(OutType &output_data [[maybe_unused]]) final {
    if (ppc::util::GetMPIRank() == 0) {
      return true;
    }
    return true;
  }

  InType GetTestInputData() final {
    return input_data_;
  }
};

TEST_P(ChernykhSRunFuncTestsHypercube, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks = ppc::util::MakeAllPerfTasks<InType, ChernykhSHypercubeMPI, ChernykhSHypercubeSEQ>(
    PPC_SETTINGS_chernykh_s_hypercube);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = ChernykhSRunFuncTestsHypercube::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, ChernykhSRunFuncTestsHypercube, kGtestValues, kPerfTestName);

}  // namespace chernykh_s_hypercube
