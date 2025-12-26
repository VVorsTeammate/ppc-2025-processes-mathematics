#include <gtest/gtest.h>

#include <algorithm>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

#include "petrov_e_allreduce/common/include/common.hpp"
#include "petrov_e_allreduce/mpi/include/ops_mpi.hpp"
#include "petrov_e_allreduce/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"
#include "util/include/util.hpp"

namespace petrov_e_allreduce {

class PetrovERunPerfAllreduceFindMaxInColumnsMatrix : public ppc::util::BaseRunPerfTests<InType, OutType> {
  InType input_data_;

  void SetUp() override {
    std::string abs_path = ppc::util::GetAbsoluteTaskPath(PPC_ID_petrov_e_allreduce, "perf_test.txt");
    std::ifstream in(abs_path);
    if (in.is_open()) {
      int n = 0;
      int m = 0;
      in >> n;
      in >> m;
      int limit = n * m;
      std::get<0>(input_data_) = n;
      std::get<1>(input_data_) = m;
      auto &matrix = std::get<2>(input_data_);
      matrix.clear();
      matrix.resize(limit);
      int i = 0;
      int j = 0;
      char tmp = 0;
      for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++) {
          in >> matrix[(i * n) + j];
        }
        in >> tmp;
      }
      in.close();
    }
  }

  bool CheckTestOutputData(OutType &output_data) final {
    int n = std::get<0>(input_data_);
    int m = std::get<1>(input_data_);
    auto &matrix = std::get<2>(input_data_);
    int i = 0;

    if (std::cmp_not_equal(m, static_cast<int>(output_data.size()))) {
      return false;
    }

    for (i = 0; i < m; i++) {
      using DifferenceType = typename OutType::difference_type;
      auto start = matrix.begin() + static_cast<DifferenceType>(i) * static_cast<DifferenceType>(n);
      auto end = start + static_cast<DifferenceType>(n);
      auto element = *std::max_element(start, end);
      if (output_data[i] != element) {
        return false;
      }
    }
    return true;
  }

  InType GetTestInputData() final {
    return input_data_;
  }
};

TEST_P(PetrovERunPerfAllreduceFindMaxInColumnsMatrix, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, PetrovEMyAllreduceMPI, PetrovEAllreduceMPI>(PPC_SETTINGS_petrov_e_allreduce);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = PetrovERunPerfAllreduceFindMaxInColumnsMatrix::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, PetrovERunPerfAllreduceFindMaxInColumnsMatrix, kGtestValues, kPerfTestName);
}  // namespace petrov_e_allreduce
