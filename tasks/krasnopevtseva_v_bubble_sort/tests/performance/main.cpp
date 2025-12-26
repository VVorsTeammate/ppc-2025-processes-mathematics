#include <gtest/gtest.h>

#include <cmath>
#include <cstddef>
#include <random>
#include <vector>

#include "krasnopevtseva_v_bubble_sort/common/include/common.hpp"
#include "krasnopevtseva_v_bubble_sort/mpi/include/ops_mpi.hpp"
#include "krasnopevtseva_v_bubble_sort/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace krasnopevtseva_v_bubble_sort {

class KrasnopevtsevaVBubbleSortPerfTests : public ppc::util::BaseRunPerfTests<InType, OutType> {
  InType input_data_;
  bool res_{};

  void SetUp() override {
    std::vector<int> vec(100000);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 100000000);

    for (size_t i = 0; i < 100000; ++i) {
      vec[i] = dist(gen);
    }
    input_data_ = vec;
  }

  bool CheckTestOutputData(OutType &output_data) final {
    size_t size = output_data.size();
    res_ = true;
    for (size_t i = 0; i < size - 1; i++) {
      if (output_data[i] > output_data[i + 1]) {
        res_ = false;
      }
    }
    return res_;
  }
  InType GetTestInputData() final {
    return input_data_;
  }
};

TEST_P(KrasnopevtsevaVBubbleSortPerfTests, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, KrasnopevtsevaVBubbleSortMPI, KrasnopevtsevaVBubbleSortSEQ>(
        PPC_SETTINGS_krasnopevtseva_v_bubble_sort);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = KrasnopevtsevaVBubbleSortPerfTests::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, KrasnopevtsevaVBubbleSortPerfTests, kGtestValues, kPerfTestName);

}  // namespace krasnopevtseva_v_bubble_sort
