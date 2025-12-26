#pragma once

#include "kulik_a_star/common/include/common.hpp"
#include "task/include/task.hpp"

namespace kulik_a_star {

class KulikAStarSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit KulikAStarSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace kulik_a_star
