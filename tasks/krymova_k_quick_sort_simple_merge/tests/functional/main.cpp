#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <string>
#include <tuple>
#include <vector>

#include "krymova_k_quick_sort_simple_merge/common/include/common.hpp"
#include "krymova_k_quick_sort_simple_merge/mpi/include/ops_mpi.hpp"
#include "krymova_k_quick_sort_simple_merge/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace krymova_k_quick_sort_simple_merge {

class KrymovaKQuickSortSimpleMergeFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return std::to_string(std::get<0>(test_param)) + "_" + std::get<1>(test_param);
  }

 protected:
  void SetUp() override {
    TestType test_params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    int size = std::get<0>(test_params);
    std::string test_type = std::get<1>(test_params);

    if (test_type == "empty") {
      input_data_ = std::vector<int>();
    } else if (test_type == "single") {
      input_data_ = {42};
    } else if (test_type == "sorted_asc") {
      input_data_.resize(size);
      for (int i = 0; i < size; ++i) {
        input_data_[i] = i;
      }
    } else if (test_type == "sorted_desc") {
      input_data_.resize(size);
      for (int i = 0; i < size; ++i) {
        input_data_[i] = size - i - 1;
      }
    } else if (test_type == "all_equal") {
      input_data_.resize(size, 7);
    } else if (test_type == "random") {
      input_data_.resize(size);
      for (int i = 0; i < size; ++i) {
        input_data_[i] = (i * 37) % 100;
      }
    } else {
      input_data_.resize(size);
      for (int i = 0; i < size; ++i) {
        input_data_[i] = (i * 37) % 100;
      }
    }

    expected_result_ = input_data_;
    std::ranges::sort(expected_result_);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    if (output_data.size() != expected_result_.size()) {
      return false;
    }
    if (!std::ranges::is_sorted(output_data)) {
      return false;
    }
    return output_data == expected_result_;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
  OutType expected_result_;
};

namespace {

TEST_P(KrymovaKQuickSortSimpleMergeFuncTests, QuickSortTest) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 22> kTestParam = {std::make_tuple(0, "empty"),
                                             std::make_tuple(1, "single"),
                                             std::make_tuple(1, "all_equal"),

                                             std::make_tuple(2, "random"),
                                             std::make_tuple(3, "random"),
                                             std::make_tuple(4, "random"),
                                             std::make_tuple(5, "random"),
                                             std::make_tuple(6, "random"),
                                             std::make_tuple(7, "random"),
                                             std::make_tuple(8, "random"),
                                             std::make_tuple(10, "random"),

                                             std::make_tuple(16, "sorted_asc"),
                                             std::make_tuple(16, "sorted_desc"),
                                             std::make_tuple(32, "all_equal"),
                                             std::make_tuple(100, "random"),

                                             std::make_tuple(256, "random_large"),
                                             std::make_tuple(500, "random_large"),
                                             std::make_tuple(1000, "random_very_large"),
                                             std::make_tuple(5000, "random_huge"),

                                             std::make_tuple(128, "sorted_asc"),
                                             std::make_tuple(128, "sorted_desc"),
                                             std::make_tuple(200, "all_equal")};

const auto kTestTasksList = std::tuple_cat(ppc::util::AddFuncTask<KrymovaKQuickSortSimpleMergeMPI, InType>(
                                               kTestParam, PPC_SETTINGS_krymova_k_quick_sort_simple_merge),
                                           ppc::util::AddFuncTask<KrymovaKQuickSortSimpleMergeSEQ, InType>(
                                               kTestParam, PPC_SETTINGS_krymova_k_quick_sort_simple_merge));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName =
    KrymovaKQuickSortSimpleMergeFuncTests::PrintFuncTestName<KrymovaKQuickSortSimpleMergeFuncTests>;

INSTANTIATE_TEST_SUITE_P(QuickSortTests, KrymovaKQuickSortSimpleMergeFuncTests, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace krymova_k_quick_sort_simple_merge
