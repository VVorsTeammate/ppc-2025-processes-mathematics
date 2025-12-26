#include <gtest/gtest.h>

#include <algorithm>  // Добавлено для std::ranges::replace
#include <array>
#include <climits>
#include <cstddef>  // Добавлено для std::size_t
#include <string>
#include <tuple>
#include <vector>

#include "kapanova_s_min_of_matrix_elements/common/include/common.hpp"
#include "kapanova_s_min_of_matrix_elements/mpi/include/ops_mpi.hpp"
#include "kapanova_s_min_of_matrix_elements/seq/include/ops_seq.hpp"  // Добавлено для SEQ версии
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace kapanova_s_min_of_matrix_elements {

class KapanovaSMinOfMatrixElementsFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    const auto &matrix = std::get<0>(test_param);
    const auto &expected = std::get<1>(test_param);

    std::string name = "matrix_" + std::to_string(matrix.size()) + "x" +
                       (matrix.empty() ? "0" : std::to_string(matrix[0].size())) + "_min_" + std::to_string(expected);

    std::ranges::replace(name, '-', 'n');
    return name;
  }

 protected:
  void SetUp() override {
    test_params_ = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    input_matrix_ = std::get<0>(test_params_);
    expected_output_ = std::get<1>(test_params_);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    if (input_matrix_.empty()) {
      return output_data == INT_MAX;
    }
    return output_data == expected_output_;
  }

  InType GetTestInputData() final {
    return input_matrix_;
  }

 private:
  TestType test_params_;
  InType input_matrix_;
  OutType expected_output_{0};
};

namespace {

const std::vector<std::vector<int>> kMatrix1 = {{3, 1, 4}, {2, 5, 9}, {6, 7, 8}};
const int kExpected1 = 1;

const std::vector<std::vector<int>> kMatrix2 = {{3, -1, 4}, {2, 5, 9}, {6, 7, 8}};
const int kExpected2 = -1;

const std::vector<std::vector<int>> kMatrix3 = {{5}};
const int kExpected3 = 5;

const std::vector<std::vector<int>> kMatrix4 = {{10, 20}, {15, 5}, {8, 12}};
const int kExpected4 = 5;

const std::vector<std::vector<int>> kMatrix5 = {{-5, -2, -8}, {-1, -3, -7}};
const int kExpected5 = -8;

const std::vector<std::vector<int>> kMatrix6 = {{INT_MAX, 1}, {2, INT_MAX}};
const int kExpected6 = 1;

const std::vector<std::vector<int>> kMatrix7 = {{5, 5, 5}, {5, 5, 5}};
const int kExpected7 = 5;

const std::vector<std::vector<int>> kEmptyMatrix = {};
const int kExpectedEmpty = INT_MAX;

const std::vector<std::vector<int>> kSingleRow = {{1, 2, 3, 4, 0}};
const int kExpectedSingleRow = 0;

const std::vector<std::vector<int>> kSingleColumn = {{1}, {2}, {0}, {4}};
const int kExpectedSingleColumn = 0;

TEST_P(KapanovaSMinOfMatrixElementsFuncTests, FindMatrixMinimum) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 10> kTestParam = {
    std::make_tuple(kMatrix1, kExpected1),           std::make_tuple(kMatrix2, kExpected2),
    std::make_tuple(kMatrix3, kExpected3),           std::make_tuple(kMatrix4, kExpected4),
    std::make_tuple(kMatrix5, kExpected5),           std::make_tuple(kMatrix6, kExpected6),
    std::make_tuple(kMatrix7, kExpected7),           std::make_tuple(kEmptyMatrix, kExpectedEmpty),
    std::make_tuple(kSingleRow, kExpectedSingleRow), std::make_tuple(kSingleColumn, kExpectedSingleColumn),
};

const auto kTestTasksList = std::tuple_cat(ppc::util::AddFuncTask<KapanovaSMinOfMatrixElementsMPI, InType>(
                                               kTestParam, PPC_SETTINGS_kapanova_s_min_of_matrix_elements),
                                           ppc::util::AddFuncTask<KapanovaSMinOfMatrixElementsSEQ, InType>(
                                               kTestParam, PPC_SETTINGS_kapanova_s_min_of_matrix_elements));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName =
    KapanovaSMinOfMatrixElementsFuncTests::PrintFuncTestName<KapanovaSMinOfMatrixElementsFuncTests>;

INSTANTIATE_TEST_SUITE_P(MatrixMinTests, KapanovaSMinOfMatrixElementsFuncTests, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace kapanova_s_min_of_matrix_elements
