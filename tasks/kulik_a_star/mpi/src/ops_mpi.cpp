#include "kulik_a_star/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <chrono>
#include <climits>
#include <cmath>
#include <cstddef>
#include <thread>
#include <vector>

#include "kulik_a_star/common/include/common.hpp"

namespace kulik_a_star {

KulikAStarMPI::KulikAStarMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  auto source_rank = std::get<0>(in);
  int proc_rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);
  if (proc_rank == source_rank) {
    GetInput() = in;
  } else {
    GetInput() = InType{};
  }
}

bool KulikAStarMPI::ValidationImpl() {
  int proc_num = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &proc_num);
  int proc_rank = 0;
  auto source_rank = std::get<0>(GetInput());
  auto destination_rank = std::get<1>(GetInput());
  MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);
  if (proc_rank == source_rank) {
    bool fs = (source_rank >= 0);
    bool fd = (destination_rank >= 0);
    if (proc_num >= 3) {
      fs = fs && (source_rank < proc_num);
      fd = fd && (destination_rank < proc_num);
    }
    return (fs && fd);
  }
  return true;
}

bool KulikAStarMPI::PreProcessingImpl() {
  return true;
}

int KulikAStarMPI::FindActualSourceRank(int proc_num, int has_data) {
  std::vector<int> all_has_data(proc_num);
  MPI_Allgather(&has_data, 1, MPI_INT, all_has_data.data(), 1, MPI_INT, MPI_COMM_WORLD);
  for (int i = 0; i < proc_num; i++) {
    if (all_has_data[i] == 1) {
      return i;
    }
  }
  return -1;
}

void KulikAStarMPI::HandleSameSourceDestination(int proc_rank, int source_rank, size_t size,
                                                const std::vector<int> &source_data, std::vector<int> &output) {
  if (proc_rank == source_rank) {
    output = source_data;
  }
  MPI_Bcast(output.data(), static_cast<int>(size), MPI_INT, source_rank, MPI_COMM_WORLD);
}

void KulikAStarMPI::ProcessZeroRouting(int source_rank, int destination_rank, size_t size,
                                       const std::vector<int> &source_data, std::vector<int> &output) {
  std::vector<int> buff(size);
  MPI_Status status;

  if (source_rank != 0) {
    MPI_Recv(buff.data(), static_cast<int>(size), MPI_INT, source_rank, 0, MPI_COMM_WORLD, &status);
    if (destination_rank != 0) {
      MPI_Send(buff.data(), static_cast<int>(size), MPI_INT, destination_rank, 0, MPI_COMM_WORLD);
    } else {
      output = buff;
    }
  } else {
    MPI_Send(source_data.data(), static_cast<int>(size), MPI_INT, destination_rank, 0, MPI_COMM_WORLD);
  }
}

void KulikAStarMPI::ProcessDestination(int destination_rank, size_t size, std::vector<int> &output) {
  std::vector<int> buff(size);
  if (destination_rank != 0) {
    MPI_Status status;
    MPI_Recv(buff.data(), static_cast<int>(size), MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
  }
  output = buff;
}

void KulikAStarMPI::ProcessSource(int source_rank, size_t size, const std::vector<int> &source_data) {
  if (source_rank != 0) {
    MPI_Send(source_data.data(), static_cast<int>(size), MPI_INT, 0, 0, MPI_COMM_WORLD);
  }
}

void KulikAStarMPI::HandleDifferentSourceDestination(int proc_rank, int source_rank, int destination_rank, size_t size,
                                                     const std::vector<int> &source_data, std::vector<int> &output) {
  if (proc_rank == 0) {
    ProcessZeroRouting(source_rank, destination_rank, size, source_data, output);
  } else if (proc_rank == destination_rank) {
    ProcessDestination(destination_rank, size, output);
  } else if (proc_rank == source_rank) {
    ProcessSource(source_rank, size, source_data);
  }
  MPI_Bcast(output.data(), static_cast<int>(size), MPI_INT, destination_rank, MPI_COMM_WORLD);
}

bool KulikAStarMPI::RunImpl() {
  int proc_num = 0;
  int proc_rank = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &proc_num);
  MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);
  const auto &input = GetInput();
  auto source_rank = std::get<0>(input);
  if (proc_num < 3) {
    GetOutput().resize(1U);
    GetOutput()[0] = INT_MAX;
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  } else {
    auto destination_rank = std::get<1>(input);
    size_t size = std::get<2>(input).size();
    int has_data = (size > 0) ? 1 : 0;
    int actual_source = FindActualSourceRank(proc_num, has_data);
    MPI_Bcast(&source_rank, 1, MPI_INT, actual_source, MPI_COMM_WORLD);
    MPI_Bcast(&destination_rank, 1, MPI_INT, actual_source, MPI_COMM_WORLD);
    MPI_Bcast(&size, 1, MPI_UINT64_T, actual_source, MPI_COMM_WORLD);
    GetOutput().resize(size);
    if (source_rank == destination_rank) {
      HandleSameSourceDestination(proc_rank, source_rank, size, std::get<2>(input), GetOutput());
    } else {
      HandleDifferentSourceDestination(proc_rank, source_rank, destination_rank, size, std::get<2>(input), GetOutput());
    }
  }
  return true;
}

bool KulikAStarMPI::PostProcessingImpl() {
  return true;
}

}  // namespace kulik_a_star
