#include "lopatin_a_trapezoidal_integration/seq/include/ops_seq.hpp"

#include "lopatin_a_trapezoidal_integration/common/include/common.hpp"

namespace lopatin_a_trapezoidal_integration {

LopatinATrapezoidalIntegrationSEQ::LopatinATrapezoidalIntegrationSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
}

bool LopatinATrapezoidalIntegrationSEQ::ValidationImpl() {
  const auto &input = GetInput();
  return (input.b > input.a) && (input.d > input.c) && (input.f != nullptr) && (input.n > 0);
}

bool LopatinATrapezoidalIntegrationSEQ::PreProcessingImpl() {
  GetOutput() = 0.0;
  return GetOutput() == 0.0;
}

bool LopatinATrapezoidalIntegrationSEQ::RunImpl() {
  const auto &input = GetInput();
  const auto &f = input.f;
  auto &output = GetOutput();

  double h = (input.b - input.a) / input.n;
  double k = (input.d - input.c) / input.n;

  double res = 0.0;
  for (int i = 0; i <= input.n; ++i) {
    double xi = input.a + (i * h);
    double wx = 1.0;
    if (i == 0 || i == input.n) {
      wx = 0.5;
    }

    for (int j = 0; j <= input.n; ++j) {
      double yi = input.c + (j * k);
      double wy = 1.0;
      if (j == 0 || j == input.n) {
        wy = 0.5;
      }
      res += wx * wy * f(xi, yi);
    }
  }

  output = h * k * res;

  return true;
}

bool LopatinATrapezoidalIntegrationSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace lopatin_a_trapezoidal_integration
