#pragma once
#include <vector>
#include <string>

struct SingleNode {
    std::string value;
    SingleNode* next = nullptr;
};

struct SinglyList {
    SingleNode* head = nullptr;
    SingleNode* tail = nullptr;
    int size = 0;
};

void singlyInsert(SinglyList& list, int index, const std::string& value);
void singlyErase(SinglyList& list, int index);
int singlyFind(const SinglyList& list, const std::string& value);
std::string singlyGet(const SinglyList& list, int index);
std::vector<std::string> singlyValues(const SinglyList& list, bool reverse = false);
void clearSinglyList(SinglyList& list);
