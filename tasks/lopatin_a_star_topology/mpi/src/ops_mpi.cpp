#include "lopatin_a_star_topology/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <utility>
#include <vector>

#include "lopatin_a_star_topology/common/include/common.hpp"

namespace lopatin_a_star_topology {

LopatinAStarTopologyMPI::LopatinAStarTopologyMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  int proc_rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);
  int src_rank = std::get<0>(in);

  if (proc_rank == src_rank) {
    GetInput() = in;
  } else {
    GetInput() = InType{};
    std::get<0>(GetInput()) = std::get<0>(in);
    std::get<1>(GetInput()) = std::get<1>(in);
    std::get<2>(GetInput()) = std::get<2>(in);
  }

  GetOutput() = OutType{};
}

bool LopatinAStarTopologyMPI::ValidationImpl() {
  int proc_num = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &proc_num);
  int proc_rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);

  int src_rank = std::get<0>(GetInput());
  int dst_rank = std::get<1>(GetInput());
  int msg_size = std::get<2>(GetInput());
  if (proc_rank == src_rank) {
    const auto &input = std::get<3>(GetInput());
    return (src_rank >= 0) && (dst_rank >= 0) && (src_rank < proc_num) && (dst_rank < proc_num) &&
           std::cmp_equal(msg_size, input.size()) && !input.empty();
  }

  return true;
}

bool LopatinAStarTopologyMPI::PreProcessingImpl() {
  GetOutput() = OutType{};
  return GetOutput().empty();
}

bool LopatinAStarTopologyMPI::RunImpl() {
  int proc_num = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &proc_num);
  int proc_rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);

  const int center_rank = 0;
  const int src_rank = std::get<0>(GetInput());
  const int dst_rank = std::get<1>(GetInput());
  const int msg_size = std::get<2>(GetInput());
  OutType &output = GetOutput();

  if (src_rank == dst_rank) {
    if (proc_rank == src_rank) {
      const auto &input = std::get<3>(GetInput());
      output = input;
    }
    return true;
  }

  if ((src_rank * dst_rank) == 0) {
    if (proc_rank == src_rank) {
      const auto &input = std::get<3>(GetInput());
      MPI_Send(input.data(), msg_size, MPI_DOUBLE, dst_rank, 0, MPI_COMM_WORLD);

    } else if (proc_rank == dst_rank) {
      output.resize(msg_size);
      MPI_Status status{};
      MPI_Recv(output.data(), msg_size, MPI_DOUBLE, src_rank, 0, MPI_COMM_WORLD, &status);
    }
  } else {
    if (proc_rank == src_rank) {
      const auto &input = std::get<3>(GetInput());
      MPI_Send(input.data(), msg_size, MPI_DOUBLE, center_rank, 0, MPI_COMM_WORLD);

    } else if (proc_rank == center_rank) {
      std::vector<double> buf(msg_size);
      MPI_Status status{};
      MPI_Recv(buf.data(), msg_size, MPI_DOUBLE, src_rank, 0, MPI_COMM_WORLD, &status);
      MPI_Send(buf.data(), msg_size, MPI_DOUBLE, dst_rank, 0, MPI_COMM_WORLD);

    } else if (proc_rank == dst_rank) {
      output.resize(msg_size);
      MPI_Status status{};
      MPI_Recv(output.data(), msg_size, MPI_DOUBLE, center_rank, 0, MPI_COMM_WORLD, &status);
    }
  }

  MPI_Barrier(MPI_COMM_WORLD);
  return true;
}

bool LopatinAStarTopologyMPI::PostProcessingImpl() {
  return true;
}

}  // namespace lopatin_a_star_topology
