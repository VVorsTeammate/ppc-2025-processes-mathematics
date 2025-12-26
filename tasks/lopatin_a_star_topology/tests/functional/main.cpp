#include <gtest/gtest.h>
#include <mpi.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <ios>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>

#include "lopatin_a_star_topology/common/include/common.hpp"
#include "lopatin_a_star_topology/mpi/include/ops_mpi.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace lopatin_a_star_topology {

class LopatinAStarTopologyFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return test_param;
  }

 protected:
  void SetUp() override {
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());

    int mpi_init = 0;
    MPI_Initialized(&mpi_init);
    if (mpi_init == 0) {
      GTEST_SKIP() << "MPI in not init";
    }

    int proc_num = 0;
    MPI_Comm_size(MPI_COMM_WORLD, &proc_num);
    if (proc_num < 2 && params != "func_self_to_self") {
      GTEST_SKIP() << "Number of processes is less than 2";
    }
    if (proc_num < 4 && params == "func_leaf_to_leaf") {
      GTEST_SKIP() << "Number of processes is less than 4";
    }

    std::string filename = params + ".txt";
    std::string abs_path = ppc::util::GetAbsoluteTaskPath(PPC_ID_lopatin_a_star_topology, filename);
    std::ifstream infile(abs_path, std::ios::in);
    if (!infile.is_open()) {
      throw std::runtime_error("Failed to open file: " + filename);
    }

    int proc_rank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);

    std::string line;
    int count = 0;
    int msg_size = 0;
    for (count = 0; count < 4 && std::getline(infile, line);) {
      if (line.empty()) {
        continue;
      }

      std::istringstream iss(line);
      int rank = 0;
      double value = NAN;

      if (count == 0) {
        iss >> rank;
        std::get<0>(input_data_) = rank;  // source_rank

      } else if (count == 1) {
        iss >> rank;
        std::get<1>(input_data_) = rank;  // dest_rank

      } else if (count == 2) {
        iss >> msg_size;
        std::get<2>(input_data_) = msg_size;

      } else if (proc_rank == std::get<0>(input_data_)) {
        while (iss >> value) {
          std::get<3>(input_data_).push_back(value);  // data
        }
        if (params == "func_self_to_self") {
          output_chekup_data_ = std::get<3>(input_data_);

        } else {
          MPI_Send(std::get<3>(input_data_).data(), msg_size, MPI_DOUBLE, std::get<1>(input_data_), 0, MPI_COMM_WORLD);
        }

      } else if (proc_rank == std::get<1>(input_data_) && params != "func_self_to_self") {
        output_chekup_data_.resize(msg_size);
        MPI_Recv(output_chekup_data_.data(), msg_size, MPI_DOUBLE, std::get<0>(input_data_), 0, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);
      }
      ++count;
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

 private:
  InType input_data_;
  OutType output_chekup_data_;
};

namespace {

TEST_P(LopatinAStarTopologyFuncTests, StarTopology) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 4> kTestParam = {std::string("func_self_to_self"), std::string("func_leaf_to_center"),
                                            std::string("func_center_to_leaf"), std::string("func_leaf_to_leaf")};

const auto kTestTasksList = std::tuple_cat(
    ppc::util::AddFuncTask<LopatinAStarTopologyMPI, InType>(kTestParam, PPC_SETTINGS_lopatin_a_star_topology));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName = LopatinAStarTopologyFuncTests::PrintFuncTestName<LopatinAStarTopologyFuncTests>;

INSTANTIATE_TEST_SUITE_P(StarTopologyFuncTests, LopatinAStarTopologyFuncTests, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace lopatin_a_star_topology
