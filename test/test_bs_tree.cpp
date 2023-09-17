#include "avl_tree.hpp"
#include "bs_tree.hpp"
#include "rb_tree.hpp"

#include <cassert>
#include <chrono>
#include <iostream>
#include <random>
#include <set>
#include <sstream>

struct bool_t {
  bool _bool;

  bool_t(bool b) : _bool(b) {}
  explicit operator bool() const noexcept { return _bool; }
};

std::ostream &operator<<(std::ostream &os, bool_t b) {
  return b ? os << "true" : os << "false";
}

template <typename T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &v) {
  if (v.empty()) {
    return os << "[]";
  }
  os << "[" << *v.begin();
  for (auto i = v.begin() + 1; i != v.end(); i++) {
    os << "," << *i;
  }
  os << "]";
  return os;
}

// using namespace binary_tree;

enum operation_t { INSERT, REMOVE, SEARCH };

struct invoke_param {
  operation_t operation{};
  int val{};
};

std::ostream &operator<<(std::ostream &os, invoke_param b) {
  return os << "{"
            << (b.operation == INSERT
                    ? "INSERT"
                    : (b.operation == REMOVE ? "REMOVE" : "SEARCH"))
            << "," << b.val << "}";
}

std::vector<invoke_param> random_opeartion(int n, int min_val = 0,
                                           int max_val = 10000) {
  std::mt19937 rd(13337);
  std::uniform_int_distribution<int> random_op{INSERT, SEARCH};
  std::normal_distribution<float> random_val{static_cast<float>(min_val),
                                             static_cast<float>(max_val)};
  std::vector<invoke_param> res;
  for (int i = 0; i < n; i++) {
    res.push_back(invoke_param{static_cast<operation_t>(random_op(rd)),
                               static_cast<int>(random_val(rd))});
  }

  // std::sort(res.begin(), res.end(), [](invoke_param a, invoke_param b) -> bool {
  //   // if (a.operation == b.operation) {
  //   //   return (a.val - b.val) < 100;
  //   // }
  //   // make sure insert is first
  //   return a.operation < b.operation;
  // });

  return res;
}

void benchmark(int round = 50, int n = 10000, int min_val = 0,
               int max_val = 10000) {
  using namespace std::chrono;

  std::stringstream s;
  for (int i = 0; i < round; i++) {
    high_resolution_clock::duration bst_time{};
    high_resolution_clock::duration avl_time{};
    high_resolution_clock::duration rb_time{};
    high_resolution_clock::duration stl_time{};
    rb_tree::tree_node *rb_root{}, *rb_node{};
    avl_tree::tree_node *avl_root{}, *avl_node{};
    binary_tree::tree_node *bst_root{}, *bst_node{};
    std::set<int> tree_set{};
    auto ops = random_opeartion(n, min_val, max_val);

    for (auto &op : ops) {
      // s.str("");
      // s.clear();
      // s << op;
      // printf("%s\n", s.str().c_str());
      // printf("before:\n%s\n", binary_tree::to_string(rb_root, true).c_str());
      // std::string prev = binary_tree::to_string(rb_root, true);
      switch (op.operation) {
      case INSERT: {
        auto tick0 = high_resolution_clock::now();
        rb_root = binary_tree::insert(rb_root, op.val);
        auto tick1 = high_resolution_clock::now();
        avl_root = binary_tree::insert(avl_root, op.val);
        auto tick2 = high_resolution_clock::now();
        bst_root = binary_tree::insert(bst_root, op.val);
        auto tick3 = high_resolution_clock::now();
        tree_set.insert(op.val);
        auto tick4 = high_resolution_clock::now();
        // rb_time += (tick1 - tick0);
        // avl_time += (tick2 - tick1);
        // bst_time += (tick3 - tick2);
        // stl_time += (tick4 - tick3);
        break;
      }

      case SEARCH: {
        auto tick0 = high_resolution_clock::now();
        rb_node = binary_tree::search(rb_root, op.val);
        auto tick1 = high_resolution_clock::now();
        avl_node = binary_tree::search(avl_root, op.val);
        auto tick2 = high_resolution_clock::now();
        bst_node = binary_tree::search(bst_root, op.val);
        auto tick3 = high_resolution_clock::now();
        auto res = tree_set.find(op.val);
        auto tick4 = high_resolution_clock::now();
        // rb_time += (tick1 - tick0);
        // avl_time += (tick2 - tick1);
        // bst_time += (tick3 - tick2);
        // stl_time += (tick4 - tick3);
        assert((!avl_node && !bst_node && !rb_node && res == tree_set.end()) ||
               (avl_node && bst_node && rb_node && res != tree_set.end() &&
                avl_node->val == bst_node->val &&
                rb_node->val == bst_node->val && *res == bst_node->val));
        break;
      }
      case REMOVE:
        auto tick0 = high_resolution_clock::now();
        rb_root = binary_tree::remove(rb_root, op.val);
        auto tick1 = high_resolution_clock::now();
        avl_root = binary_tree::remove(avl_root, op.val);
        auto tick2 = high_resolution_clock::now();
        bst_root = binary_tree::remove(bst_root, op.val);
        auto tick3 = high_resolution_clock::now();
        tree_set.erase(op.val);
        auto tick4 = high_resolution_clock::now();
        rb_time += (tick1 - tick0);
        avl_time += (tick2 - tick1);
        bst_time += (tick3 - tick2);
        stl_time += (tick4 - tick3);
        break;
      }
      // assert(rb_tree::check_rb_tree(rb_root));
      // assert(avl_tree::check_avl_tree(avl_root));
      LOG("===============================================\n");
    }

    printf("round [%02d]: rb_tree: %.3fs, avl_tree: %.3fs, bst_tree: %.3fs, "
           "stl_tree_set: %.3fs\n",
           i, duration_cast<milliseconds>(rb_time).count() / 1000.0f,
           duration_cast<milliseconds>(avl_time).count() / 1000.0f,
           duration_cast<milliseconds>(bst_time).count() / 1000.0f,
           duration_cast<milliseconds>(stl_time).count() / 1000.0f);
    fflush(stdout);
    binary_tree::deleter<rb_tree::tree_node>()(rb_root);
    binary_tree::deleter<avl_tree::tree_node>()(avl_root);
    binary_tree::deleter<binary_tree::tree_node>()(bst_root);
  }
}

