#include <gtest/gtest.h>

#include <cmath>

#include "lopatin_a_trapezoidal_integration/common/include/common.hpp"
#include "lopatin_a_trapezoidal_integration/mpi/include/ops_mpi.hpp"
#include "lopatin_a_trapezoidal_integration/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace lopatin_a_trapezoidal_integration {

class LopatinATrapezoidalIntegrationPerfTests : public ppc::util::BaseRunPerfTests<InType, OutType> {
  InType input_data_;
  OutType output_chekup_data_{};

  void SetUp() override {
    input_data_ = IntegrationData(-0.2, 1.3, 1.5, 2, [](double, double y) -> double { return std::exp(-y * y); }, 5000);
    output_chekup_data_ = 0.038826;
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return std::abs(output_data - output_chekup_data_) < 1e-4;
  }

  InType GetTestInputData() final {
    return input_data_;
  }
};

TEST_P(LopatinATrapezoidalIntegrationPerfTests, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, LopatinATrapezoidalIntegrationMPI, LopatinATrapezoidalIntegrationSEQ>(
        PPC_SETTINGS_lopatin_a_trapezoidal_integration);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = LopatinATrapezoidalIntegrationPerfTests::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(TrapezoidalIntegrationPerfTests, LopatinATrapezoidalIntegrationPerfTests, kGtestValues,
                         kPerfTestName);

}  // namespace lopatin_a_trapezoidal_integration
