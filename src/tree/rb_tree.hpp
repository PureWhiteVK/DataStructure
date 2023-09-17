#pragma once
#include "bs_tree.hpp"

namespace rb_tree {

enum color_t { RED, BLACK };

struct tree_node {
  int val{};
  color_t color{RED};
  // 对于红黑树而言，还需要额外存储一个父节点指针（便于维护红黑树结构）
  tree_node *parent{};
  tree_node *left{};
  tree_node *right{};
};

using tree_handle = std::unique_ptr<tree_node, binary_tree::deleter<tree_node>>;

bool check_rb_tree(tree_node *root);

}; // namespace rb_tree

namespace binary_tree {
// 插入
template <> rb_tree::tree_node *insert(rb_tree::tree_node *root, int val);

// 删除
template <> rb_tree::tree_node *remove(rb_tree::tree_node *root, int val);

template <> std::string to_string(rb_tree::tree_node *root, bool pretty);
} // namespace binary_tree