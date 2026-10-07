#include "singly.h"

// Освобождение памяти всех узлов списка.
void clearSinglyList(SinglyList& list) {
    while (list.head) {
        SingleNode* removed = list.head;
        list.head = list.head->next;
        delete removed;
    }
    list.tail = nullptr;
    list.size = 0;
}

// Поиск узла по индексу.
SingleNode* singlyNodeAt(const SinglyList& list, int index) {
    SingleNode* current = list.head;
    for (int i = 0; i < index; ++i)
        current = current->next;
    return current;
}

// Добавление элемента по индексу.
void singlyInsert(SinglyList& list, int index,
                  const std::string& value) {
    SingleNode* added = new SingleNode{value};

    // Находим предыдущий узел.
    SingleNode* before = index == 0 ? nullptr :
        (index == list.size ? list.tail :
         singlyNodeAt(list, index - 1));

    added->next = before ? before->next : list.head;

    if (before)
        before->next = added;
    else
        list.head = added;

    if (!added->next)
        list.tail = added;

    ++list.size;
}

// Удаление элемента по индексу.
void singlyErase(SinglyList& list, int index) {
    SingleNode* before = index == 0 ? nullptr :
        singlyNodeAt(list, index - 1);

    SingleNode* removed = before ? before->next : list.head;

    if (before)
        before->next = removed->next;
    else
        list.head = removed->next;

    if (removed == list.tail)
        list.tail = before;

    delete removed;
    --list.size;
}

// Поиск первого совпадения: индекс или -1.
int singlyFind(const SinglyList& list,
               const std::string& value) {
    int index = 0;
    for (SingleNode* current = list.head;
         current; current = current->next) {
        if (current->value == value)
            return index;
        ++index;
    }
    return -1;
}

// Получение всех значений для вывода и сохранения.
std::vector<std::string> singlyValues(const SinglyList& list,
                                      bool reverse) {
    std::vector<std::string> result;

    for (SingleNode* current = list.head;
         current; current = current->next)
        result.push_back(current->value);

    if (reverse) {
        for (int i = 0; i < list.size / 2; ++i)
            std::swap(result[i], result[list.size - 1 - i]);
    }

    return result;
}

// Чтение значения по индексу.
std::string singlyGet(const SinglyList& list, int index) {
    return singlyNodeAt(list, index)->value;
}