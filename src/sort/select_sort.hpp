#pragma once
#include <fmt/core.h>
#include <functional>
#include <sstream>
#include <vector>

template <typename T, typename Comparator = std::less<T>>
void select_sort(std::vector<T> &arr) {
  auto comparator = Comparator();
  for (int i = 0; i < arr.size() - 1; i++) {
    int min = i;
    for (int j = i + 1; j < arr.size(); j++) {
      if (comparator(arr[j], arr[min])) {
        min = j;
      }
    }
    if (min != i) {
      std::swap(arr[i], arr[min]);
    }
  }
}

template <typename T, typename Comparator = std::less<T>>
bool is_heap(const std::vector<T> &arr) {
  auto comparator = Comparator();

  std::function<bool(int)> is_heap = [&](int root) {
    if (root >= arr.size()) {
      return true;
    }
    int left = root * 2 + 1;
    int right = left + 1;
    bool res = true;
    if (left < arr.size()) {
      res = comparator(arr[left], arr[root]) && is_heap(left);
      if (right < arr.size()) {
        res &= comparator(arr[right], arr[root]) && is_heap(right);
      }
    }
    return res;
  };

  return is_heap(0);
}

template <typename T> void print_heap(const std::vector<T> &arr) {
  std::function<void(int, int)> print_heap = [&](int root, int depth) {
    std::stringstream s;
    for (int i = 0; i < depth; i++) {
      s << "─";
    }
    if (root >= arr.size()) {
      if (depth == 0) {
        fmt::println("·", s.str());
      } else {
        fmt::println("{}·", s.str());
      }
      return;
    }

    if (depth == 0) {
      fmt::println("{}", arr[root]);
    } else {
      fmt::println("{}{}", s.str(), arr[root]);
    }

    int left = root * 2 + 1;
    int right = left + 1;
    if (left < arr.size()) {
      print_heap(left, depth + 1);
      if (right < arr.size()) {
        print_heap(right, depth + 1);
      }
    }
  };
  fmt::println("is arr {} heap? {}", arr, is_heap(arr));
  print_heap(0, 0);
}

template <typename T, typename Comparator = std::greater<T>>
void heap_sort(std::vector<T> &arr) {
  auto comparator = Comparator();
  int n = arr.size();
  for (int i = (n - 1) / 2; i >= 0; i--) {
    // 遍历堆中所有的中间节点
    int idx = i;
    int child = idx * 2 + 1;
    while (child < n) {
      // 当前节点为 idx，左节点为 idx*2+1，右节点为 idx*2+2
      // 比较当前节点与左右节点之间的偏序关系，判断是否需要进行交换
      if (child < n - 1 && comparator(arr[child + 1], arr[child])) {
        child++;
      }
      // 当前子树已经是堆了
      if (comparator(arr[idx], arr[child])) {
        break;
      }
      std::swap(arr[child], arr[idx]);
      // goto next
      idx = child;
      child = idx * 2 + 1;
    }
  }
  while (n-- > 1) {
    std::swap(arr[0], arr[n]);
    int idx = 0;
    int child = idx * 2 + 1;
    while (child < n) {
      // 当前节点为 idx，左节点为 idx*2+1，右节点为 idx*2+2
      // 比较当前节点与左右节点之间的偏序关系，判断是否需要进行交换
      if (child < n - 1 && comparator(arr[child + 1], arr[child])) {
        child++;
      }
      // 当前子树已经是堆了
      if (comparator(arr[idx], arr[child])) {
        break;
      }
      std::swap(arr[child], arr[idx]);
      // goto next
      idx = child;
      child = idx * 2 + 1;
    }
  }
}

template <typename T, typename Comparator = std::greater<T>>
void heap_sort_with_function_call(std::vector<T> &arr) {
  auto comparator = Comparator();
  // 存储的是当前 heap 的大小
  int n = arr.size();

  std::function<int(int)> parent = [](int idx) { return (idx - 1) / 2; };

  std::function<int(int)> left_child = [](int idx) { return idx * 2 + 1; };

  std::function<int(int)> right_child = [](int idx) { return idx * 2 + 2; };

  // when we need to add element (a.k.a implement priority_queue, we need to
  // implement shift_up )

  std::function<void(int)> shift_down = [&](int idx) {
    // fmt::println("shift down element: {}", arr[idx]);
    // 比较 idx 及其子节点的大小，交换
    int child = left_child(idx);
    while (child < n) {
      // 当前节点为 idx，左节点为 idx*2+1，右节点为 idx*2+2
      // 比较当前节点与左右节点之间的偏序关系，判断是否需要进行交换
      if (child < n - 1 && comparator(arr[child + 1], arr[child])) {
        child++;
      }
      // 当前子树已经是堆了
      if (comparator(arr[idx], arr[child])) {
        break;
      }
      std::swap(arr[child], arr[idx]);
      // goto next
      idx = child;
      child = left_child(child);
    }
    // fmt::println("arr: {}", arr);
  };

  std::function<void(int)> shift_up = [&](int idx) {
    int parent_idx = parent(idx);
    while (parent_idx > 0) {
      if (comparator(arr[parent_idx], arr[idx])) {
        break;
      }
      std::swap(arr[parent_idx], arr[idx]);
      idx = parent_idx;
      parent_idx = parent(idx);
    }
  };

  // build heap
  for (int i = (n - 1) / 2; i >= 0; i--) {
    // 遍历堆中所有的中间节点
    shift_down(i);
  }

  // fmt::println("before insert 81");
  // print_heap(arr);
  // arr.emplace_back(81);
  // n += 1;
  // shift_up(n - 1);
  // fmt::println("after insert 81");
  // print_heap(arr);

  // start sort
  while (n-- > 1) {
    std::swap(arr[0], arr[n]);
    shift_down(0);
  }
}