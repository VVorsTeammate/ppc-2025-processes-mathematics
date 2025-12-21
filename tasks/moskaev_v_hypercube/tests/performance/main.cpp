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
    int size = 0;
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    bool is_power_of_two = (size > 0) && ((size & (size - 1)) == 0);
    if (!is_power_of_two) {
      GTEST_SKIP() << "Hypercube requires power-of-two processes, but " << size << " provided. Skipping test.";
      return;
    }

    input_data_ = HypercubeTestData(1000, 12345, true, true);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == 0) {
      if (!output_data.topology_verified) {
        std::cerr << "Perf test failed: topology not verified" << std::endl;
        return false;
      }

      std::cout << "Perf test completed: " << output_data.total_tests_passed << " tests passed" << std::endl;
    }

    return true;
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

const auto kAllPerfTasks = ppc::util::MakeAllPerfTasks<InType, MoskaevVTestMPI, MoskaevVTestSEQ>("moskaev_v_hypercube");

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = MoskaevVHypercubePerfTests::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(PerfTests, MoskaevVHypercubePerfTests, kGtestValues, kPerfTestName);

}  // namespace moskaev_v_hypercube
