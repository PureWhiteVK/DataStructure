#include "config.hpp"
#include <array>
#include <functional>
#include <iostream>
#include <queue>
#include <sstream>

template <typename Key, int MAX_NODE_COUNT = 5> class b_set {
public:
  // follow knuth b-tree definition
  static constexpr int MIN_KEY_COUNT = (MAX_NODE_COUNT + 1) / 2 - 1;
  static constexpr int MAX_KEY_COUNT = MAX_NODE_COUNT - 1;

  static inline int new_count = 0;

  static void new_callback() {
    LOG("new node\n");
    ++new_count;
  }

  static void delete_callback() {
    --new_count;
    LOG("delete node %s\n", std::to_string(new_count).c_str());
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

    bool valid() {
      // 检查当前节点的链接信息是否正确
      bool res = true;
      if (parent) {
        auto iter = std::find(parent->nodes.begin(), parent->nodes.end(), this);
        res &= (iter != parent->nodes.end());
      } else {
        res &= !static_cast<bool>(prev());
        res &= !static_cast<bool>(next());
      }
      // 然后检查子节点是否正确
      int count = 0;
      for (int i = 0; i < node_count; i++) {
        res &= nodes[i]->valid();
        count++;
      }
      res &= (count == node_count);
      if (!is_leaf()) {
        res &= (node_count == key_count + 1);
      }
      return res;
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
      for (int i = MIN_KEY_COUNT + 1; i < MAX_KEY_COUNT; i++) {
        right->push_back_key(keys[i]);
      }
      right->push_back_key(last_key);
      key_count = MIN_KEY_COUNT;

      if (!is_leaf()) {
        for (int i = MIN_KEY_COUNT + 1; i < MAX_NODE_COUNT; i++) {
          right->push_back_node(nodes[i]);
        }
        right->push_back_node(last_node);
        node_count = MIN_KEY_COUNT + 1;
      }

      assert(valid() && right->valid());
      return {split_key, right};
    }
  };

public:
  using key_type = Key;

  b_set() = default;

  ~b_set() { destroy_tree(root); }

  bool search(const Key &key) {
    return static_cast<bool>(search_tree(root, key));
  }

  void insert(const Key &key) { root = insert_tree(root, key); }

  void remove(const Key &key) { root = remove_tree(root, key); }

  std::string to_string() const {
    std::stringstream os;
    os << "{ ";
    inorder_tree(root,
                 [&](const Key &key) { os << std::to_string(key) << " "; });
    os << "}";
    return os.str();
  }

  std::string tree_view() const {
    std::stringstream os;
    preorder_tree_pretty(root, 0, os);
    return os.str();
  }

private:
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
    inorder_tree(node->front_node(), func);
    for (int i = 0; i < node->key_count; i++) {
      func(node->keys[i]);
      inorder_tree(node->nodes[i + 1], func);
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

  static tree_node *search_tree(tree_node *root, const Key &key) {
    tree_node *curr = root;
    while (curr) {
      // 进行区间检索，找到 curr 在区间的位置
      if (curr->contains(key)) {
        LOG("found %s in %s\n", std::to_string(key).c_str(),
            to_string(curr).c_str());
        break;
      }
      if (curr->is_leaf()) {
        return nullptr;
      }
      int pos = curr->range_search(key);
      LOG("pos = %d,search %s in node %s is %s\n", pos,
          std::to_string(key).c_str(), to_string(curr).c_str(),
          to_string(curr->nodes[pos]).c_str());
      curr = curr->nodes[pos];
    }
    return curr;
  }

  static tree_node *range_search_tree(tree_node *root, const Key &key) {
    tree_node *curr = root;
    while (true) {
      // 进行区间检索，找到 curr 在区间的位置
      if (curr->contains(key) || curr->is_leaf()) {
        LOG("found %s in %s\n", std::to_string(key).c_str(),
            to_string(curr).c_str());
        break;
      }
      int pos = curr->range_search(key);
      LOG("pos = %d,search %s in node %s is %s\n", pos,
          std::to_string(key).c_str(), to_string(curr).c_str(),
          to_string(curr->nodes[pos]).c_str());
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
      return root;
    }
    Key split_key = key;
    tree_node *right{};
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
      // 进行 split
      std::tie(split_key, right) = node->split(last_key.value(), last_node);
      LOG("left: %s, right: %s, split_key: %s\n", to_string(node).c_str(),
          to_string(right).c_str(), std::to_string(split_key).c_str());
      if (node->is_root()) {
        // split root
        root = new tree_node{};
        root->push_back_key(split_key);
        root->push_back_node(node);
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
    LOG("found %s in %s\n", std::to_string(key).c_str(),
        to_string(node).c_str());
    // key 在 keys 中的位置
    int pos = node->position(key);
    if (!node->is_leaf()) {
      // 找到 prev 节点，（找 prev
      // 是因为其前驱一定是叶子节点的最后一个节点，如果是后继的话一定是叶子节点的起始节点）
      LOG("pos of key %s in node %s is %s\n", std::to_string(key).c_str(),
          to_string(node).c_str(), std::to_string(pos).c_str());
      tree_node *prev = node->nodes[pos];
      while (!prev->is_leaf()) {
        prev = prev->back_node();
      }
      LOG("predecessor of key %s is %s\n", std::to_string(key).c_str(),
          to_string(prev).c_str());
      // 最后一个元素
      std::swap(node->keys[pos], prev->back_key());
      node = prev;
      pos = node->key_count - 1;
    }

    // 此时一定是叶子节点情况了，有三种情况
    LOG("remove key %s in leaf node %s\n", std::to_string(key).c_str(),
        to_string(node).c_str());
    // 当前叶子节点可以直接删除，对于根节点不限制节点数目

    node->remove_key(pos);
    node->remove_node(pos);
    // 先删除
    if (node->key_count > MIN_KEY_COUNT || node->is_root()) {
      if (node->is_root() && node->key_count == 0) {
        root = nullptr;
        delete node;
      }
      return root;
    }
    // 当前叶子删除会导致值的数目太少，有两种情况，且需要递归进行调整
    while (node->key_count < MIN_KEY_COUNT && !node->is_root()) {
      tree_node *parent = node->parent;
      tree_node *prev = node->prev();
      tree_node *next = node->next();
      // 需要得到 node 在父节点上的位置，然后找到其 prev 和 next
      if (prev && prev->key_count > MIN_KEY_COUNT) {
        // 从 prev 偷
        Key &parent_key = parent->keys[node->parent_pos - 1];
        LOG("steal from prev %s,%s\n", std::to_string(prev->back_key()).c_str(),
            std::to_string(parent_key).c_str());
        node->insert_key(0, parent_key);
        node->insert_node(0, prev->back_node());
        // 调整 parent node
        parent_key = prev->back_key();
        // 然后删除 sibling_key
        prev->pop_back_key();
        prev->pop_back_node();
        break;
      }
      if (next && next->key_count > MIN_KEY_COUNT) {
        // 从 next 偷
        Key &parent_key = parent->keys[node->parent_pos];
        LOG("steal from next %s,%s\n", std::to_string(parent_key).c_str(),
            std::to_string(next->front_key()).c_str());
        node->push_back_key(parent_key);
        node->push_back_node(next->front_node());

        // 调整 parent node
        parent_key = next->front_key();
        // 然后删除 sibling_key
        next->remove_key(0);
        next->remove_node(0);
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
      merge_node->push_back_key(parent_key);
      for (int i = 0; i < delete_node->key_count; i++) {
        merge_node->push_back_key(delete_node->keys[i]);
        merge_node->push_back_node(delete_node->nodes[i]);
      }
      // 还有最后一个节点
      merge_node->push_back_node(delete_node->back_node());
      // 删除
      parent->remove_key(parent_key_pos);
      parent->remove_node(delete_node->parent_pos);
      LOG("after remove, parent: %s\n", to_string(parent).c_str());
      LOG("merged node: %s\n", to_string(merge_node).c_str());
      delete delete_node;
      node = merge_node;
      if (parent->key_count == 0) {
        LOG("parent is empty!\n");
        LOG("node: %s\n", to_string(node).c_str());
        node->parent = parent->parent;
        node->parent_pos = parent->parent_pos;
        delete parent;
        if (node->parent) {
          node->parent->nodes[node->parent_pos] = node;
        } else {
          root = node;
          break;
        }
        parent = node;
      }
      // 进行下一步的删除
      node = parent;
    }
    return root;
  }

private:
  tree_node *root{};
};