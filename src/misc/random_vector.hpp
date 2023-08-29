#pragma once
#include "integer.hpp"
#include <random>
#include <vector>

template <typename T>
void random_vector(std::vector<T> &arr, std::seed_seq seq = {133337}) {
  std::mt19937 random(seq);
  size_t n = arr.capacity();
  // limit range to (0,n/2) to make sure we can generate equal numbers (for stablity check)
  std::uniform_int_distribution<int> uniform_dist(0, n / 2);
  arr.clear();
  while (n-- > 0) {
    arr.emplace_back(uniform_dist(random));
  }
}