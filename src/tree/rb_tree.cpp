#include "rb_tree.hpp"
#include <cassert>
#include <queue>
#include <sstream>
#include <stack>

#define DETACH(node)                                                           \
  do {                                                                         \
    if (node->parent) {                                                        \
      if (node->parent->left == node) {                                        \
        node->parent->left = nullptr;                                          \
      } else {                                                                 \
        node->parent->right = nullptr;                                         \
      }                                                                        \
    }                                                                          \
  } while (0)

#define SIBLING(node)                                                          \
  (node && node->parent ? (node->parent->left == node ? node->parent->right    \
                                                      : node->parent->left)    \
                        : nullptr)

namespace rb_tree {

static tree_node *binary_tree_insert(tree_node *root, tree_node *curr) {
  if (!root) {
    // 默认创建的节点颜色为红色
    return curr;
  }
  if (root->val > curr->val) {
    root->left = binary_tree_insert(root->left, curr);
    root->left->parent = root;
  } else if (root->val < curr->val) {
    root->right = binary_tree_insert(root->right, curr);
    root->right->parent = root;
  } else {
    curr->parent = root->parent;
    curr->color = root->color;
    curr->left = root->left;
    curr->right = root->right;
    if (curr->parent) {
      if (curr->parent->left == root) {
        curr->parent->left = curr;
      } else {
        curr->parent->right = curr;
      }
    }
    if (curr->left) {
      curr->left->parent = curr;
    }
    if (curr->right) {
      curr->right->parent = curr;
    }
    delete root;
    root = curr;
  }
  // perform rb_tree build here
  return root;
}

static tree_node *right_rotation(tree_node *A) {
  using binary_tree::to_string;

  // LOG("right rotation\n");
  // LOG("before\n");
  // LOG("%s\n", to_string(A, true).c_str());
  // make node's left tree as new root
  assert(A->left);
  tree_node *B = A->left;
  tree_node *A_parent = A->parent;
  tree_node *B_right = B->right;
  B->right = A;
  A->parent = B;
  B->parent = A_parent;
  if (A_parent) {
    if (A_parent->left == A) {
      A_parent->left = B;
    } else {
      A_parent->right = B;
    }
  }
  A->left = B_right;
  if (B_right) {
    B_right->parent = A;
  }
  // LOG("after\n");
  // LOG("%s\n", to_string(B, true).c_str());
  return B;
}

static tree_node *left_rotation(tree_node *A) {
  using binary_tree::to_string;

  // LOG("left rotation\n");
  // LOG("before\n");
  // LOG("%s\n", to_string(A, true).c_str());
  // make node's right tree as new root
  assert(A->right);
  tree_node *B = A->right;
  tree_node *A_parent = A->parent;
  tree_node *B_left = B->left;
  B->left = A;
  A->parent = B;
  B->parent = A_parent;
  if (A_parent) {
    if (A_parent->left == A) {
      A_parent->left = B;
    } else {
      A_parent->right = B;
    }
  }
  A->right = B_left;
  if (B_left) {
    B_left->parent = A;
  }
  // LOG("after\n");
  // LOG("%s\n", to_string(B, true).c_str());
  return B;
}

static tree_node *find_predecessor(tree_node *node) {
  if (!node) {
    return nullptr;
  }
  tree_node *curr = node->left;
  while (curr && curr->right) {
    curr = curr->right;
  }
  LOG("predecessor of %s is %s\n", std::to_string(node->val).c_str(),
      (curr ? std::to_string(curr->val) : "null").c_str());
  return curr;
}

bool check_rb_tree(tree_node *root) {
  // check property of red black tree
  if (!root) {
    return true;
  }
  if (root->color != BLACK) {
    fprintf(stderr, "root->color is not BLACK\n");
    return false;
  }

  struct queue_item {
    tree_node *node{};
    int black_depth{};
  };

  std::queue<queue_item> queue;
  queue.push({root, 1});
  int leaf_black_depth = -1;
  while (!queue.empty()) {
    auto [curr, black_depth] = queue.front();
    queue.pop();
    // 首先检查 相邻的红黑
    if (!curr) {
      // leaf node
      black_depth++;
      if (leaf_black_depth == -1) {
        leaf_black_depth = black_depth;
      } else if (leaf_black_depth != black_depth) {
        fprintf(stderr, "leaf black height is not consistent!\n");
        return false;
      }
    } else {
      // check link valid
      if (curr->parent) {
        tree_node *sibling = SIBLING(curr->parent);
        if (sibling == curr) {
          fprintf(stderr, "sibling equals to curr!\n");
          return false;
        }
        if (curr->parent->left != curr && curr->parent->right != curr) {
          fprintf(stderr, "parent link mismatch!\n");
          return false;
        }
      }
      if (curr->color == RED) {
        // check parent and children
        if ((curr->parent && curr->parent->color == RED) ||
            (curr->left && curr->left->color == RED) ||
            (curr->right && curr->right->color == RED)) {
          fprintf(stderr, "two consecutive red node!\n");
          return false;
        }
        queue.push({curr->left, black_depth});
        queue.push({curr->left, black_depth});
      } else {
        queue.push({curr->left, black_depth + 1});
        queue.push({curr->left, black_depth + 1});
      }
    }
  }
  return true;
}

} // namespace rb_tree

