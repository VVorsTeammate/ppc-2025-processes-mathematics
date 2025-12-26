#pragma once

#include <cstddef>
#include <vector>

#include "krasnopevtseva_v_bubble_sort/common/include/common.hpp"
#include "task/include/task.hpp"

namespace krasnopevtseva_v_bubble_sort {

class KrasnopevtsevaVBubbleSortMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit KrasnopevtsevaVBubbleSortMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
  static std::vector<int> DistributeData(const std::vector<int> &input, int rank, int kol);
  static void MergeProc(std::vector<int> &data, int partner_rank, bool keep_smaller);
  static std::vector<int> GatherData(const std::vector<int> &local_data, int rank, int kol, size_t global_size);
  static void SeqSort(std::vector<int> &data);
  static void ParallelSort(std::vector<int> &local_data, int rank, int kol);
  static int FindPartner(int rank, int kol, int phase);
};

}  // namespace krasnopevtseva_v_bubble_sort
