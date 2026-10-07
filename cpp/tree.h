#include "tree.h"
#include <queue>

// РЕКУРСИВНОЕ УДАЛЕНИЕ: обход снизу вверх (post-order), чтобы не потерять детей
void destroyTreeNodes(TreeNode* node) {
    if (!node) return;
    destroyTreeNodes(node->left);
    destroyTreeNodes(node->right);
    delete node;
}

// обход по уровням через очередь std::queue до первого свободного места
void treeInsert(CompleteTree& tree, const std::string& value) {
    TreeNode* added = new TreeNode{value};
    if (!tree.root) { tree.root = added; return; } // дерево было пустым — узел стал корнем

    std::queue<TreeNode*> pending;
    pending.push(tree.root);

    // Достаем узлы по очереди слева направо и ищем свободное левое или правое место
    while (!pending.empty()) {
        TreeNode* current = pending.front();
        pending.pop();

        if (!current->left) { current->left = added; return; }   // нашли пустое слева!
        if (!current->right) { current->right = added; return; } // нашли пустое справа!

        pending.push(current->left);
        pending.push(current->right);
    }
}

// Поиск значения среди всех узлов
bool treeFind(const CompleteTree& tree, const std::string& value) {
    for (const std::string& stored : treeValues(tree)) if (stored == value) return true;
    return false;
}

// ПРОВЕРКА ПОЛНОТЫ: дерево Complete, если после первой дырки нет реальных узлов
bool treeComplete(const CompleteTree& tree) {
    std::queue<TreeNode*> pending;
    pending.push(tree.root);
    bool gap = false; // флаг обнаружения первого пустого места

    while (!pending.empty()) {
        TreeNode* current = pending.front();
        pending.pop();

        if (!current) {
            gap = true; // встретили пустоту
        } else {
            if (gap) return false; // встретили узел ПОСЛЕ дырки — дерево не Complete!
            pending.push(current->left);
            pending.push(current->right);
        }
    }
    return true;
}

// ОБХОД ПО УРОВНЯМ (BFS): собирает узлы по рядам для печати и сохранения в файл
std::vector<std::string> treeValues(const CompleteTree& tree) {
    std::vector<std::string> result;
    std::queue<TreeNode*> pending;
    if (tree.root) pending.push(tree.root);

    while (!pending.empty()) {
        TreeNode* current = pending.front();
        pending.pop();
        result.push_back(current->value);
        if (current->left) pending.push(current->left);
        if (current->right) pending.push(current->right);
    }
    return result;
}

// Освобождаем узлы дерева и сбрасываем корень.
void clearTree(CompleteTree& tree) {
    destroyTreeNodes(tree.root);
    tree.root = nullptr;
}
