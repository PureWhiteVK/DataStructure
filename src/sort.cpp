#include <algorithm>
#include <cstdlib>
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <functional>
#include <iostream>
#include <optional>
#include <ostream>
#include <random>
#include <sstream>
#include <stack>
#include <type_traits>
#include <unordered_map>

// customize int (for stable sort demostration)
class Integer {
public:
  Integer(){
      // fmt::println("called default constructer of Integer");
  };

  Integer(int v) {
    // fmt::println("called int constructer of Integer");
    increase_refcount(v);
    m_value = v;
    m_id = s_count[v];
  }
  Integer(const Integer &v) {
    // fmt::println("called copy constructer of Integer");
    increase_refcount(v);
    m_value = v;
    m_id = v.m_id;
  }

  Integer(Integer &&v) {
    // fmt::println("called move constructer of Integer");
    m_value = v.m_value;
    m_id = v.m_id;
    v.m_value = 0;
    v.m_id = INVALID_ID;
  }

  ~Integer() {
    // fmt::println("called destructer of Integer");
    if (m_id != INVALID_ID) {
      decrease_refcount(m_value);
    }
  }

  Integer &operator=(const Integer &v) {
    // fmt::println("called copy assignment of Integer, curr: {}, target: {}",
    //              *this, v);
    increase_refcount(v);
    if (m_id != INVALID_ID) {
      decrease_refcount(m_value);
    }
    m_value = v.m_value;
    m_id = v.m_id;
    return *this;
  }

  Integer &operator=(Integer &&v) {
    // fmt::println("called move assignment of Integer");
    if (this == &v) {
      // self assignment case, otherwise there will be problems
      return *this;
    }
    if (m_id != INVALID_ID) {
      decrease_refcount(m_value);
    }
    m_value = std::move(v.m_value);
    m_id = std::move(v.m_id);
    v.m_value = 0;
    v.m_id = INVALID_ID;
    return *this;
  }

  Integer &operator=(int v) {
    // fmt::println("called int assignment of Integer");
    increase_refcount(v);
    if (m_id != INVALID_ID) {
      decrease_refcount(m_value);
    }
    m_value = v;
    m_id = s_count[m_value];
    return *this;
  }

  operator int() const { return m_value; }

  bool operator<(const Integer &i) const { return m_value < i.m_value; }

  bool operator<=(const Integer &i) const { return m_value <= i.m_value; }

  bool operator>(const Integer &i) const { return m_value > i.m_value; }

  bool operator>=(const Integer &i) const { return m_value >= i.m_value; }

  bool operator==(const Integer &i) const {
    return m_value == i.m_value && m_id == i.m_id;
  }

  friend std::ostream &operator<<(std::ostream &os, const Integer &v) {
    if (v.m_id != 1) {
      os << fmt::format("{}[{}]", v.m_value, v.m_id - 1);
    } else if (v.m_id == INVALID_ID) {
      os << fmt::format("null", v.m_value);
    } else {
      os << v.m_value;
    }
    return os;
  }

  static void print_reference_count() { fmt::println("s_count: {}", s_count); }

private:
  static void decrease_refcount(int v) {
    s_count[v]--;
    if (s_count[v] == 0) {
      s_count.erase(v);
    }
  }

  static void increase_refcount(int v) {
    if (s_count.find(v) == s_count.end()) {
      s_count[v] = 0;
    }
    s_count[v]++;
  }

private:
  static constexpr int INVALID_ID = 0;
  int m_value{};
  int m_id{INVALID_ID};
  inline static std::unordered_map<int, int> s_count{};
};

template <> struct fmt::formatter<Integer> : ostream_formatter {};

