#pragma once
#include <vector>
#include <functional>
#include <stack>

template <typename T, typename Comparator = std::less<T>>
void bubble_sort(std::vector<T> &arr) {
  auto comparator = Comparator();
  for (int i = 0; i < arr.size() - 1; i++) {
    bool swapped = false;
    for (int j = arr.size() - 1; j > i; j--) {
      if (comparator(arr[j], arr[j - 1])) {
        std::swap(arr[j], arr[j - 1]);
        swapped = true;
      }
    }
    // fmt::println("arr = {}", arr);
    if (!swapped) {
      return;
    }
  }
}

template <typename T, typename Comparator = std::less<T>>
void bidirectional_bubble_sort(std::vector<T> &arr) {
  auto comparator = Comparator();
  int l = 0;
  int r = arr.size() - 1;
  bool swapped = true;
  while (l < r && swapped) {
    swapped = false;
    for (int i = l; i < r; i++) {
      if (comparator(arr[i + 1], arr[i])) {
        std::swap(arr[i], arr[i + 1]);
        swapped = true;
      }
    }
    r--;
    for (int i = r; i > l; i--) {
      if (comparator(arr[i], arr[i - 1])) {
        std::swap(arr[i - 1], arr[i]);
        swapped = true;
      }
    }
    l++;
  }
}

template <typename T, typename Comparator = std::less<T>>
void quick_sort(std::vector<T> &arr) {
  auto comparator = Comparator();

  std::function<int(int, int)> partition = [&](int l, int r) {
    T pivot = arr[l];
    while (l < r) {
      // find first value strictly smaller than pivot
      while (l < r && !comparator(arr[r], pivot)) {
        r--;
      }
      std::swap(arr[l], arr[r]);
      // find first value larger or equal  than pivot
      while (l < r && comparator(arr[l], pivot)) {
        l++;
      }
      std::swap(arr[r], arr[l]);
    }
    return l;
  };

  std::function<void(int, int)> sort = [&](int l, int r) {
    if (l >= r) {
      return;
    }
    int pivot = partition(l, r);
    sort(l, pivot - 1);
    sort(pivot + 1, r);
  };

  sort(0, arr.size() - 1);
}

template <typename T, typename Comparator = std::less<T>>
void quick_sort_nonrecursive(std::vector<T> &arr) {
  auto comparator = Comparator();

  std::function<int(int, int)> partition = [&](int l, int r) {
    T pivot = arr[l];
    while (l < r) {
      // find first value strictly smaller than pivot
      while (l < r && !comparator(arr[r], pivot)) {
        r--;
      }
      std::swap(arr[l], arr[r]);
      // find first value larger or equal  than pivot
      while (l < r && comparator(arr[l], pivot)) {
        l++;
      }
      std::swap(arr[r], arr[l]);
    }
    return l;
  };

  std::stack<std::pair<int, int>> s;
  s.emplace(0, arr.size() - 1);
  while (!s.empty()) {
    auto [l, r] = s.top();
    s.pop();
    if (l >= r) {
      continue;
    }
    int pivot = partition(l, r);
    s.emplace(l, pivot - 1);
    s.emplace(pivot + 1, r);
  }
}