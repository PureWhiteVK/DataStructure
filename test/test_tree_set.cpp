#include <functional>
#include <iostream>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <type_traits>

#include "tree_23.hpp"
#include "tree_avl.hpp"
#include "tree_b.hpp"
#include "tree_b_plus.hpp"
#include "tree_bst.hpp"
#include "tree_rb.hpp"

#include <math.h>

enum Operation { INSERT, SEARCH, REMOVE };

template <typename Key> class std_set {
public:
  using key_type = Key;

  std_set() = default;

  void insert(const Key &key) { set.insert(key); }

  void remove(const Key &key) { set.erase(key); }

  bool search(const Key &key) { return static_cast<bool>(set.count(key)); }

  std::string to_string() {
    std::stringstream os{};
    os << "{ ";
    for (auto i : set) {
      os << i << " ";
    }
    os << "}";
    return os.str();
  }

private:
  std::set<Key> set{};
};

template <typename TestSet> void check() {
  // random sequence
  using key_type = typename TestSet::key_type;
  std_set<key_type> ref{};
  TestSet test{};

  std::mt19937 rg{};
  std::uniform_int_distribution<key_type> op_dist{INSERT, SEARCH};
  std::uniform_int_distribution<key_type> val_dist{
      // std::numeric_limits<key_type>::min(),
      // std::numeric_limits<key_type>::max()
      0, 200};
  std::vector<int> set_val{};

  for (int i = 0; i < 4000; i++) {
    Operation op = static_cast<Operation>(op_dist(rg));
    std::string before = test.tree_view();
    LOG("before:\n%s\n", before.c_str());
    switch (op) {
    case INSERT: {
      key_type val = val_dist(rg);
      LOG("insert [ %s ]\n", std::to_string(val).c_str());
      if (!ref.search(val)) {
        set_val.push_back(val);
      }
      ref.insert(val);
      test.insert(val);
      if (ref.to_string() != test.to_string()) {
        std::cerr << "failed on insert " << val << std::endl;
        std::cerr << "ref: " << ref.to_string() << std::endl;
        std::cerr << "test: " << test.to_string() << std::endl;
        std::cerr << "tree_view before: \n" << before << std::endl;
        std::cerr << "tree_view after: \n" << test.tree_view() << std::endl;
        exit(-1);
      }
      break;
    }
    case SEARCH: {
      key_type val = val_dist(rg);
      LOG("search [ %s ]\n", std::to_string(val).c_str());
      bool ref_res = ref.search(val);
      bool test_res = test.search(val);
      if (ref_res != test_res) {
        std::cerr << "failed on search " << val << std::endl;
        std::cerr << "ref: " << ref.to_string() << std::endl;
        std::cerr << "test: " << test.to_string() << std::endl;
        std::cerr << "tree_view before: \n" << before << std::endl;
        std::cerr << "tree_view after: \n" << test.tree_view() << std::endl;
        exit(-1);
      }
      break;
    }
    case REMOVE: {
      key_type remove_val{};
      if (set_val.empty()) {
        remove_val = val_dist(rg);
      } else {
        std::uniform_int_distribution<key_type> remove_val_dist{
            static_cast<key_type>(0),
            static_cast<key_type>(set_val.size() - 1)};
        key_type remove_val = set_val[remove_val_dist(rg)];
        auto iter = std::find(set_val.begin(), set_val.end(), remove_val);
        set_val.erase(iter);
      }

      LOG("remove [ %s ]\n", std::to_string(remove_val).c_str());
      ref.remove(remove_val);
      test.remove(remove_val);

      if (ref.to_string() != test.to_string()) {
        std::cerr << "failed on remove " << remove_val << std::endl;
        std::cerr << "ref: " << ref.to_string() << std::endl;
        std::cerr << "test: " << test.to_string() << std::endl;
        std::cerr << "tree_view before: \n" << before << std::endl;
        std::cerr << "tree_view after: \n" << test.tree_view() << std::endl;
        exit(-1);
      }
      break;
    }
    }
    // assert(test.check());
  }
}

int main() {
  using std::cout, std::endl;
  
  check<b_set<int>>();
  assert(b_set<int>::new_count == 0);
  cout << "b-tree pass" << endl;

  check<b_plus_set<int>>();
  assert(b_plus_set<int>::new_count == 0);
  cout << "b+tree pass" << endl;

  check<tree_23_set<int>>();
  assert(tree_23_set<int>::tree_node::new_count == 0);
  cout << "23 tree pass" << endl;

  check<bst_set<int>>();
  cout << "bst pass" << endl;

  check<avl_set<int>>();
  cout << "avl tree pass" << endl;

  check<rb_set<int>>();
  cout << "rb tree pass" << endl;

  return 0;
}