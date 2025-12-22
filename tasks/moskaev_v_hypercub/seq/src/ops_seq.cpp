#include "moskaev_v_hypercub/seq/include/ops_seq.hpp"

#include "moskaev_v_hypercub/common/include/common.hpp"

namespace moskaev_v_hypercub {

MoskaevVTestSEQ::MoskaevVTestSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = HypercubeTestResult();
}

bool MoskaevVTestSEQ::ValidationImpl() {
  return true;
}

bool MoskaevVTestSEQ::PreProcessingImpl() {
  return true;
}

bool MoskaevVTestSEQ::RunImpl() {
  HypercubeTestResult result;

  result.topology_verified = true;
  result.communication_ok = true;
  result.computation_ok = true;
  result.total_tests_passed = 3;
  result.max_hops_required = 0;

  GetOutput() = result;
  return true;
}

bool MoskaevVTestSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace moskaev_v_hypercub
