#pragma once

#include "config.hpp"

#include <iostream>
#include <queue>
#include <sstream>
#include <string>

template <typename Key> class avl_set {
public:
  using key_type = Key;
  avl_set() = default;

  ~avl_set() { destory_tree(root); }

  bool search(const Key &key) {
    return static_cast<bool>(search_tree(root, key));
  }

  void insert(const Key &key) { root = insert_tree(root, key); }

  void remove(const Key &key) { root = remove_tree(root, key); }

  std::string to_string() const {
    std::stringstream s;
    s << "{ ";
    inorder_tree(root, [&](const Key &key) { s << key << " "; });
    s << "}";
    return s.str();
  }

  std::string tree_view() const {
    std::stringstream s;
    preorder_tree_pretty(root, 0, s);
    return s.str();
  }

private:
  struct tree_node {
    Key key{};
    tree_node *left{};
    tree_node *right{};
    int height{1};
  };

private:
  static std::string to_string(tree_node *node) {
    std::stringstream s;
    if (node) {
      s << "tree_node{key=" << node->key << ",h=" << node->height
        << ",f=" << (height(node->left) - height(node->right)) << "}";
    } else {
      s << "tree_node{null}";
    }
    return s.str();
  }

  static void update_height(tree_node *node) {
    node->height = 1 + std::max(height(node->left), height(node->right));
  }

  static void destory_tree(tree_node *root) {
    std::queue<tree_node *> queue{};
    queue.push(root);
    while (!queue.empty()) {
      tree_node *curr = queue.front();
      queue.pop();
      if (curr) {
        queue.push(curr->left);
        queue.push(curr->right);
        // std::cout << "delete " << to_string(curr) << std::endl;
        delete curr;
      }
    }
  }

  static tree_node *left_rotate(tree_node *node) {
    assert(node->right);
    tree_node *right = node->right;
    node->right = right->left;
    right->left = node;
    // 需要重新调整高度
    update_height(node);
    update_height(right);
    return right;
  }

  static tree_node *right_rotate(tree_node *node) {
    assert(node->left);
    tree_node *left = node->left;
    node->left = left->right;
    left->right = node;
    update_height(node);
    update_height(left);
    return left;
  }

  static int height(tree_node *node) { return node ? node->height : 0; }

  static tree_node *rebalance(tree_node *root) {
    if (!root) {
      return root;
    }
    // 首先更新树的高度信息
    update_height(root);

    // 判断当前节点的情况调整树的结构
    int balance_factor = height(root->left) - height(root->right);

    if (balance_factor == -2) {
      // check RL or RR
      // 检查右子树的高度情况
      tree_node *right = root->right;
      assert(right);
      if (height(right->left) > height(right->right)) {
        root->right = right_rotate(root->right);
      }
      root = left_rotate(root);
      // 需要重新调整高度
    } else if (balance_factor == 2) {
      tree_node *left = root->left;
      assert(left);
      if (height(left->right) > height(left->left)) {
        root->left = left_rotate(root->left);
      }
      root = right_rotate(root);
    }
    return root;
  }

  static tree_node *insert_tree(tree_node *root, const Key &key) {
    using std::cout, std::endl;
    if (root == nullptr) {
      return new tree_node{key};
    }
    if (root->key > key) {
      root->left = insert_tree(root->left, key);
    } else if (root->key < key) {
      root->right = insert_tree(root->right, key);
    }
    return rebalance(root);
  }

  static tree_node *remove_tree(tree_node *root, const Key &key) {
    if (root == nullptr) {
      return root;
    }
    if (root->key > key) {
      root->left = remove_tree(root->left, key);
    } else if (root->key < key) {
      root->right = remove_tree(root->right, key);
    } else {
      if (root->left && root->right) {
        tree_node *prev = root->left;
        while (prev->right) {
          prev = prev->right;
        }
        std::swap(prev->key, root->key);
        root->left = remove_tree(root->left, key);
      } else if (!root->left && !root->right) {
        delete root;
      } else if ((root->left && !root->right) || (root->right && !root->left)) {
        tree_node *child = root->left ? root->left : root->right;
        delete root;
        root = child;
      }
    }

    return rebalance(root);
  }

  static tree_node *search_tree(tree_node *root, const Key &key) {
    while (root) {
      if (root->key > key) {
        root = root->left;
      } else if (root->key < key) {
        root = root->right;
      } else {
        break;
      }
    }
    return root;
  }

  static void inorder_tree(tree_node *root,
                           const std::function<void(const Key &)> &func) {
    if (root == nullptr) {
      return;
    }
    inorder_tree(root->left, func);
    func(root->key);
    inorder_tree(root->right, func);
  }

  static void preorder_tree_pretty(tree_node *root, int depth,
                                   std::stringstream &os) {
    for (int i = 0; i < depth; i++) {
      os << " ";
    }
    os << to_string(root) << "\n";
    if (root && (root->left || root->right)) {
      preorder_tree_pretty(root->left, depth + 1, os);
      preorder_tree_pretty(root->right, depth + 1, os);
    }
  }

private:
  tree_node *root{};
};