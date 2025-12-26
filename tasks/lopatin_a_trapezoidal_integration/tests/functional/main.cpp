#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <numbers>
#include <string>
#include <tuple>

#include "lopatin_a_trapezoidal_integration/common/include/common.hpp"
#include "lopatin_a_trapezoidal_integration/mpi/include/ops_mpi.hpp"
#include "lopatin_a_trapezoidal_integration/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace lopatin_a_trapezoidal_integration {

class LopatinATrapezoidalIntegrationFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    switch (test_param) {
      case 0:
        return "polynomial";
      case 1:
        return "exponential";
      case 2:
        return "trigonometric";
      case 3:
        return "logarithmic";
      case 4:
        return "numerical";
      default:
        return "error";
    }
  }

 protected:
  void SetUp() override {
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());

    switch (params) {
      case 0:
        input_data_ = IntegrationData(-1.2, 3, -0.5, 4,
                                      [](double x, double y) -> double { return (x * x * x) - (y * y) + 3; }, 500);
        output_chekup_data_ = 55.7172;
        break;
      case 1:
        input_data_ = IntegrationData(0, 1.1, -0.5, 1,
                                      [](double x, double y) -> double { return std::exp((2 * x) + y) * 2; }, 500);
        output_chekup_data_ = (std::numbers::e - std::exp(-0.5)) * (std::exp(2.2) - 1);
        break;
      case 2:
        input_data_ = IntegrationData(-3.2, -1, -2.5, -1,
                                      [](double x, double y) -> double { return std::sin(y) - std::cos(x); }, 500);
        output_chekup_data_ = (2.2 * (std::cos(2.5) - std::cos(1))) + (1.5 * (std::sin(1) - std::sin(3.2)));
        break;
      case 3:
        input_data_ = IntegrationData(1.2, 3, 1, 5, [](double x, double y) -> double { return x * std::log(y); }, 500);
        output_chekup_data_ = 3.78 * (5 * std::log(5) - 4);
        break;
      case 4:
        input_data_ =
            IntegrationData(-0.2, 1.3, 1.5, 2, [](double x, double) -> double { return std::exp(-x * x); }, 500);
        output_chekup_data_ = 0.51252;
        break;
      default:
        input_data_ = IntegrationData{};
        break;
    }
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return std::abs(output_data - output_chekup_data_) < 1;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
  OutType output_chekup_data_{};
};

namespace {

TEST_P(LopatinATrapezoidalIntegrationFuncTests, TrapezoidalIntegration) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 5> kTestParam = {0, 1, 2, 3, 4};

const auto kTestTasksList = std::tuple_cat(ppc::util::AddFuncTask<LopatinATrapezoidalIntegrationMPI, InType>(
                                               kTestParam, PPC_SETTINGS_lopatin_a_trapezoidal_integration),
                                           ppc::util::AddFuncTask<LopatinATrapezoidalIntegrationSEQ, InType>(
                                               kTestParam, PPC_SETTINGS_lopatin_a_trapezoidal_integration));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName =
    LopatinATrapezoidalIntegrationFuncTests::PrintFuncTestName<LopatinATrapezoidalIntegrationFuncTests>;

INSTANTIATE_TEST_SUITE_P(TrapezoidalIntegrationFuncTests, LopatinATrapezoidalIntegrationFuncTests, kGtestValues,
                         kPerfTestName);

}  // namespace

}  // namespace lopatin_a_trapezoidal_integration