namespace binary_tree {
// 插入
template <> rb_tree::tree_node *insert(rb_tree::tree_node *root, int val) {
  using rb_tree::BLACK;
  using rb_tree::RED;
  using rb_tree::tree_node;

  if (!root) {
    return new tree_node{val, BLACK};
  }
  // 需要手动区分一下根节点情况
  tree_node *curr = new tree_node{val, RED};
  // 使用普通二叉树进行插入
  root = binary_tree_insert(root, curr);
  LOG("after binary tree insert\n");
  LOG("%s\n", to_string(root, true).c_str());
  // 进行修正
  tree_node *grand_parent{};
  tree_node *parent{};
  tree_node *parent_sibling{};
  while (curr != root && curr->color == RED && curr->parent->color == RED) {
    // resolve red/red conflict
    parent = curr->parent;
    grand_parent = parent->parent;
    if (parent == grand_parent->left) {
      parent_sibling = grand_parent->right;
      if (parent_sibling && parent_sibling->color == RED) {
        grand_parent->color = RED;
        parent->color = BLACK;
        parent_sibling->color = BLACK;
        curr = grand_parent;
      } else {
        if (curr == parent->right) {
          // perform LR
          left_rotation(parent);
          std::swap(parent, curr);
        }
        // perform LL
        right_rotation(grand_parent);
        std::swap(parent, grand_parent);
        std::swap(parent->color, grand_parent->color);
        curr = grand_parent;
      }
    } else {
      // mirror case
      parent_sibling = grand_parent->left;
      if (parent_sibling && parent_sibling->color == RED) {
        grand_parent->color = RED;
        parent->color = BLACK;
        parent_sibling->color = BLACK;
        curr = grand_parent;
      } else {
        if (curr == parent->left) {
          // perform RL
          right_rotation(parent);
          std::swap(parent, curr);
        }
        // perform RR
        left_rotation(grand_parent);
        std::swap(parent, grand_parent);
        std::swap(parent->color, grand_parent->color);
        curr = grand_parent;
      }
    }
    if (!curr->parent) {
      root = curr;
    }
  }
  root->color = BLACK;
  return root;
}

// 删除
template <> rb_tree::tree_node *remove(rb_tree::tree_node *root, int val) {
  using rb_tree::BLACK;
  using rb_tree::RED;
  using rb_tree::tree_node;
  // 首先找到需要删除的节点
  tree_node *node = binary_tree::search<tree_node>(root, val);
  if (!node) {
    // 没有找到节点，说明不存在 val，不作任何调整
    return root;
  }
  // 接下来判断节点状态
  if (node->left && node->right) {
    // 两个孩子的情况
    tree_node *predecessor = find_predecessor(node);
    // 交换
    LOG("swap %d and %d\n", node->val, predecessor->val);
    std::swap(node->val, predecessor->val);
    node = predecessor;
  }
  // BLACK single child node
  if ((node->left && !node->right) || (!node->left && node->right)) {
    tree_node *child = node->left ? node->left : node->right;
    assert(node->color == BLACK && child->color == RED);
    std::swap(node->val, child->val);
    node->left = nullptr;
    node->right = nullptr;
    LOG("delete BLACK single child node [%d]\n", child->val);
    delete child;
    // 此时就已经可以直接返回了
    return root;
  }
  assert(!node->left && !node->right);
  if (node->color == RED) {
    // RED leaf node
    LOG("delete leaf RED node [%d]\n", node->val);
    LOG("%s\n", to_string(root, true).c_str());
    // 可以直接删除当前节点
    DETACH(node);
    LOG("%s\n", to_string(root, true).c_str());
    if (!node->parent) {
      root = nullptr;
    }
    delete node;
    return root;
  }
  // BLACK leaf node
  LOG("delete leaf BLACK node [%d]\n", node->val);
  if (!node->parent) {
    // 对于根节点就直接删除吧
    delete node;
    return nullptr;
  }
  // 把要删除的节点暂存一下
  tree_node *delete_node = node;
  tree_node *parent{};
  tree_node *sibling{};
  bool is_left_child{};
  // 此时已经可以吧节点给删掉了
  while (node != root) {
    parent = node->parent;
    sibling = SIBLING(node);
    is_left_child = node == parent->left;
    if (!sibling) {
      node = parent;
    } else if (sibling->color == RED) {
      // 此时只需要执行一些简单的
      // 直接进行左旋即可
      std::swap(sibling->color, parent->color);
      if (is_left_child) {
        parent = left_rotation(parent);
      } else {
        parent = right_rotation(parent);
      }
      // 此处需要判断一下是不是根节点？
      if (!parent->parent) {
        root = parent;
      }
    } else if (sibling->color == BLACK) {
      bool is_sibling_left_red = sibling->left && sibling->left->color == RED;
      bool is_sibling_right_red =
          sibling->right && sibling->right->color == RED;
      if (!is_sibling_left_red && !is_sibling_right_red) {
        sibling->color = RED;
        if (parent->color == RED) {
          parent->color = BLACK;
          break;
        }
        node = parent;
        // 进入下一次循环
      } else {
        if (is_left_child) {
          // sibling is right child of parent
          if (is_sibling_left_red && !is_sibling_right_red) {
            // covert this case to right right case
            std::swap(sibling->color, sibling->left->color);
            sibling = right_rotation(sibling);
          }
          // perform left rotation
          std::swap(sibling->color, parent->color);
          sibling->right->color = BLACK;
          parent = left_rotation(parent);
          if (!parent->parent) {
            root = parent;
          }
        } else {
          // sibling is left child of parent
          if (is_sibling_right_red && !is_sibling_left_red) {
            // convert this cast to left left
            std::swap(sibling->color, sibling->right->color);
            sibling = left_rotation(sibling);
          }
          // perform right rotation
          std::swap(sibling->color, parent->color);
          sibling->left->color = BLACK;
          parent = right_rotation(parent);
          if (!parent->parent) {
            root = parent;
          }
        }
        break;
      }
    }
  }

  // 此时就已经可以删除了
  DETACH(delete_node);
  delete delete_node;
  return root;
}
template <> std::string to_string(rb_tree::tree_node *root, bool pretty) {
  using rb_tree::BLACK;
  using rb_tree::RED;
  using rb_tree::tree_node;
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
        s << "val=" << item.node->val
          << ",color=" << (item.node->color == RED ? "RED" : "BLACK")
          << ",parent="
          << (item.node->parent ? std::to_string(item.node->parent->val)
                                : "null");
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
} // namespace binary_tree