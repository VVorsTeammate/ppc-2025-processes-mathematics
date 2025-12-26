#include <gtest/gtest.h>
#include <mpi.h>

#include <cmath>
#include <fstream>
#include <ios>
#include <stdexcept>
#include <string>

#include "lopatin_a_star_topology/common/include/common.hpp"
#include "lopatin_a_star_topology/mpi/include/ops_mpi.hpp"
#include "util/include/perf_test_util.hpp"
#include "util/include/util.hpp"

namespace lopatin_a_star_topology {

class LopatinAStarTopologyPerfTests : public ppc::util::BaseRunPerfTests<InType, OutType> {
  InType input_data_;
  OutType output_chekup_data_;

  void SetUp() override {
    int mpi_init = 0;
    MPI_Initialized(&mpi_init);
    if (mpi_init == 0) {
      GTEST_SKIP() << "MPI in not init";
    }

    int proc_num = 0;
    MPI_Comm_size(MPI_COMM_WORLD, &proc_num);
    if (proc_num < 4) {
      GTEST_SKIP() << "Number of processes is less than 4";
    }

    int proc_rank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);

    std::string filename = "perf_leaf_to_leaf.bin";
    std::string abs_path = ppc::util::GetAbsoluteTaskPath(PPC_ID_lopatin_a_star_topology, filename);
    std::ifstream infile(abs_path, std::ios::binary | std::ios::in);
    if (!infile.is_open()) {
      throw std::runtime_error("Failed to open file: " + filename);
    }

    int src_rank = 0;
    infile.read(reinterpret_cast<char *>(&src_rank), sizeof(src_rank));
    std::get<0>(input_data_) = src_rank;

    int dst_rank = 0;
    infile.read(reinterpret_cast<char *>(&dst_rank), sizeof(dst_rank));
    std::get<1>(input_data_) = dst_rank;

    int msg_size = 0;
    infile.read(reinterpret_cast<char *>(&msg_size), sizeof(msg_size));
    std::get<2>(input_data_) = msg_size;

    if (proc_rank == src_rank) {
      std::get<3>(input_data_).resize(msg_size);
      infile.read(reinterpret_cast<char *>(std::get<3>(input_data_).data()),
                  static_cast<std::streamsize>(msg_size * sizeof(double)));

    } else if (proc_rank == dst_rank) {
      output_chekup_data_.resize(msg_size);
      infile.read(reinterpret_cast<char *>(output_chekup_data_.data()),
                  static_cast<std::streamsize>(msg_size * sizeof(double)));
    }

    infile.close();
  }

  bool CheckTestOutputData(OutType &output_data) final {
    int mpi_init = 0;
    MPI_Initialized(&mpi_init);

    if (mpi_init == 1) {
      int proc_rank = 0;
      MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);

      if (proc_rank == std::get<1>(input_data_)) {
        return output_data == output_chekup_data_;
      }
    }
    return true;
  }

  InType GetTestInputData() final {
    return input_data_;
  }
};

TEST_P(LopatinAStarTopologyPerfTests, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, LopatinAStarTopologyMPI>(PPC_SETTINGS_lopatin_a_star_topology);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = LopatinAStarTopologyPerfTests::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(StarTopologyPerfTests, LopatinAStarTopologyPerfTests, kGtestValues, kPerfTestName);

}  // namespace lopatin_a_star_topology
