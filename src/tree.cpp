#include <fmt/core.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <functional>
#include <limits>
#include <memory>
#include <queue>
#include <sstream>
#include <stack>
#include <vector>

struct TreeNode {
  int val{};
  TreeNode *left{};
  TreeNode *right{};
  TreeNode(int val, TreeNode *left, TreeNode *right)
      : val(val), left(left), right(right) {}
  TreeNode(int val) : val(val) {}
  TreeNode() = default;

  struct Deleter {
    using pointer = TreeNode *;
    void operator()(pointer ptr) {
      std::queue<pointer> q;
      q.emplace(ptr);
      int total = 0;
      while (!q.empty()) {
        pointer curr = q.front();
        q.pop();
        if (curr) {
          q.emplace(curr->left);
          q.emplace(curr->right);
          delete curr;
          total++;
        }
      }
      fmt::println("delete {} nodes", total);
    }
  };
};

using TreeHandle = std::unique_ptr<TreeNode, TreeNode::Deleter>;

void print_tree(TreeNode *root) {
  std::stack<std::tuple<TreeNode *, bool, int>> s;
  s.emplace(root, false, 0);
  while (!s.empty()) {
    auto [node, visited, depth] = s.top();
    s.pop();
    if (visited) {
      std::stringstream s;
      for (int i = 0; i < depth; i++) {
        s << '-';
      }
      if (!node) {
        fmt::println("{}null", s.str());
      } else {
        fmt::println("{}{}", s.str(), node->val);
      }
    } else {
      if (node && (node->left || node->right)) {
        s.emplace(node->right, false, depth + 1);
        s.emplace(node->left, false, depth + 1);
      }
      s.emplace(node, true, depth);
    }
  }
}

static constexpr int NULL_NODE_VALUE = -1;

TreeHandle build_tree(const std::vector<int> &arr) {
  if (arr.empty()) {
    return nullptr;
  }
  // arr 包含一个二叉树的序列化,对于空节点而言,其值为 INVALID_VALUE
  TreeNode *root = new TreeNode{arr[0]};
  std::queue<TreeNode *> s;
  s.emplace(root);
  int i = 1;
  for (; i < arr.size(); i++) {
    TreeNode *node{nullptr};
    if (arr[i] != NULL_NODE_VALUE) {
      node = new TreeNode{arr[i]};
    }
    if (i & 1) {
      // left child
      s.front()->left = node;
      if (node) {
        s.emplace(node);
      }
    } else {
      // right child
      s.front()->right = node;
      if (node) {
        s.emplace(node);
      }
      s.pop();
    }
  }
  return TreeHandle(root);
}

TreeHandle build_winner_tree(const std::vector<int> &arr) {
  // 自底向下构建 winner tree？
  std::queue<TreeNode *> q;
  for (int n : arr) {
    q.emplace(new TreeNode{n});
  }
  while (q.size() != 1) {
    int size = q.size();
    for (int i = 0; i + 2 <= size; i += 2) {
      TreeNode *left = q.front();
      q.pop();
      TreeNode *right = q.front();
      q.pop();
      TreeNode *winner =
          new TreeNode(std::min(left->val, right->val), left, right);
      q.emplace(winner);
    }
  }
  return TreeHandle(q.front());
}

