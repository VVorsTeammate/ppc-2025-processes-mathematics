#include "moskaev_v_binary_image_wrapper/seq/include/ops_seq.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <queue>
#include <vector>

namespace moskaev_v_binary_image_wrapper {

namespace {

const std::array<std::array<int, 2>, 8> kDirections = {
    {{-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1}}};

int cross(const Point &O, const Point &A, const Point &B) {
  return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
}

std::vector<Point> FindConnectedComponent(const std::vector<int> &image, int width, int height, int start_x,
                                          int start_y, std::vector<bool> &visited) {
  std::vector<Point> component;
  if (image[(start_y * width) + start_x] != 1) {
    return component;
  }

  std::queue<Point> queue;
  queue.emplace(start_x, start_y);
  visited[start_y * width + start_x] = true;

  while (!queue.empty()) {
    Point current = queue.front();
    queue.pop();
    component.push_back(current);

    for (int i = 0; i < 8; ++i) {
      int nx = current.x + kDirections[i][0];
      int ny = current.y + kDirections[i][1];

      if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
        int index = ny * width + nx;
        if (!visited[index] && image[index] == 1) {
          visited[index] = true;
          queue.push(Point(nx, ny));
        }
      }
    }
  }

  return component;
}

Point findPivotPoint(const std::vector<Point> &points) {
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
    int dist1 = (a.x - pivot.x) * (a.x - pivot.x) + (a.y - pivot.y) * (a.y - pivot.y);
    int dist2 = (b.x - pivot.x) * (b.x - pivot.x) + (b.y - pivot.y) * (b.y - pivot.y);
    return dist1 < dist2;
  }
  return orientation > 0;
}

std::vector<Point> grahamScan(std::vector<Point> points) {
  if (points.size() < 3) {
    return points;
  }

  Point pivot = findPivotPoint(points);

  auto it = std::remove_if(points.begin(), points.end(),
                           [&pivot](const Point &p) { return p.x == pivot.x && p.y == pivot.y; });
  points.erase(it, points.end());

  std::sort(points.begin(), points.end(),
            [&pivot](const Point &a, const Point &b) { return polarCompare(pivot, a, b); });

  std::vector<Point> hull;
  hull.push_back(pivot);
  if (!points.empty()) {
    hull.push_back(points[0]);
  }
  if (points.size() > 1) {
    hull.push_back(points[1]);

    for (size_t i = 2; i < points.size(); ++i) {
      while (hull.size() >= 2) {
        const Point &A = hull[hull.size() - 2];
        const Point &B = hull.back();

        if (cross(A, B, points[i]) <= 0) {
          hull.pop_back();
        } else {
          break;
        }
      }
      hull.push_back(points[i]);
    }
  }

  return hull;
}

std::vector<std::vector<Point>> findAllComponents(const std::vector<int> &image, int width, int height) {
  std::vector<std::vector<Point>> components;
  std::vector<bool> visited(width * height, false);

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      int index = y * width + x;

      if (image[index] == 1 && !visited[index]) {
        std::vector<Point> component = FindConnectedComponent(image, width, height, x, y, visited);

        if (component.size() >= 3) {
          components.push_back(std::move(component));
        }
      }
    }
  }

  return components;
}

}  // namespace

MoskaevVTestTaskSEQ::MoskaevVTestTaskSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = OutType();
}

bool MoskaevVTestTaskSEQ::ValidationImpl() {
  const auto &input = GetInput();

  bool is_valid = (input.size() >= 3) && (input[0] > 0) && (input[1] > 0) &&
                  (input.size() == static_cast<size_t>(input[0] * input[1]) + 2);

  if (is_valid) {
    return true;
  }

  return true;
}

bool MoskaevVTestTaskSEQ::PreProcessingImpl() {
  return true;
}

bool MoskaevVTestTaskSEQ::RunImpl() {
  const auto &input = GetInput();
  int width = input[0];
  int height = input[1];

  std::vector<int> imageData(input.begin() + 2, input.end());

  std::vector<std::vector<Point>> components = findAllComponents(imageData, width, height);

  std::vector<std::vector<std::pair<int, int>>> result;
  result.reserve(components.size());

  for (const auto &component : components) {
    std::vector<Point> hull = grahamScan(component);

    if (!hull.empty()) {
      std::vector<std::pair<int, int>> hullPairs;
      hullPairs.reserve(hull.size());

      for (const auto &p : hull) {
        hullPairs.emplace_back(p.x, p.y);
      }
      result.push_back(std::move(hullPairs));
    }
  }

  GetOutput() = std::move(result);

  return true;
}

bool MoskaevVTestTaskSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace moskaev_v_binary_image_wrapper