int main() {
  // using binary_tree::insert;
  // using binary_tree::remove;
  // using binary_tree::to_string;
  // using rb_tree::tree_handle;
  // using rb_tree::tree_node;
  // using std::cout;
  // using std::endl;
  // tree_node *n0 = new tree_node{45};
  // tree_node *n1 = new tree_node{24};
  // tree_node *n2 = new tree_node{55};
  // tree_node *n3 = new tree_node{12};
  // tree_node *n4 = new tree_node{37};
  // tree_node *n5 = new tree_node{53};
  // tree_node *n6 = new tree_node{60};
  // tree_node *n7 = new tree_node{28};
  // tree_node *n8 = new tree_node{40};
  // tree_node *n9 = new tree_node{70};
  // n0->left = n1;
  // n0->right = n2;
  // n1->left = n3;
  // n1->right = n4;
  // n2->left = n5;
  // n2->right = n6;
  // n4->left = n7;
  // n4->right = n8;
  // n6->right = n9;
  // tree_handle root = tree_handle(n0);
  // using std::cout;
  // using std::endl;
  // cout << "no pretty" << endl;
  // cout << to_string(root.get()) << endl;
  // cout << "pretty" << endl;
  // cout << to_string(root.get(), true) << endl;
  // cout << "is avl tree: "
  //      << static_cast<bool_t>(avl_tree::is_avl_tree(root.get())) << endl;
  // tree_node *root{};
  // // std::vector<int> seq{17, 32, 44, 48, 50, 62, 78, 88};
  // // std::vector<int> insert_seq{42, 10, 64, 7, 29, 50, 83, 5, 31, 90};
  // std::vector<int> insert_seq{7, 4, 8, 2, 5, 9};
  // for (int v : insert_seq) {
  //   root = insert(root, v);
  //   cout << "after insert: " << v << endl;
  //   cout << to_string(root, true);
  //   cout << "is rb_tree: " <<
  //   static_cast<bool_t>(rb_tree::check_rb_tree(root))
  //        << endl;
  //   cout << "==================================" << endl;
  // }
  // // std::vector<int> remove_seq{90, 7, 10, 50};
  // std::vector<int> remove_seq{7};
  // for (int v : remove_seq) {
  //   root = remove(root, v);
  //   cout << "after remove: " << v << endl;
  //   cout << to_string(root, true);
  //   cout << "is rb_tree: " <<
  //   static_cast<bool_t>(rb_tree::check_rb_tree(root))
  //        << endl;
  //   cout << "==================================" << endl;
  // }
  // // RAII
  // tree_handle _(root);

  benchmark(10, 1000000, 0, std::numeric_limits<int>::max());
  return 0;
}