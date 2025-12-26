#include "krymova_k_scatter/seq/include/ops_seq.hpp"

#include <mpi.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#include "krymova_k_scatter/common/include/common.hpp"

namespace krymova_k_scatter {

KrymovaKScatterSEQ::KrymovaKScatterSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
}

bool KrymovaKScatterSEQ::ValidationImpl() {
  const auto &args = GetInput();

  if (args.send_count <= 0 || args.send_count != args.recv_count) {
    return false;
  }

  if (args.send_type != args.recv_type) {
    return false;
  }

  if (args.src_buffer == nullptr && args.dst_buffer == nullptr) {
    return false;
  }

  auto check_type = [](MPI_Datatype t) { return (t == MPI_INT || t == MPI_FLOAT || t == MPI_DOUBLE); };

  return check_type(args.send_type);
}

bool KrymovaKScatterSEQ::PreProcessingImpl() {
  return true;
}

bool KrymovaKScatterSEQ::RunImpl() {
  auto &args = GetInput();

  size_t elem_size = 0;
  if (args.send_type == MPI_INT) {
    elem_size = sizeof(int);
  } else if (args.send_type == MPI_FLOAT) {
    elem_size = sizeof(float);
  } else {
    elem_size = sizeof(double);
  }

  auto total_bytes = static_cast<size_t>(args.send_count) * elem_size;

  const auto *start_ptr = static_cast<const uint8_t *>(args.src_buffer);
  const auto *end_ptr = start_ptr + total_bytes;

  std::vector<uint8_t> source_data(start_ptr, end_ptr);

  std::vector<uint8_t> output_data;
  output_data = source_data;

  if (args.dst_buffer != nullptr) {
    auto *dst_ptr = static_cast<uint8_t *>(args.dst_buffer);
    std::ranges::copy(output_data, dst_ptr);
  }

  GetOutput() = std::move(output_data);
  return true;
}

bool KrymovaKScatterSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace krymova_k_scatter
