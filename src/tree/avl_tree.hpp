#pragma once

#include "bs_tree.hpp"

#include <functional>

namespace avl_tree {
struct tree_node {
  int val{};
  int height{1};
  tree_node *left{};
  tree_node *right{};

  void update_height();
};

using tree_handle = std::unique_ptr<tree_node, binary_tree::deleter<tree_node>>;

// 判断是否是一颗平衡二叉树
template <typename node_type> bool check_avl_tree(node_type *root) {
  std::function<int(node_type *)> avl_height =
      [&avl_height](node_type *root) -> int {
    if (!root) {
      return 0;
    }
    int left_height = avl_height(root->left);
    int right_height = avl_height(root->right);
    if (right_height == -1 || left_height == -1 ||
        (std::abs(left_height - right_height) >= 2)) {
      fprintf(stderr, "%s\n", binary_tree::to_string(root, true).c_str());
      exit(-1);
    }
    return std::abs(right_height - left_height) < 2 && right_height != -1 &&
                   left_height != -1
               ? std::max(left_height, right_height) + 1
               : -1;
  };
  bool res = avl_height(root) > -1;
  if (!res) {
    fprintf(stderr, "%s\n", binary_tree::to_string(root, true).c_str());
  }
  return res;
}
} // namespace avl_tree

namespace binary_tree {
// 插入
template <> avl_tree::tree_node *insert(avl_tree::tree_node *root, int val);

// 删除
template <> avl_tree::tree_node *remove(avl_tree::tree_node *root, int val);

template <> std::string to_string(avl_tree::tree_node *root, bool pretty);
} // namespace binary_tree