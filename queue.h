#include "queue.h"

// Добавляем элемент в хвост.
void queuePush(Queue& queue, const std::string& value) {
    QueueNode* added = new QueueNode{value};
    if (queue.tail) queue.tail->next = added;
    else queue.head = added; // Для пустой очереди.
    queue.tail = added;
    ++queue.size;
}

// Удаляем элемент из головы.
void queuePop(Queue& queue) {
    QueueNode* removed = queue.head;
    queue.head = removed->next;
    if (!queue.head) queue.tail = nullptr;
    delete removed; // Освобождаем память.
    --queue.size;
}

// Читаем первый элемент без удаления.
std::string queueGet(const Queue& queue) {
    return queue.head->value;
}

// Собираем значения для вывода и сохранения.
std::vector<std::string> queueValues(const Queue& queue) {
    std::vector<std::string> result;
    for (QueueNode* current = queue.head; current; current = current->next)
        result.push_back(current->value);
    return result;
}

// Удаляем все узлы очереди.
void clearQueue(Queue& queue) {
    while (queue.head) queuePop(queue);
}