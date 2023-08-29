#pragma once
#include <vector>
#include <functional>
#include <stack>

template <typename T, typename Comparator = std::less_equal<T>>
void merge_sort(std::vector<T> &arr) {
  auto comparator = Comparator();
  // 存储的是当前 heap 的大小
  std::vector<T> temp(arr.size());

  std::function<void(int, int)> merge_sort = [&](int l, int r) {
    if (l == r) {
      return;
    }
    int m = (l + r) / 2;
    merge_sort(l, m);
    merge_sort(m + 1, r);
    // merge two list
    int t0 = l;
    int t1 = m + 1;
    int i = l;
    while (t0 <= m && t1 <= r) {
      if (comparator(arr[t0], arr[t1])) {
        temp[i++] = arr[t0++];
      } else {
        temp[i++] = arr[t1++];
      }
    }
    while (t0 <= m) {
      temp[i++] = arr[t0++];
    }
    while (t1 <= r) {
      temp[i++] = arr[t1++];
    }
    for (int i = l; i <= r; i++) {
      arr[i] = temp[i];
    }
  };
  merge_sort(0, arr.size() - 1);
}

template <typename T, typename Comparator = std::less_equal<T>>
void merge_sort_nonrecursive(std::vector<T> &arr) {
  auto comparator = Comparator();
  // 存储的是当前 heap 的大小
  std::vector<T> temp(arr.size());

  std::stack<std::tuple<int, int, bool>> s;
  s.emplace(0, arr.size() - 1, false);
  while (!s.empty()) {
    auto [l, r, visited] = s.top();
    s.pop();
    if (l == r) {
      continue;
    }
    int m = (l + r) / 2;
    if (!visited) {
      // here controls the recursive order
      s.emplace(l, r, true);
      s.emplace(m + 1, r, false);
      s.emplace(l, m, false);
      continue;
    }
    // merge two list
    int t0 = l;
    int t1 = m + 1;
    int i = l;
    while (t0 <= m && t1 <= r) {
      if (comparator(arr[t0], arr[t1])) {
        temp[i++] = arr[t0++];
      } else {
        temp[i++] = arr[t1++];
      }
    }
    while (t0 <= m) {
      temp[i++] = arr[t0++];
    }
    while (t1 <= r) {
      temp[i++] = arr[t1++];
    }
    for (int i = l; i <= r; i++) {
      arr[i] = temp[i];
    }
  }
}