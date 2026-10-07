#include "database.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <limits>
#include <cstdio>
#include <stdexcept>
#include <iomanip>
#include <cctype>

// Кавычки позволяют прочитать текст с пробелами как одно значение.
bool readToken(std::istream& input, std::string& token) {
    input >> std::ws;
    if (input.eof()) return false;
    if (!(input >> std::quoted(token)))
        throw std::runtime_error("незакрытые кавычки");
    int next = input.peek();
    if (next != std::char_traits<char>::eof() && !std::isspace(static_cast<unsigned char>(next)))
        throw std::runtime_error("после значения нужен пробел");
    return true;
}

// Простые значения пишем как раньше, текст с пробелами — в кавычках.
void writeValue(std::ostream& output, const std::string& value) {
    if (value.empty() || value.find_first_of(" \t\r\n\"\\") != std::string::npos)
        output << std::quoted(value);
    else output << value;
}

// ВАЛИДАЦИЯ ИНДЕКСА: преобразует строку в int с проверкой на буквы и переполнение
int number(const std::string& token) {
    std::size_t used = 0;
    long long value;
    try { value = std::stoll(token, &used); }
    catch (...) { throw std::runtime_error("требуется целое число: " + token); }

    // Проверка, что вся строка была числом и значение влезает в стандартный int (32 бита)
    if (used != token.size() || value < std::numeric_limits<int>::min()
                             || value > std::numeric_limits<int>::max())
        throw std::runtime_error("требуется целое число int: " + token);
    return static_cast<int>(value);
}

// ВАЛИДАЦИЯ ИМЕНИ: разрешает только латиницу (a-z, A-Z), цифры (0-9) и символ '_'
void checkName(const std::string& name) {
    if (name.empty()) throw std::runtime_error("не указано имя структуры");
    for (char symbol : name)
        if (!(symbol >= 'a' && symbol <= 'z') &&
            !(symbol >= 'A' && symbol <= 'Z') &&
            !(symbol >= '0' && symbol <= '9') && symbol != '_')
            throw std::runtime_error("имя: латинские буквы, цифры и _");
}

// ПРОВЕРКА СУЩЕСТВОВАНИЯ: ищет имя в соответствующем std::map по букве типа
bool dbExists(Database& database, char type, const std::string& name) {
    if (type == 'M') return database.arrays.count(name);
    if (type == 'L') return database.doubles.count(name);
    if (type == 'T') return database.trees.count(name);
    if (type == 'S') return database.stacks.count(name);
    if (type == 'Q') return database.queues.count(name);
    return database.lists.count(name);
}

// СОЗДАНИЕ СТРУКТУРЫ: try_emplace создает пустой объект в map, если ключа еще нет
void dbCreate(Database& database, char type, const std::string& name) {
    if (type == 'M') database.arrays.try_emplace(name);
    else if (type == 'L') database.doubles.try_emplace(name);
    else if (type == 'T') database.trees.try_emplace(name);
    else if (type == 'S') database.stacks.try_emplace(name);
    else if (type == 'Q') database.queues.try_emplace(name);
    else database.lists.try_emplace(name);
}

// Получение количества элементов структуры по ее имени
int dbSize(Database& database, char type, const std::string& name) {
    if (type == 'M') return database.arrays.at(name).size;
    if (type == 'L') return database.doubles.at(name).size;
    if (type == 'S') return database.stacks.at(name).size;
    if (type == 'Q') return database.queues.at(name).size;
    return database.lists.at(name).size;
}

// Сбор всех значений структуры в вектор (для вывода PRINT и сохранения в файл)
std::vector<std::string> dbValues(Database& database, char type, const std::string& name,
                                bool reverse) {
    if (type == 'M') return arrayValues(database.arrays.at(name));
    if (type == 'L') return doublyValues(database.doubles.at(name), reverse);
    if (type == 'T') return treeValues(database.trees.at(name));
    if (type == 'S') return stackValues(database.stacks.at(name));
    if (type == 'Q') return queueValues(database.queues.at(name));
    return singlyValues(database.lists.at(name), reverse);
}

// МАРШРУТИЗАЦИЯ ВСТАВКИ: перенаправляет вызов в отдельную функцию конкретной структуры данных
void dbInsert(Database& database, char type, const std::string& name, int index, const std::string& value) {
    if (type == 'M') arrayInsert(database.arrays.at(name), index, value);
    else if (type == 'L') doublyInsert(database.doubles.at(name), index, value);
    else if (type == 'T') treeInsert(database.trees.at(name), value); // у дерева вставка без индекса (BFS)
    else if (type == 'S') stackPush(database.stacks.at(name), value);
    else if (type == 'Q') queuePush(database.queues.at(name), value);
    else singlyInsert(database.lists.at(name), index, value);
}

// МАРШРУТИЗАЦИЯ УДАЛЕНИЯ: вызывает удаление по индексу из массива или списка
void dbErase(Database& database, char type, const std::string& name, int index) {
    if (type == 'M') arrayErase(database.arrays.at(name), index);
    else if (type == 'L') doublyErase(database.doubles.at(name), index);
    else if (type == 'S') stackPop(database.stacks.at(name));
    else if (type == 'Q') queuePop(database.queues.at(name));
    else singlyErase(database.lists.at(name), index);
}

