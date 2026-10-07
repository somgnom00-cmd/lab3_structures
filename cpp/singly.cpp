#pragma once
#include <string>
#include <vector>

struct QueueNode {
    std::string value;
    QueueNode* next = nullptr;
};

struct Queue {
    QueueNode* head = nullptr;
    QueueNode* tail = nullptr;
    int size = 0;
};

void queuePush(Queue& queue, const std::string& value);
void queuePop(Queue& queue);
std::string queueGet(const Queue& queue);
std::vector<std::string> queueValues(const Queue& queue);
void clearQueue(Queue& queue);
