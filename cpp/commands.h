#include "commands.h"
#include <iostream>
#include <sstream>
#include <stdexcept>

// Вывод справочной таблицы всех доступных команд
void help() {
    std::cout << "Команды (значения — текст или числа, индексы с нуля):\n"
        "MCREATE имя | MPUSH имя значение | MINSERT имя индекс значение\n"
        "MGET имя индекс | MSET имя индекс значение | MDEL имя индекс\n"
        "MLEN имя | FCREATE имя | LCREATE имя\n"
        "FPUSH/LPUSH имя HEAD|TAIL значение\n"
        "FPUSH/LPUSH имя BEFORE|AFTER опорное_значение новое_значение\n"
        "FDEL/LDEL имя HEAD|TAIL|VALUE значение_для_VALUE\n"
        "FDEL/LDEL имя BEFORE|AFTER опорное_значение\n"
        "FGET/LGET имя индекс | FFIND/LFIND имя значение\n"
        "SCREATE имя | SPUSH имя значение | SPOP имя | SGET имя\n"
        "QCREATE имя | QPUSH имя значение | QPOP имя | QGET имя\n"
        "TCREATE имя | TINSERT имя значение | TFIND имя значение\n"
        "TCOMPLETE имя | TGET имя\n"
        "PRINT тип имя [REVERSE] | HELP | SAVE | EXIT\n"
        "Типы: M массив, F односвязный, L двусвязный, S стек,\n"
        "Q очередь, T дерево. REVERSE разрешён для F и L.\n"
        "HEAD/TAIL для удаления вводятся без числа.\n"
        "Текст с пробелами вводится в двойных кавычках.\n";
}

// Вспомогательная функция-предохранитель: если условие false, выбрасывает ошибку
static void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}

