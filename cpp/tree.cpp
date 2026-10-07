#pragma once
#include <string>
#include <vector>

struct StackNode {
    std::string value;
    StackNode* next = nullptr;
};

struct Stack {
    StackNode* top = nullptr;
    int size = 0;
};

void stackPush(Stack& stack, const std::string& value);
void stackPop(Stack& stack);
std::string stackGet(const Stack& stack);
std::vector<std::string> stackValues(const Stack& stack);
void clearStack(Stack& stack);
