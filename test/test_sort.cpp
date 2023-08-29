#include "bubble_sort.hpp"
#include "insert_sort.hpp"
#include "merge_sort.hpp"
#include "radix_sort.hpp"
#include "select_sort.hpp"

#include "integer.hpp"
#include "random_vector.hpp"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <functional>
#include <limits>
#include <mutex>
#include <numeric>
#include <queue>
#include <thread>
#include <unordered_map>

#include <fmt/core.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <fmt/std.h>

#define FuncPair(x)                                                            \
  { #x, x }

#define FuncWithName(x) #x, x

#define LOG(format, ...)                                                       \
  do {                                                                         \
    std::lock_guard _{log_mutex};                                              \
    log_queue.emplace([=]() { fmt::println(format, __VA_ARGS__); });           \
  } while (false)

namespace ch = std::chrono;

template <typename T> using SortFunc = std::function<void(std::vector<T> &)>;

template <typename T>
SortFunc<T> std_sort = [](std::vector<T> &v) { std::sort(v.begin(), v.end()); };

std::queue<std::function<void()>> log_queue;
std::mutex log_mutex;

template <typename T>
void test_sort_algorithm(int round, const std::string &name, SortFunc<T> func,
                         std::vector<T> &v, bool nearly_sort_case = false) {
  ch::steady_clock::duration total{};
  for (int i = 0; i < round; i++) {
    random_vector(v);
    if (nearly_sort_case) {
      // using shell sort (with d > 1) to generate nearly sort sequence
      shell_sort<T, std::less<>, 2>(v);
    }
    auto start = ch::steady_clock::now();
    func(v);
    auto end = ch::steady_clock::now();
    total += (end - start);
  }
  LOG("test {} on {} data with {} rounds, avg: {:.3f}ms", name, v.size(), round,
      static_cast<double>(ch::duration_cast<ch::microseconds>(total).count()) /
          1000.0 / static_cast<double>(round));
}

// using Int = Integer<false, false, false>;
using Int = int;

using StableInt = Integer;

template <> struct fmt::formatter<StableInt> : ostream_formatter {};

void stable_check(const std::string &name, SortFunc<StableInt> func) {
  std::vector<StableInt> v;
  v.resize(10000);
  random_vector(v);
  std::vector<StableInt> new_v1(v.begin(), v.end());
  std::vector<StableInt> new_v2(v.begin(), v.end());
  merge_sort<StableInt>(new_v1);
  func(new_v2);
  if (new_v1 != new_v2) {
    LOG("stable check failed, {} is not a stable sort algorithm!", name);

  } else {
    LOG("stable check passed: {}", name);
  }
}

template <typename T = int>
void valid_check(const std::string &name, SortFunc<T> func) {
  std::vector<T> v;
  v.resize(1000);
  random_vector(v);
  std::vector<T> new_v1(v.begin(), v.end());
  std::vector<T> new_v2(v.begin(), v.end());
  std_sort<T>(new_v1);
  func(new_v2);
  if (new_v1 != new_v2) {
    LOG("valid check failed, {} is not a valid sort algorithm!", name);
  } else {
    LOG("valid check passed: {}", name);
  }
}

int main() {
  constexpr int test_size = 1000;
  constexpr int round = 10;
  constexpr bool nearly_sort_case = true;

  stable_check(FuncWithName(insert_sort<StableInt>));
  stable_check(FuncWithName(insert_sort_with_binary_search<StableInt>));
  stable_check(FuncWithName(shell_sort<StableInt>));
  stable_check(FuncWithName(bubble_sort<StableInt>));
  stable_check(FuncWithName(quick_sort<StableInt>));
  stable_check(FuncWithName(select_sort<StableInt>));
  stable_check(FuncWithName(heap_sort<StableInt>));
  stable_check(FuncWithName(merge_sort<StableInt>));
  stable_check(FuncWithName(radix_sort<StableInt>));
  stable_check(FuncWithName(std_sort<StableInt>));

  std::unordered_map<std::string, SortFunc<Int>> test_funcs{
      // FuncPair(insert_sort<Integer>),
      // FuncPair(insert_sort_with_binary_search<Integer>),
      FuncPair(shell_sort<Int>), FuncPair(radix_sort<Int>),
      // in quick sort there may be stack overflow problem!
      // FuncPair(quick_sort<Int>),
      FuncPair(quick_sort_nonrecursive<Int>), FuncPair(heap_sort<Int>),
      FuncPair(heap_sort_with_function_call<Int>), FuncPair(merge_sort<Int>),
      FuncPair(merge_sort_nonrecursive<Int>), FuncPair(std_sort<Int>)};

  std::queue<std::function<void(std::vector<Int> &)>> task_queue;
  std::vector<std::thread> thread_pool;
  std::mutex m_queue;
  bool work_posted = false;
  bool done = false;

  for (int i = 0; i < test_funcs.size(); i++) {
    thread_pool.emplace_back(std::thread([&, size = test_size]() {
      thread_local std::vector<Int> v;
      // constexpr int size = std::numeric_limits<int>::max() >> 4;
      v.resize(size);
      while (true) {
        std::function<void(std::vector<Int> &)> work;
        {
          std::lock_guard l(m_queue);
          if (task_queue.empty() && done) {
            return;
          }
          if (!task_queue.empty()) {
            work = task_queue.front();
            task_queue.pop();
          }
        }
        work(v);
        // execute work load
      }
    }));
  }

  for (auto &pair : test_funcs) {
    // fmt::println("dispatch work!");
    {
      std::lock_guard lock(m_queue);
      task_queue.emplace(
          [&pair, round = round,
           nearly_sort_case = nearly_sort_case](std::vector<Int> &v) {
            valid_check(pair.first, pair.second);
            test_sort_algorithm(round, pair.first, pair.second, v,
                                nearly_sort_case);
          });
    }
  }

  {
    std::lock_guard lock(m_queue);
    done = true;
  }

  for (auto &&th : thread_pool) {
    th.join();
  }

  while (!log_queue.empty()) {
    log_queue.front()();
    log_queue.pop();
  }

  /**
    test radix_sort<Int> on 134217727 data with 10 rounds, avg: 2981.562ms
    test std_sort<Int> on 134217727 data with 10 rounds, avg: 14362.184ms
    test quick_sort_nonrecursive<Int> on 134217727 data with 10 rounds, avg:
    17215.065ms test merge_sort_nonrecursive<Int> on 134217727 data with 10
    rounds, avg: 20744.493ms test merge_sort<Int> on 134217727 data with 10
    rounds, avg: 21802.250ms test heap_sort_functor<Int>() on 134217727 data
    with 10 rounds, avg: 36147.461ms test heap_sort<Int> on 134217727 data with
    10 rounds, avg: 43911.316ms
  */

  return 0;
}