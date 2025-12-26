#pragma once

#include "lopatin_a_trapezoidal_integration/common/include/common.hpp"
#include "task/include/task.hpp"

namespace lopatin_a_trapezoidal_integration {

class LopatinATrapezoidalIntegrationSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit LopatinATrapezoidalIntegrationSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace lopatin_a_trapezoidal_integration
