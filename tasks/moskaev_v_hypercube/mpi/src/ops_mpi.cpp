#include "moskaev_v_hypercube/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <cmath>
#include <iostream>
#include <random>

namespace moskaev_v_hypercube {

namespace {

int calculateHops(int src, int dest) {
  int diff = src ^ dest;
  int hops = 0;

  while (diff) {
    hops++;
    diff &= (diff - 1);
  }

  return hops;
}

void getHypercubeNeighbors(int rank, int size, int neighbors[], int &count) {
  count = 0;

  int dims = 0;
  while ((1 << dims) < size) {
    dims++;
  }

  for (int d = 0; d < dims; d++) {
    int neighbor = rank ^ (1 << d);
    if (neighbor < size) {
      neighbors[count++] = neighbor;
    }
  }
}

int hypercubeSum(int local_value, int rank, int size) {
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
  int rank, size;
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
        int send_val = rank * 1000 + dim;
        int recv_val;

        MPI_Sendrecv(&send_val, 1, MPI_INT, neighbor, 100, &recv_val, 1, MPI_INT, neighbor, 100, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);

        int expected = neighbor * 1000 + dim;
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
      std::cout << "Topology: " << dimensions << "D hypercube with " << size << " processes - PASSED" << std::endl;
    } else {
      std::cout << "Topology: FAILED (not a valid hypercube)" << std::endl;
    }
  }

  MPI_Bcast(&global_topology_ok, 1, MPI_INT, 0, MPI_COMM_WORLD);
  result.topology_verified = (global_topology_ok == 1);

  if (!result.topology_verified) {
    GetOutput() = result;
    return false;
  }

  if (GetInput().test_communication) {
    bool comm_ok = true;
    int max_hops = 0;

    if (rank == 0 || rank == size - 1) {
      int test_value = 12345;
      int hops = calculateHops(0, size - 1);

      if (rank == 0) {
        MPI_Send(&test_value, 1, MPI_INT, size - 1, 200, MPI_COMM_WORLD);
        comm_ok = true;
      } else if (rank == size - 1) {
        int recv_value;
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
        std::cout << "Communication: PASSED (max hops: " << global_max_hops << ")" << std::endl;
      } else {
        std::cout << "Communication: FAILED" << std::endl;
      }
    }

    MPI_Bcast(&global_comm_ok, 1, MPI_INT, 0, MPI_COMM_WORLD);
    result.communication_ok = (global_comm_ok == 1);
  }

  if (GetInput().test_computation) {
    std::mt19937 gen(GetInput().seed + rank);
    std::uniform_int_distribution<int> dist(1, 100);

    int local_sum = 0;
    for (int i = 0; i < GetInput().test_size; i++) {
      local_sum += dist(gen);
    }

    int global_sum = hypercubeSum(local_sum, rank, size);

    if (rank == 0) {
      std::mt19937 check_gen(GetInput().seed);
      int expected_sum = 0;

      for (int r = 0; r < size; r++) {
        std::mt19937 proc_gen(GetInput().seed + r);
        for (int i = 0; i < GetInput().test_size; i++) {
          expected_sum += dist(proc_gen);
        }
      }

      result.computation_ok = std::abs(global_sum - expected_sum) <= 10;

      if (result.computation_ok) {
        result.total_tests_passed++;
        std::cout << "Computation: PASSED (sum: " << global_sum << ", expected: " << expected_sum << ")" << std::endl;
      } else {
        std::cout << "Computation: FAILED (sum: " << global_sum << ", expected: " << expected_sum << ")" << std::endl;
      }
    }

    int comp_ok = result.computation_ok ? 1 : 0;
    MPI_Bcast(&comp_ok, 1, MPI_INT, 0, MPI_COMM_WORLD);
    result.computation_ok = (comp_ok == 1);
  }

  GetOutput() = result;

  if (rank == 0) {
    std::cout << "Total tests passed: " << result.total_tests_passed << "/3" << std::endl;
  }

  return result.total_tests_passed > 0;
}

bool MoskaevVTestMPI::PostProcessingImpl() {
  return GetOutput().total_tests_passed > 0;
}

}  // namespace moskaev_v_hypercube
