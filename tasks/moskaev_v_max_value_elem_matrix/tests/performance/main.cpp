#include <gtest/gtest.h>
#include <mpi.h>

#include <chrono>
#include <iostream>
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
  const int kMatrixSize_ = 5000;
  InType input_data_;

  void SetUp() override {
    input_data_ = GeneratePerfTestMatrix(kMatrixSize_);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return output_data > 0;
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

TEST(MoskaevVMaxValueElemMatrixMpi, TestPipelineRun) {
  int initialized = 0;
  MPI_Initialized(&initialized);
  if (initialized == 0) {
    MPI_Init(nullptr, nullptr);
  }
  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  auto matrix = GeneratePerfTestMatrix(5000);
  MoskaevVMaxValueElemMatrixMPI task(matrix);

  EXPECT_TRUE(task.Validation());
  EXPECT_TRUE(task.PreProcessing());

  auto start_time = std::chrono::high_resolution_clock::now();
  EXPECT_TRUE(task.Run());
  auto end_time = std::chrono::high_resolution_clock::now();

  EXPECT_TRUE(task.PostProcessing());
  EXPECT_GT(task.GetOutput(), 0);

  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

  if (rank == 0) {
    std::cout << "MPI Pipeline time: " << duration.count() << "ms\n";
  }
}

TEST(MoskaevVMaxValueElemMatrixMpi, TestTaskRun) {
  int initialized = 0;
  MPI_Initialized(&initialized);
  if (initialized == 0) {
    MPI_Init(nullptr, nullptr);
  }
  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  auto matrix = GeneratePerfTestMatrix(5000);
  MoskaevVMaxValueElemMatrixMPI task(matrix);

  auto start_time = std::chrono::high_resolution_clock::now();
  EXPECT_TRUE(task.Validation());
  EXPECT_TRUE(task.PreProcessing());
  EXPECT_TRUE(task.Run());
  EXPECT_TRUE(task.PostProcessing());
  auto end_time = std::chrono::high_resolution_clock::now();

  EXPECT_GT(task.GetOutput(), 0);

  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

  if (rank == 0) {
    std::cout << "MPI Task time: " << duration.count() << "ms\n";
  }
}

TEST(MoskaevVMaxValueElemMatrixSeq, TestPipelineRun) {
  auto matrix = GeneratePerfTestMatrix(5000);
  MoskaevVMaxValueElemMatrixSEQ task(matrix);

  EXPECT_TRUE(task.Validation());
  EXPECT_TRUE(task.PreProcessing());

  auto start_time = std::chrono::high_resolution_clock::now();
  EXPECT_TRUE(task.Run());
  auto end_time = std::chrono::high_resolution_clock::now();

  EXPECT_TRUE(task.PostProcessing());
  EXPECT_GT(task.GetOutput(), 0);

  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
  std::cout << "SEQ Pipeline time: " << duration.count() << "ms\n";
}

TEST(MoskaevVMaxValueElemMatrixSeq, TestTaskRun) {
  auto matrix = GeneratePerfTestMatrix(5000);
  MoskaevVMaxValueElemMatrixSEQ task(matrix);

  auto start_time = std::chrono::high_resolution_clock::now();
  EXPECT_TRUE(task.Validation());
  EXPECT_TRUE(task.PreProcessing());
  EXPECT_TRUE(task.Run());
  EXPECT_TRUE(task.PostProcessing());
  auto end_time = std::chrono::high_resolution_clock::now();

  EXPECT_GT(task.GetOutput(), 0);

  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
  std::cout << "SEQ Task time: " << duration.count() << "ms\n";
}

}  // namespace moskaev_v_max_value_elem_matrix
