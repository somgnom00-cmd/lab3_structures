#pragma once
#include <vector>
#include <string>

// Структура хранит данные, отдельные функции выполняют операции.
struct Array {
    std::string* data = nullptr;
    int capacity = 0;
    int size = 0;
};

void arrayInsert(Array& array, int index, const std::string& value);
void arrayErase(Array& array, int index);
std::string arrayGet(const Array& array, int index);
void arraySet(Array& array, int index, const std::string& value);
std::vector<std::string> arrayValues(const Array& array);
void clearArray(Array& array);
