#pragma once

#include "lopatin_a_trapezoidal_integration/common/include/common.hpp"
#include "task/include/task.hpp"

namespace lopatin_a_trapezoidal_integration {

class LopatinATrapezoidalIntegrationMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit LopatinATrapezoidalIntegrationMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;

  static double CalcTail(int start, const IntegrationData &input, double h, double k);
};

}  // namespace lopatin_a_trapezoidal_integration
