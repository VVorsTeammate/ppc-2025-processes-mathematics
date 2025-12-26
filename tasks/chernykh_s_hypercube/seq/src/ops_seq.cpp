#include "chernykh_s_hypercube/seq/include/ops_seq.hpp"

#include "chernykh_s_hypercube/common/include/common.hpp"

namespace chernykh_s_hypercube {

ChernykhSHypercubeSEQ::ChernykhSHypercubeSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = InType(in);
  GetOutput() = 0;
}

bool ChernykhSHypercubeSEQ::ValidationImpl() {
  return !GetInput().empty();
}

bool ChernykhSHypercubeSEQ::PreProcessingImpl() {
  return true;
}

bool ChernykhSHypercubeSEQ::RunImpl() {
  return true;
}

bool ChernykhSHypercubeSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace chernykh_s_hypercube
