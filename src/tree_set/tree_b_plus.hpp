#include "config.hpp"
#include <array>
#include <functional>
#include <iostream>
#include <queue>
#include <sstream>

template <typename Key, int N = 5> class b_plus_set {
public:
  static constexpr int MAX_NODE_COUNT = N;
  static constexpr int MIN_KEY_COUNT = (MAX_NODE_COUNT + 1) / 2 - 1;
  static constexpr int MAX_KEY_COUNT = MAX_NODE_COUNT - 1;

public:
  static inline int new_count = 0;

  static void new_callback() {
    LOG("new node\n");
    ++new_count;
  }

  static void delete_callback() {
    --new_count;
    LOG("delete node, new_count = %s\n", std::to_string(new_count).c_str());
  }

public:
  struct tree_node {
    int key_count{};
    int node_count{};
    // 多一个确保可以进行先插入再删除
    std::array<Key, MAX_KEY_COUNT> keys{};
    std::array<tree_node *, MAX_NODE_COUNT> nodes{};
    // 还需要存储一个当前 node 在 parent 中的位置信息
    tree_node *parent{};
    int parent_pos{};

    tree_node() { new_callback(); }

    ~tree_node() { delete_callback(); }

    bool is_leaf() const { return node_count == 0; }

    bool is_root() const { return !static_cast<bool>(parent); }

    bool full() const { return key_count > MAX_KEY_COUNT; }

    void check() {
      fflush(stdout);
      LOG("check curr node: %s parent node: %s \n", to_string(this).c_str(),
          to_string(parent).c_str());
      // 检查当前节点的链接信息是否正确
      if (parent) {
        assert(parent->nodes[parent_pos] == this);
      } else {
        assert(!prev() && !next());
      }
      // 然后检查子节点是否正确
      int count = 0;
      for (int i = 0; i < node_count; i++) {
        nodes[i]->check();
        count++;
      }
      assert(count == node_count);
      if (is_leaf()) {
        tree_node *prev = leaf_prev();
        tree_node *next = leaf_next();
        assert(!prev || prev->leaf_next() == this);
        assert(!next || next->leaf_prev() == this);
      } else {
        assert(node_count == key_count + 1);
        // 每一个 nodes 中的 key 都必须在其中存在
        for (int i = 0; i < key_count; i++) {
          LOG("search leaf value %s\n", std::to_string(keys[i]).c_str());
          assert(search_tree(nodes[i + 1], keys[i]));
        }
      }
    }

    bool contains(const Key &key) const {
      auto end = keys.begin() + key_count;
      auto iter = std::lower_bound(keys.begin(), end, key);
      return iter != end && *iter == key;
    }

    int position(const Key &key) const {
      auto end = keys.begin() + key_count;
      auto iter = std::lower_bound(keys.begin(), end, key);
      int pos = iter - keys.begin();
      assert(pos < key_count);
      return pos;
    }

    int range_search(const Key &key) const {
      auto end = keys.begin() + key_count;
      auto iter = std::lower_bound(keys.begin(), end, key);
      return iter == keys.end() ? (key < keys[0] ? 0 : key_count)
                                : iter - keys.begin();
    }

    tree_node *front_node() { return is_leaf() ? nullptr : nodes[0]; }

    tree_node *back_node() {
      return is_leaf() ? nullptr : nodes[node_count - 1];
    }

    tree_node *&leaf_prev() {
      assert(is_leaf());
      return nodes[0];
    }

    tree_node *&leaf_next() {
      assert(is_leaf());
      return nodes[1];
    }

    tree_node *prev() {
      return !parent || parent_pos == 0 ? nullptr
                                        : parent->nodes[parent_pos - 1];
    }

    tree_node *next() {
      return !parent || parent_pos == parent->node_count - 1
                 ? nullptr
                 : parent->nodes[parent_pos + 1];
    }

    Key &front_key() { return keys[0]; }
    Key &back_key() { return keys[key_count - 1]; }

    void move_keys_right(int start_pos) {
      for (int i = std::min(key_count - 1, MAX_KEY_COUNT - 2); i >= start_pos;
           i--) {
        // i + 1 可能溢出
        keys[i + 1] = std::move(keys[i]);
      }
    }

    void move_keys_left(int start_pos) {
      for (int i = start_pos + 1; i < key_count; i++) {
        keys[i - 1] = std::move(keys[i]);
      }
    }

    void move_nodes_right(int start_pos) {
      for (int i = std::min(key_count - 1, MAX_NODE_COUNT - 2); i >= start_pos;
           i--) {
        // i + 1可能溢出
        nodes[i + 1] = std::move(nodes[i]);
        nodes[i + 1]->parent_pos = i + 1;
      }
    }

    void move_nodes_left(int start_pos) {
      for (int i = start_pos + 1; i < node_count; i++) {
        nodes[i - 1] = std::move(nodes[i]);
        nodes[i - 1]->parent_pos = i - 1;
      }
    }

    // 此处需要返回额外多的那一个 key
    std::optional<Key> insert_key(int pos, const Key &key) {
      assert(!contains(key) && pos <= MAX_KEY_COUNT && pos >= 0 &&
             key_count <= MAX_KEY_COUNT);
      LOG("before insert key %s into %s\n", std::to_string(key).c_str(),
          to_string(this).c_str());
      std::optional<Key> last_key{};
      if (key_count == MAX_KEY_COUNT) {
        if (pos == MAX_KEY_COUNT) {
          last_key = key;
        } else {
          last_key = keys.back();
          move_keys_right(pos);
          keys[pos] = key;
        }
      } else {
        move_keys_right(pos);
        keys[pos] = key;
        key_count++;
      }
      LOG("after insert key %s into %s\n", std::to_string(key).c_str(),
          to_string(this).c_str());
      return last_key;
    }

    // 此处需要返回额外多的那一个 node
    tree_node *insert_node(int pos, tree_node *node) {
      if (!node) {
        return {};
      }
      assert(pos <= MAX_NODE_COUNT && pos >= 0 && node_count <= MAX_NODE_COUNT);
      tree_node *last_node{};
      node->parent = this;
      if (node_count == MAX_NODE_COUNT) {
        if (pos == MAX_NODE_COUNT) {
          last_node = node;
        } else {
          last_node = nodes.back();
          move_nodes_right(pos);
          nodes[pos] = node;
          node->parent_pos = pos;
        }
      } else {
        move_nodes_right(pos);
        nodes[pos] = node;
        node->parent_pos = pos;
        node_count++;
      }
      return last_node;
    }

    void remove_key(int pos) {
      assert(pos < key_count && pos >= 0);
      LOG("before remove key at pos %s from %s\n", std::to_string(pos).c_str(),
          to_string(this).c_str());
      move_keys_left(pos);
      key_count--;
      LOG("after remove key at pos %s from %s\n", std::to_string(pos).c_str(),
          to_string(this).c_str());
    }

    void remove_node(int pos) {
      if (is_leaf()) {
        return;
      }
      assert(pos < node_count && pos >= 0);
      move_nodes_left(pos);
      node_count--;
    }

    void push_back_key(const Key &key) {
      keys[key_count] = key;
      key_count++;
    }

    void push_back_node(tree_node *node) {
      if (!node) {
        return;
      }
      node->parent = this;
      node->parent_pos = node_count;
      nodes[node_count] = node;
      node_count++;
    }

    void pop_back_key() {
      assert(key_count > 0);
      key_count--;
    }

    void pop_back_node() {
      if (is_leaf()) {
        return;
      }
      assert(node_count > 0);
      node_count--;
    }

    std::tuple<Key, tree_node *> split(const Key &last_key,
                                       tree_node *last_node) {
      assert(key_count == MAX_KEY_COUNT);
      tree_node *right = new tree_node{};
      Key split_key = keys[MIN_KEY_COUNT];
      // copy right part to right node
      // 此处也会将根节点拷贝的节点中

      if (is_leaf()) {
        // 当我们需要划分 leaf node 的时候才需要拷贝根节点
        for (int i = MIN_KEY_COUNT; i < MAX_KEY_COUNT; i++) {
          right->push_back_key(keys[i]);
        }
        right->push_back_key(last_key);
        key_count = MIN_KEY_COUNT;
        // 将 node 一分为二
        // 直接更新链接信息
        tree_node *left = this;
        // nodes[0] 和 nodes[1] 记录前驱以及后继
        right->leaf_next() = left->leaf_next();
        left->leaf_next() = right;
        right->leaf_prev() = left;
        if (right->leaf_next()) {
          right->leaf_next()->leaf_prev() = right;
        }
      } else {
        for (int i = MIN_KEY_COUNT + 1; i < MAX_KEY_COUNT; i++) {
          right->push_back_key(keys[i]);
        }
        right->push_back_key(last_key);
        key_count = MIN_KEY_COUNT;
        for (int i = MIN_KEY_COUNT + 1; i < MAX_NODE_COUNT; i++) {
          right->push_back_node(nodes[i]);
        }
        right->push_back_node(last_node);
        node_count = MIN_KEY_COUNT + 1;
      }

      return {split_key, right};
    }
  };

public:
  using key_type = Key;

  b_plus_set() = default;

  ~b_plus_set() { destroy_tree(root); }

  bool search(const Key &key) {
    return static_cast<bool>(search_tree(root, key));
  }

  void insert(const Key &key) {
    root = insert_tree(root, key);
    left_most = left_most_tree(root);
    root->check();
  }

  void remove(const Key &key) {
    // TODO: B+ tree 的 remove 还有问题
    root = remove_tree(root, key);
    left_most = left_most_tree(root);
    if (root) {
      root->check();
    }
  }

  std::string to_string() const {
    std::stringstream os;
    os << "{ ";
    // 先跑一个 check circle
    bool contains_circle = false;
    tree_node *slow = left_most;
    tree_node *fast = left_most;
    while (true) {
      if (!fast || !fast->leaf_next()) {
        break;
      }
      fast = fast->leaf_next()->leaf_next();
      slow = slow->leaf_next();
      if (fast == slow) {
        contains_circle = true;
        break;
      }
    }

    assert(!contains_circle);

    for (tree_node *curr = left_most; curr != nullptr;
         curr = curr->leaf_next()) {
      for (int i = 0; i < curr->key_count; i++) {
        os << curr->keys[i] << " ";
      }
    }
    os << "}";
    return os.str();
  }

  std::string tree_view() const {
    std::stringstream os;
    preorder_tree_pretty(root, 0, os);
    return os.str();
  }

public:
  static std::string to_string(tree_node *node) {
    if (!node) {
      return "tree_node{null}";
    }
    std::stringstream os;
    os << "tree_node{"
       << "nodes_count=" << node->node_count << ",keys=";
    os << "[ ";
    for (int i = 0; i < node->key_count; i++) {
      os << node->keys[i] << " ";
    }
    os << "]}";
    return os.str();
  }

  static void inorder_tree(tree_node *node,
                           const std::function<void(const Key &)> &func) {
    if (!node) {
      return;
    }
    if (node->is_leaf()) {
      // 对于 B+ 树而言，只需要输出叶子节点的 key 即可
      for (int i = 0; i < node->key_count; i++) {
        func(node->keys[i]);
      }
    } else {
      inorder_tree(node->front_node(), func);
      for (int i = 0; i < node->key_count; i++) {
        inorder_tree(node->nodes[i + 1], func);
      }
    }
  }

  static void preorder_tree_pretty(tree_node *node, int depth,
                                   std::stringstream &os) {
    for (int i = 0; i < depth; i++) {
      os << " ";
    }
    os << to_string(node) << '\n';
    if (!node || node->is_leaf()) {
      return;
    }
    for (int i = 0; i < node->node_count; i++) {
      preorder_tree_pretty(node->nodes[i], depth + 1, os);
    }
  }

  static void destroy_tree(tree_node *root) {
    std::queue<tree_node *> q{};
    q.push(root);
    while (!q.empty()) {
      tree_node *curr = q.front();
      q.pop();
      if (!curr) {
        continue;
      }
      for (int i = 0; i < curr->node_count; i++) {
        q.push(curr->nodes[i]);
      }
      LOG("delete %s\n", to_string(curr).c_str());
      delete curr;
    }
  }

  // 找到 key 在 root 中第一次出现的位置
  static tree_node *search_tree(tree_node *root, const Key &key) {
    tree_node *curr = root;
    while (curr) {
      if (curr->contains(key)) {
        LOG("found %s in %s\n", std::to_string(key).c_str(),
            to_string(curr).c_str());
        break;
      }
      curr = curr->is_leaf() ? nullptr : curr->nodes[curr->range_search(key)];
    }
    return curr;
  }

  static tree_node *range_search_tree(tree_node *root, const Key &key) {
    tree_node *curr = root;
    while (true) {
      // 进行区间检索，找到 curr 在区间的位置
      if (curr->contains(key) || curr->is_leaf()) {
        LOG("will insert %s in %s\n", std::to_string(key).c_str(),
            to_string(curr).c_str());
        break;
      }
      int pos = curr->range_search(key);
      curr = curr->nodes[pos];
    }
    return curr;
  }

  static tree_node *insert_tree(tree_node *root, const Key &key) {
    if (!root) {
      tree_node *node = new tree_node{};
      node->push_back_key(key);
      return node;
    }
    tree_node *node = range_search_tree(root, key);
    if (node->contains(key)) {
      LOG("%s already exists!\n", std::to_string(key).c_str());
      return root;
    }
    Key split_key = key;
    tree_node *left{}, *right{};
    while (true) {
      int pos = node->range_search(split_key);
      // 执行插入，然后返回
      LOG("insert %s in pos %s of %s\n", std::to_string(split_key).c_str(),
          std::to_string(pos).c_str(), to_string(node).c_str());
      std::optional<Key> last_key = node->insert_key(pos, split_key);
      tree_node *last_node = node->insert_node(pos + 1, right);
      if (!last_key.has_value()) {
        break;
      }
      left = node;
      std::tie(split_key, right) = left->split(last_key.value(), last_node);
      LOG("left: %s, right: %s, split_key: %s\n", to_string(left).c_str(),
          to_string(right).c_str(), std::to_string(split_key).c_str());
      if (node->is_root()) {
        root = new tree_node{};
        root->push_back_node(left);
        root->push_back_key(split_key);
        root->push_back_node(right);
        break;
      }
      node = node->parent;
    }
    // 进行插入
    return root;
  }

  static tree_node *remove_tree(tree_node *root, const Key &key) {
    // 首先找到 key 的位置罗
    LOG("remove %s\n", std::to_string(key).c_str());
    tree_node *node = search_tree(root, key);
    if (!node) {
      return root;
    }
    int pos = node->position(key);
    if (!node->is_leaf()) {
      tree_node *internal_node = node;
      int internal_pos = pos;
      node = search_tree(internal_node->nodes[internal_pos + 1], key);
      pos = node->position(key);
      assert(node);
      // update internal node to it's inorder successor
      if (pos == node->key_count - 1 && node->leaf_next()) {
        internal_node->keys[internal_pos] = node->leaf_next()->front_key();
      } else if (pos < node->key_count - 1) {
        internal_node->keys[internal_pos] = node->keys[pos + 1];
      }
      LOG("update internal node to %s\n", to_string(internal_node).c_str());
    }

    // 此时一定是叶子节点情况了，有三种情况
    LOG("remove key %s in leaf node %s\n", std::to_string(key).c_str(),
        to_string(node).c_str());

    node->remove_key(pos);
    // 叶子节点不为空，就尽量不要调整索引部分
    if (node->key_count >= MIN_KEY_COUNT || node->is_root()) {
      if (node->is_root() && node->key_count == 0) {
        root = nullptr;
        delete node;
      }
      return root;
    }

    while (node->key_count < MIN_KEY_COUNT && !node->is_root()) {
      tree_node *parent = node->parent;
      // 对于叶子节点而言，prev 结果可能不对，需要单独判断一下？
      tree_node *prev = node->prev();
      tree_node *next = node->next();
      // 需要得到 node 在父节点上的位置，然后找到其 prev 和 next
      // 更新对应节点处的 key 值为 steal 过来的值

      if (prev && prev->key_count > MIN_KEY_COUNT) {
        Key &parent_key = parent->keys[node->parent_pos - 1];
        LOG("steal from prev, prev key = %s, parent key = %s\n",
            std::to_string(prev->back_key()).c_str(),
            std::to_string(parent_key).c_str());
        if (node->is_leaf()) {
          // 需要同步更新 internal node 的值
          node->insert_key(0, prev->back_key());
          parent_key = prev->back_key();
          prev->pop_back_key();
        } else {
          node->insert_key(0, parent_key);
          node->insert_node(0, prev->back_node());
          // 调整 parent node
          parent_key = prev->back_key();
          // 然后删除 sibling_key
          prev->pop_back_key();
          prev->pop_back_node();
        }
        break;
      }
      if (next && next->key_count > MIN_KEY_COUNT) {
        Key &parent_key = parent->keys[node->parent_pos];
        LOG("steal from next, parent key = %s, next key = %s\n",
            std::to_string(parent_key).c_str(),
            std::to_string(next->front_key()).c_str());
        if (node->is_leaf()) {
          node->push_back_key(next->front_key());
          next->remove_key(0);
          parent_key = next->front_key();
        } else {
          node->push_back_key(parent_key);
          node->push_back_node(next->front_node());
          // 调整 parent node
          parent_key = next->front_key();
          // 然后删除 sibling_key
          next->remove_key(0);
          next->remove_node(0);
        }
        break;
      }
      // 没得偷了，合并吧，这里合并也有方向的
      tree_node *merge_node = prev ? prev : node;
      tree_node *delete_node = prev ? node : next;
      int parent_key_pos = prev ? node->parent_pos - 1 : node->parent_pos;
      // 优先 prev or next ？
      Key parent_key = parent->keys[parent_key_pos];
      LOG("merge %s, %s and %s\n", to_string(merge_node).c_str(),
          std::to_string(parent_key).c_str(), to_string(delete_node).c_str());
      if (merge_node->is_leaf()) {
        for (int i = 0; i < delete_node->key_count; i++) {
          merge_node->push_back_key(delete_node->keys[i]);
        }
        // 此处需要调整 leaf_next 和 leaf_prev
        merge_node->leaf_next() = delete_node->leaf_next();
        if (delete_node->leaf_next()) {
          delete_node->leaf_next()->leaf_prev() = merge_node;
        }
      } else {
        merge_node->push_back_key(parent_key);
        for (int i = 0; i < delete_node->key_count; i++) {
          merge_node->push_back_key(delete_node->keys[i]);
          merge_node->push_back_node(delete_node->nodes[i]);
        }
        // 还有最后一个节点
        merge_node->push_back_node(delete_node->back_node());
      }
      // 删除
      parent->remove_key(parent_key_pos);
      parent->remove_node(delete_node->parent_pos);
      LOG("after remove, parent: %s\n", to_string(parent).c_str());
      LOG("merged node: %s\n", to_string(merge_node).c_str());
      delete delete_node;
      node = parent;
    }
    if (root->key_count == 0) {
      LOG("shrink tree\n");
      assert(root->node_count == 1);
      tree_node *delete_node = root;
      root = root->nodes[0];
      delete delete_node;
      root->parent = nullptr;
      root->parent_pos = 0;
    }
    return root;
  }

  static tree_node *left_most_tree(tree_node *root) {
    if (!root) {
      return root;
    }
    tree_node *curr = root;
    while (curr->front_node()) {
      curr = curr->front_node();
    }
    return curr;
  }

private:
  tree_node *root{};

  tree_node *left_most{};
};

