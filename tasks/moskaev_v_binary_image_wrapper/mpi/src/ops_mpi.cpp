#include "moskaev_v_binary_image_wrapper/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstddef>
#include <queue>
#include <utility>
#include <vector>

#include "moskaev_v_binary_image_wrapper/common/include/common.hpp"

namespace moskaev_v_binary_image_wrapper {

namespace {
const std::array<std::array<int, 2>, 8> kBfsDirs = {
    {{-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1}}};

std::vector<Point> BfsFindComponent(const std::vector<int> &image, int width, int start_row, int end_row, int start_x,
                                    int start_y, std::vector<bool> &visited);

std::vector<std::vector<Point>> FindLocalComponents(const std::vector<int> &image, int width, int height, int start_row,
                                                    int end_row);

void ProcessLocalComponents(const std::vector<std::vector<Point>> &local_components,
                            std::vector<std::vector<std::pair<int, int>>> &result);

void ReceiveAndProcessRemoteComponents(int size, std::vector<std::vector<std::pair<int, int>>> &result);

void SendComponentsToRoot(const std::vector<std::vector<Point>> &local_components);

class UnionFind {
 public:
  explicit UnionFind(int n) : parent_(n), rank_(n, 0) {
    for (int i = 0; i < n; ++i) {
      parent_[i] = i;
    }
  }

  int Find(int x) {
    while (parent_[x] != x) {
      parent_[x] = parent_[parent_[x]];
      x = parent_[x];
    }
    return x;
  }

  void Unite(int x, int y) {
    int rx = Find(x);
    int ry = Find(y);
    if (rx != ry) {
      if (rank_[rx] < rank_[ry]) {
        parent_[rx] = ry;
      } else if (rank_[rx] > rank_[ry]) {
        parent_[ry] = rx;
      } else {
        parent_[ry] = rx;
        rank_[rx]++;
      }
    }
  }

  bool Connected(int x, int y) {
    return Find(x) == Find(y);
  }

