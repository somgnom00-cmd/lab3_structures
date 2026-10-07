#include "array.h"

// ВСТАВКА: с автоматическим расширением памяти и сдвигом элементов
void arrayInsert(Array& array, int index, const std::string& value) {
    // 1. Если массив заполнен — выделяем новый блок памяти в 2 раза больше
    if (array.size == array.capacity) {
        array.capacity = array.capacity ? array.capacity * 2 : 4; // если был 0, берем 4, иначе удваиваем
        std::string* expanded = new std::string[array.capacity];
        for (int i = 0; i < array.size; ++i) expanded[i] = array.data[i]; // копируем старые данные
        delete[] array.data;                                        // освобождаем старый блок
        array.data = expanded;                                      // переключаем указатель
    }
    // 2. Сдвигаем все элементы после index на 1 ячейку вправо, освобождая место
    for (int i = array.size; i > index; --i) array.data[i] = array.data[i - 1];
    array.data[index] = value;
    ++array.size;
}

// УДАЛЕНИЕ: сдвигаем все элементы после index на 1 ячейку влево, закрывая дырку
void arrayErase(Array& array, int index) {
    for (int i = index; i + 1 < array.size; ++i) array.data[i] = array.data[i + 1];
    --array.size;
}

// ЭКСПОРТ: выгрузка данных в вектор для вывода PRINT и записи в файл
std::vector<std::string> arrayValues(const Array& array) {
    std::vector<std::string> result;
    for (int i = 0; i < array.size; ++i) result.push_back(array.data[i]);
    return result;
}

// Чтение и замена значения по индексу.
std::string arrayGet(const Array& array, int index) { return array.data[index]; }
void arraySet(Array& array, int index, const std::string& value) { array.data[index] = value; }

// Освобождение памяти вызывается явно при завершении программы.
void clearArray(Array& array) {
    delete[] array.data;
    array.data = nullptr;
    array.size = array.capacity = 0;
}