// Получение значения элемента по индексу
std::string dbGet(Database& database, char type, const std::string& name, int index) {
    if (type == 'M') return arrayGet(database.arrays.at(name), index);
    if (type == 'L') return doublyGet(database.doubles.at(name), index);
    if (type == 'S') return stackGet(database.stacks.at(name));
    if (type == 'Q') return queueGet(database.queues.at(name));
    return singlyGet(database.lists.at(name), index);
}

// Замена значения по индексу (только для массива: команда MSET)
void dbSet(Database& database, const std::string& name, int index, const std::string& value) {
    arraySet(database.arrays.at(name), index, value);
}

// Поиск значения: возвращает индекс (для списков) или 0/-1 (для дерева)
int dbFind(Database& database, char type, const std::string& name, const std::string& value) {
    if (type == 'L') return doublyFind(database.doubles.at(name), value);
    if (type == 'F') return singlyFind(database.lists.at(name), value);
    return treeFind(database.trees.at(name), value) ? 0 : -1;
}

// Проверка полноты Complete Binary Tree 
bool dbComplete(Database& database, const std::string& name) {
    return treeComplete(database.trees.at(name));
}

// ЗАГРУЗКА ИЗ ФАЙЛА: считывает базу построчно при запуске программы
void dbLoad(Database& database, const std::string& file) {
    if (!std::filesystem::exists(file)) return; // если файла еще нет — база просто пустая
    std::ifstream input(file);
    if (!input) throw std::runtime_error("не удалось открыть файл");
    std::string line;

    // Читаем файл строка за строкой
    while (std::getline(input, line)) {
        std::istringstream row(line);
        std::string type, name, token;
        if (!(row >> type)) continue; // пропуск пустых строк

        // Проверка формата: тип должен быть одной буквой из списка MFLSQT
        if (!(row >> name) || type.size() != 1 ||
            std::string("MFLSQT").find(type) == std::string::npos)
            throw std::runtime_error("неверный формат файла");

        checkName(name);
        char code = type[0];
        if (dbExists(database, code, name)) throw std::runtime_error("повтор имени в файле");

        // Считываем значения; текст с пробелами заключён в кавычки
        std::vector<std::string> loaded;
        while (readToken(row, token)) loaded.push_back(token);

        // Воссоздаем структуру в памяти и наполняем считанными значениями
        dbCreate(database, code, name);
        // В файле стек записан от вершины: загружаем с конца,
        // чтобы добавление на вершину сохранило прежний порядок.
        if (code == 'S') {
            for (int i = static_cast<int>(loaded.size()) - 1; i >= 0; --i)
                stackPush(database.stacks.at(name), loaded[i]);
        } else {
            for (const std::string& value : loaded)
                dbInsert(database, code, name, code == 'T' ? 0 : dbSize(database, code, name), value);
        }
    }
    if (input.bad()) throw std::runtime_error("ошибка чтения файла");
}

// АТОМАРНОЕ СОХРАНЕНИЕ: безопасная запись через временный файл .tmp
void dbSave(Database& database, const std::string& file) {
    std::string temporary = file + ".tmp";
    if (std::filesystem::exists(temporary))
        throw std::runtime_error("временный файл уже существует: " + temporary);

    std::ofstream output(temporary);
    if (!output) throw std::runtime_error("не удалось создать файл для записи");

    // Лямбда: форматирует строку вида: <Тип> <Имя> <Значения через пробел>
    auto write = [&](char type, const std::string& name) {
        output << type << ' ' << name;
        for (const std::string& value : dbValues(database, type, name)) {
            output << ' ';
            writeValue(output, value);
        }
        output << '\n';
    };

    // Выгружаем каждую структуру из соответствующих std::map
    for (const auto& item : database.arrays) write('M', item.first);
    for (const auto& item : database.doubles) write('L', item.first);
    for (const auto& item : database.lists) write('F', item.first);
    for (const auto& item : database.queues) write('Q', item.first);
    for (const auto& item : database.stacks) write('S', item.first);
    for (const auto& item : database.trees) write('T', item.first);

    output.close();

    // Замена старого файла новым: если rename не удался, удаляем .tmp и кидаем ошибку
    if (!output || std::rename(temporary.c_str(), file.c_str()) != 0) {
        std::remove(temporary.c_str());
        throw std::runtime_error("не удалось сохранить файл");
    }
}


// Очищаем динамическую память всех структур перед выходом.
void clearDatabase(Database& database) {
    for (auto& item : database.arrays) clearArray(item.second);
    for (auto& item : database.doubles) clearDoublyList(item.second);
    for (auto& item : database.lists) clearSinglyList(item.second);
    for (auto& item : database.stacks) clearStack(item.second);
    for (auto& item : database.queues) clearQueue(item.second);
    for (auto& item : database.trees) clearTree(item.second);
    database.arrays.clear();
    database.doubles.clear();
    database.lists.clear();
    database.stacks.clear();
    database.queues.clear();
    database.trees.clear();
}
