#include "krasnopevtseva_v_bubble_sort/seq/include/ops_seq.hpp"

#include <algorithm>
#include <cstddef>
#include <vector>

#include "krasnopevtseva_v_bubble_sort/common/include/common.hpp"

namespace krasnopevtseva_v_bubble_sort {

KrasnopevtsevaVBubbleSortSEQ::KrasnopevtsevaVBubbleSortSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = std::vector<int>();
}

bool KrasnopevtsevaVBubbleSortSEQ::ValidationImpl() {
  const auto &input = GetInput();
  return (!input.empty());
}

bool KrasnopevtsevaVBubbleSortSEQ::PreProcessingImpl() {
  GetOutput() = std::vector<int>();
  return true;
}

bool KrasnopevtsevaVBubbleSortSEQ::RunImpl() {
  const auto &input = GetInput();
  size_t size = input.size();
  std::vector<int> sort_v = input;
  bool is_sort = false;
  while (!is_sort) {
    is_sort = true;
    for (size_t i = 0; i < size - 1; i += 2) {
      if (sort_v[i] > sort_v[i + 1]) {
        std::swap(sort_v[i], sort_v[i + 1]);
        is_sort = false;
      }
    }
    for (size_t i = 1; i < size - 1; i += 2) {
      if (sort_v[i] > sort_v[i + 1]) {
        std::swap(sort_v[i], sort_v[i + 1]);
        is_sort = false;
      }
    }
  }

  GetOutput() = sort_v;
  return true;
}

bool KrasnopevtsevaVBubbleSortSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace krasnopevtseva_v_bubble_sort
