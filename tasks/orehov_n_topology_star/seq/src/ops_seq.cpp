#include "orehov_n_topology_star/seq/include/ops_seq.hpp"

#include <chrono>
#include <thread>
#include <vector>

#include "orehov_n_topology_star/common/include/common.hpp"

namespace orehov_n_topology_star {

OrehovNTopologyStarSEQ::OrehovNTopologyStarSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = std::vector<int>();
}

bool OrehovNTopologyStarSEQ::ValidationImpl() {
  return true;
}

bool OrehovNTopologyStarSEQ::PreProcessingImpl() {
  return true;
}

bool OrehovNTopologyStarSEQ::RunImpl() {
  std::this_thread::sleep_for(std::chrono::milliseconds(2));
  GetOutput() = std::get<3>(GetInput());
  return true;
}

bool OrehovNTopologyStarSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace orehov_n_topology_star
