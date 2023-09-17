#include "bs_tree.hpp"

#include <iostream>
#include <sstream>
#include <stack>

namespace binary_tree {
template <> std::string to_string(tree_node *root, bool pretty) {
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
        s << item.node->val;
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

// 插入
template <> tree_node *insert(tree_node *root, int val) {
  if (!root) {
    return new tree_node{val};
  }
  if (root->val > val) {
    root->left = insert(root->left, val);
  } else if (root->val < val) {
    root->right = insert(root->right, val);
  }
  return root;
}

// 删除
template <> tree_node *remove(tree_node *root, int val) {
  if (!root) {
    return root;
  }
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
  return root;
}

} // namespace binary_tree