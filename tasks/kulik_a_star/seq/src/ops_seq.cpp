#include "kulik_a_star/seq/include/ops_seq.hpp"

#include <chrono>
#include <cmath>
#include <thread>
#include <vector>

#include "kulik_a_star/common/include/common.hpp"

namespace kulik_a_star {

KulikAStarSEQ::KulikAStarSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
}

bool KulikAStarSEQ::ValidationImpl() {
  return true;
}

bool KulikAStarSEQ::PreProcessingImpl() {
  return true;
}

bool KulikAStarSEQ::RunImpl() {
  const auto &input = GetInput();
  GetOutput() = std::get<2>(input);
  std::this_thread::sleep_for(std::chrono::milliseconds(1));
  return true;
}

bool KulikAStarSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace kulik_a_star
