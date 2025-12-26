#pragma once

#include "krasnopevtseva_v_bubble_sort/common/include/common.hpp"
#include "task/include/task.hpp"

namespace krasnopevtseva_v_bubble_sort {

class KrasnopevtsevaVBubbleSortSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit KrasnopevtsevaVBubbleSortSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace krasnopevtseva_v_bubble_sort
