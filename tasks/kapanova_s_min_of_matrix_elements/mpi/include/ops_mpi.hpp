#pragma once

#include "kapanova_s_min_of_matrix_elements/common/include/common.hpp"
#include "task/include/task.hpp"

namespace kapanova_s_min_of_matrix_elements {

class KapanovaSMinOfMatrixElementsMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit KapanovaSMinOfMatrixElementsMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace kapanova_s_min_of_matrix_elements
