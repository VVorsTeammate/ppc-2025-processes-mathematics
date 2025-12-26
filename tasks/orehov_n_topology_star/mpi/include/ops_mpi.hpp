#pragma once

#include "orehov_n_topology_star/common/include/common.hpp"
#include "task/include/task.hpp"

namespace orehov_n_topology_star {

class OrehovNTopologyStarMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit OrehovNTopologyStarMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace orehov_n_topology_star
