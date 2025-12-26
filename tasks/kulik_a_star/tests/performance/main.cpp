#include <gtest/gtest.h>

#include <climits>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <ios>
#include <stdexcept>
#include <string>

#include "kulik_a_star/common/include/common.hpp"
#include "kulik_a_star/mpi/include/ops_mpi.hpp"
#include "kulik_a_star/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"
#include "util/include/util.hpp"

namespace kulik_a_star {

class KulikAStarPerfTests : public ppc::util::BaseRunPerfTests<InType, OutType> {
  InType input_data_;

  void SetUp() override {
    std::string filename = "perf.bin";
    std::string abs_path = ppc::util::GetAbsoluteTaskPath(PPC_ID_kulik_a_star, filename);
    std::ifstream filestream(abs_path, std::ios::binary | std::ios::in);
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
};

TEST_P(KulikAStarPerfTests, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks = ppc::util::MakeAllPerfTasks<InType, KulikAStarMPI, KulikAStarSEQ>(PPC_SETTINGS_kulik_a_star);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = KulikAStarPerfTests::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, KulikAStarPerfTests, kGtestValues, kPerfTestName);

}  // namespace kulik_a_star
