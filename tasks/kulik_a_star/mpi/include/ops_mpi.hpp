#pragma once

#include <cstddef>
#include <vector>

#include "kulik_a_star/common/include/common.hpp"
#include "task/include/task.hpp"

namespace kulik_a_star {

class KulikAStarMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit KulikAStarMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
  static int FindActualSourceRank(int proc_num, int has_data);
  static void HandleSameSourceDestination(int proc_rank, int source_rank, size_t size,
                                          const std::vector<int> &source_data, std::vector<int> &output);
  static void HandleDifferentSourceDestination(int proc_rank, int source_rank, int destination_rank, size_t size,
                                               const std::vector<int> &source_data, std::vector<int> &output);
  static void ProcessZeroRouting(int source_rank, int destination_rank, size_t size,
                                 const std::vector<int> &source_data, std::vector<int> &output);
  static void ProcessDestination(int destination_rank, size_t size, std::vector<int> &output);
  static void ProcessSource(int source_rank, size_t size, const std::vector<int> &source_data);
};

}  // namespace kulik_a_star