/*
Case 1:
  The key to be deleted is present only at the leaf node
  not in the indexes (or internal nodes).
*/

inline void delete_case1_1() {
  // There is more than the minimum number of keys in the node.
  // Simply delete the key.
  using set = b_plus_set<int, 3>;
  using tree_node = set::tree_node;
  using std::cout, std::endl;
  std::vector<int> insert_seq{5, 15, 25, 35, 45, 55, 20, 30, 40};
  set s{};
  for (int i : insert_seq) {
    s.insert(i);
  }
  cout << s.to_string() << endl;
  cout << s.tree_view() << endl;

  s.remove(40);

  cout << s.to_string() << endl;
  cout << s.tree_view() << endl;
}

inline void delete_case1_2() {
  // There is exact minimum number of keys in the node.
  // Delete the key and borrow a key from the immediate sibling.
  // Add the median key of the sibling node to the parent
  using set = b_plus_set<int, 3>;
  using tree_node = set::tree_node;
  using std::cout, std::endl;
  std::vector<int> insert_seq{5, 15, 25, 35, 45, 55, 20, 30, 40};
  set s{};
  for (int i : insert_seq) {
    s.insert(i);
  }
  cout << s.to_string() << endl;
  cout << s.tree_view() << endl;

  s.remove(40);
  s.remove(5);

  cout << s.to_string() << endl;
  cout << s.tree_view() << endl;
}

