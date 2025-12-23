#pragma once

#include "moskaev_v_hypercub/common/include/common.hpp"
#include "task/include/task.hpp"

namespace moskaev_v_hypercub {

class MoskaevVTestMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit MoskaevVTestMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace moskaev_v_hypercub
