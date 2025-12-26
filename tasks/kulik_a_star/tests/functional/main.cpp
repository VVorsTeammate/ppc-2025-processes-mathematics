#include <gtest/gtest.h>

#include <array>
#include <climits>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <ios>
#include <stdexcept>
#include <string>
#include <tuple>

#include "kulik_a_star/common/include/common.hpp"
#include "kulik_a_star/mpi/include/ops_mpi.hpp"
#include "kulik_a_star/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace kulik_a_star {

class KulikAStarFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return test_param;
  }

 protected:
  void SetUp() override {
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    std::string filename = params + ".bin";
    std::string abs_path = ppc::util::GetAbsoluteTaskPath(PPC_ID_kulik_a_star, filename);
    std::ifstream filestream(abs_path, std::ios::in | std::ios::binary);
    if (!filestream.is_open()) {
      throw std::runtime_error("Failed to open file: " + filename);
    }
    int source_proc = 0;
    int destination_proc = 0;
    size_t size = 0;
    filestream.read(reinterpret_cast<char *>(&source_proc), sizeof(int));
    filestream.read(reinterpret_cast<char *>(&destination_proc), sizeof(int));
    filestream.read(reinterpret_cast<char *>(&size), sizeof(size_t));
    std::get<0>(input_data_) = source_proc;
    std::get<1>(input_data_) = destination_proc;
    std::get<2>(input_data_).resize(size);
    filestream.read(reinterpret_cast<char *>(std::get<2>(input_data_).data()),
                    static_cast<std::streamsize>(size * sizeof(int)));
    filestream.close();
  }

  bool CheckTestOutputData(OutType &output_data) final {
    size_t input_size = std::get<2>(input_data_).size();
    size_t output_size = output_data.size();
    bool check = true;
    if (input_size != output_size) {
      check = false;
    } else {
      for (size_t i = 0; i < input_size; i++) {
        if (std::get<2>(input_data_)[i] != output_data[i]) {
          check = false;
        }
      }
    }
    if (output_data.size() == 1 && output_data[0] == INT_MAX) {
      check = true;
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

TEST_P(KulikAStarFuncTests, MatmulFromPic) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 5> kTestParam = {std::string("0132"), std::string("1032"), std::string("1232"),
                                            std::string("2232"), std::string("perf")};

const auto kTestTasksList =
    std::tuple_cat(ppc::util::AddFuncTask<KulikAStarMPI, InType>(kTestParam, PPC_SETTINGS_kulik_a_star),
                   ppc::util::AddFuncTask<KulikAStarSEQ, InType>(kTestParam, PPC_SETTINGS_kulik_a_star));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName = KulikAStarFuncTests::PrintFuncTestName<KulikAStarFuncTests>;

INSTANTIATE_TEST_SUITE_P(PicMatrixTests, KulikAStarFuncTests, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace kulik_a_star
