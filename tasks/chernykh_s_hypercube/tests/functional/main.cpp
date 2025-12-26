#include <gtest/gtest.h>
#include <mpi.h>

#include <algorithm>  // Для std::max
#include <array>
#include <cmath>
#include <numeric>  // Для std::accumulate
#include <string>
#include <tuple>
#include <vector>

#include "chernykh_s_hypercube/common/include/common.hpp"
#include "chernykh_s_hypercube/mpi/include/ops_mpi.hpp"
#include "util/include/func_test_util.hpp"

namespace chernykh_s_hypercube {

class ChernykhSRunFuncTestsHypercube : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
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
      GTEST_SKIP() << "Hypercube requires power-of-two processes, but " << size << " provided.";
      return;
    }

    TestType test_params = std::get<2>(GetParam());
    input_data_ = std::get<0>(test_params);

    int max_required_rank = -1;
    for (int r : input_data_) {
      max_required_rank = std::max(max_required_rank, r);
    }
    if (max_required_rank >= size) {
      GTEST_SKIP() << "Test requires " << (max_required_rank + 1) << " processes, but only " << size << " available.";
    }
  }

  bool CheckTestOutputData(OutType &output_data) final {
    int rank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
      int expected_sum = std::accumulate(input_data_.begin(), input_data_.end(), 0);
      return output_data == expected_sum;
    }
    return true;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
};

namespace {

TEST_P(ChernykhSRunFuncTestsHypercube, SumHypercube) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 4> kTestParam = {std::make_tuple(std::vector<int>{0}, "rank_0_only"),
                                            std::make_tuple(std::vector<int>{0, 1}, "ranks_0_1"),
                                            std::make_tuple(std::vector<int>{0, 1, 2, 3}, "full_4_nodes"),
                                            std::make_tuple(std::vector<int>{1, 4, 7}, "sparse_nodes_1_4_7")};

const auto kTestTasksList =
    ppc::util::AddFuncTask<ChernykhSHypercubeMPI, InType>(kTestParam, PPC_SETTINGS_chernykh_s_hypercube);

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName = ChernykhSRunFuncTestsHypercube::PrintFuncTestName<ChernykhSRunFuncTestsHypercube>;

INSTANTIATE_TEST_SUITE_P(HypercubeTests, ChernykhSRunFuncTestsHypercube, kGtestValues, kPerfTestName);

}  // namespace
}  // namespace chernykh_s_hypercube
