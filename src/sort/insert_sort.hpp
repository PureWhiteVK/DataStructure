#pragma once
#include <fmt/core.h>
#include <numeric>
#include <vector>

template <typename T, typename Comparator = std::less<T>>
void insert_sort(std::vector<T> &arr) {
  T sentinel{};
  auto comparator = Comparator();
  for (int i = 1; i < arr.size(); i++) {
    if (comparator(arr[i], arr[i - 1])) {
      sentinel = arr[i];
      int j = i - 1;
      while (j >= 0 && comparator(sentinel, arr[j])) {
        arr[j + 1] = arr[j];
        j--;
      }
      arr[j + 1] = sentinel;
    }
  }
}

template <typename T, typename Comparator = std::less<T>>
void insert_sort_with_binary_search(std::vector<T> &arr) {
  auto comparator = Comparator();
  for (int i = 1; i < arr.size(); i++) {
    if (comparator(arr[i], arr[i - 1])) {
      int l = 0;
      int r = i - 1;
      while (l <= r) {
        int m = (l + r) / 2;
        // fmt::println("l: {}, r: {}, m: {}", l, r, m);
        // 找到小于等于 m 的最后一个位置
        if (!comparator(arr[i], arr[m])) {
          // 等号必须在这里进行处理，否则就会出现问题
          l = m + 1;
        } else {
          r = m - 1;
        }
      }
      // fmt::println("l: {}, r: {}", l, r);
      T temp = arr[i];
      for (int j = i - 1; j >= l; --j) {
        arr[j + 1] = arr[j];
      }
      arr[l] = temp;
      // fmt::println("curr arr: {}", arr);
    }
  }
}

// using min_d > 1 to generate nearly sorted sequence
template <typename T, typename Comparator = std::less<T>, int min_d = 1>
void shell_sort(std::vector<T> &arr) {
  // core idea is the partition here!
  auto comparator = Comparator();
  for (int d = arr.size() / 2; d >= min_d; d /= 2) {
    // i,i+d,i+2d,i+3d,...
    for (int i = d; i < arr.size(); i++) {
      if (comparator(arr[i], arr[i - d])) {
        T sentinel = arr[i];
        int j = i - d;
        while (j >= 0 && comparator(sentinel, arr[j])) {
          arr[j + d] = arr[j];
          j -= d;
        }
        arr[j + d] = sentinel;
      }
    }
    // fmt::println("arr = {}", arr);
  }
}