#include <gtest/gtest.h>

#include <algorithm>
#include <random>
#include <vector>

#include "../../modules/util/include/perf_test_util.hpp"
#include "moskaev_v_max_value_elem_matrix/common/include/common.hpp"
#include "moskaev_v_max_value_elem_matrix/mpi/include/ops_mpi.hpp"
#include "moskaev_v_max_value_elem_matrix/seq/include/ops_seq.hpp"

namespace moskaev_v_max_value_elem_matrix {

static InType GeneratePerfTestMatrix(int size) {
  InType matrix(size, std::vector<int>(size));
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dist(1, 5000);

  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
      matrix[i][j] = dist(gen);
    }
  }

  matrix[size / 2][size / 2] = 99999;

  return matrix;
}

class MoskaevVMaxValueElemMatrixPerfTests : public ppc::util::BaseRunPerfTests<InType, OutType> {
  const int kMatrixSize_ = 10000;
  InType input_data_;

  void SetUp() override {
    input_data_ = GeneratePerfTestMatrix(kMatrixSize_);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    const auto &matrix = input_data_;

    int max_element = matrix[0][0];
    for (const auto &row : matrix) {
      for (int element : row) {
        max_element = std::max(element, max_element);
      }
    }
    return output_data == max_element;
  }

  InType GetTestInputData() final {
    return input_data_;
  }
};

TEST_P(MoskaevVMaxValueElemMatrixPerfTests, TestPipelineRun) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, MoskaevVMaxValueElemMatrixMPI, MoskaevVMaxValueElemMatrixSEQ>(
        PPC_SETTINGS_moskaev_v_max_value_elem_matrix);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = MoskaevVMaxValueElemMatrixPerfTests::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(PerfTests, MoskaevVMaxValueElemMatrixPerfTests, kGtestValues, kPerfTestName);

}  // namespace moskaev_v_max_value_elem_matrix
