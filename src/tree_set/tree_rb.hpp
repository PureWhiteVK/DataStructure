#pragma once

#include "config.hpp"
#include <cassert>
#include <iostream>
#include <queue>
#include <sstream>

#define COLOR(node) ((node) ? (node)->color : BLACK)

#define DETACH(node)                                                           \
  do {                                                                         \
    if ((node)) {                                                              \
      if ((node)->is_left_child()) {                                           \
        (node)->parent->left = nullptr;                                        \
        (node)->parent = nullptr;                                              \
      } else if ((node)->is_right_child()) {                                   \
        (node)->parent->right = nullptr;                                       \
        (node)->parent = nullptr;                                              \
      }                                                                        \
    }                                                                          \
  } while (false)

template <typename Key> class rb_set {
public:
  using key_type = Key;

  enum color_t : bool { RED = 0, BLACK = 1 };

  struct tree_node {
    Key key{};
    color_t color{RED};
    tree_node *left{};
    tree_node *right{};
    tree_node *parent{};

    tree_node *sibling() {
      return parent ? (parent->left == this ? parent->right : parent->left)
                    : nullptr;
    }
    // 对于根节点而言，默认其是左孩子，避免 corner case
    bool is_left_child() const { return parent && parent->left == this; }

    bool is_right_child() const { return parent && parent->right == this; }

    bool is_root() const { return !parent; }

    bool is_leaf() const { return !left && !right; }
  };

private:
  static inline std::string color_name[2]{"RED", "BLACK"};

public:
  rb_set() = default;

  ~rb_set() { destroy_tree(root); }

  void insert(const Key &key) { root = insert_tree(root, key); }

  void remove(const Key &key) { root = remove_tree(root, key); }

  bool search(const Key &key) {
    return static_cast<bool>(search_tree(root, key));
  }

  bool check() const {
    return check(root);
  }

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
  static void destroy_tree(tree_node *root) {
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

  static std::string to_string(tree_node *node) {
    std::stringstream s;
    if (node) {
      s << "tree_node{key=" << node->key
        << ",color=" << color_name[node ? node->color : BLACK] << "}";
    } else {
      s << "tree_node{null}";
    }
    return s.str();
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

  static tree_node *range_search_tree(tree_node *root, const Key &key) {
    if (!root) {
      return root;
    }
    while (true) {
      if (root->key > key && root->left) {
        root = root->left;
      } else if (root->key < key && root->right) {
        root = root->right;
      } else {
        break;
      }
    }
    return root;
  }

  static tree_node *left_rotate(tree_node *node) {
    assert(node->right);
    tree_node *right = node->right;
    if (node->is_left_child()) {
      L(node->parent, right);
    } else if (node->is_right_child()) {
      R(node->parent, right);
    } else {
      // root case
      right->parent = nullptr;
    }
    R(node, right->left);
    L(right, node);
    std::swap(node->color, right->color);
    return right;
  }

  static tree_node *right_rotate(tree_node *node) {
    assert(node->left);
    tree_node *left = node->left;
    if (node->is_left_child()) {
      L(node->parent, left);
    } else if (node->is_right_child()) {
      R(node->parent, left);
    } else {
      // root case
      left->parent = nullptr;
    }
    L(node, left->right);
    R(left, node);
    std::swap(node->color, left->color);
    return left;
  }

  static tree_node *insert_tree(tree_node *root, const Key &key) {
    tree_node *parent = range_search_tree(root, key);
    if (!parent) {
      return new tree_node{key, BLACK};
    }
    if (parent->key == key) {
      return root;
    }
    tree_node *node = new tree_node{key, RED};
    LOG("insert %s to %s\n", to_string(node).c_str(),
        to_string(parent).c_str());
    if (parent->key > key) {
      L(parent, node);
    } else {
      R(parent, node);
    }
    if (parent->color == BLACK) {
      return root;
    }
    LOG("resolve red-red conflict\n");
    // 如果叔节点是黑色，那么只需要旋转节点即可，旋转方式和avl树类似
    while (COLOR(node) == RED && COLOR(parent) == RED) {
      tree_node *grand_parent = parent->parent;
      tree_node *uncle = parent->sibling();
      LOG("parent: %s, uncle: %s, grand parent: %s\n",
          to_string(parent).c_str(), to_string(uncle).c_str(),
          to_string(grand_parent).c_str());
      if (COLOR(uncle) == BLACK) {
        LOG("rotate case\n");
        // 需要在变更之前就判断是否是根节点
        if (parent->is_left_child()) {
          if (node->is_right_child()) {
            // LR case
            LOG("rotate LR\n");
            grand_parent->left = left_rotate(grand_parent->left);
          }
          // LL case
          LOG("rotate LL\n");
          grand_parent = right_rotate(grand_parent);
        } else if (parent->is_right_child()) {
          if (node->is_left_child()) {
            // RL case
            LOG("rotate RL\n");
            grand_parent->right = right_rotate(grand_parent->right);
          }
          // RR case
          LOG("rotate RR\n");
          grand_parent = left_rotate(grand_parent);
        } else {
          assert("unexpected case!");
        }
        if (grand_parent->is_root()) {
          root = grand_parent;
        }
      } else {
        LOG("re-color case\n");
        // 如果是这种情况，只需要将父节点和叔节点都变成黑色，然后将爷节点变成红色即可
        uncle->color = parent->color = BLACK;
        grand_parent->color = RED;
      }
      node = grand_parent;
      parent = node->parent;
      if (node->is_root() || parent->is_root()) {
        break;
      }
      LOG("move up\n");
    }
    root->color = BLACK;
    return root;
  }

  static tree_node *remove_tree(tree_node *root, const Key &key) {
    LOG("remove %s\n", std::to_string(key).c_str());
    tree_node *node = search_tree(root, key);
    if (!node) {
      return root;
    }

    if (node->left && node->right) {
      LOG("RED/BLACK internal node with 2 children case\n");
      tree_node *prev = node->left;
      while (prev->right) {
        prev = prev->right;
      }
      std::swap(prev->key, node->key);
      // 转去删除 prev 节点
      node = prev;
    }

    if (node->right && !node->left) {
      LOG("BLACK internal with right child node case\n");
      assert(node->right->color == RED);
      // 直接把红色节点值上移即可
      std::swap(node->key, node->right->key);
      node->right = nullptr;
      delete node->right;
      return root;
    }

    if (node->left && !node->right) {
      LOG("BLACK internal with left child node case\n");
      assert(node->left->color == RED);
      std::swap(node->key, node->left->key);
      node->left = nullptr;
      delete node->left;
      return root;
    }

    if (COLOR(node) == RED || node->is_root()) {
      LOG("RED leaf node case / ROOT node case\n");
      if (node->is_root()) {
        root = nullptr;
      }
      DETACH(node);
      delete node;
      return root;
    }

    LOG("BLACK leaf node case\n");
    // 此时待删除节点是双黑节点，然后我们需要将其转换成单黑节点
    tree_node *delete_node = node;
    tree_node *parent{};
    tree_node *sibling{};
    bool running = true;
    while (running) {
      parent = node->parent;
      sibling = node->sibling();
      // 单节点情况？
      if (!sibling) {
        node = parent;
        continue;
      }
      switch (COLOR(sibling)) {
      case RED: {
        // 进行旋转
        if (node->is_left_child()) {
          parent = left_rotate(parent);
        } else if (node->is_right_child()) {
          parent = right_rotate(parent);
        } else {
          assert(false && "unexpected case!");
        }
        // update parent
        if (parent->is_root()) {
          root = parent;
        }
        break;
      }
      case BLACK: {
        uint8_t node_status =
            (COLOR(sibling->left) << 1) | (COLOR(sibling->right));
        switch (node_status) {
        // red-black case
        case 0b01: {
          if (node->is_left_child()) {
            sibling = right_rotate(sibling);
            std::swap(sibling->color, sibling->right->color);
          } else if (node->is_right_child()) {
            sibling = left_rotate(sibling);
            std::swap(sibling->color, sibling->left->color);
          } else {
            assert("unexpected case!");
          }
          // 然后继续进行旋转
        }
        // red-red case & black-red case
        case 0b10:
        case 0b00: {
          // 直接进行左旋
          if (node->is_left_child()) {
            parent = left_rotate(parent);
            sibling->right->color = BLACK;
          } else if (node->is_right_child()) {
            parent = right_rotate(parent);
            sibling->left->color = BLACK;
          } else {
            assert("unexpected case!");
          }
          if (parent->is_root()) {
            root = parent;
          }
          running = false;
          break;
        }
        // black-black case
        case 0b11: {
          sibling->color = RED;
          if (COLOR(parent) == RED) {
            parent->color = BLACK;
            running = false;
          }
          node = parent;
          break;
        }
        }
      }
      }
    }
    DETACH(delete_node);
    delete delete_node;
    return root;
  }

  static bool check(tree_node *root) {
    if (!root) {
      return true;
    }
    // 首先检查根节点是否是黑色
    if (COLOR(root) != BLACK) {
      return false;
    }
    // 开始遍历，需要同步存储黑高
    std::queue<std::pair<tree_node *, int>> q{};
    q.push({root, 1});
    std::optional<int> estimate_black_height{};
    while (!q.empty()) {
      auto [curr, black_height] = q.top();
      q.pop();
      if (!curr) {
        black_height++;
        if (!estimate_black_height) {
          estimate_black_height = black_height;
        } else if (estimate_black_height != black_height) {
          // 检查黑高是否相同
          return false;
        }
      }
      // 检查相邻节点是否是红色
      switch (COLOR(curr)) {
      case RED: {
        if (COLOR(curr->parent) == RED || COLOR(curr->left) == RED ||
            COLOR(curr->right) == RED) {
          return false;
        }
        q.push({curr->left, black_height});
        q.push({curr->right, black_height});
      }
      case BLACK: {
        q.push({curr->left, black_height + 1});
        q.push({curr->right, black_height + 1});
      }
      }
    }
    return true;
  }

private:
  tree_node *root{};
};