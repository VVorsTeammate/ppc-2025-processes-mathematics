#include "moskaev_v_hypercub/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <cmath>
#include <iostream>
#include <random>

#include "moskaev_v_hypercub/common/include/common.hpp"

namespace moskaev_v_hypercub {

namespace {

int CalculateHops(int src, int dest) {
  int diff = src ^ dest;
  int hops = 0;

  while (diff != 0) {
    hops++;
    diff &= (diff - 1);
  }

  return hops;
}

int HypercubeSum(int local_value, int rank, int size) {
  int result = local_value;
  int dims = 0;

  while ((1 << dims) < size) {
    dims++;
  }

  for (int dim = 0; dim < dims; dim++) {
    int partner = rank ^ (1 << dim);

    if (partner < size) {
      int recv_value = 0;

      if (rank < partner) {
        MPI_Send(&result, 1, MPI_INT, partner, dim, MPI_COMM_WORLD);
        MPI_Recv(&recv_value, 1, MPI_INT, partner, dim, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      } else {
        MPI_Recv(&recv_value, 1, MPI_INT, partner, dim, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Send(&result, 1, MPI_INT, partner, dim, MPI_COMM_WORLD);
      }

      result += recv_value;
    }

    MPI_Barrier(MPI_COMM_WORLD);
  }

  return result;
}

}  // namespace

namespace {
void SyncResultToAllProcesses(HypercubeTestResult &result, int rank) {
  struct SyncData {
    int total_tests_passed;
    int topology_verified;
    int communication_ok;
    int computation_ok;
    int max_hops_required;
  };

  SyncData data{};

  if (rank == 0) {
    data.total_tests_passed = result.total_tests_passed;
    data.topology_verified = result.topology_verified ? 1 : 0;
    data.communication_ok = result.communication_ok ? 1 : 0;
    data.computation_ok = result.computation_ok ? 1 : 0;
    data.max_hops_required = result.max_hops_required;
  }

  MPI_Bcast(&data, sizeof(SyncData), MPI_BYTE, 0, MPI_COMM_WORLD);

  if (rank != 0) {
    result.total_tests_passed = data.total_tests_passed;
    result.topology_verified = data.topology_verified == 1;
    result.communication_ok = data.communication_ok == 1;
    result.computation_ok = data.computation_ok == 1;
    result.max_hops_required = data.max_hops_required;
  }
}
}  // namespace

MoskaevVTestMPI::MoskaevVTestMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = HypercubeTestResult();
}

bool MoskaevVTestMPI::ValidationImpl() {
  return GetInput().test_size > 0;
}

bool MoskaevVTestMPI::PreProcessingImpl() {
  return true;
}

bool MoskaevVTestMPI::RunImpl() {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  HypercubeTestResult result;

  bool topology_ok = false;
  int dimensions = 0;

  if (size > 0 && (size & (size - 1)) == 0) {
    topology_ok = true;

    while ((1 << dimensions) < size) {
      dimensions++;
    }

    for (int dim = 0; dim < dimensions; dim++) {
      int neighbor = rank ^ (1 << dim);
      if (neighbor < size) {
        int send_val = (rank * 1000) + dim;
        int recv_val;

        MPI_Sendrecv(&send_val, 1, MPI_INT, neighbor, 100, &recv_val, 1, MPI_INT, neighbor, 100, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);

        int expected = (neighbor * 1000) + dim;
        if (recv_val != expected) {
          topology_ok = false;
        }
      }
    }
  }

  int local_topology_ok = topology_ok ? 1 : 0;
  int global_topology_ok;
  MPI_Reduce(&local_topology_ok, &global_topology_ok, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);

  if (rank == 0) {
    result.topology_verified = (global_topology_ok == 1);
    if (result.topology_verified) {
      result.total_tests_passed++;
      std::cout << "Topology: " << dimensions << "D hypercube with " << size << " processes - PASSED" << "\n";
    } else {
      std::cout << "Topology: FAILED (not a valid hypercube)" << "\n";
    }
  }

  MPI_Bcast(&global_topology_ok, 1, MPI_INT, 0, MPI_COMM_WORLD);
  result.topology_verified = (global_topology_ok == 1);

  if (rank != 0 && result.topology_verified) {
    result.total_tests_passed++;
  }

  if (!result.topology_verified) {
    SyncResultToAllProcesses(result, rank);
    GetOutput() = result;
    return false;
  }

  if (GetInput().test_communication) {
    bool comm_ok = true;
    int max_hops = 0;

    if (rank == 0 || rank == size - 1) {
      int test_value = 12345;
      int hops = CalculateHops(0, size - 1);

      if (rank == 0) {
        MPI_Send(&test_value, 1, MPI_INT, size - 1, 200, MPI_COMM_WORLD);
        comm_ok = true;
      } else if (rank == size - 1) {
        int recv_value = 0;
        MPI_Recv(&recv_value, 1, MPI_INT, 0, 200, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        comm_ok = (recv_value == 12345);
      }

      max_hops = hops;
    }

    int local_comm_ok = comm_ok ? 1 : 0;
    int global_comm_ok;
    MPI_Reduce(&local_comm_ok, &global_comm_ok, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);

    int global_max_hops;
    MPI_Reduce(&max_hops, &global_max_hops, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
      result.communication_ok = (global_comm_ok == 1);
      result.max_hops_required = global_max_hops;

      if (result.communication_ok) {
        result.total_tests_passed++;
        std::cout << "Communication: PASSED (max hops: " << global_max_hops << ")" << "\n";
      } else {
        std::cout << "Communication: FAILED" << "\n";
      }
    }

    MPI_Bcast(&global_comm_ok, 1, MPI_INT, 0, MPI_COMM_WORLD);
    result.communication_ok = (global_comm_ok == 1);
    MPI_Bcast(&global_max_hops, 1, MPI_INT, 0, MPI_COMM_WORLD);
    result.max_hops_required = global_max_hops;

    if (rank != 0 && result.communication_ok) {
      result.total_tests_passed++;
    }
  }

  if (GetInput().test_computation) {
    std::mt19937 gen(GetInput().seed + rank);
    std::uniform_int_distribution<int> dist(1, 100);

    int local_sum = 0;
    for (int i = 0; i < GetInput().test_size; i++) {
      local_sum += dist(gen);
    }

    int global_sum = HypercubeSum(local_sum, rank, size);

    if (rank == 0) {
      std::mt19937 check_gen(GetInput().seed);
      int expected_sum = 0;

      for (int proc_rank = 0; proc_rank < size; proc_rank++) {
        std::mt19937 proc_gen(GetInput().seed + proc_rank);
        for (int i = 0; i < GetInput().test_size; i++) {
          expected_sum += dist(proc_gen);
        }
      }

      result.computation_ok = std::abs(global_sum - expected_sum) <= 10;

      if (result.computation_ok) {
        result.total_tests_passed++;
        std::cout << "Computation: PASSED (sum: " << global_sum << ", expected: " << expected_sum << ")" << "\n";
      } else {
        std::cout << "Computation: FAILED (sum: " << global_sum << ", expected: " << expected_sum << ")" << "\n";
      }
    }

    int comp_ok = (rank == 0 && result.computation_ok) ? 1 : 0;
    MPI_Bcast(&comp_ok, 1, MPI_INT, 0, MPI_COMM_WORLD);
    result.computation_ok = (comp_ok == 1);

    if (rank != 0 && result.computation_ok) {
      result.total_tests_passed++;
    }
  }

  int final_total_passed = result.total_tests_passed;
  MPI_Bcast(&final_total_passed, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (rank != 0) {
    result.total_tests_passed = final_total_passed;
  }

  GetOutput() = result;

  if (rank == 0) {
    std::cout << "Total tests passed: " << result.total_tests_passed << "/3" << "\n";
  }

  return result.total_tests_passed > 0;
}

bool MoskaevVTestMPI::PostProcessingImpl() {
  return GetOutput().total_tests_passed > 0;
}

}  // namespace moskaev_v_hypercub
