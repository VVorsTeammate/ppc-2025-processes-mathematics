#pragma once

#include "moskaev_v_hypercub/common/include/common.hpp"
#include "task/include/task.hpp"

namespace moskaev_v_hypercub {

class MoskaevVTestMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit MoskaevVTestMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;

  void TestTopology(int rank, int size, HypercubeTestResult &result);
  void TestCommunication(int rank, int size, HypercubeTestResult &result);
  void TestComputation(int rank, int size, HypercubeTestResult &result);
  void FinalizeResults(int rank, HypercubeTestResult &result);
};

}  // namespace moskaev_v_hypercub
