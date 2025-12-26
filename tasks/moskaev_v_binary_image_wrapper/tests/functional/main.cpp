// tests/functional/main.cpp
#include <gtest/gtest.h>
#include <mpi.h>

#include <array>
#include <string>
#include <tuple>

#include "moskaev_v_binary_image_wrapper/common/include/common.hpp"
#include "moskaev_v_binary_image_wrapper/mpi/include/ops_mpi.hpp"
#include "moskaev_v_binary_image_wrapper/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"

namespace moskaev_v_binary_image_wrapper {

class MoskaevVFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return std::to_string(std::get<0>(test_param)) + "_" + std::get<1>(test_param);
  }

 protected:
  void SetUp() override {
    input_data_.clear();

    auto params = GetParam();
    TestType test_param = std::get<2>(params);

    int test_case = std::get<0>(test_param);

    switch (test_case) {
      case 1:
        CreateTestImage1();
        break;
      case 2:
        CreateTestImage2();
        break;
      case 3:
        CreateTestImage3();
        break;
      default:
        CreateDefaultImage();
        break;
    }
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

  void CreateDefaultImage() {
    int width = 5;
    int height = 5;

    input_data_.push_back(width);
    input_data_.push_back(height);

    // Простой квадрат 3x3
    for (int yy = 0; yy < height; ++yy) {
      for (int xx = 0; xx < width; ++xx) {
        int value = (xx >= 1 && xx <= 3 && yy >= 1 && yy <= 3) ? 1 : 0;
        input_data_.push_back(value);
      }
    }
  }

  void CreateTestImage1() {
    int width = 10;
    int height = 10;

    input_data_.push_back(width);
    input_data_.push_back(height);

    // Квадрат 4x4
    for (int yy = 0; yy < height; ++yy) {
      for (int xx = 0; xx < width; ++xx) {
        int value = (xx >= 3 && xx <= 6 && yy >= 3 && yy <= 6) ? 1 : 0;
        input_data_.push_back(value);
      }
    }
  }

  void CreateTestImage2() {
    int width = 20;
    int height = 20;

    input_data_.push_back(width);
    input_data_.push_back(height);

    // Два объекта
    for (int yy = 0; yy < height; ++yy) {
      for (int xx = 0; xx < width; ++xx) {
        int value = 0;
        // Квадрат
        if (xx >= 2 && xx <= 5 && yy >= 2 && yy <= 5) {
          value = 1;
        }
        // Треугольная область
        if (xx >= 10 && xx <= 15 && yy >= 10 && yy <= 15 && (xx - 10) + (yy - 10) <= 5) {
          value = 1;
        }
        input_data_.push_back(value);
      }
    }
  }

  void CreateTestImage3() {
    int width = 15;
    int height = 15;

    input_data_.push_back(width);
    input_data_.push_back(height);

    // Фигура в форме L
    for (int yy = 0; yy < height; ++yy) {
      for (int xx = 0; xx < width; ++xx) {
        int value = 0;
        if ((xx >= 4 && xx <= 7 && yy >= 4 && yy <= 12) || (xx >= 4 && xx <= 11 && yy >= 10 && yy <= 12)) {
          value = 1;
        }
        input_data_.push_back(value);
      }
    }
  }
};

TEST_P(MoskaevVFuncTests, TestConvexHull) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 3> kTestParam = {std::make_tuple(1, "test1"), std::make_tuple(2, "test2"),
                                            std::make_tuple(3, "test3")};

const auto kTestTasksList = std::tuple_cat(
    ppc::util::AddFuncTask<MoskaevVTestTaskMPI, InType>(kTestParam, PPC_SETTINGS_moskaev_v_binary_image_wrapper),
    ppc::util::AddFuncTask<MoskaevVTestTaskSEQ, InType>(kTestParam, PPC_SETTINGS_moskaev_v_binary_image_wrapper));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName = MoskaevVFuncTests::PrintFuncTestName<MoskaevVFuncTests>;

INSTANTIATE_TEST_SUITE_P(ConvexHullTests, MoskaevVFuncTests, kGtestValues, kPerfTestName);

}  // namespace moskaev_v_binary_image_wrapper
