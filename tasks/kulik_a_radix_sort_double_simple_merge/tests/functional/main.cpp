#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <ios>
#include <stdexcept>
#include <string>
#include <tuple>

#include "kulik_a_radix_sort_double_simple_merge/common/include/common.hpp"
#include "kulik_a_radix_sort_double_simple_merge/mpi/include/ops_mpi.hpp"
#include "kulik_a_radix_sort_double_simple_merge/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace kulik_a_radix_sort_double_simple_merge {

class KulikARadixSortDoubleSimpleMergeFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return test_param;
  }

 protected:
  void SetUp() override {
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    std::string filename = params + ".bin";
    std::string abs_path = ppc::util::GetAbsoluteTaskPath(PPC_ID_kulik_a_radix_sort_double_simple_merge, filename);
    std::ifstream filestream(abs_path, std::ios::in | std::ios::binary);
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

 private:
  InType input_data_;
};

namespace {

TEST_P(KulikARadixSortDoubleSimpleMergeFuncTests, MatmulFromPic) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 3> kTestParam = {std::string("vector1"), std::string("vector2"), std::string("vector3")};

const auto kTestTasksList = std::tuple_cat(ppc::util::AddFuncTask<KulikARadixSortDoubleSimpleMergeMPI, InType>(
                                               kTestParam, PPC_SETTINGS_kulik_a_radix_sort_double_simple_merge),
                                           ppc::util::AddFuncTask<KulikARadixSortDoubleSimpleMergeSEQ, InType>(
                                               kTestParam, PPC_SETTINGS_kulik_a_radix_sort_double_simple_merge));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName =
    KulikARadixSortDoubleSimpleMergeFuncTests::PrintFuncTestName<KulikARadixSortDoubleSimpleMergeFuncTests>;

INSTANTIATE_TEST_SUITE_P(PicMatrixTests, KulikARadixSortDoubleSimpleMergeFuncTests, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace kulik_a_radix_sort_double_simple_merge