 private:
  std::vector<int> parent_;
  std::vector<int> rank_;
};

int Cross(const Point &origin, const Point &a, const Point &b) {
  return ((a.x - origin.x) * (b.y - origin.y)) - ((a.y - origin.y) * (b.x - origin.x));
}

Point FindPivot(const std::vector<Point> &points) {
  Point pivot = points[0];
  for (size_t i = 1; i < points.size(); ++i) {
    if (points[i].y < pivot.y || (points[i].y == pivot.y && points[i].x < pivot.x)) {
      pivot = points[i];
    }
  }
  return pivot;
}

bool PolarCompare(const Point &pivot, const Point &a, const Point &b) {
  int orientation = Cross(pivot, a, b);
  if (orientation == 0) {
    int dx1 = a.x - pivot.x;
    int dy1 = a.y - pivot.y;
    int dx2 = b.x - pivot.x;
    int dy2 = b.y - pivot.y;
    int dist1 = (dx1 * dx1) + (dy1 * dy1);
    int dist2 = (dx2 * dx2) + (dy2 * dy2);
    return dist1 < dist2;
  }
  return orientation > 0;
}

std::vector<Point> GrahamScan(std::vector<Point> points) {
  if (points.size() < 3) {
    return points;
  }

  Point pivot = FindPivot(points);

  std::erase_if(points, [&pivot](const Point &p) { return p.x == pivot.x && p.y == pivot.y; });

  if (points.empty()) {
    return {pivot};
  }

  std::ranges::sort(points, [&pivot](const Point &a, const Point &b) { return PolarCompare(pivot, a, b); });

  std::vector<Point> hull;
  hull.push_back(pivot);

  if (!points.empty()) {
    hull.push_back(points[0]);
  }

  if (points.size() > 1) {
    hull.push_back(points[1]);

    for (size_t i = 2; i < points.size(); ++i) {
      while (hull.size() >= 2 && Cross(hull[hull.size() - 2], hull.back(), points[i]) <= 0) {
        hull.pop_back();
      }
      hull.push_back(points[i]);
    }
  }

  return hull;
}

std::vector<Point> BfsFindComponent(const std::vector<int> &image, int width, int start_row, int end_row, int start_x,
                                    int start_y, std::vector<bool> &visited) {
  std::vector<Point> component;
  std::queue<Point> q;

  q.emplace(start_x, start_y);
  int local_index = ((start_y - start_row) * width) + start_x;
  visited[static_cast<size_t>(local_index)] = true;

  while (!q.empty()) {
    Point p = q.front();
    q.pop();
    component.push_back(p);

    for (const auto &dir : kBfsDirs) {
      int nx = p.x + dir[0];
      int ny = p.y + dir[1];

      if (nx >= 0 && nx < width && ny >= start_row && ny < end_row) {
        int n_local_idx = ((ny - start_row) * width) + nx;
        size_t n_global_idx = (static_cast<size_t>(ny) * static_cast<size_t>(width)) + static_cast<size_t>(nx);

        if (!visited[static_cast<size_t>(n_local_idx)] && image[n_global_idx] == 1) {
          visited[static_cast<size_t>(n_local_idx)] = true;
          q.emplace(nx, ny);
        }
      }
    }
  }

  return component;
}

std::vector<std::vector<Point>> FindLocalComponents(const std::vector<int> &image, int width, int height, int start_row,
                                                    int end_row) {
  (void)height;
  std::vector<std::vector<Point>> local_components;

  if (end_row <= start_row) {
    return local_components;
  }

  size_t local_size = static_cast<size_t>(width) * static_cast<size_t>(end_row - start_row);
  std::vector<bool> visited(local_size, false);

  for (int row = start_row; row < end_row; ++row) {
    for (int col = 0; col < width; ++col) {
      size_t global_idx = (static_cast<size_t>(row) * static_cast<size_t>(width)) + static_cast<size_t>(col);
      int local_idx = ((row - start_row) * width) + col;

      if (image[global_idx] == 1 && !visited[static_cast<size_t>(local_idx)]) {
        auto comp = BfsFindComponent(image, width, start_row, end_row, col, row, visited);
        if (comp.size() >= 3) {
          local_components.push_back(std::move(comp));
        }
      }
    }
  }

  return local_components;
}

void ProcessLocalComponents(const std::vector<std::vector<Point>> &local_components,
                            std::vector<std::vector<std::pair<int, int>>> &result) {
  for (const auto &comp : local_components) {
    auto hull = GrahamScan(comp);
    if (!hull.empty()) {
      std::vector<std::pair<int, int>> hull_pairs;
      hull_pairs.reserve(hull.size());

      for (const auto &p : hull) {
        hull_pairs.emplace_back(p.x, p.y);
      }
      result.push_back(std::move(hull_pairs));
    }
  }
}

void ReceiveComponentFromSource(int src, std::vector<std::vector<std::pair<int, int>>> &result) {
  int num_components = 0;
  MPI_Recv(&num_components, 1, MPI_INT, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  for (int i = 0; i < num_components; ++i) {
    int comp_size = 0;
    MPI_Recv(&comp_size, 1, MPI_INT, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    if (comp_size > 0) {
      std::vector<int> buffer(static_cast<size_t>(comp_size) * 2, 0);
      MPI_Recv(buffer.data(), comp_size * 2, MPI_INT, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      std::vector<Point> comp;
      comp.reserve(static_cast<size_t>(comp_size));

      for (int j = 0; j < comp_size; ++j) {
        comp.emplace_back(buffer[static_cast<size_t>(j) * 2], buffer[(static_cast<size_t>(j) * 2) + 1]);
      }

      auto hull = GrahamScan(comp);
      if (!hull.empty()) {
        std::vector<std::pair<int, int>> hull_pairs;
        hull_pairs.reserve(hull.size());

        for (const auto &p : hull) {
          hull_pairs.emplace_back(p.x, p.y);
        }
        result.push_back(std::move(hull_pairs));
      }
    }
  }
}

void ReceiveAndProcessRemoteComponents(int size, std::vector<std::vector<std::pair<int, int>>> &result) {
  for (int src = 1; src < size; ++src) {
    ReceiveComponentFromSource(src, result);
  }
}

void SendComponentsToRoot(const std::vector<std::vector<Point>> &local_components) {
  int num_components = static_cast<int>(local_components.size());
  MPI_Send(&num_components, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

  for (const auto &comp : local_components) {
    int comp_size = static_cast<int>(comp.size());
    MPI_Send(&comp_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

    if (comp_size > 0) {
      std::vector<int> buffer(static_cast<size_t>(comp_size) * 2, 0);

      for (int i = 0; i < comp_size; ++i) {
        buffer[static_cast<size_t>(i) * 2] = comp[static_cast<size_t>(i)].x;
        buffer[(static_cast<size_t>(i) * 2) + 1] = comp[static_cast<size_t>(i)].y;
      }

      MPI_Send(buffer.data(), comp_size * 2, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
  }
}

std::vector<std::vector<std::pair<int, int>>> ParallelConvexHullsSimple(const std::vector<int> &image, int width,
                                                                        int height) {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int rows_per_process = (size > 0 && height > 0) ? height / size : 0;
  int extra_rows = (size > 0 && height > 0) ? height % size : 0;

  int start_row = 0;
  for (int i = 0; i < rank; ++i) {
    start_row += rows_per_process + (i < extra_rows ? 1 : 0);
  }
  int end_row = start_row + rows_per_process + (rank < extra_rows ? 1 : 0);

  auto local_components = FindLocalComponents(image, width, height, start_row, end_row);

  std::vector<std::vector<std::pair<int, int>>> result;

  if (rank == 0) {
    ProcessLocalComponents(local_components, result);

    ReceiveAndProcessRemoteComponents(size, result);
  } else {
    SendComponentsToRoot(local_components);
  }

  MPI_Barrier(MPI_COMM_WORLD);
  return result;
}

}  // namespace

MoskaevVTestTaskMPI::MoskaevVTestTaskMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = OutType();
}

bool MoskaevVTestTaskMPI::ValidationImpl() {
  const auto &input = GetInput();

  if (input.size() < 3) {
    return false;
  }

  int width = input[0];
  int height = input[1];

  if (width <= 0 || height <= 0) {
    return false;
  }

  size_t expected_size = static_cast<size_t>(width * height) + 2;
  return input.size() == expected_size;
}

bool MoskaevVTestTaskMPI::PreProcessingImpl() {
  return true;
}

bool MoskaevVTestTaskMPI::RunImpl() {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  auto start_time = std::chrono::high_resolution_clock::now();

  const auto &input = GetInput();
  int width = input[0];
  int height = input[1];

  std::vector<int> image_data(input.begin() + 2, input.end());
  auto hulls = ParallelConvexHullsSimple(image_data, width, height);

  auto end_time = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end_time - start_time;

  if (rank == 0) {
    GetOutput() = std::move(hulls);
  } else {
    GetOutput() = OutType();
  }

  return true;
}

bool MoskaevVTestTaskMPI::PostProcessingImpl() {
  return true;
}

}  // namespace moskaev_v_binary_image_wrapper
