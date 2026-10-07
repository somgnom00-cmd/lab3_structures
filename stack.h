#include "stack.h"

// Добавляем новый узел на вершину стека.
void stackPush(Stack& stack, const std::string& value) {
    stack.top = new StackNode{value, stack.top};
    ++stack.size;
}

// Удаляем верхний узел. Проверка на пустоту выполняется в commands.cpp.
void stackPop(Stack& stack) {
    StackNode* removed = stack.top;
    stack.top = removed->next;
    delete removed;
    --stack.size;
}

std::string stackGet(const Stack& stack) {
    return stack.top->value;
}

// Значения идут от вершины к основанию.
std::vector<std::string> stackValues(const Stack& stack) {
    std::vector<std::string> result;
    for (StackNode* current = stack.top; current; current = current->next)
        result.push_back(current->value);
    return result;
}

void clearStack(Stack& stack) {
    while (stack.top) stackPop(stack);
}