// Главная функция выполнения: возвращает true при изменении данных (для автосохранения)
bool execute(Database& database, const std::string& query) {
    // 1. ТОКЕНИЗАЦИЯ: текст в кавычках читаем целиком, остальное — по пробелам
    std::istringstream input(query);
    std::vector<std::string> parts;
    std::string token;
    while (readToken(input, token)) parts.push_back(token);
    require(!parts.empty() && !parts[0].empty(), "пустая команда; введите HELP");

    std::string command = parts[0];

    // Лямбда-функция для быстрой проверки точного количества аргументов в команде
    auto arguments = [&](int count) {
        require(static_cast<int>(parts.size()) == count,
                "неверное число аргументов; введите HELP");
    };

    if (command == "HELP") { arguments(1); help(); return false; }

    // 2. РАЗБОР ИМЕНИ И ТИПА первая буква — тип
    bool print = command == "PRINT";
    char type = command[0];                    // 'M', 'F', 'L', 'S', 'Q' или 'T'
    std::string operation = command.substr(1); // 'PUSH', 'DEL', 'GET' и т.д.
    std::string name;

    if (print) {
        require(parts.size() == 3 || parts.size() == 4,
                "формат: PRINT тип имя [REVERSE]");
        require(parts[1].size() == 1, "тип: M, F, L, S, Q или T");
        type = parts[1][0]; name = parts[2];
    } else {
        require(parts.size() >= 2, "такого варианта нет или нет аргументов; HELP");
        name = parts[1];
    }

    // Проверка допустимости буквы структуры и корректности её имени (только латиница и цифры)
    require(std::string("MFLSQT").find(type) != std::string::npos,
            "такого варианта не существует; выберите команду из HELP");
    checkName(name);

    // 3. СОЗДАНИЕ СТРУКТУРЫ (MCREATE, FCREATE и т.д.)
    if (operation == "CREATE" && !print) {
        arguments(2);
        require(!dbExists(database, type, name), "структура уже существует");
        dbCreate(database, type, name);
        std::cout << "Создано: " << type << ' ' << name << '\n';
        return true; // возвращает true, так как база изменилась
    }

    // Защита: для любых других операций структура уже должна быть создана
    require(dbExists(database, type, name), "структура не создана");

    // 4. ВЫВОД НА ЭКРАН: PRINT или TGET достает все значения из базы
    if (print || (type == 'T' && operation == "GET")) {
        if (!print) arguments(2);
        bool reverse = parts.size() == 4;
        if (reverse) require(parts[3] == "REVERSE" && (type == 'F' || type == 'L'),
                             "REVERSE разрешён только для списков");

        std::vector<std::string> items = dbValues(database, type, name, reverse);

        // Для дерева выводим поэтажно (по уровням: 1 узел, затем 2, затем 4 и т.д.)
        if (type == 'T') {
            std::cout << "Дерево по уровням:\n";
            int boundary = 1;
            for (int i = 0; i < static_cast<int>(items.size()); ++i) {
                writeValue(std::cout, items[i]); std::cout << ' ';
                // boundary удваивается: перенос строки на границе каждого уровня
                if (i + 1 == boundary) { std::cout << '\n'; boundary = boundary * 2 + 1; }
            }
            if (items.empty()) std::cout << "Пусто";
        } else {
            for (const std::string& value : items) {
                writeValue(std::cout, value); std::cout << ' ';
            }
        }
        std::cout << '\n'; return false; // чтение не меняет данные, сохранять не нужно
    }

    // 5. ОПЕРАЦИИ С ДЕРЕВОМ 
    if (type == 'T') {
        if (operation == "COMPLETE") {
            arguments(2);
            // Проверка на Complete Binary Tree
            std::cout << (dbComplete(database, name) ? "TRUE\n" : "FALSE\n");
            return false;
        }
        require(operation == "INSERT" || operation == "FIND", "неизвестная команда; HELP");
        arguments(3); std::string value = parts[2];
        if (operation == "FIND") {
            std::cout << (dbFind(database, type, name, value) >= 0 ? "TRUE\n" : "FALSE\n");
            return false;
        }
        dbInsert(database, type, name, 0, value); // Вставка нового узла
    } 
    // 6. ОПЕРАЦИИ С ДРУГИМИ СТРУКТУРАМИ (M, F, L, S, Q)
    else {
        int length = dbSize(database, type, name);
        int index = 0;

        // Размер массива
        if (operation == "LEN" && type == 'M') {
            arguments(2); std::cout << length << '\n'; return false;
        }

        // Поиск значения в списках: возвращает индекс
        if (operation == "FIND" && (type == 'F' || type == 'L')) {
            arguments(3);
            std::cout << dbFind(database, type, name, parts[2]) << '\n';
            return false;
        }

        // Чтение элемента по индексу (GET) или извлечение из стека/очереди (POP)
        if (operation == "GET" || (operation == "POP" && (type == 'S' || type == 'Q'))) {
            if (type == 'M' || type == 'F' || type == 'L') {
                arguments(3); index = number(parts[2]);
            } else arguments(2); // для стека/очереди индекс не нужен, всегда голова (0)

            require(index >= 0 && index < length, "структура пуста или индекс вне диапазона");
            std::cout << dbGet(database, type, name, index) << '\n';
            if (operation == "GET") return false;
            dbErase(database, type, name, index); // если это POP — удаляем извлеченный элемент
        } 
        // Логика Массива (M): вставка в конец (PUSH), по индексу (INSERT), замена (SET), удаление (DEL)
        else if (type == 'M') {
            require(operation == "PUSH" || operation == "INSERT" ||
                    operation == "SET" || operation == "DEL", "неизвестная команда; HELP");
            arguments(operation == "PUSH" || operation == "DEL" ? 3 : 4);

            index = operation == "PUSH" ? length : number(parts[2]);
            bool adding = operation == "PUSH" || operation == "INSERT";

            require(index >= 0 && (adding ? index <= length : index < length),
                    "индекс вне диапазона");

            if (operation == "DEL") {
                dbErase(database, type, name, index);
            } else {
                std::string value = parts.back();
                if (adding) dbInsert(database, type, name, index, value);
                else dbSet(database, name, index, value);
            }
        } 
        // Логика Стека и Очереди: добавление в голову для S (индекс 0), в хвост для Q (индекс length)
        else if (type == 'S' || type == 'Q') {
            require(operation == "PUSH", "неизвестная команда; HELP");
            arguments(3);
            dbInsert(database, type, name, type == 'S' ? 0 : length, parts[2]);
        } 
        // Логика Списков (F, L): 4 способа (HEAD, TAIL, BEFORE, AFTER) + VALUE для удаления
        else {
            require(operation == "PUSH" || operation == "DEL", "неизвестная команда; HELP");
            require(parts.size() >= 3, "нужен способ: HEAD, TAIL, BEFORE, AFTER, VALUE");
            std::string position = parts[2];
            bool adding = operation == "PUSH";

            // Работа с концами списка (HEAD или TAIL)
            if (position == "HEAD" || position == "TAIL") {
                arguments(adding ? 4 : 3);
                index = position == "HEAD" ? 0 : (adding ? length : length - 1);
            } 
            // Работа по значению (BEFORE, AFTER, VALUE) — ищем опорное значение
            else {
                require(position == "BEFORE" || position == "AFTER" ||
                        (!adding && position == "VALUE"), "неизвестный способ; HELP");
                arguments(adding ? 5 : 4);
                index = dbFind(database, type, name, parts[3]);
                require(index >= 0, "опорное значение не найдено");
                if (position == "AFTER") ++index;
                if (position == "BEFORE" && !adding) --index;
            }

            require(index >= 0 && (adding ? index <= length : index < length),
                    "нет элемента в выбранной позиции");

            if (adding) dbInsert(database, type, name, index, parts.back());
            else dbErase(database, type, name, index);
        }
    }

    std::cout << "Выполнено\n";
    return true; // возвращает true: данные изменились, запускаем автосохранение
}
