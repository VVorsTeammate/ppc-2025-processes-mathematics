#include "orehov_n_topology_star/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <tuple>
#include <vector>

#include "orehov_n_topology_star/common/include/common.hpp"

namespace orehov_n_topology_star {

OrehovNTopologyStarMPI::OrehovNTopologyStarMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = std::vector<int>();
}

bool OrehovNTopologyStarMPI::ValidationImpl() {
  int count_process = 0;
  int rank = 0;
  int status = 1;
  bool error = true;
  MPI_Comm_size(MPI_COMM_WORLD, &count_process);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if (rank == 0) {
    int source = std::get<0>(GetInput());
    int dest = std::get<1>(GetInput());
    int size_array = std::get<2>(GetInput());

    error = (source >= 0 && source < count_process) && (dest >= 0 && dest < count_process) && (size_array >= 0);

    if (count_process == 1) {
      error = true;
    }
  }
  status = static_cast<int>(error);
  MPI_Bcast(&status, 1, MPI_INT, 0, MPI_COMM_WORLD);
  return (status == 1);
}

bool OrehovNTopologyStarMPI::PreProcessingImpl() {
  return true;
}

bool OrehovNTopologyStarMPI::RunImpl() {
  int size = 0;
  int rank = 0;
  int dest = 0;
  int source = 0;
  int size_array = 0;

  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  source = std::get<0>(GetInput());
  dest = std::get<1>(GetInput());
  size_array = std::get<2>(GetInput());

  std::vector<int> data(size_array);
  if (size == 1) {
    data = std::get<3>(GetInput());
    GetOutput() = data;
    return true;
  }

  if (source == dest) {
    data = std::get<3>(GetInput());
    MPI_Bcast(data.data(), size_array, MPI_INT, dest, MPI_COMM_WORLD);
    GetOutput() = data;
    return true;
  }

  if (rank == source) {
    data = std::get<3>(GetInput());
    if (source != 0) {
      MPI_Send(data.data(), size_array, MPI_INT, 0, 0, MPI_COMM_WORLD);
    } else {
      MPI_Send(data.data(), size_array, MPI_INT, dest, 0, MPI_COMM_WORLD);
    }
  }
  if (rank == 0 && source != 0) {
    MPI_Recv(data.data(), size_array, MPI_INT, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    if (dest != 0) {
      MPI_Send(data.data(), size_array, MPI_INT, dest, 0, MPI_COMM_WORLD);
    }
  }
  if (rank == dest && dest != 0) {
    MPI_Recv(data.data(), size_array, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }

  MPI_Bcast(data.data(), size_array, MPI_INT, dest, MPI_COMM_WORLD);
  GetOutput() = data;
  return true;
}

bool OrehovNTopologyStarMPI::PostProcessingImpl() {
  return true;
}

}  // namespace orehov_n_topology_star
