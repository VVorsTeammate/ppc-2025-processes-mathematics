#include "lopatin_a_star_topology/seq/include/ops_seq.hpp"

#include <vector>

#include "lopatin_a_star_topology/common/include/common.hpp"

namespace lopatin_a_star_topology {

LopatinAStarTopologySEQ::LopatinAStarTopologySEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = OutType{};
}

bool LopatinAStarTopologySEQ::ValidationImpl() {
  return true;
}

bool LopatinAStarTopologySEQ::PreProcessingImpl() {
  return true;
}

bool LopatinAStarTopologySEQ::RunImpl() {
  return true;
}

bool LopatinAStarTopologySEQ::PostProcessingImpl() {
  return true;
}

}  // namespace lopatin_a_star_topology
