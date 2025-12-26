#pragma once

#include <functional>
#include <utility>

#include "task/include/task.hpp"

namespace lopatin_a_trapezoidal_integration {

struct IntegrationData {
  // boundaries of the rectangular integration region
  double a, b;
  double c, d;
  // integrand
  std::function<double(double, double)> f;
  // grid dimension
  int n;

  explicit IntegrationData(double a = 0.0, double b = 0.0, double c = 0.0, double d = 0.0,
                           std::function<double(double, double)> &&f = [](double, double) { return 0.0; },
                           int n = 0) noexcept
      : a(a), b(b), c(c), d(d), f(std::move(f)), n(n) {};
};

using InType = IntegrationData;
using OutType = double;
using TestType = int;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace lopatin_a_trapezoidal_integration
