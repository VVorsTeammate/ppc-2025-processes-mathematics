#include "moskaev_v_max_value_elem_matrix/seq/include/ops_seq.hpp"

#include <algorithm>

#include "moskaev_v_max_value_elem_matrix/common/include/common.hpp"

namespace moskaev_v_max_value_elem_matrix {

MoskaevVMaxValueElemMatrixSEQ::MoskaevVMaxValueElemMatrixSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = InType(in);
  GetOutput() = 0;
}

bool MoskaevVMaxValueElemMatrixSEQ::ValidationImpl() {
  return (GetOutput() == 0);
}

bool MoskaevVMaxValueElemMatrixSEQ::PreProcessingImpl() {
  return true;
}

bool MoskaevVMaxValueElemMatrixSEQ::RunImpl() {
  if (GetInput().empty()) {
    GetOutput() = 0;
    return true;
  }
  const auto &matrix = GetInput();

  int max_element = matrix[0][0];
  for (const auto &row : matrix) {
    for (int element : row) {
      max_element = std::max(element, max_element);
    }
  }

  GetOutput() = max_element;
  return true;
}

bool MoskaevVMaxValueElemMatrixSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace moskaev_v_max_value_elem_matrix
