#include "lopatin_a_trapezoidal_integration/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include "lopatin_a_trapezoidal_integration/common/include/common.hpp"

namespace lopatin_a_trapezoidal_integration {

LopatinATrapezoidalIntegrationMPI::LopatinATrapezoidalIntegrationMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
}

bool LopatinATrapezoidalIntegrationMPI::ValidationImpl() {
  const auto &input = GetInput();
  return (input.b > input.a) && (input.d > input.c) && (input.f != nullptr) && (input.n > 0);
}

bool LopatinATrapezoidalIntegrationMPI::PreProcessingImpl() {
  GetOutput() = 0.0;
  return GetOutput() == 0.0;
}

double LopatinATrapezoidalIntegrationMPI::CalcTail(int start, const IntegrationData &input, double h, double k) {
  double tail_res = 0.0;
  for (int i = start; i <= input.n; ++i) {
    double xi = input.a + (i * h);
    double wx = 1.0;
    if (xi == 0 || xi == input.n) {
      wx = 0.5;
    }

    for (int j = 0; j <= input.n; ++j) {
      double yi = input.c + (j * k);
      double wy = 1.0;
      if (yi == 0 || yi == input.n) {
        wy = 0.5;
      }
      tail_res += wx * wy * input.f(xi, yi);
    }
  }
  tail_res *= h * k;
  return tail_res;
}

bool LopatinATrapezoidalIntegrationMPI::RunImpl() {
  int proc_num = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &proc_num);
  int proc_rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);

  const auto &input = GetInput();
  const auto &f = input.f;

  double h = (input.b - input.a) / input.n;
  double k = (input.d - input.c) / input.n;

  int local_n = (input.n + 1) / proc_num;
  double local_res = 0.0;

  int start = proc_rank * local_n;
  int end = start + local_n;

  for (int i = start; i < end; ++i) {
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
      local_res += wx * wy * f(xi, yi);
    }
  }
  local_res *= h * k;

  double global_res = 0.0;
  MPI_Reduce(&local_res, &global_res, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

  if (proc_rank == 0) {
    start = proc_num * local_n;
    double tail_res = CalcTail(start, input, h, k);
    global_res += tail_res;
  }

  MPI_Bcast(&global_res, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  GetOutput() = global_res;

  MPI_Barrier(MPI_COMM_WORLD);
  return true;
}

bool LopatinATrapezoidalIntegrationMPI::PostProcessingImpl() {
  return true;
}

}  // namespace lopatin_a_trapezoidal_integration
