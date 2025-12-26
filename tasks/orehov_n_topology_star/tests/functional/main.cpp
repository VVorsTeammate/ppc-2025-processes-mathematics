#include <gtest/gtest.h>
#include <stb/stb_image.h>

#include <array>
#include <cstddef>
#include <fstream>
#include <string>
#include <tuple>
#include <vector>

#include "orehov_n_topology_star/common/include/common.hpp"
#include "orehov_n_topology_star/mpi/include/ops_mpi.hpp"
#include "orehov_n_topology_star/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace orehov_n_topology_star {

class OrehovNTopologyStarFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return test_param;
  }

 protected:
  void SetUp() override {
    TestType params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    std::string abs_path = ppc::util::GetAbsoluteTaskPath(PPC_ID_orehov_n_topology_star, params + ".txt");
    std::ifstream file(abs_path);
    if (!file.is_open()) {
      throw "file not open\n";
    }

    int source = 0;
    int dest = 0;
    int size = 0;
    file >> source >> dest >> size;
    std::vector<int> data(size);
    for (int i = 0; i < size; i++) {
      file >> data[i];
    }

    file.close();

    input_data_ = std::make_tuple(source, dest, size, data);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    std::vector<int> source_data = std::get<3>(input_data_);
    int size = std::get<2>(input_data_);

    for (int i = 0; i < size; i++) {
      if (source_data[i] != output_data[i]) {
        return false;
      }
    }
    return true;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
};

namespace {

TEST_P(OrehovNTopologyStarFuncTests, MatmulFromPic) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 5> kTestParam = {"data1", "data2", "data3", "data4", "data5"};

const auto kTestTasksList = std::tuple_cat(
    ppc::util::AddFuncTask<OrehovNTopologyStarMPI, InType>(kTestParam, PPC_SETTINGS_orehov_n_topology_star),
    ppc::util::AddFuncTask<OrehovNTopologyStarSEQ, InType>(kTestParam, PPC_SETTINGS_orehov_n_topology_star));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName = OrehovNTopologyStarFuncTests::PrintFuncTestName<OrehovNTopologyStarFuncTests>;

INSTANTIATE_TEST_SUITE_P(PicMatrixTests, OrehovNTopologyStarFuncTests, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace orehov_n_topology_star
