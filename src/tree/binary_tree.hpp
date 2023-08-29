#pragma once
#include <iostream>
#include <memory>
#include <queue>

template <typename T> struct TreeNode {
  using value_type = T;
  using ptr_type = TreeNode<T> *;

  value_type val{};
  ptr_type left{nullptr};
  ptr_type right{nullptr};

  struct Deleter {
    void operator()(ptr_type root) {
      std::queue<ptr_type> queue;
      queue.emplace(root);
      while (!queue.empty()) {
        auto curr = queue.front();
        queue.pop();
        if (!curr) {
          continue;
        }
        queue.emplace(curr->left);
        queue.emplace(curr->right);
        delete curr;
      }
    }
  };

  using TreeHandle = std::unique_ptr<TreeNode, Deleter>;
};

template <typename T>
std::ostream &operator<<(std::ostream &os, const TreeNode<T> &node) {
  return os << node.val;
}

inline TreeNode<int>::TreeHandle create_test_binary_tree() {
  using node = TreeNode<int>;
  using node_ptr = node *;
  node_ptr v0 = new node{5};
  node_ptr v1 = new node{3};
  node_ptr v2 = new node{1};
  node_ptr v3 = new node{2};
  node_ptr v4 = new node{4};
  node_ptr v5 = new node{8};
  node_ptr v6 = new node{7};
  v0->left = v1;
  v0->right = v2;
  v1->left = v3;
  v1->right = v4;
  v3->left = v5;
  v4->right = v6;
  return TreeNode<int>::TreeHandle(v0);
}