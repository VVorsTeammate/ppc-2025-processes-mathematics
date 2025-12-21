#include <gtest/gtest.h>
#include <mpi.h>

#include "moskaev_v_hypercube/common/include/common.hpp"
#include "moskaev_v_hypercube/mpi/include/ops_mpi.hpp"
#include "moskaev_v_hypercube/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"

namespace moskaev_v_hypercube {

class MoskaevVHypercubeFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return std::get<1>(test_param);
  }

 protected:
  void SetUp() override {
    int size = 0;
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    bool is_power_of_two = (size > 0) && ((size & (size - 1)) == 0);
    if (!is_power_of_two) {
      GTEST_SKIP() << "Hypercube requires power-of-two processes, but " << size << " provided. Skipping test.";
      return;
    }

    TestType test_params = std::get<2>(GetParam());
    input_data_ = std::get<0>(test_params);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank != 0) {
      return true;
    }

    // Проверяем, что прошли все ожидаемые тесты
    int expected_tests = 1;  // Топология всегда проверяется
    if (input_data_.test_communication) {
      expected_tests++;
    }
    if (input_data_.test_computation) {
      expected_tests++;
    }

    if (output_data.total_tests_passed < expected_tests) {
      std::cout << "FAIL: Only " << output_data.total_tests_passed << " tests passed, expected " << expected_tests
                << std::endl;
      return false;
    }

    std::cout << "PASS: " << output_data.total_tests_passed << " tests passed successfully" << std::endl;
    return true;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
};

namespace {

TEST_P(MoskaevVHypercubeFuncTests, ComprehensiveHypercubeTest) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 3> kTestParam = {std::make_tuple(HypercubeTestData(10, 42, true, false), "comm_only"),
                                            std::make_tuple(HypercubeTestData(10, 123, false, true), "comp_only"),
                                            std::make_tuple(HypercubeTestData(10, 777, true, true), "full_test")};

const auto kTestTasksList =
    std::tuple_cat(ppc::util::AddFuncTask<MoskaevVTestMPI, InType>(kTestParam, "moskaev_v_hypercube"),
                   ppc::util::AddFuncTask<MoskaevVTestSEQ, InType>(kTestParam, "moskaev_v_hypercube"));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kFuncTestName = MoskaevVHypercubeFuncTests::PrintFuncTestName<MoskaevVHypercubeFuncTests>;

INSTANTIATE_TEST_SUITE_P(ComprehensiveTests, MoskaevVHypercubeFuncTests, kGtestValues, kFuncTestName);

}  // namespace

}  // namespace moskaev_v_hypercube
