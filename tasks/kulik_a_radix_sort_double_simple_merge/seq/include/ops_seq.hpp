#pragma once

#include <cstddef>
#include <vector>

#include "kulik_a_radix_sort_double_simple_merge/common/include/common.hpp"
#include "task/include/task.hpp"

namespace kulik_a_radix_sort_double_simple_merge {

class KulikARadixSortDoubleSimpleMergeSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit KulikARadixSortDoubleSimpleMergeSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
  static double *LSDSortBytes(double *arr, double *buffer, size_t size);
  static void AdjustNegativeNumbers(std::vector<double> &arr, size_t size);
  static void LSDSortDouble(std::vector<double> &arr);
};

}  // namespace kulik_a_radix_sort_double_simple_merge
