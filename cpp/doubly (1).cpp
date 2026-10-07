#include "doubly.h"

void clearDoublyList(DoublyList& list) {
    while (list.head) {
        DoubleNode* removed = list.head;
        list.head = list.head->next;
        delete removed;
    }
    list.tail = nullptr;
    list.size = 0;
}

DoubleNode* doublyNodeAt(const DoublyList& list, int index) {
    DoubleNode* current = list.head;
    for (int i = 0; i < index; ++i) current = current->next;
    return current;
}

void doublyInsert(DoublyList& list, int index, const std::string& value) {
    DoubleNode* added = new DoubleNode{value};
    DoubleNode* before = index == 0 ? nullptr :
        (index == list.size ? list.tail : doublyNodeAt(list, index - 1));
    added->next = before ? before->next : list.head;
    added->prev = before;
    if (added->next) added->next->prev = added;
    if (before) before->next = added;
    else list.head = added;
    if (!added->next) list.tail = added;
    ++list.size;
}

void doublyErase(DoublyList& list, int index) {
    DoubleNode* removed = index == list.size - 1 ? list.tail : doublyNodeAt(list, index);
    DoubleNode* before = removed->prev;
    if (removed->next) removed->next->prev = before;
    if (before) before->next = removed->next;
    else list.head = removed->next;
    if (removed == list.tail) list.tail = before;
    delete removed;
    --list.size;
}

int doublyFind(const DoublyList& list, const std::string& value) {
    int index = 0;
    for (DoubleNode* current = list.head; current; current = current->next) {
        if (current->value == value) return index;
        ++index;
    }
    return -1;
}

std::vector<std::string> doublyValues(const DoublyList& list, bool reverse) {
    std::vector<std::string> result;
    DoubleNode* current = reverse ? list.tail : list.head;
    while (current) {
        result.push_back(current->value);
        current = reverse ? current->prev : current->next;
    }
    return result;
}


// Чтение значения узла по индексу.
std::string doublyGet(const DoublyList& list, int index) {
    return doublyNodeAt(list, index)->value;
}
