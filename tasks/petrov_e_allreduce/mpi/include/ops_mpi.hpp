#pragma once

#include <mpi.h>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "petrov_e_allreduce/common/include/common.hpp"
#include "task/include/task.hpp"

namespace petrov_e_allreduce {

template <typename T>
void ApplyOperation(T *dest, T *src, int count, MPI_Op op) {
  int flag = 0;
  if (op == MPI_SUM) {
    flag = 1;
  } else if (op == MPI_PROD) {
    flag = 2;
  } else if (op == MPI_MAX) {
    flag = 3;
  } else if (op == MPI_MIN) {
    flag = 4;
  }
  switch (flag) {
    case 1:
      for (int i = 0; i < count; i++) {
        dest[i] += src[i];
      }
      break;
    case 2:
      for (int i = 0; i < count; i++) {
        dest[i] *= src[i];
      }
      break;
    case 3:
      for (int i = 0; i < count; i++) {
        dest[i] = std::max(dest[i], src[i]);
      }
      break;
    case 4:
      for (int i = 0; i < count; i++) {
        dest[i] = std::min(dest[i], src[i]);
      }
      break;
    default:
      break;
  }
}

inline void Operation(void *dest, void *src, int count, MPI_Datatype datatype, MPI_Op op) {
  int flag = 0;
  if (datatype == MPI_UNSIGNED_CHAR) {
    flag = 1;
  } else if (datatype == MPI_CHAR) {
    flag = 2;
  } else if (datatype == MPI_SHORT) {
    flag = 3;
  } else if (datatype == MPI_INT) {
    flag = 4;
  } else if (datatype == MPI_LONG) {
    flag = 5;
  } else if (datatype == MPI_FLOAT) {
    flag = 6;
  } else if (datatype == MPI_DOUBLE) {
    flag = 7;
  } else if (datatype == MPI_LONG_DOUBLE) {
    flag = 8;
  }
  switch (flag) {
    case 1:
      ApplyOperation(static_cast<unsigned char *>(dest), static_cast<unsigned char *>(src), count, op);
      break;
    case 2:
      ApplyOperation(static_cast<char *>(dest), static_cast<char *>(src), count, op);
      break;
    case 3:
      ApplyOperation(static_cast<int16_t *>(dest), static_cast<int16_t *>(src), count, op);
      break;
    case 4:
      ApplyOperation(static_cast<int *>(dest), static_cast<int *>(src), count, op);
      break;
    case 5:
      ApplyOperation(static_cast<int64_t *>(dest), static_cast<int64_t *>(src), count, op);
      break;
    case 6:
      ApplyOperation(static_cast<float *>(dest), static_cast<float *>(src), count, op);
      break;
    case 7:
      ApplyOperation(static_cast<double *>(dest), static_cast<double *>(src), count, op);
      break;
    case 8:
      ApplyOperation(static_cast<long double *>(dest), static_cast<long double *>(src), count, op);
      break;
    default:
      break;
  }
}

inline void GetSizeOf2(MPI_Datatype type, int &size) {
  int res = 4;
  if (type == MPI_CHAR) {
    res = sizeof(char);
  } else if (type == MPI_UNSIGNED_CHAR) {
    res = sizeof(unsigned char);
  } else if (type == MPI_SHORT) {
    res = sizeof(int16_t);
  } else if (type == MPI_UNSIGNED_SHORT) {
    res = sizeof(uint16_t);
  } else if (type == MPI_INT) {
    res = sizeof(int);
  } else if (type == MPI_UNSIGNED) {
    res = sizeof(unsigned);
  } else if (type == MPI_LONG) {
    res = sizeof(int64_t);
  } else if (type == MPI_UNSIGNED_LONG) {
    res = sizeof(uint64_t);
  } else if (type == MPI_FLOAT) {
    res = sizeof(float);
  } else if (type == MPI_DOUBLE) {
    res = sizeof(double);
  } else if (type == MPI_LONG_DOUBLE) {
    res = sizeof(long double);
  }
  size = res;
}

template <typename MatrixElemType>
MPI_Datatype GetMPIDatatype() {
  MPI_Datatype res = MPI_DATATYPE_NULL;
  if (std::is_same_v<MatrixElemType, char>) {
    res = MPI_CHAR;
  } else if (std::is_same_v<MatrixElemType, unsigned char>) {
    res = MPI_UNSIGNED_CHAR;
  } else if (std::is_same_v<MatrixElemType, int16_t>) {
    res = MPI_SHORT;
  } else if (std::is_same_v<MatrixElemType, uint16_t>) {
    res = MPI_UNSIGNED_SHORT;
  } else if (std::is_same_v<MatrixElemType, int>) {
    res = MPI_INT;
  } else if (std::is_same_v<MatrixElemType, unsigned>) {
    res = MPI_UNSIGNED;
  } else if (std::is_same_v<MatrixElemType, int64_t>) {
    res = MPI_LONG;
  } else if (std::is_same_v<MatrixElemType, uint64_t>) {
    res = MPI_UNSIGNED_LONG;
  } else if (std::is_same_v<MatrixElemType, float>) {
    res = MPI_FLOAT;
  } else if (std::is_same_v<MatrixElemType, double>) {
    res = MPI_DOUBLE;
  } else if (std::is_same_v<MatrixElemType, long double>) {
    res = MPI_LONG_DOUBLE;
  }
  return res;
}

inline int MpiMyAllreduce(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, MPI_Op op,
                          MPI_Comm comm) {
  int proc_num = 0;
  int proc_rank = 0;
  MPI_Comm_size(comm, &proc_num);
  MPI_Comm_rank(comm, &proc_rank);
  int type_size = 0;
  GetSizeOf2(datatype, type_size);
  int data_size = count * type_size;
  MPI_Status status;
  const char *src = static_cast<const char *>(sendbuf);
  char *dst = static_cast<char *>(recvbuf);
  for (int i = 0; i < data_size; i++) {
    dst[i] = src[i];
  }
  std::vector<char> tempbufvec(data_size);
  void *tempbuf = tempbufvec.data();

  int parent = (proc_rank - 1) / 2;
  int left = (2 * proc_rank) + 1;
  int right = (2 * proc_rank) + 2;

  if (left < proc_num) {
    MPI_Recv(tempbuf, count, datatype, left, 0, comm, &status);
    Operation(recvbuf, tempbuf, count, datatype, op);
  }

  if (right < proc_num) {
    MPI_Recv(tempbuf, count, datatype, right, 0, comm, &status);
    Operation(recvbuf, tempbuf, count, datatype, op);
  }

  if (proc_rank != 0) {
    MPI_Send(recvbuf, count, datatype, parent, 0, comm);
    MPI_Recv(recvbuf, count, datatype, parent, 1, comm, &status);
  }

  if (left < proc_num) {
    MPI_Send(recvbuf, count, datatype, left, 1, comm);
  }

  if (right < proc_num) {
    MPI_Send(recvbuf, count, datatype, right, 1, comm);
  }

  return MPI_SUCCESS;
}

class PetrovEMyAllreduceMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit PetrovEMyAllreduceMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace petrov_e_allreduce
