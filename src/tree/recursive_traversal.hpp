#pragma once
#include "binary_tree.hpp"
#include "general_tree.hpp"

#include <functional>

template <typename T>
void binary_inorder(TreeNode<T> *root,
                    const std::function<void(TreeNode<T> *)> &visit) {
  if (!root) {
    return;
  }
  binary_inorder(root->left, visit);
  visit(root);
  binary_inorder(root->right, visit);
}

template <typename T>
void binary_preorder(TreeNode<T> *root,
                     const std::function<void(TreeNode<T> *)> &visit) {
  if (!root) {
    return;
  }
  visit(root);
  binary_preorder(root->left, visit);
  binary_preorder(root->right, visit);
}

template <typename T>
void binary_postorder(TreeNode<T> *root,
                      const std::function<void(TreeNode<T> *)> &visit) {
  if (!root) {
    return;
  }
  binary_postorder(root->left, visit);
  binary_postorder(root->right, visit);
  visit(root);
}

template <typename T>
void general_preorder(GeneralTreeNode<T> *root,
                      const std::function<void(GeneralTreeNode<T> *)> &visit) {
  if (!root) {
    return;
  }
  visit(root);
  for (auto child : root->children) {
    general_preorder(child, visit);
  }
}

template <typename T>
void general_postorder(GeneralTreeNode<T> *root,
                       const std::function<void(GeneralTreeNode<T> *)> &visit) {
  if (!root) {
    return;
  }
  for (auto child : root->children) {
    general_postorder(child, visit);
  }
  visit(root);
}