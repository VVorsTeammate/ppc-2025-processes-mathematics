#include "krasnopevtseva_v_bubble_sort/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <utility>
#include <vector>

#include "krasnopevtseva_v_bubble_sort/common/include/common.hpp"

namespace krasnopevtseva_v_bubble_sort {

KrasnopevtsevaVBubbleSortMPI::KrasnopevtsevaVBubbleSortMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = std::vector<int>();
}

bool KrasnopevtsevaVBubbleSortMPI::ValidationImpl() {
  const auto &input = GetInput();
  return (!input.empty());
}

bool KrasnopevtsevaVBubbleSortMPI::PreProcessingImpl() {
  GetOutput() = std::vector<int>();
  return true;
}

bool KrasnopevtsevaVBubbleSortMPI::RunImpl() {
  auto input = GetInput();
  int rank = 0;
  int kol = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &kol);
  if (input.size() <= static_cast<size_t>(kol) || (kol == 1)) {
    SeqSort(input);
    GetOutput() = input;
    return true;
  }

  size_t global_size = input.size();

  std::vector<int> local_data = DistributeData(input, rank, kol);

  ParallelSort(local_data, rank, kol);

  std::vector<int> result = GatherData(local_data, rank, kol, global_size);
  GetOutput() = result;

  return true;
}

void KrasnopevtsevaVBubbleSortMPI::ParallelSort(std::vector<int> &local_data, int rank, int kol) {
  if (local_data.empty() || kol <= 1) {
    return;
  }

  SeqSort(local_data);

  for (int phase = 0; phase < kol; phase++) {
    int partner = FindPartner(rank, kol, phase);

    if (partner != -1) {
      bool keep_smaller = (rank < partner);
      MergeProc(local_data, partner, keep_smaller);
    }

    MPI_Barrier(MPI_COMM_WORLD);
  }
}

int KrasnopevtsevaVBubbleSortMPI::FindPartner(int rank, int kol, int phase) {
  const bool same_parity = ((phase % 2) == (rank % 2));

  if (same_parity && rank + 1 < kol) {
    return rank + 1;
  }
  if (!same_parity && rank > 0) {
    return rank - 1;
  }
  return -1;
}

std::vector<int> KrasnopevtsevaVBubbleSortMPI::DistributeData(const std::vector<int> &input, int rank, int kol) {
  int global_size = static_cast<int>(input.size());
  MPI_Bcast(&global_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

  int base_chunk = global_size / kol;
  int remainder = global_size % kol;

  std::vector<int> send_counts(kol, base_chunk);
  std::vector<int> displacements(kol, 0);

  for (int i = 0; i < kol; i++) {
    if (i < remainder) {
      send_counts[i]++;
    }
    if (i > 0) {
      displacements[i] = displacements[i - 1] + send_counts[i - 1];
    }
  }

  int local_size = send_counts[rank];
  std::vector<int> local_data(local_size);

  MPI_Scatterv(rank == 0 ? input.data() : nullptr, send_counts.data(), displacements.data(), MPI_INT, local_data.data(),
               local_size, MPI_INT, 0, MPI_COMM_WORLD);

  return local_data;
}

void KrasnopevtsevaVBubbleSortMPI::MergeProc(std::vector<int> &data, int partner_rank, bool keep_smaller) {
  int my_size = static_cast<int>(data.size());
  int partner_size = 0;

  MPI_Sendrecv(&my_size, 1, MPI_INT, partner_rank, 0, &partner_size, 1, MPI_INT, partner_rank, 0, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);

  if (partner_size == 0 || my_size == 0) {
    return;
  }

  std::vector<int> partner_data(partner_size);
  MPI_Sendrecv(data.data(), my_size, MPI_INT, partner_rank, 1, partner_data.data(), partner_size, MPI_INT, partner_rank,
               1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  std::vector<int> merged(my_size + partner_size);
  std::ranges::merge(data.begin(), data.end(), partner_data.begin(), partner_data.end(), merged.begin());

  if (keep_smaller) {
    std::ranges::copy(merged.begin(), merged.begin() + my_size, data.begin());
  } else {
    std::ranges::copy(merged.end() - my_size, merged.end(), data.begin());
  }
}

std::vector<int> KrasnopevtsevaVBubbleSortMPI::GatherData(const std::vector<int> &local_data, int rank, int kol,
                                                          size_t global_size) {
  std::vector<int> result;

  if (rank == 0) {
    result.resize(global_size);
    size_t offset = 0;

    int my_size = static_cast<int>(local_data.size());
    if (my_size > 0) {
      std::ranges::copy(local_data.begin(), local_data.end(), result.begin() + static_cast<ptrdiff_t>(offset));
      offset += my_size;
    }

    for (int i = 1; i < kol; i++) {
      int remote_size = 0;
      MPI_Recv(&remote_size, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      if (remote_size > 0) {
        MPI_Recv(result.data() + offset, remote_size, MPI_INT, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        offset += remote_size;
      }
    }
  } else {
    int local_size = static_cast<int>(local_data.size());
    MPI_Send(&local_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

    if (local_size > 0) {
      MPI_Send(local_data.data(), local_size, MPI_INT, 0, 1, MPI_COMM_WORLD);
    }
  }

  int result_size = 0;
  if (rank == 0) {
    result_size = static_cast<int>(global_size);
  }

  MPI_Bcast(&result_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (rank != 0) {
    result.resize(result_size);
  }

  MPI_Bcast(result.data(), result_size, MPI_INT, 0, MPI_COMM_WORLD);

  return result;
}

void KrasnopevtsevaVBubbleSortMPI::SeqSort(std::vector<int> &data) {
  if (data.size() <= 1) {
    return;
  }

  size_t n = data.size();
  bool sorted = false;

  while (!sorted) {
    sorted = true;
    for (size_t i = 1; i < n - 1; i += 2) {
      if (data[i] > data[i + 1]) {
        std::swap(data[i], data[i + 1]);
        sorted = false;
      }
    }
    for (size_t i = 0; i < n - 1; i += 2) {
      if (data[i] > data[i + 1]) {
        std::swap(data[i], data[i + 1]);
        sorted = false;
      }
    }
  }
}

bool KrasnopevtsevaVBubbleSortMPI::PostProcessingImpl() {
  return true;
}
}  // namespace krasnopevtseva_v_bubble_sort
