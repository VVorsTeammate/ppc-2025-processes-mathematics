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
    auto params = GetParam();

    auto task_factory = std::get<0>(params);
    auto test_name = std::get<1>(params);
    auto test_data = std::get<2>(params);

    input_data_ = std::get<0>(test_data);
    std::string test_name_str = test_name;
    bool is_mpi_test =
        (test_name_str.find("mpi") != std::string::npos) || (test_name_str.find("MPI") != std::string::npos);

    if (is_mpi_test) {
      int size = 0;
      MPI_Comm_size(MPI_COMM_WORLD, &size);

      if (!((size > 0) && ((size & (size - 1)) == 0))) {
        GTEST_SKIP() << "Need power-of-two processes for hypercube, got " << size;
      }
    }
  }

  InType GetTestInputData() final {
    return input_data_;
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
    std::tuple_cat(ppc::util::AddFuncTask<MoskaevVTestMPI, InType>(kTestParam, PPC_SETTINGS_moskaev_v_hypercube),
                   ppc::util::AddFuncTask<MoskaevVTestSEQ, InType>(kTestParam, PPC_SETTINGS_moskaev_v_hypercube));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kFuncTestName = MoskaevVHypercubeFuncTests::PrintFuncTestName<MoskaevVHypercubeFuncTests>;

INSTANTIATE_TEST_SUITE_P(ComprehensiveTests, MoskaevVHypercubeFuncTests, kGtestValues, kFuncTestName);

}  // namespace

}  // namespace moskaev_v_hypercube
