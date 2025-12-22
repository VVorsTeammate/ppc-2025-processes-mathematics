#pragma once

#include "moskaev_v_hypercub/common/include/common.hpp"
#include "task/include/task.hpp"

namespace moskaev_v_hypercub {

class MoskaevVTestSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit MoskaevVTestSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace moskaev_v_hypercub