/*
Case 2:
  The key to be deleted is present in the internal nodes as well.
  Then we have to remove them from the internal nodes as well.
*/

inline void delete_case2_1() {
  // if there is more than the minimum number of keys in the node,
  // simply delete the key from the leaf node and
  // delete the key from the internal node as well
  // Fill the empty space in the internal node with the inorder successor
  using set = b_plus_set<int, 3>;
  using tree_node = set::tree_node;
  using std::cout, std::endl;
  std::vector<int> insert_seq{5, 15, 25, 35, 45, 55, 20, 30, 40};
  set s{};
  for (int i : insert_seq) {
    s.insert(i);
  }
  s.remove(40);
  s.remove(5);
  s.remove(45);
  cout << s.to_string() << endl;
  cout << s.tree_view() << endl;
}

inline void delete_case2_2() {
  // if there is an exact minimum number of keys in the node,
  // then delete the key and borrow a key from its immediate sibling (through
  // the parent) Fill the empty space created in the index (internal node) with
  // the borrowed key.
  using set = b_plus_set<int, 3>;
  using tree_node = set::tree_node;
  using std::cout, std::endl;
  std::vector<int> insert_seq{5, 15, 25, 35, 45, 55, 20, 30, 40};
  set s{};
  for (int i : insert_seq) {
    s.insert(i);
  }
  s.remove(40);
  s.remove(5);
  s.remove(45);
  s.remove(35);
  cout << s.to_string() << endl;
  cout << s.tree_view() << endl;
}

inline void delete_case2_3() {
  using set = b_plus_set<int, 3>;
  using tree_node = set::tree_node;
  using std::cout, std::endl;
  std::vector<int> insert_seq{5, 15, 25, 35, 45, 55, 20, 30, 40};
  set s{};
  for (int i : insert_seq) {
    s.insert(i);
  }
  s.remove(40);
  s.remove(5);
  s.remove(45);
  s.remove(35);
  cout << s.to_string() << endl;
  cout << s.tree_view() << endl;
  s.remove(25);
  cout << s.to_string() << endl;
  cout << s.tree_view() << endl;
}

inline void delete_case3() {
  using set = b_plus_set<int, 3>;
  using tree_node = set::tree_node;
  using std::cout, std::endl;
  std::vector<int> insert_seq{5, 15, 25, 35, 45, 55, 20, 30, 40};
  set s{};
  for (int i : insert_seq) {
    s.insert(i);
  }
  s.remove(40);
  s.remove(5);
  s.remove(45);
  s.remove(35);
  s.remove(25);
  cout << s.to_string() << endl;
  cout << s.tree_view() << endl;
  s.remove(55);
  cout << s.to_string() << endl;
  cout << s.tree_view() << endl;
}