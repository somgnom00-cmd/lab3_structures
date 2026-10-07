#include "commands.h"
#include <iostream>
#include <stdexcept>

int main(int argc, char* argv[]) { //argc — счетчик слов, argv  — список этих слов
    std::string file = "data.txt", query;
    bool singleQuery = false;
    Database database; // Данные хранятся в обычной структуре.
    try {
        for (int i = 1; i < argc; i += 2) { // РАЗБОР КЛЮЧЕЙ: идем с шагом 2 (i += 2), так как аргументы идут парами: [ключ] [значение]пределяя имя файла базы данных и наличие разового запроса
            if (i + 1 >= argc) throw std::runtime_error("у ключа нет значения"); //Защита от выхода за границы массива 
            std::string option = argv[i];
            if (option == "--file") file = argv[i + 1];
            else if (option == "--query") { query = argv[i + 1]; singleQuery = true; }
            else throw std::runtime_error("ключи: --file и --query");
        }
        // ЗАГРУЗКА: считываем сохраненные структуры из файла в оперативную память
        dbLoad(database, file);
        if (!singleQuery) { // если запрос не разовый, приветствие.
            std::cout << "Структуры данных. HELP — справка, EXIT — выход.\n";
        }
        // ГЛАВНЫЙ ЦИКЛ: ждет ввода команд. При флаге --query выполняется ровно 1 раз и выходит
        do {
            if (!singleQuery) {
                std::cout << "> ";
                if (!std::getline(std::cin, query)) break;
            }
            try {
                if (query == "EXIT") break;
                 // ВЫПОЛНЕНИЕ: changed = true только если данные реально изменились (добавление/удаление)
                bool changed = query == "SAVE" || execute(database, query);
                if (changed) {
                    try { dbSave(database, file); } // автосохранение на диск только при изменениях
                    catch (const std::exception& error) {
                        throw std::runtime_error(std::string(error.what()) +
                            "; данные в памяти сохранены, повторите SAVE");
                    }
                }
            } catch (const std::exception& error) {
                 // ЗАЩИТА: ловим ошибки пользователя (неверный индекс и т.д.), программа НЕ падает
                std::cout << "Ошибка: " << error.what() << '\n';
                if (singleQuery) { clearDatabase(database); return 1; }
            }
        } while (!singleQuery);
    } catch (const std::exception& error) {
         // Ловит фатальные ошибки старта (битый входной файл или неверные ключи запуска)
        std::cerr << "Ошибка: " << error.what() << '\n';
        clearDatabase(database);
        return 1;
    }
    clearDatabase(database); // Явно освобождаем память перед обычным выходом.
}
