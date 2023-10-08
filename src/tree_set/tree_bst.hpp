#pragma once

#include <queue>
#include <sstream>
#include <string>

template <typename Key> class bst_set {
public:
  using key_type = Key;
  bst_set() = default;

  ~bst_set() { destory_tree(root); }

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
  };

private:
  static void destory_tree(tree_node *root) {
    std::queue<tree_node *> queue{};
    queue.push(root);
    while (!queue.empty()) {
      tree_node *curr = queue.front();
      queue.pop();
      if (curr) {
        queue.push(curr->left);
        queue.push(curr->right);
        delete curr;
      }
    }
  }

  static tree_node *insert_tree(tree_node *root, const Key &key) {
    if (root == nullptr) {
      return new tree_node{key};
    }
    if (root->key > key) {
      root->left = insert_tree(root->left, key);
    } else if (root->key < key) {
      root->right = insert_tree(root->right, key);
    }
    return root;
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
        root = nullptr;
      } else if ((root->left && !root->right) || (root->right && !root->left)) {
        tree_node *child = root->left ? root->left : root->right;
        delete root;
        root = child;
      }
    }
    return root;
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

  static std::string to_string(tree_node *node) {
    return "tree_node(" +
           (node ? std::to_string(node->key) : std::string("null")) + ")";
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