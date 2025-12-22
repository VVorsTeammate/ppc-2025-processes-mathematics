#include <gtest/gtest.h>
#include <mpi.h>

#include "moskaev_v_hypercube/common/include/common.hpp"
#include "moskaev_v_hypercube/mpi/include/ops_mpi.hpp"
#include "moskaev_v_hypercube/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace moskaev_v_hypercube {

class MoskaevVHypercubePerfTests : public ppc::util::BaseRunPerfTests<InType, OutType> {
 protected:
  void SetUp() override {
    auto params = GetParam();
    auto test_name = std::get<1>(params);

    bool is_mpi_test = (test_name.find("mpi") != std::string::npos) || (test_name.find("MPI") != std::string::npos);

    if (is_mpi_test) {
      int size = 0;
      MPI_Comm_size(MPI_COMM_WORLD, &size);

      if (size < 2) {
        GTEST_SKIP() << "Hypercube test requires at least 2 MPI processes. "
                     << "Got " << size << " process. Skipping.";
        return;
      }

      bool is_power_of_two = (size > 0) && ((size & (size - 1)) == 0);
      if (!is_power_of_two) {
        GTEST_SKIP() << "Hypercube requires power-of-two processes...";
        return;
      }
    }

    input_data_ = HypercubeTestData(1000, 12345, true, true);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    int expected_tests = 1;
    if (input_data_.test_communication) {
      expected_tests++;
    }
    if (input_data_.test_computation) {
      expected_tests++;
    }

    return output_data.total_tests_passed >= expected_tests;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
};

TEST_P(MoskaevVHypercubePerfTests, HypercubePerformance) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, MoskaevVTestMPI, MoskaevVTestSEQ>(PPC_SETTINGS_moskaev_v_hypercube);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = MoskaevVHypercubePerfTests::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(PerfTests, MoskaevVHypercubePerfTests, kGtestValues, kPerfTestName);

}  // namespace moskaev_v_hypercube
