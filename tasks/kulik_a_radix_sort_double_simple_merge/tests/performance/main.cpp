#include <gtest/gtest.h>

#include <cmath>
#include <cstddef>
#include <fstream>
#include <ios>
#include <stdexcept>
#include <string>

#include "kulik_a_radix_sort_double_simple_merge/common/include/common.hpp"
#include "kulik_a_radix_sort_double_simple_merge/mpi/include/ops_mpi.hpp"
#include "kulik_a_radix_sort_double_simple_merge/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"
#include "util/include/util.hpp"

namespace kulik_a_radix_sort_double_simple_merge {

class KulikARadixSortDoubleSimpleMergePerfTests : public ppc::util::BaseRunPerfTests<InType, OutType> {
  InType input_data_;

  void SetUp() override {
    std::string filename = "vector2.bin";
    std::string abs_path = ppc::util::GetAbsoluteTaskPath(PPC_ID_kulik_a_radix_sort_double_simple_merge, filename);
    std::ifstream filestream(abs_path, std::ios::binary | std::ios::in);
    if (!filestream.is_open()) {
      throw std::runtime_error("Failed to open file: " + filename);
    }
    size_t vector_size = 0;
    filestream.read(reinterpret_cast<char *>(&vector_size), sizeof(size_t));
    input_data_.resize(vector_size);
    filestream.read(reinterpret_cast<char *>(input_data_.data()),
                    static_cast<std::streamsize>(vector_size * sizeof(double)));
    filestream.close();
  }

  bool CheckTestOutputData(OutType &output_data) final {
    size_t n = input_data_.size();
    bool check = true;
    for (size_t i = 1; i < n; ++i) {
      if (output_data[i - 1] > output_data[i]) {
        check = false;
      }
    }
    return check;
  }

  InType GetTestInputData() final {
    return input_data_;
  }
};

TEST_P(KulikARadixSortDoubleSimpleMergePerfTests, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, KulikARadixSortDoubleSimpleMergeMPI, KulikARadixSortDoubleSimpleMergeSEQ>(
        PPC_SETTINGS_kulik_a_radix_sort_double_simple_merge);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = KulikARadixSortDoubleSimpleMergePerfTests::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, KulikARadixSortDoubleSimpleMergePerfTests, kGtestValues, kPerfTestName);

}  // namespace kulik_a_radix_sort_double_simple_merge