void replacement_substitute_sort(const std::vector<int> &source,
                                 std::vector<std::vector<int>> &output,
                                 int capacity = 3) {
  struct PlayerRec {
    int value{};
    int seq_id{};
  };
  int n = capacity;
  int minimax = std::numeric_limits<int>::max();
  int curr = 0;
  int curr_seq_id = 0;
  std::vector<int> curr_output;
  std::vector<int> loser_tree(n);
  std::vector<PlayerRec> player(n);

  std::function<int(int)> parent = [&](int node) -> int { return node / 2; };

  std::function<void(int)> compete = [&](int player_id) {
    // 如果当前值比父节点记录的败者值大，说明当前值是胜者，可以参加下一轮竞赛
    int next_loser = parent(player_id + n);
    while (next_loser > 0) {
      // update loser
      int next_player_id = loser_tree[next_loser];
      const PlayerRec &player_rec = player[player_id];
      const PlayerRec &next_player_rec = player[next_player_id];
      if (player_rec.seq_id > next_player_rec.seq_id ||
          (player_rec.seq_id == next_player_rec.seq_id &&
           player_rec.value > next_player_rec.value)) {
        std::swap(loser_tree[next_loser], player_id);
      }
      next_loser = parent(next_loser);
    }
    // set final winner
    loser_tree[0] = player_id;
  };

  std::function<void(int)> load = [&](int player_id) {
    if (curr == source.size()) {
      player[player_id] = {std::numeric_limits<int>::max(), curr_seq_id};
      return;
    }
    int v = source[curr++];
    player[player_id] = {v, v > minimax ? curr_seq_id : curr_seq_id + 1};
  };

  // 遍历所有的叶子节点，更新比赛情况
  for (int i = n - 1; i >= 0; i--) {
    load(i);
    compete(i);
  }

  curr_seq_id++;
  while (player[loser_tree[0]].value != std::numeric_limits<int>::max()) {
    const PlayerRec &minimax_rec = player[loser_tree[0]];
    minimax = minimax_rec.value;
    if (minimax_rec.seq_id > curr_seq_id) {
      output.emplace_back(curr_output);
      curr_output.clear();
      curr_seq_id++;
    }
    curr_output.emplace_back(minimax_rec.value);
    load(loser_tree[0]);
    compete(loser_tree[0]);
  }
  output.emplace_back(curr_output);
}

void multi_way_merge_sort(const std::vector<std::vector<int>> &sources,
                          std::vector<int> &output) {
  int n = sources.size();
  // n for internal node, n for external node (leaf node) and 1 for judge
  std::vector<int> loser_tree(n);
  std::vector<int> player(n, std::numeric_limits<int>::min());
  std::vector<int> pos(sources.size(), 0);

  std::function<int(int)> parent = [&](int node) -> int { return node / 2; };

  std::function<void(int)> compete = [&](int player_id) {
    // 如果当前值比父节点记录的败者值大，说明当前值是胜者，可以参加下一轮竞赛
    int next_loser = parent(player_id + n);
    while (next_loser > 0) {
      // update loser
      if (player[player_id] > player[loser_tree[next_loser]]) {
        std::swap(loser_tree[next_loser], player_id);
      }
      next_loser = parent(next_loser);
    }
    // set final winner
    loser_tree[0] = player_id;
  };

  std::function<void(int)> load = [&](int player_id) {
    if (pos[player_id] == sources[player_id].size()) {
      player[player_id] = std::numeric_limits<int>::max();
      return;
    }
    player[player_id] = sources[player_id][pos[player_id]++];
  };

  // 遍历所有的叶子节点，更新比赛情况
  for (int i = n - 1; i >= 0; i--) {
    load(i);
    compete(i);
  }

  while (player[loser_tree[0]] != std::numeric_limits<int>::max()) {
    output.emplace_back(player[loser_tree[0]]);
    load(loser_tree[0]);
    compete(loser_tree[0]);
  }
}

int main() {
  // TreeHandle root = build_winner_tree({49, 38, 65, 97, 76});
  // print_tree(root.get());
  std::vector<int> res;
  // 要求 sources 必须都是有序的，因此需要内排序算法进行局部的排序
  multi_way_merge_sort(
      {{10, 15, 16}, {9, 18, 20}, {20, 22, 40}, {6, 15, 25}, {12, 37, 48}},
      res);
  fmt::println("multiway merge sort result: {}", res);
  res.clear();
  std::vector<std::vector<int>> sources;
  replacement_substitute_sort({51, 49, 39, 46, 38, 29, 14, 61, 15, 30, 1,  48,
                               52, 3,  63, 27, 4,  13, 89, 24, 46, 58, 33, 76},
                              sources, 6);
  // replacement_substitute_sort({17, 21, 5, 44, 10, 12, 56, 32, 29}, sources);
  fmt::println("sources: {}", sources);
  multi_way_merge_sort(sources, res);
  fmt::println("sorted: {}", res);
  return 0;
}