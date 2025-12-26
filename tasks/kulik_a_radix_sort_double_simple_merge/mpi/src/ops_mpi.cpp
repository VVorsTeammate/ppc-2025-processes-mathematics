#include "kulik_a_radix_sort_double_simple_merge/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#include "kulik_a_radix_sort_double_simple_merge/common/include/common.hpp"

namespace kulik_a_radix_sort_double_simple_merge {

KulikARadixSortDoubleSimpleMergeMPI::KulikARadixSortDoubleSimpleMergeMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  int proc_rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);
  if (proc_rank == 0) {
    GetInput() = in;
  } else {
    GetInput() = InType{};
  }
}

bool KulikARadixSortDoubleSimpleMergeMPI::ValidationImpl() {
  int proc_rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);
  if (proc_rank == 0) {
    return (!GetInput().empty());
  }
  return true;
}

bool KulikARadixSortDoubleSimpleMergeMPI::PreProcessingImpl() {
  return true;
}

double *KulikARadixSortDoubleSimpleMergeMPI::LSDSortBytes(double *arr, double *buffer, size_t size) {
  double *parr = arr;
  double *pbuffer = buffer;
  for (uint64_t byte = 0; byte < sizeof(double); ++byte) {
    std::vector<uint64_t> count(256, 0);
    auto *bytes = reinterpret_cast<unsigned char *>(parr);
    for (size_t i = 0; i < size; ++i) {
      count[bytes[(sizeof(double) * i) + byte]]++;
    }
    uint64_t pos = 0;
    for (uint64_t i = 0; i < 256; ++i) {
      uint64_t temp = count[i];
      count[i] = pos;
      pos += temp;
    }
    for (size_t i = 0; i < size; ++i) {
      unsigned char byte_value = bytes[(sizeof(double) * i) + byte];
      uint64_t new_pos = count[byte_value]++;
      pbuffer[new_pos] = parr[i];
    }
    std::swap(parr, pbuffer);
  }
  return parr;
}

void KulikARadixSortDoubleSimpleMergeMPI::AdjustNegativeNumbers(std::vector<double> &arr, size_t size) {
  size_t neg_start = 0;
  while (neg_start < size && arr[neg_start] >= 0.0) {
    ++neg_start;
  }
  if (neg_start < size) {
    for (size_t i = neg_start, j = size - 1; i < j; ++i, --j) {
      std::swap(arr[i], arr[j]);
    }
    std::vector<double> temp(size);
    size_t index = 0;
    for (size_t i = neg_start; i < size; ++i) {
      temp[index++] = arr[i];
    }
    for (size_t i = 0; i < neg_start; ++i) {
      temp[index++] = arr[i];
    }
    arr = std::move(temp);
  }
}

void KulikARadixSortDoubleSimpleMergeMPI::LSDSortLocal(std::vector<double> &local_arr) {
  size_t size = local_arr.size();
  if (size <= 1) {
    return;
  }
  std::vector<double> buffer(size);
  double *sorted_ptr = LSDSortBytes(local_arr.data(), buffer.data(), size);
  if (sorted_ptr == buffer.data()) {
    std::ranges::copy(buffer, local_arr.begin());
  }
  AdjustNegativeNumbers(local_arr, size);
}

std::vector<double> KulikARadixSortDoubleSimpleMergeMPI::SimpleMerge(
    const std::vector<std::vector<double>> &sorted_arrays) {
  if (sorted_arrays.empty()) {
    return {};
  }
  size_t global_size = 0;
  for (const auto &arr : sorted_arrays) {
    global_size += arr.size();
  }
  std::vector<double> result;
  result.reserve(global_size);
  std::vector<size_t> indices(sorted_arrays.size(), 0);
  while (result.size() < global_size) {
    int min_idx = -1;
    double min_val = 0.0;
    bool first = true;
    for (size_t i = 0; i < sorted_arrays.size(); ++i) {
      if (indices[i] < sorted_arrays[i].size()) {
        double val = sorted_arrays[i][indices[i]];
        if (first || val < min_val) {
          min_val = val;
          min_idx = static_cast<int>(i);
          first = false;
        }
      }
    }
    if (min_idx != -1) {
      result.push_back(min_val);
      indices[min_idx]++;
    }
  }
  return result;
}

void KulikARadixSortDoubleSimpleMergeMPI::LSDSortDouble(std::vector<double> &arr) {
  int proc_rank = 0;
  int proc_num = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &proc_num);
  size_t global_size = arr.size();
  MPI_Bcast(&global_size, 1, MPI_UINT64_T, 0, MPI_COMM_WORLD);
  size_t local_size = global_size / proc_num;
  size_t r = global_size % proc_num;
  if (std::cmp_less(proc_rank, r)) {
    local_size++;
  }
  std::vector<double> local_arr(local_size);
  std::vector<int> sendcounts(proc_num);
  std::vector<int> displs(proc_num);
  if (proc_rank == 0) {
    int offset = 0;
    for (int i = 0; i < proc_num; ++i) {
      sendcounts[i] = std::cmp_less(i, r) ? static_cast<int>((global_size / proc_num) + 1)
                                          : static_cast<int>(global_size / proc_num);
      displs[i] = offset;
      offset += sendcounts[i];
    }
  }
  MPI_Scatterv(arr.data(), sendcounts.data(), displs.data(), MPI_DOUBLE, local_arr.data(), static_cast<int>(local_size),
               MPI_DOUBLE, 0, MPI_COMM_WORLD);
  LSDSortLocal(local_arr);
  std::vector<int> recv_counts(proc_num);
  int local_size_int = static_cast<int>(local_size);
  MPI_Gather(&local_size_int, 1, MPI_INT, recv_counts.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);
  std::vector<int> recv_displs(proc_num);
  if (proc_rank == 0) {
    recv_displs[0] = 0;
    for (int i = 1; i < proc_num; ++i) {
      recv_displs[i] = recv_displs[i - 1] + recv_counts[i - 1];
    }
  }
  if (proc_rank == 0) {
    arr.resize(global_size);
  }
  MPI_Gatherv(local_arr.data(), local_size_int, MPI_DOUBLE, arr.data(), recv_counts.data(), recv_displs.data(),
              MPI_DOUBLE, 0, MPI_COMM_WORLD);
  if (proc_rank == 0) {
    std::vector<std::vector<double>> sorted_parts;
    for (int i = 0; i < proc_num; ++i) {
      std::vector<double> part(arr.begin() + recv_displs[i], arr.begin() + recv_displs[i] + recv_counts[i]);
      sorted_parts.push_back(std::move(part));
    }
    arr = SimpleMerge(sorted_parts);
  }
}

bool KulikARadixSortDoubleSimpleMergeMPI::RunImpl() {
  GetOutput() = GetInput();
  LSDSortDouble(GetOutput());
  return true;
}

bool KulikARadixSortDoubleSimpleMergeMPI::PostProcessingImpl() {
  int proc_rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &proc_rank);
  size_t size = 0;
  if (proc_rank == 0) {
    size = GetOutput().size();
  }
  MPI_Bcast(&size, 1, MPI_UINT64_T, 0, MPI_COMM_WORLD);
  GetOutput().resize(size);
  MPI_Bcast(GetOutput().data(), static_cast<int>(size), MPI_DOUBLE, 0, MPI_COMM_WORLD);
  return (!GetOutput().empty());
}

}  // namespace kulik_a_radix_sort_double_simple_merge
