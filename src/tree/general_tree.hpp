#pragma once
#include <iostream>
#include <memory>
#include <queue>
#include <vector>

template <typename T> struct GeneralTreeNode {
  using value_type = T;
  using ptr_type = GeneralTreeNode<T> *;

  value_type val{};
  std::vector<ptr_type> children{};

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
        for(auto child : root->children) {
          queue.emplace(child);
        }
        delete curr;
      }
    }
  };

  using TreeHandle = std::unique_ptr<GeneralTreeNode, Deleter>;
};

template <typename T>
std::ostream &operator<<(std::ostream &os, const GeneralTreeNode<T> &node) {
  return os << node.val;
}

inline GeneralTreeNode<int>::TreeHandle create_test_general_tree() {
  using node = GeneralTreeNode<int>;
  using node_ptr = node *;
  node_ptr v0 = new node{5};
  node_ptr v1 = new node{3};
  node_ptr v2 = new node{1};
  node_ptr v3 = new node{2};
  node_ptr v4 = new node{4};
  node_ptr v5 = new node{8};
  node_ptr v6 = new node{7};
  v0->children = { v1,v2,v3 };
  v2->children = { v4,v5 };
  v3->children = { v6 };
  return GeneralTreeNode<int>::TreeHandle(v0);
}