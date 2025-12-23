#pragma once

#include "moskaev_v_max_value_elem_matrix/common/include/common.hpp"
#include "task/include/task.hpp"

namespace moskaev_v_max_value_elem_matrix {

class MoskaevVMaxValueElemMatrixSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit MoskaevVMaxValueElemMatrixSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace moskaev_v_max_value_elem_matrix
