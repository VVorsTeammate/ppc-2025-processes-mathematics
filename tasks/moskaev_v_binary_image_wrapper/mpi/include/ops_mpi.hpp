#pragma once

#include "moskaev_v_binary_image_wrapper/common/include/common.hpp"
#include "task/include/task.hpp"

namespace moskaev_v_binary_image_wrapper {

class MoskaevVTestTaskMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit MoskaevVTestTaskMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace moskaev_v_binary_image_wrapper