template <typename T, typename Comparator = std::less<T>>
void insert_sort(std::vector<T> &arr) {
  T sentinel{};
  auto comparator = Comparator();
  for (int i = 1; i < arr.size(); i++) {
    if (comparator(arr[i], arr[i - 1])) {
      sentinel = arr[i];
      int j = i - 1;
      while (comparator(sentinel, arr[j])) {
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

template <typename T, typename Comparator = std::less<T>>
void shell_sort(std::vector<T> &arr) {
  // core idea is the partition here!
  auto comparator = Comparator();
  for (int d = arr.size() / 2; d >= 1; d -= 2) {
    fmt::println("d = {}", d);
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
    fmt::println("arr = {}", arr);
  }
}

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
    fmt::println("arr = {}", arr);
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

template <typename T, typename Comparator = std::less<T>>
void heap_sort(std::vector<T> &arr) {
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

  fmt::println("before insert 81");
  print_heap(arr);
  arr.emplace_back(81);
  n += 1;
  shift_up(n - 1);
  fmt::println("after insert 81");
  print_heap(arr);

  // start sort
  while (n-- > 1) {
    std::swap(arr[0], arr[n]);
    shift_down(0);
  }
}

template <typename T, typename Comparator = std::less<T>>
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

template <typename T, typename Comparator = std::less<T>>
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

// template <typename T, typename Comparator = std::less<T>>
void radix_sort(std::vector<int> &arr) {
  std::array<std::vector<int>, 16> radices;
  for (int i = 0; i < 8; i++) {
    int prev = -1;
    int bits = i << 2;
    bool same_radix = true;
    for (int v : arr) {
      int radix = (v >> bits) & 0xf;
      radices[radix].emplace_back(v);
      if (prev != -1 && radix != prev) {
        same_radix = false;
      }
      prev = radix;
    }
    fmt::println("split phase: {}", radices);
    int k = 0;
    for (auto &r : radices) {
      for (int v : r) {
        arr[k++] = v;
      }
      r.clear();
    }
    fmt::println("collect phase: {}", arr);
    if (same_radix) {
      break;
    }
  }
}

template <typename T, typename Comparator = std::less<T>>
T nth_element(std::vector<T> &arr, int n) {
  // 找到第 n 大的元素
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

  std::function<void(int, int, int)> nth_element = [&](int l, int r, int n) {
    if (l >= r) {
      return;
    }
    // 经过 partition 后 pivot 元素一定位于有序位置上面
    int pivot = partition(l, r);
    // 此时 pivot 位于有序位置上，三种情况，如果 pivot 等于 n，就可以直接返回了
    // 如果 pivot 大于 n，说明 nth_element 还在左侧序列中
    // 否则 nth_element 就在右侧序列中
    if (pivot < n) {
      nth_element(l, pivot - 1, n);
    } else if (pivot > n) {
      nth_element(pivot + 1, r, n);
    }
  };

  nth_element(0, arr.size() - 1, n);
  return arr[n];
}

template <typename T, typename Comparator = std::less<T>>
T nth_element_nonrecursive(std::vector<T> &arr, int n) {
  // 找到第 n 大的元素
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

  int l = 0;
  int r = arr.size() - 1;
  while (l < r) {
    int pivot = partition(l, r);
    if (pivot < n) {
      r = pivot - 1;
    } else {
      l = pivot + 1;
    }
  }
  return arr[n];
}

void random_vector(std::vector<Integer> &arr) {
  std::random_device rd;
  std::mt19937 random(rd());
  int n = arr.capacity();
  std::uniform_int_distribution<int> uniform_dist(0, n);
  arr.clear();
  while (n-- > 0) {
    arr.emplace_back(uniform_dist(random));
  }
}

// 将数组划分成两部分
// 将数组划分成三部分

int main() {
  // std::vector<Integer> v{49, 38, 65, 97, 76, 13, 27, 49};
  // std::vector<Integer> arr{49, 38, 65, 97, 76, 13, 27, 49, 55, 04};
  // fmt::println("before sort:\n{}", arr);
  // quick_sort<Integer, std::less<>>(arr);
  // fmt::println("after sort:\n{}", arr);
  // using SortFunc = std::function<void(std::vector<Integer> &)>;
  // stable 语法还取决于比较器的选择
  // std::vector<SortFunc> funcs{
  //     radix_sort<Integer, std::less<>>,
  //     radix_sort<Integer, std::less_equal<>>,
  //     radix_sort<Integer, std::greater<>>,
  //     radix_sort<Integer, std::greater_equal<>>,
  // };
  // std::vector<Integer> v;
  // v.reserve(30);
  // for (auto &&sort_func : funcs) {
  //   random_vector(v);
  //   fmt::println("before sort:\n{}", v);
  //   sort_func(v);
  //   fmt::println("after sort:\n{}", v);
  // }
  // v.clear();
  // Integer::print_reference_count();
  // random_vector(v, arr_size);
  // Integer res = nth_element_nonrecursive(v, arr_size / 2);
  // fmt::println("{} th element: {}", arr_size / 2, res);
  // fmt::println("arr: {}", v);
  std::vector<int> v{278, 109, 63, 930, 589, 184, 505, 269, 8, 83};
  radix_sort(v);
  return 0;
}