#pragma once

#include "config.hpp"

#include <array>
#include <iostream>
#include <queue>
#include <sstream>
#include <string>

template <typename Key> class tree_23_set {
public:
  struct tree_node {
  public:
    using node_ptr = tree_node *;

  public:
    node_ptr parent{};
    node_ptr left{};
    node_ptr middle{};
    node_ptr right{};

    std::optional<Key> key0{}, key1{};

  public:
    static inline uint64_t new_count = 0;

    void new_callback() {
      LOG("new node\n");
      ++new_count;
    }

    void delete_callback() {
      --new_count;
      LOG("delete node\n");
    }

    tree_node(const Key &key) : key0(key) { new_callback(); }
    tree_node(const Key &key0, const Key &key1) : key0(key0), key1(key1) {
      new_callback();
    }

    ~tree_node() { delete_callback(); }

    bool is_2_node() const { return !is_3_node(); }

    bool is_3_node() const { return static_cast<bool>(key1); }

    bool is_null() const {
      return !static_cast<bool>(key0) && !static_cast<bool>(key1);
    }

    bool is_leaf() const { return !left && !middle && !right; }

    bool contains(const Key &key) const {
      return is_3_node() ? key0.value() == key || key1.value() == key
                         : key0.value() == key;
    }
  };

public:
  using key_type = Key;

  tree_23_set() = default;

  ~tree_23_set() { destroy_tree(root); }

  std::string tree_view() const {
    std::stringstream os;
    preorder_pretty(root, os);
    return os.str();
  }

  std::string to_string() const {
    std::stringstream os;
    os << "{ ";
    inorder_tree(root, [&](const Key &k) { os << k << " "; });
    os << "}";
    return os.str();
  }

  bool search(const Key &key) {
    return static_cast<bool>(search_tree(root, key));
  }

  void insert(const Key &key) { root = insert_tree(root, key); }

  void remove(const Key &key) { root = remove_tree(root, key); }

private:
  static void destroy_tree(tree_node *root) {
    std::queue<tree_node *> q{};
    q.push(root);
    while (!q.empty()) {
      tree_node *curr = q.front();
      q.pop();
      if (curr) {
        q.push(curr->left);
        if (curr->is_3_node()) {
          q.push(curr->middle);
        }
        q.push(curr->right);
        delete curr;
      }
    }
  }

  static void preorder_pretty(tree_node *root, std::stringstream &os,
                              int depth = 0) {
    for (int i = 0; i < depth; i++) {
      os << '~';
    }
    os << to_string(root) << '\n';
    if (!root || (!root->left && !root->middle && !root->right)) {
      return;
    }
    preorder_pretty(root->left, os, depth + 1);
    if (root->is_3_node()) {
      preorder_pretty(root->middle, os, depth + 1);
    }
    preorder_pretty(root->right, os, depth + 1);
  }

  static void inorder_tree(tree_node *root,
                           const std::function<void(const Key &)> &f) {
    if (!root) {
      return;
    }
    inorder_tree(root->left, f);
    f(root->key0.value());
    if (root->is_3_node()) {
      inorder_tree(root->middle, f);
      f(root->key1.value());
    }
    inorder_tree(root->right, f);
  }

  static tree_node *search_tree(tree_node *root, const Key &key) {
    if (!root) {
      return root;
    }
    if (root->contains(key)) {
      return root;
    }
    if (root->key0 > key) {
      return search_tree(root->left, key);
    }
    // 等于的情况呢？
    if (root->is_3_node() && root->key1 > key) {
      return search_tree(root->middle, key);
    }
    return search_tree(root->right, key);
  }

  // 找到可以插入 key 的叶子结点，和 search 有一点区别
  static tree_node *range_search_tree(tree_node *root, const Key &key) {
    if (!root) {
      return root;
    }
    if (root->contains(key)) {
      // 已经存在于 root 中
      return root;
    }
    // 不存在的情况
    if (root->key0 > key) {
      return root->left ? range_search_tree(root->left, key) : root;
    }
    if (root->is_3_node() && root->key1 > key) {
      return root->middle ? range_search_tree(root->middle, key) : root;
    }
    return root->right ? range_search_tree(root->right, key) : root;
  }

  static tree_node *insert_tree(tree_node *root, const Key &key) {
    tree_node *node = range_search_tree(root, key);
    // 根节点为空情况
    if (!node) {
      return new tree_node{key};
    }
    // 已经存在，不需要再添加
    if (node->contains(key)) {
      return root;
    }
    // 进入 merge loop
    enum node_pos { LEFT, MIDDLE, RIGHT };
    node_pos curr_pos{LEFT}, next_pos{LEFT};
    tree_node *new_node = new tree_node{key};
    tree_node *n0{}, *n1{}, *n2{};

    while (true) {
      if (!node) {
        root = new_node;
        break;
      } else if (node->is_2_node()) {
        // insert internal node
        LOG("insert %s into %s\n", to_string(new_node).c_str(),
            to_string(node).c_str());
        if (new_node->key0 > node->key0) {
          node->key1 = new_node->key0;
          M(node, new_node->left);
          R(node, new_node->right);
        } else {
          node->key1 = node->key0;
          node->key0 = new_node->key0;
          L(node, new_node->left);
          M(node, new_node->right);
        }
        delete new_node;
        break;
      } else {
        LOG("insert %s into %s\n", to_string(new_node).c_str(),
            to_string(node).c_str());
        tree_node *parent = node->parent;
        node->parent = nullptr;
        if (parent) {
          if (node == parent->left) {
            parent->left = nullptr;
            next_pos = LEFT;
          } else if (node == parent->middle) {
            parent->middle = nullptr;
            next_pos = MIDDLE;
          } else if (node == parent->right) {
            parent->right = nullptr;
            next_pos = RIGHT;
          } else {
            assert(false && "unexpected case!");
          }
        }
        if (node->is_leaf()) {
          // 对于叶子节点的合并比较简单，
          Key a{}, b{}, c{};
          if (key < node->key0) {
            a = key;
            b = node->key0.value();
            c = node->key1.value();
            n0 = new_node;
            n1 = node;
            n2 = new tree_node{c};
          } else if (key < node->key1) {
            a = node->key0.value();
            b = key;
            c = node->key1.value();
            n0 = node;
            n1 = new_node;
            n2 = new tree_node{c};
          } else {
            a = node->key0.value();
            b = node->key1.value();
            c = key;
            n0 = node;
            n1 = new tree_node{b};
            n2 = new_node;
          }
        } else {
          switch (curr_pos) {
          case LEFT: {
            n0 = new_node;
            n1 = node;
            n2 = new tree_node{node->key1.value()};
            L(n2, node->middle);
            R(n2, node->right);
            break;
          }
          case MIDDLE: {
            n0 = node;
            n1 = new_node;
            n2 = new tree_node{node->key1.value()};
            L(n2, new_node->right);
            R(n2, node->right);
            R(n0, new_node->left);
            break;
          }
          case RIGHT: {
            n0 = node;
            n1 = new tree_node{node->key1.value()};
            n2 = new_node;
            R(n0, n0->middle);
            break;
          }
          }
        }
        n0->key1.reset();
        n1->key1.reset();
        n2->key1.reset();
        L(n1, n0);
        R(n1, n2);
        n0->middle = nullptr;
        n1->middle = nullptr;
        n1->parent = nullptr;
        n2->middle = nullptr;
        // next status
        new_node = n1;
        node = parent;
        curr_pos = next_pos;
      }
    }
    return root;
  }

  static tree_node *remove_tree(tree_node *root, const Key &key) {
    LOG("start remove\n");
    // 首先还是找到该节点的位置
    tree_node *node = search_tree(root, key);
    if (!node) {
      // 没找到，直接返回
      LOG("%s\n", (std::to_string(key) + " not found").c_str());
      return root;
    }

    LOG("%s\n", ("found " + std::to_string(key) + " in node " + to_string(node))
                    .c_str());

    if (!node->is_leaf()) {
      // find inorder precedence
      tree_node *curr = node->key0.value() == key ? node->left : node->middle;
      while (curr && curr->right) {
        curr = curr->right;
      }
      LOG("predecessor for %s is %s\n", std::to_string(key).c_str(),
          to_string(curr).c_str());

      // 交换值，然后删除叶子节点
      if (node->is_2_node()) {
        std::swap(curr->key0, node->key0);
      } else {
        std::swap(curr->key0,
                  node->key0.value() == key ? node->key0 : node->key1);
      }
      node = curr;
    }
    LOG("remove leaf node %s with %s\n", to_string(node).c_str(),
        std::to_string(key).c_str());
    if (node->is_3_node()) {
      // 将其调整为 2 node 即可
      if (node->key0 == key) {
        node->key0 = node->key1;
      }
      node->key1.reset();
      return root;
    }
    // 剩下的情况就很复杂的，需要考虑很多种情况，而且可能需要循环一直向上进行调整
    // 但是只有一种情况需要一直向上删除，就是产生新的 null 节点的情况
    tree_node *parent = node->parent;
    // 首先把 node 中对应 key 的那个元素给删了再说
    // 此时 node 一定是 2-node
    assert(node->is_2_node());
    node->key0.reset();
    bool running = true;
    while (running) {
      // perform merge
      if (!parent) {
        // 删除根节点了
        LOG("remove root node, update to root->left\n");
        root = node->left;
        delete node;
        // 必须确保根节点的parent为空
        root->parent = nullptr;
        break;
      }

      if (parent->is_2_node()) {
        // 父节点是 2 node
        // 4 中情况
        // 22,23,32,33
        uint8_t node_status =
            (parent->left->is_3_node() << 1) | (parent->right->is_3_node());
        static std::string status_str[4]{"22", "23", "32", "33"};
        LOG("2-node status: %s\n", status_str[node_status].c_str());
        switch (node_status) {
        case 0b00: {
          LOG("merge 2-node\n");
          tree_node *left = parent->left;
          tree_node *right = parent->right;
          if (left->is_null()) {
            left->key0 = std::move(parent->key0);
            left->key1 = std::move(right->key0);
            M(left, right->left);
            R(left, right->right);
          } else if (right->is_null()) {
            // 此处必须保证生成的 null 节点位于左侧？
            left->key1 = std::move(parent->key0);
            M(left, left->right);
            R(left, right->left);
          } else {
            assert(false && "unexpected case!");
          }
          parent->right = nullptr;
          delete right;
          parent->key0.reset();
          LOG("move up\n");
          node = parent;
          parent = parent->parent;
          break;
        }
        case 0b01: {
          assert(parent->left->is_null());
          // 这个实际上有点像左旋
          LOG("redistribute [left case]\n");
          tree_node *right = parent->right;
          node->key0 = std::move(parent->key0);
          parent->key0 = std::move(right->key0);
          right->key0 = std::move(right->key1);
          R(node, right->left);
          L(right, right->middle);
          right->middle = nullptr;
          right->key1.reset();
          running = false;
          break;
        }
        case 0b10: {
          assert(parent->right->is_null());
          // redistribute will utilize the old node,
          // so there is no need to create new node
          LOG("redistribute [right case]\n");
          tree_node *left = parent->left;
          node->key0 = std::move(parent->key0);
          parent->key0 = std::move(left->key1);
          R(node, node->left);
          L(node, left->right);
          R(left, left->middle);
          left->middle = nullptr;
          left->key1.reset();
          running = false;
          break;
        }
        default: {
          assert(false && "unexpected node_status 33");
        }
        }
      } else {
        // 子节点是 3 node
        // 333,332,323,322,233,232,223,222
        // 8种情况，其中 333 可以忽略
        uint8_t node_status = (parent->left->is_3_node() << 2) |
                              (parent->middle->is_3_node() << 1) |
                              (parent->right->is_3_node());
        static std::string status_str[8]{"222", "223", "232", "233",
                                         "322", "323", "332", "333"};
        LOG("3-node status: %s\n", status_str[node_status].c_str());

        switch (node_status) {
        case 0b000: {
          // 三种情况
          tree_node *left = parent->left;
          tree_node *middle = parent->middle;
          tree_node *right = parent->right;
          if (left->is_null()) {
            LOG("merge [left case]\n");
            left->key0 = std::move(parent->key0);
            left->key1 = std::move(middle->key0);
            parent->key0 = std::move(parent->key1);
            M(left, middle->left);
            R(left, middle->right);
          } else if (middle->is_null()) {
            LOG("merge [middle case]\n");
            left->key1 = std::move(parent->key0);
            parent->key0 = std::move(parent->key1);
            M(left, left->right);
            R(left, middle->left);
          } else if (right->is_null()) {
            LOG("merge [right case]\n");
            right->key1 = std::move(parent->key1);
            right->key0 = std::move(middle->key0);
            R(right, right->left);
            M(right, middle->right);
            L(right, middle->left);
          } else {
            assert(false && "unexpected case!");
          }
          parent->key1.reset();
          parent->middle = nullptr;
          delete middle;
          running = false;
          break;
        }
        case 0b001: {
          LOG("merge [left case]\n");
          tree_node *left = parent->left;
          tree_node *middle = parent->middle;
          if (left->is_null()) {
            left->key0 = std::move(parent->key0);
            left->key1 = std::move(middle->key0);
            M(left, middle->left);
            R(left, middle->right);
          } else if (middle->is_null()) {
            left->key1 = std::move(parent->key0);
            M(left, left->right);
            R(left, middle->left);
          } else {
            assert(false && "unexpected case!");
          }
          parent->key0 = std::move(parent->key1);
          parent->key1.reset();
          delete middle;
          parent->middle = nullptr;
          running = false;
          break;
        }
        case 0b010: {
          LOG("merge [middle case]\n");
          tree_node *left = parent->left;
          tree_node *middle = parent->middle;
          tree_node *right = parent->right;
          if (left->is_null()) {
            left->key0 = std::move(parent->key0);
            left->key1 = std::move(middle->key0);
            parent->key0 = std::move(middle->key1);
            right->key1 = std::move(right->key0);
            right->key0 = std::move(parent->key1);
            M(left, middle->left);
            R(left, middle->middle);
            M(right, right->left);
            L(right, middle->right);
          } else if (right->is_null()) {
            left->key1 = std::move(parent->key0);
            parent->key0 = std::move(middle->key0);
            right->key0 = std::move(middle->key1);
            right->key1 = std::move(parent->key1);
            M(left, left->right);
            R(left, middle->left);
            R(right, right->left);
            M(right, middle->right);
            L(right, middle->middle);
          } else {
            assert(false && "unexpected case!");
          }
          parent->key1.reset();
          delete middle;
          parent->middle = nullptr;
          running = false;
          break;
        }
        case 0b100: {
          LOG("merge [right case]\n");
          tree_node *middle = parent->middle;
          tree_node *right = parent->right;
          if (middle->is_null()) {
            right->key1 = std::move(right->key0);
            right->key0 = std::move(parent->key1);
            M(right, right->left);
            L(right, middle->left);
          } else if (right->is_null()) {
            right->key1 = std::move(parent->key1);
            right->key0 = std::move(middle->key0);
            R(right, right->left);
            M(right, middle->right);
            L(right, middle->left);
          } else {
            assert(false && "unexpected case!");
          }
          parent->key1.reset();
          delete middle;
          parent->middle = nullptr;
          running = false;
          break;
        }
        case 0b011: {
          // 只有一种情况
          LOG("redistribute [left case]\n");
          tree_node *middle = parent->middle;
          node->key0 = std::move(parent->key0);
          parent->key0 = std::move(middle->key0);
          middle->key0 = std::move(middle->key1);
          middle->key1.reset();
          R(node, middle->left);
          L(middle, middle->middle);
          middle->middle = nullptr;
          running = false;
          break;
        }
        case 0b101: {
          // 只有一种情况
          LOG("redistribute [middle case]\n");
          tree_node *right = parent->right;
          node->key0 = std::move(parent->key1);
          parent->key1 = std::move(right->key0);
          right->key0 = std::move(right->key1);
          right->key1.reset();
          R(node, right->left);
          L(right, right->middle);
          right->middle = nullptr;
          running = false;
          break;
        }
        case 0b110: {
          // 只有一种情况
          LOG("redistribute [right case]\n");
          tree_node *middle = parent->middle;
          node->key0 = std::move(parent->key1);
          parent->key1 = std::move(middle->key1);
          middle->key1.reset();
          R(node, node->left);
          L(node, middle->right);
          R(middle, middle->middle);
          middle->middle = nullptr;
          running = false;
          break;
        }
        default: {
          assert(false && "unexpected node_status 333");
        }
        }
      }
    }
    return root;
  }

  static std::string to_string(tree_node *node) {
    if (!node) {
      return "null";
    }
    std::stringstream os;

    if (node->is_2_node()) {
      os << "2_node{";
      os << "key:" << node->key0.value();
    } else {
      os << "3_node{"
         << "left:" << node->key0.value() << ",right:" << node->key1.value();
    }
    os << "}";
    return os.str();
  }

private:
  tree_node *root{};
};