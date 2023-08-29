#include "integer.hpp"
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <functional>
#include <vector>

using MyInteger = Integer;

template <> struct fmt::formatter<MyInteger> : ostream_formatter {};

void invoke(const std::function<MyInteger(MyInteger &, MyInteger &)> &func) {
  fmt::println("=== {}", __LINE__);

  MyInteger a = 10;
  MyInteger b = 20;
  fmt::println("=== {}", __LINE__);

  MyInteger res = func(a, b);
  fmt::println("=== {}", __LINE__);

  fmt::println("func(10,12) = {}", res);
  fmt::println("=== {}", __LINE__);
}

int main() {
  // // default constructor
  // Integer a;
  // Integer::print_reference_count();
  // // int constructor
  // Integer b(10);
  // Integer::print_reference_count();
  // // copy constructor
  // Integer c = b;
  // Integer::print_reference_count();
  // // move constructor
  // Integer d = std::move(b);
  // Integer::print_reference_count();
  // // int assignment
  // c = 20;
  // Integer::print_reference_count();
  // // copy assignment
  // d = c;
  // Integer::print_reference_count();
  // // move assignment
  // a = std::move(c);
  // Integer::print_reference_count();

  // std::vector<Integer> v(5,23);
  // Integer::print_reference_count();
  // fmt::println("capacity: {} size: {}",v.capacity(),v.size());
  // v.reserve(10);
  // fmt::println("capacity: {} size: {}",v.capacity(),v.size());
  // v.resize(10);
  // fmt::println("capacity: {} size: {}",v.capacity(),v.size());
  // v.reserve(5);
  // fmt::println("capacity: {} size: {}",v.capacity(),v.size());
  // v.resize(5);
  // fmt::println("capacity: {} size: {}",v.capacity(),v.size());
  // v.reserve(5);
  // fmt::println("capacity: {} size: {}",v.capacity(),v.size());
  // v.shrink_to_fit();
  // fmt::println("capacity: {} size: {}",v.capacity(),v.size());
  // std::vector<Integer> v;
  // size_t prev_capacity = 0;
  // for (int i = 0; i < 1000; i++) {
  //   v.emplace_back(123);
  //   if (prev_capacity != v.capacity()) {
  //     fmt::println("capacity change from {} to {}, delta: {}", prev_capacity,
  //                  v.capacity(), v.capacity() - prev_capacity);
  //     prev_capacity = v.capacity();
  //   }
  //   // fmt::println("capacity: {} size: {}",v.capacity(),v.size());
  // }

  // 调用 clear 并不会回收 vector 所占用的空间
  // v.clear();
  // fmt::println("capacity: {} size: {}", v.capacity(), v.size());
  // 需要手动执行一下 shrink_to_fit 才行
  // v.shrink_to_fit();
  // fmt::println("capacity: {} size: {}", v.capacity(), v.size());
  // capacity 描述的 vector 实际占用的内存空间
  // size 描述的 vector 当前使用的内存空间
  //

  MyInteger c = 49;
  fmt::println("=== {}", __LINE__);
  std::function<MyInteger(MyInteger &, MyInteger &)> add_base =
      [d = c](MyInteger &a, MyInteger &b) {
        fmt::println("=== {}", __LINE__);

        MyInteger c = a * b + d;
        fmt::println("=== {}", __LINE__);

        return c;
      };
  fmt::println("=== {}", __LINE__);

  invoke(add_base);
  fmt::println("=== {}", __LINE__);

  // 函数闭包传参过程中也是会发生拷贝的，默认是 copy construct，最好替换成 &&
  // 或者 &
  return 0;
}