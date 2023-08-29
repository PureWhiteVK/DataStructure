#pragma once
#include <array>
#include <vector>

// only support unsigned int
template <typename Integer> void radix_sort(std::vector<Integer> &arr) {
  thread_local std::array<std::vector<Integer>, 16> radices;
  for (int i = 0; i < 8; i++) {
    int prev = -1;
    int bits = i << 2;
    bool same_radix = true;
    for (Integer v : arr) {
      int radix = (v >> bits) & 0xf;
      radices[radix].emplace_back(v);
      if (prev != -1 && radix != prev) {
        same_radix = false;
      }
      prev = radix;
    }
    // fmt::println("split phase: {}", radices);
    int k = 0;
    for (auto &r : radices) {
      for (Integer v : r) {
        arr[k++] = v;
      }
      r.clear();
    }
    // fmt::println("collect phase: {}", arr);
    if (same_radix) {
      break;
    }
  }
}