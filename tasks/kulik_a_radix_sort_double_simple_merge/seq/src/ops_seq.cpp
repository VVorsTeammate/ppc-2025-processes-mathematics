#include "kulik_a_radix_sort_double_simple_merge/seq/include/ops_seq.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#include "kulik_a_radix_sort_double_simple_merge/common/include/common.hpp"

namespace kulik_a_radix_sort_double_simple_merge {

KulikARadixSortDoubleSimpleMergeSEQ::KulikARadixSortDoubleSimpleMergeSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
}

bool KulikARadixSortDoubleSimpleMergeSEQ::ValidationImpl() {
  return (!GetInput().empty());
}

bool KulikARadixSortDoubleSimpleMergeSEQ::PreProcessingImpl() {
  return true;
}

double *KulikARadixSortDoubleSimpleMergeSEQ::LSDSortBytes(double *arr, double *buffer, size_t size) {
  double *parr = arr;
  double *pbuffer = buffer;
  for (uint64_t byte = 0; byte < sizeof(double); ++byte) {
    std::vector<uint64_t> count(256, 0);
    auto *bytes = reinterpret_cast<unsigned char *>(parr);
    for (size_t i = 0; i < size; ++i) {
      count[bytes[(sizeof(double) * i) + byte]]++;
    }
    uint64_t pos = 0;
    for (uint64_t i = 0; i < 256; ++i) {
      uint64_t temp = count[i];
      count[i] = pos;
      pos += temp;
    }
    for (size_t i = 0; i < size; ++i) {
      unsigned char byte_value = bytes[(sizeof(double) * i) + byte];
      uint64_t new_pos = count[byte_value]++;
      pbuffer[new_pos] = parr[i];
    }
    std::swap(parr, pbuffer);
  }
  return parr;
}

void KulikARadixSortDoubleSimpleMergeSEQ::AdjustNegativeNumbers(std::vector<double> &arr, size_t size) {
  size_t neg_start = 0;
  while (neg_start < size && arr[neg_start] >= 0.0) {
    ++neg_start;
  }
  if (neg_start < size) {
    for (size_t i = neg_start, j = size - 1; i < j; ++i, --j) {
      std::swap(arr[i], arr[j]);
    }
    std::vector<double> temp(size);
    size_t index = 0;
    for (size_t i = neg_start; i < size; ++i) {
      temp[index++] = arr[i];
    }
    for (size_t i = 0; i < neg_start; ++i) {
      temp[index++] = arr[i];
    }
    arr = std::move(temp);
  }
}

void KulikARadixSortDoubleSimpleMergeSEQ::LSDSortDouble(std::vector<double> &arr) {
  size_t size = arr.size();
  std::vector<double> buffer(size);
  double *sorted_ptr = LSDSortBytes(arr.data(), buffer.data(), size);
  if (sorted_ptr == buffer.data()) {
    std::ranges::copy(buffer, arr.begin());
  }
  AdjustNegativeNumbers(arr, size);
}

bool KulikARadixSortDoubleSimpleMergeSEQ::RunImpl() {
  GetOutput() = GetInput();
  LSDSortDouble(GetOutput());
  return true;
}

bool KulikARadixSortDoubleSimpleMergeSEQ::PostProcessingImpl() {
  return (!GetOutput().empty());
}

}  // namespace kulik_a_radix_sort_double_simple_merge
