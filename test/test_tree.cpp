#include "binary_tree.hpp"
#include "general_tree.hpp"
#include "recursive_traversal.hpp"
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <functional>

template <typename T> struct fmt::formatter<TreeNode<T>> : ostream_formatter {};

template <typename T>
struct fmt::formatter<GeneralTreeNode<T>> : ostream_formatter {};

int main() {
  auto binary_tree = create_test_binary_tree();
  std::function<void(TreeNode<int> *)> visit_binary = [](TreeNode<int> *n) {
    fmt::println("{}", *n);
  };
  binary_preorder(binary_tree.get(), visit_binary);
  fmt::println("===");
  auto general_tree = create_test_general_tree();
  std::function<void(GeneralTreeNode<int> *)> visit_general =
      [](GeneralTreeNode<int> *n) { fmt::println("{}", *n); };

  general_preorder(general_tree.get(), visit_general);
}