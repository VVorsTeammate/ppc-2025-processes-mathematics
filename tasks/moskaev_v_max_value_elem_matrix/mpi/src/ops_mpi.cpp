#include "moskaev_v_max_value_elem_matrix/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <climits>
#include <cstddef>
#include <vector>

#include "moskaev_v_max_value_elem_matrix/common/include/common.hpp"

namespace moskaev_v_max_value_elem_matrix {

MoskaevVMaxValueElemMatrixMPI::MoskaevVMaxValueElemMatrixMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = InType(in);
  GetOutput() = 0;
}

bool MoskaevVMaxValueElemMatrixMPI::ValidationImpl() {
  return (GetOutput() == 0);
}

bool MoskaevVMaxValueElemMatrixMPI::PreProcessingImpl() {
  return true;
}

bool MoskaevVMaxValueElemMatrixMPI::RunImpl() {
  if (GetInput().empty()) {
    GetOutput() = 0;
    return true;
  }
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int total_rows = 0;
  int cols = 0;
  std::vector<int> flat_matrix;

  if (rank == 0) {
    const auto &matrix = GetInput();
    total_rows = static_cast<int>(matrix.size());
    if (total_rows > 0) {
      cols = static_cast<int>(matrix[0].size());

      flat_matrix.reserve(static_cast<size_t>(total_rows) * static_cast<size_t>(cols));
      for (const auto &row : matrix) {
        flat_matrix.insert(flat_matrix.end(), row.begin(), row.end());
      }
    }
  }

  MPI_Bcast(&total_rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&cols, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (total_rows == 0 || cols == 0) {
    GetOutput() = 0;
    return true;
  }

  int rows_per_process = total_rows / size;
  int remainder = total_rows % size;

  int my_rows = rows_per_process + (rank < remainder ? 1 : 0);
  int my_elements = my_rows * cols;

  std::vector<int> local_data(my_elements, 0);

  std::vector<int> sendcounts(size);
  std::vector<int> displs(size);

  int offset = 0;
  for (int i = 0; i < size; i++) {
    int rows_for_i = rows_per_process + (i < remainder ? 1 : 0);
    sendcounts[i] = rows_for_i * cols;
    displs[i] = offset * cols;
    offset += rows_for_i;
  }

  MPI_Scatterv(rank == 0 ? flat_matrix.data() : nullptr, sendcounts.data(), displs.data(), MPI_INT, local_data.data(),
               my_elements, MPI_INT, 0, MPI_COMM_WORLD);

  int local_max = INT_MIN;
  for (int element : local_data) {
    local_max = std::max(local_max, element);
  }

  int global_max = 0;
  MPI_Allreduce(&local_max, &global_max, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

  GetOutput() = global_max;

  return true;
}

bool MoskaevVMaxValueElemMatrixMPI::PostProcessingImpl() {
  return true;
}

}  // namespace moskaev_v_max_value_elem_matrix
