#pragma once

#include "lopatin_a_star_topology/common/include/common.hpp"
#include "task/include/task.hpp"

namespace lopatin_a_star_topology {

class LopatinAStarTopologyMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit LopatinAStarTopologyMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace lopatin_a_star_topology
