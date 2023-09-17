#include "avl_tree.hpp"

#include <cassert>
#include <functional>
#include <iostream>
#include <sstream>
#include <stack>

#define HEIGHT(node) (node ? node->height : 0)

#define BALANCE_FACTOR(node)                                                   \
  (node ? (HEIGHT(node->left) - HEIGHT(mode->right)) : 0)

namespace avl_tree {

static int avl_height(tree_node *node) { return node ? node->height : 0; }

static int balance_factor(tree_node *root) {
  return root ? avl_height(root->left) - avl_height(root->right) : 0;
}

static tree_node *right_rotation(tree_node *node) {
  // LOG("right rotation\n");
  // LOG("before\n");
  // LOG("%s\n", binary_tree::to_string(node, true).c_str());
  // make node's left tree as new root
  tree_node *left = node->left;
  assert(left);
  node->left = left->right;
  left->right = node;
  node->update_height();
  left->update_height();
  // LOG("after\n");
  // LOG("%s\n", binary_tree::to_string(left, true).c_str());
  return left;
}

static tree_node *left_rotation(tree_node *node) {
  // LOG("left rotation\n");
  // LOG("before\n");
  // LOG("%s\n", binary_tree::to_string(node, true).c_str());
  // make node's right tree as new root
  tree_node *right = node->right;
  assert(right);
  node->right = right->left;
  right->left = node;
  node->update_height();
  right->update_height();
  // LOG("after\n");
  // LOG("%s\n", binary_tree::to_string(right, true).c_str());
  return right;
}

static tree_node *re_balance(tree_node *root) {
  int balance = balance_factor(root);
  // 有几种可能的值：1,-1,0,-2,2
  // left unbalance and key is insert to left tree's left tree
  if (balance == 2) {
    tree_node *left = root->left;
    if (avl_height(left->left) >= avl_height(left->right)) {
      // left left
      LOG("LL case\n");
      root = right_rotation(root);
    } else {
      // left right
      LOG("LR case\n");
      // 先左旋左子树，然后右旋当前
      root->left = left_rotation(root->left);
      root = right_rotation(root);
    }
  } else if (balance == -2) {
    tree_node *right = root->right;
    // right right
    if (avl_height(right->right) >= avl_height(right->left)) {
      LOG("RR case\n");

      root = left_rotation(root);
    } else {
      // right left
      LOG("RL case\n");
      // 先右旋右子树，然后左旋当前
      root->right = right_rotation(root->right);
      root = left_rotation(root);
    }
  }
  return root;
}

void tree_node::update_height() {
  height = 1 + std::max(avl_height(left), avl_height(right));
}
} // namespace avl_tree

namespace binary_tree {
// 插入（是否有可能以非递归方式实现？）
template <> avl_tree::tree_node *insert(avl_tree::tree_node *root, int val) {
  using avl_tree::tree_node;
  if (!root) {
    LOG("insert %d\n", val);
    return new tree_node{val};
  }
  // 比较 val 和 root->val 的大小
  if (root->val > val) {
    root->left = insert(root->left, val);
  } else if (root->val < val) {
    root->right = insert(root->right, val);
  } else {
    // already exists!!
    return root;
  }
  root->update_height();
  return re_balance(root);
}

template <> std::string to_string(avl_tree::tree_node *root, bool pretty) {
  using avl_tree::tree_node;
  std::stringstream s;
  if (!root) {
    return {};
  }
  struct stack_item {
    tree_node *node{};
    int indent{};
    bool first_visit{true};
    bool has_right_sibling{false};
  };
  std::stack<stack_item> stack;
  stack.push({root, 0});
  while (!stack.empty()) {
    auto &item = stack.top();
    stack.pop();
    if (item.first_visit) {
      stack.push({item.node, item.indent, false, item.has_right_sibling});
      if (pretty) {
        for (int i = 0; i < item.indent; i++) {
          s << " ";
        }
      }
      s << "TreeNode(";
      if (item.node) {
        s << "val=" << item.node->val << ",height=" << item.node->height
          << ",balance_factor=" << balance_factor(item.node);
        if (item.node->left || item.node->right) {
          s << ",";
          if (pretty) {
            s << "\n";
          }
          if (item.node->right) {
            stack.push({item.node->right, item.indent + 1, true, false});
          }
          stack.push(
              {item.node->left, item.indent + 1, true, bool(item.node->right)});
        }
      }
    } else {
      if (pretty && item.node && (item.node->left || item.node->right)) {
        for (int i = 0; i < item.indent; i++) {
          s << " ";
        }
      }
      s << ")";
      if (item.has_right_sibling) {
        s << ",";
      }
      if (pretty) {
        s << "\n";
      }
    }
  }
  return s.str();
}

// 删除
template <> avl_tree::tree_node *remove(avl_tree::tree_node *root, int val) {
  using avl_tree::tree_node;
  // 首先进行正常的删除？然后判断是否平衡，并执行对应的操作？
  // 找到左子树中比这个值最接近的
  if (!root) {
    return root;
  }
  // 首先要不断找值为 val 的，然后再进行操作
  if (root->val > val) {
    root->left = remove(root->left, val);
  } else if (root->val < val) {
    root->right = remove(root->right, val);
  } else {
    // found now
    if (!root->left && !root->right) {
      // leaf node
      delete root;
      return nullptr;
    } else if (root->left && root->right) {
      // both, find right most node in left tree
      tree_node *right_most = root->left;
      while (right_most->right) {
        right_most = right_most->right;
      }
      std::swap(right_most->val, root->val);
      // recursive delete !!!
      // the new value is in left tree now (also a bstree)
      root->left = remove(root->left, val);
    } else if (!root->left && root->right) {
      // only right node
      tree_node *right = root->right;
      delete root;
      root = right;
    } else if (root->left && !root->right) {
      // only left node
      tree_node *left = root->left;
      delete root;
      root = left;
    }
  }

  root->update_height();
  return re_balance(root);
}
} // namespace binary_tree