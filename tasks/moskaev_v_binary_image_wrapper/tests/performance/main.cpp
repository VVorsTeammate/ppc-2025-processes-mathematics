#include <gtest/gtest.h>
#include <mpi.h>

#include <chrono>
#include <cstddef>
#include <iostream>
#include <random>
#include <string>
#include <tuple>
#include <vector>

#include "moskaev_v_binary_image_wrapper/common/include/common.hpp"
#include "moskaev_v_binary_image_wrapper/mpi/include/ops_mpi.hpp"
#include "moskaev_v_binary_image_wrapper/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace moskaev_v_binary_image_wrapper {

namespace {

void GenerateTestImage(InType &input_data, int width, int height, int seed = 42) {
  input_data.clear();
  input_data.push_back(width);
  input_data.push_back(height);

  std::mt19937 gen(seed);
  std::uniform_int_distribution<> pixel_dist(0, 1);
  std::uniform_int_distribution<> shape_dist(0, 100);

  const std::size_t total_size = static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
  std::vector<int> image(total_size, 0);

  // Добавляем несколько больших объектов
  for (int obj = 0; obj < 5; ++obj) {
    int center_x = (obj * width / 6) + (width / 12);
    int center_y = (obj * height / 6) + (height / 12);
    int size = 20 + (obj * 10);

    // Квадрат
    for (int dx = -size; dx <= size; ++dx) {
      for (int dy = -size; dy <= size; ++dy) {
        int x = center_x + dx;
        int y = center_y + dy;
        if (x >= 0 && x < width && y >= 0 && y < height) {
          image[(static_cast<std::size_t>(y) * width) + x] = 1;
        }
      }
    }
  }

  // Добавляем случайные точки
  for (int i = 0; i < width * height / 100; ++i) {
    int x = static_cast<int>(gen() % static_cast<unsigned long>(width));
    int y = static_cast<int>(gen() % static_cast<unsigned long>(height));
    image[(static_cast<std::size_t>(y) * width) + x] = 1;
  }

  // Копируем в input_data
  for (int pixel : image) {
    input_data.push_back(pixel);
  }
}

}  // namespace

class MoskaevVPerfTests : public ppc::util::BaseRunPerfTests<InType, OutType> {
 protected:
  void SetUp() override {
    GenerateTestImage(input_data_, 10000, 10000, 42);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    auto params = GetParam();
    auto test_name = std::get<1>(params);
    const std::string &test_name_str = test_name;
    bool is_mpi_test =
        (test_name_str.find("mpi") != std::string::npos) || (test_name_str.find("MPI") != std::string::npos);

    if (is_mpi_test) {
      int rank = 0;
      MPI_Comm_rank(MPI_COMM_WORLD, &rank);
      if (rank == 0) {
        return !output_data.empty();
      }
      return true;
    }

    return !output_data.empty();
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
};

TEST_P(MoskaevVPerfTests, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, MoskaevVTestTaskMPI>(PPC_SETTINGS_moskaev_v_binary_image_wrapper);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = MoskaevVPerfTests::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, MoskaevVPerfTests, kGtestValues, kPerfTestName);

TEST(MoskaevVSeqMinimal, TimeMeasurement) {
  // Проверяем, инициализирован ли MPI
  int mpi_initialized = 0;
  MPI_Initialized(&mpi_initialized);

  int rank = 0;
  if (mpi_initialized != 0) {
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  }

  if ((rank == 0) || (mpi_initialized == 0)) {
    const int test_size = 10000;
    InType input_data;

    GenerateTestImage(input_data, test_size, test_size, 42);

    auto start = std::chrono::high_resolution_clock::now();

    MoskaevVTestTaskSEQ task(input_data);

    EXPECT_TRUE(task.Validation());
    EXPECT_TRUE(task.PreProcessing());
    bool run_success = task.Run();
    EXPECT_TRUE(run_success);
    EXPECT_TRUE(task.PostProcessing());

    auto end = std::chrono::high_resolution_clock::now();

    OutType output = task.GetOutput();
    EXPECT_FALSE(output.empty());

    std::chrono::duration<double> elapsed = end - start;

    std::cout << elapsed.count() << '\n';
  }
}

}  // namespace moskaev_v_binary_image_wrapper
