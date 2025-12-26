#include "chernykh_s_hypercube/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <vector>

#include "chernykh_s_hypercube/common/include/common.hpp"

namespace chernykh_s_hypercube {

namespace {
bool ProcessHypercubeStep(int &local_data, int step, int rank, int size) {
  int mask = 1 << step;
  int neighbor = rank ^ mask;

  if ((rank & (mask - 1)) != 0) {
    return false;
  }

  if ((rank & mask) != 0) {
    MPI_Send(&local_data, 1, MPI_INT, neighbor, 0, MPI_COMM_WORLD);
    return false;
  }

  if (neighbor < size) {
    int received_val = 0;
    MPI_Recv(&received_val, 1, MPI_INT, neighbor, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    local_data += received_val;
  }
  return true;
}
}  // namespace

ChernykhSHypercubeMPI::ChernykhSHypercubeMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = InType(in);
  GetOutput() = 0;
}

bool ChernykhSHypercubeMPI::ValidationImpl() {
  int size = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  bool is_power_of_two = (size > 0) && ((size & (size - 1)) == 0);
  if (!is_power_of_two) {
    return false;
  }

  if (GetInput().empty()) {
    return false;
  }

  return std::ranges::all_of(GetInput(), [size](int r) { return r >= 0 && r < size; });
}

bool ChernykhSHypercubeMPI::PreProcessingImpl() {
  return true;
}

bool ChernykhSHypercubeMPI::RunImpl() {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  const std::vector<int> &active_nodes = GetInput();

  int current_val = 0;
  if (std::ranges::find(active_nodes, rank) != active_nodes.end()) {
    current_val = rank;
  }

  volatile double workload_accumulator = 0.0;
  for (int idx = 0; idx < 10000; idx++) {
    for (int jdx = 1; jdx < 10000; jdx++) {
      workload_accumulator += static_cast<double>(idx) / static_cast<double>(jdx);
    }
  }

  if (workload_accumulator < 0) {
    rank += 1;
  }

  int dims = 0;
  while ((1 << dims) < size) {
    dims++;
  }

  for (int i = 0; i < dims; ++i) {
    if (!ProcessHypercubeStep(current_val, i, rank, size)) {
      break;
    }
  }

  if (rank == 0) {
    GetOutput() = current_val;
  }

  return true;
}

bool ChernykhSHypercubeMPI::PostProcessingImpl() {
  return true;
}
}  // namespace chernykh_s_hypercube
