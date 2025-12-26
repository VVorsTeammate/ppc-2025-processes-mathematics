#include "krymova_k_quick_sort_simple_merge/seq/include/ops_seq.hpp"

#include <vector>

#include "krymova_k_quick_sort_simple_merge/common/include/common.hpp"

namespace krymova_k_quick_sort_simple_merge {

KrymovaKQuickSortSimpleMergeSEQ::KrymovaKQuickSortSimpleMergeSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = std::vector<int>();
}

bool KrymovaKQuickSortSimpleMergeSEQ::ValidationImpl() {
  return true;
}

bool KrymovaKQuickSortSimpleMergeSEQ::PreProcessingImpl() {
  return true;
}

bool KrymovaKQuickSortSimpleMergeSEQ::RunImpl() {
  std::vector<int> data = GetInput();
  QuickSortIterative(data);
  GetOutput() = data;
  return true;
}

bool KrymovaKQuickSortSimpleMergeSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace krymova_k_quick_sort_simple_merge
