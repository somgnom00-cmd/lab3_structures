#pragma once
#include <vector>
#include <string>

struct DoubleNode {
    std::string value;
    DoubleNode* next = nullptr;
    DoubleNode* prev = nullptr;
};

struct DoublyList {
    DoubleNode* head = nullptr;
    DoubleNode* tail = nullptr;
    int size = 0;
};

void doublyInsert(DoublyList& list, int index, const std::string& value);
void doublyErase(DoublyList& list, int index);
int doublyFind(const DoublyList& list, const std::string& value);
std::string doublyGet(const DoublyList& list, int index);
std::vector<std::string> doublyValues(const DoublyList& list, bool reverse = false);
void clearDoublyList(DoublyList& list);
