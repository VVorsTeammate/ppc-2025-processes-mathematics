#include "moskaev_v_binary_image_wrapper/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <iostream>
#include <queue>
#include <set>
#include <unordered_map>
#include <vector>

namespace moskaev_v_binary_image_wrapper {

namespace {
const int PARALLEL_DIRS[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
const int BFS_DIRS[8][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
class UnionFind {
 private:
  std::vector<int> parent_;
  std::vector<int> rank_;

 public:
  UnionFind(int n) : parent_(n), rank_(n, 0) {
    for (int i = 0; i < n; ++i) {
      parent_[i] = i;
    }
  }

  int find(int x) {
    if (parent_[x] != x) {
      parent_[x] = find(parent_[x]);
    }
    return parent_[x];
  }

  void unite(int x, int y) {
    int rx = find(x);
    int ry = find(y);
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

  bool connected(int x, int y) {
    return find(x) == find(y);
  }
};

int cross(const Point &O, const Point &A, const Point &B) {
  return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
}

Point findPivot(const std::vector<Point> &points) {
  Point pivot = points[0];
  for (size_t i = 1; i < points.size(); ++i) {
    if (points[i].y < pivot.y || (points[i].y == pivot.y && points[i].x < pivot.x)) {
      pivot = points[i];
    }
  }
  return pivot;
}

bool polarCompare(const Point &pivot, const Point &a, const Point &b) {
  int orientation = cross(pivot, a, b);
  if (orientation == 0) {
    int dx1 = a.x - pivot.x, dy1 = a.y - pivot.y;
    int dx2 = b.x - pivot.x, dy2 = b.y - pivot.y;
    return (dx1 * dx1 + dy1 * dy1) < (dx2 * dx2 + dy2 * dy2);
  }
  return orientation > 0;
}

std::vector<Point> grahamScan(std::vector<Point> points) {
  if (points.size() < 3) {
    return points;
  }

  Point pivot = findPivot(points);

  points.erase(std::remove_if(points.begin(), points.end(),
                              [&pivot](const Point &p) { return p.x == pivot.x && p.y == pivot.y; }),
               points.end());

  if (points.empty()) {
    return {pivot};
  }

  std::sort(points.begin(), points.end(),
            [&pivot](const Point &a, const Point &b) { return polarCompare(pivot, a, b); });

  std::vector<Point> hull;
  hull.push_back(pivot);
  hull.push_back(points[0]);

  if (points.size() > 1) {
    hull.push_back(points[1]);

    for (size_t i = 2; i < points.size(); ++i) {
      while (hull.size() >= 2 && cross(hull[hull.size() - 2], hull.back(), points[i]) <= 0) {
        hull.pop_back();
      }
      hull.push_back(points[i]);
    }
  }

  return hull;
}

std::vector<Point> findComponentBFS(const std::vector<int> &img, int w, int h, int startX, int startY,
                                    std::vector<bool> &visited) {
  std::vector<Point> comp;
  std::queue<Point> q;

  if (img[startY * w + startX] != 1) {
    return comp;
  }

  q.push(Point(startX, startY));
  visited[startY * w + startX] = true;

  while (!q.empty()) {
    Point p = q.front();
    q.pop();
    comp.push_back(p);

    for (int i = 0; i < 8; ++i) {
      int nx = p.x + BFS_DIRS[i][0];
      int ny = p.y + BFS_DIRS[i][1];

      if (nx >= 0 && nx < w && ny >= 0 && ny < h) {
        int idx = ny * w + nx;
        if (!visited[idx] && img[idx] == 1) {
          visited[idx] = true;
          q.push(Point(nx, ny));
        }
      }
    }
  }

  return comp;
}

std::vector<int> getBoundaryPixels(const std::vector<int> &local_image, int local_width, int local_height, int rank,
                                   int size) {
  std::vector<int> boundary_pixels;

  if (size == 1) {
    return boundary_pixels;
  }

  bool has_up = (rank > 0);
  bool has_down = (rank < size - 1);

  if (has_up && local_height > 0) {
    for (int x = 0; x < local_width; ++x) {
      if (local_image[x] == 1) {
        boundary_pixels.push_back(x);
        boundary_pixels.push_back(0);
        boundary_pixels.push_back(rank - 1);
      }
    }
  }

  if (has_down && local_height > 0) {
    int last_row_start = (local_height - 1) * local_width;
    for (int x = 0; x < local_width; ++x) {
      if (local_image[last_row_start + x] == 1) {
        boundary_pixels.push_back(x);
        boundary_pixels.push_back(local_height - 1);
        boundary_pixels.push_back(rank + 1);
      }
    }
  }

  return boundary_pixels;
}

std::vector<std::vector<std::pair<int, int>>> parallelConvexHullsSimple(const std::vector<int> &image, int width,
                                                                        int height) {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int rows_per_process = 0;
  int extra_rows = 0;
  int start_row = 0;
  int end_row = 0;

  if (size > 0 && height > 0) {
    rows_per_process = height / size;
    extra_rows = height % size;

    start_row = 0;
    for (int i = 0; i < rank; ++i) {
      start_row += rows_per_process + (i < extra_rows ? 1 : 0);
    }
    end_row = start_row + rows_per_process + (rank < extra_rows ? 1 : 0);
  }

  std::vector<std::vector<Point>> local_components;

  if (end_row > start_row) {
    std::vector<bool> visited(width * (end_row - start_row), false);

    for (int y = start_row; y < end_row; ++y) {
      for (int x = 0; x < width; ++x) {
        int global_idx = y * width + x;
        int local_idx = (y - start_row) * width + x;

        if (image[global_idx] == 1 && !visited[local_idx]) {
          std::queue<Point> q;
          std::vector<Point> comp;

          q.push(Point(x, y));
          visited[local_idx] = true;

          while (!q.empty()) {
            Point p = q.front();
            q.pop();
            comp.push_back(p);

            for (int d = 0; d < 8; ++d) {
              int nx = p.x + BFS_DIRS[d][0];
              int ny = p.y + BFS_DIRS[d][1];

              if (nx >= 0 && nx < width && ny >= start_row && ny < end_row) {
                int n_global_idx = ny * width + nx;
                int n_local_idx = (ny - start_row) * width + nx;

                if (image[n_global_idx] == 1 && !visited[n_local_idx]) {
                  visited[n_local_idx] = true;
                  q.push(Point(nx, ny));
                }
              }
            }
          }

          if (comp.size() >= 3) {
            local_components.push_back(std::move(comp));
          }
        }
      }
    }
  }

  std::vector<std::vector<std::pair<int, int>>> result;

  if (rank == 0) {
    for (const auto &comp : local_components) {
      auto hull = grahamScan(comp);
      if (!hull.empty()) {
        std::vector<std::pair<int, int>> hull_pairs;
        hull_pairs.reserve(hull.size());

        for (const auto &p : hull) {
          hull_pairs.emplace_back(p.x, p.y);
        }
        result.push_back(std::move(hull_pairs));
      }
    }

    for (int src = 1; src < size; ++src) {
      int num_components = 0;
      MPI_Recv(&num_components, 1, MPI_INT, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      for (int i = 0; i < num_components; ++i) {
        int comp_size = 0;
        MPI_Recv(&comp_size, 1, MPI_INT, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        if (comp_size > 0) {
          std::vector<int> buffer(comp_size * 2, 0);
          MPI_Recv(buffer.data(), comp_size * 2, MPI_INT, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

          std::vector<Point> comp;
          comp.reserve(comp_size);

          for (int j = 0; j < comp_size; ++j) {
            comp.emplace_back(buffer[j * 2], buffer[j * 2 + 1]);
          }

          auto hull = grahamScan(comp);
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
  } else {
    int num_components = static_cast<int>(local_components.size());
    MPI_Send(&num_components, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

    for (const auto &comp : local_components) {
      int comp_size = static_cast<int>(comp.size());
      MPI_Send(&comp_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);

      if (comp_size > 0) {
        std::vector<int> buffer(comp_size * 2, 0);
        for (int i = 0; i < comp_size; ++i) {
          buffer[i * 2] = comp[i].x;
          buffer[i * 2 + 1] = comp[i].y;
        }
        MPI_Send(buffer.data(), comp_size * 2, MPI_INT, 0, 0, MPI_COMM_WORLD);
      }
    }
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

  bool is_valid = (input.size() >= 3) && (input[0] > 0) && (input[1] > 0) &&
                  (input.size() == static_cast<size_t>(input[0] * input[1]) + 2);

  if (is_valid) {
    return true;
  }

  return true;
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

  std::vector<int> imageData(input.begin() + 2, input.end());
  auto hulls = parallelConvexHullsSimple(imageData, width, height);

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
