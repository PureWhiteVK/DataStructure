#pragma once
#include <iostream>
#include <memory>
#include <queue>
#include <string>

#ifdef DEBUG
#define LOG(...) printf(__VA_ARGS__)
#else
#define LOG(...)                                                               \
  {}
#endif

namespace binary_tree {
struct tree_node {
  int val{};
  tree_node *left{};
  tree_node *right{};
};

template <typename node_type> struct deleter {
  void operator()(node_type *root) {
    std::queue<node_type *> queue;
    queue.push(root);
    while (!queue.empty()) {
      node_type *curr = queue.front();
      queue.pop();
      if (curr) {
        queue.push(curr->left);
        queue.push(curr->right);
        LOG("delete %d\n", curr->val);
        delete curr;
      }
    }
  }
};

using tree_handle = std::unique_ptr<tree_node, deleter<tree_node>>;

template <typename node_type>
std::string to_string(node_type *root, bool pretty = false);

template <typename node_type> int height(node_type *root) {
  return root ? std::max(height(root->left), height(root->right)) + 1 : 0;
}

// 插入
template <typename node_type> node_type *insert(node_type *root, int val);

// 删除
template <typename node_type> node_type *remove(node_type *root, int val);

// 检索
template <typename node_type> node_type *search(node_type *root, int val) {
  while (root) {
    if (root->val == val) {
      return root;
    }
    root = root->val > val ? root->left : root->right;
  }
  return root;
}

} // namespace binary_tree