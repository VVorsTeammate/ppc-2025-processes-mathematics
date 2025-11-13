#include "moskaev_v_max_value_elem_matrix/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <climits>
#include <cstddef>
#include <utility>

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

  const auto &matrix = GetInput();
  auto total_rows = matrix.size();

  auto rows_per_process = total_rows / size;
  auto remainder = total_rows % size;

  size_t start_row = ((rank * rows_per_process) + (std::min(static_cast<size_t>(rank), remainder)));
  size_t end_row = start_row + rows_per_process;
  if (std::cmp_less(static_cast<size_t>(rank), remainder)) {
    end_row += 1;
  }

  int local_max = INT_MIN;
  for (size_t i = start_row; i < end_row; ++i) {
    for (int element : matrix[i]) {
      local_max = std::max(element, local_max);
    }
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
