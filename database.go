package main

import (
	"fmt"
	"strings"
)

func help() {
	fmt.Print(`Команды (значения — целые числа, индексы с нуля):
MCREATE имя | MPUSH имя число | MINSERT имя индекс число
MGET имя индекс | MSET имя индекс число | MDEL имя индекс
MLEN имя | FCREATE имя | LCREATE имя
FPUSH/LPUSH имя HEAD|TAIL число
FPUSH/LPUSH имя BEFORE|AFTER опорное_число новое_число
FDEL/LDEL имя HEAD|TAIL|VALUE число_для_VALUE
FDEL/LDEL имя BEFORE|AFTER опорное_число
FGET/LGET имя индекс | FFIND/LFIND имя число
SCREATE имя | SPUSH имя число | SPOP имя | SGET имя
QCREATE имя | QPUSH имя число | QPOP имя | QGET имя
TCREATE имя | TINSERT имя число | TFIND имя число
TCOMPLETE имя | TGET имя
PRINT тип имя [REVERSE] | HELP | SAVE | EXIT
Типы: M массив, F односвязный, L двусвязный, S стек,
Q очередь, T дерево. REVERSE разрешён для F и L.
HEAD/TAIL для удаления вводятся без числа.
`)
}

func execute(database Database, query string) (bool, error) {
	parts := strings.Fields(query)
	if len(parts) == 0 {
		return false, fmt.Errorf("пустая команда; введите HELP")
	}
	command := parts[0]
	arguments := func(count int) error {
		if len(parts) != count {
			return fmt.Errorf("неверное число аргументов; введите HELP")
		}
		return nil
	}
	if command == "HELP" {
		if err := arguments(1); err != nil {
			return false, err
		}
		help()
		return false, nil
	}
	printCommand := command == "PRINT"
	kind, operation := command[:1], command[1:]
	name := ""
	if printCommand {
		if len(parts) != 3 && len(parts) != 4 {
			return false, fmt.Errorf("формат: PRINT тип имя [REVERSE]")
		}
		kind, name = parts[1], parts[2]
	} else {
		if len(parts) < 2 {
			return false, fmt.Errorf("такого варианта нет или нет аргументов; HELP")
		}
		name = parts[1]
	}
	if len(kind) != 1 || !strings.Contains("MFLSQT", kind) {
		return false, fmt.Errorf("такого варианта не существует; выберите команду из HELP")
	}
	if err := checkName(name); err != nil {
		return false, err
	}
	record := database[kind+":"+name]
	if operation == "CREATE" && !printCommand {
		if err := arguments(2); err != nil {
			return false, err
		}
		if record != nil {
			return false, fmt.Errorf("структура уже существует")
		}
		database.Create(kind, name)
		fmt.Println("Создано:", kind, name)
		return true, nil
	}
	if record == nil {
		return false, fmt.Errorf("структура не создана")
	}
	if printCommand || (kind == "T" && operation == "GET") {
		if !printCommand {
			if err := arguments(2); err != nil {
				return false, err
			}
		}
		reverse := len(parts) == 4
		if reverse && (parts[3] != "REVERSE" || (kind != "F" && kind != "L")) {
			return false, fmt.Errorf("REVERSE разрешён только для списков")
		}
		items := record.Values(reverse)
		boundary := 1
		if kind == "T" {
			fmt.Println("Дерево по уровням:")
		}
		for index, value := range items {
			fmt.Print(value, " ")
			if kind == "T" && index+1 == boundary {
				fmt.Println()
				boundary = boundary*2 + 1
			}
		}
		if kind == "T" && len(items) == 0 {
			fmt.Print("Пусто")
		}
		fmt.Println()
		return false, nil
	}
	if kind == "T" {
		if operation == "COMPLETE" {
			if err := arguments(2); err != nil {
				return false, err
			}
			if record.tree.Complete() {
				fmt.Println("TRUE")
			} else {
				fmt.Println("FALSE")
			}
			return false, nil
		}
		if operation != "INSERT" && operation != "FIND" {
			return false, fmt.Errorf("неизвестная команда; HELP")
		}
		if err := arguments(3); err != nil {
			return false, err
		}
		value, err := number(parts[2])
		if err != nil {
			return false, err
		}
		if operation == "FIND" {
			if record.tree.Find(value) {
				fmt.Println("TRUE")
			} else {
				fmt.Println("FALSE")
			}
			return false, nil
		}
		record.tree.Insert(value)
	} else {
		length, index := record.data.Len(), 0
		if operation == "LEN" && kind == "M" {
			if err := arguments(2); err != nil {
				return false, err
			}
			fmt.Println(length)
			return false, nil
		}
		if operation == "FIND" && (kind == "F" || kind == "L") {
			if err := arguments(3); err != nil {
				return false, err
			}
			value, err := number(parts[2])
			if err != nil {
				return false, err
			}
			fmt.Println(record.data.Find(value))
			return false, nil
		}
		if operation == "GET" || (operation == "POP" && (kind == "S" || kind == "Q")) {
			if kind == "M" || kind == "F" || kind == "L" {
				if err := arguments(3); err != nil {
					return false, err
				}
				var err error
				index, err = number(parts[2])
				if err != nil {
					return false, err
				}
			} else {
				if err := arguments(2); err != nil {
					return false, err
				}
			}
			if index < 0 || index >= length {
				return false, fmt.Errorf("структура пуста или индекс вне диапазона")
			}
			fmt.Println(record.data.Get(index))
			if operation == "GET" {
				return false, nil
			}
			record.data.Erase(index)
		} else if kind == "M" {
			if operation != "PUSH" && operation != "INSERT" &&
				operation != "SET" && operation != "DEL" {
				return false, fmt.Errorf("неизвестная команда; HELP")
			}
			count := 4
			if operation == "PUSH" || operation == "DEL" {
				count = 3
			}
			if err := arguments(count); err != nil {
				return false, err
			}
			index = length
			if operation != "PUSH" {
				var err error
				index, err = number(parts[2])
				if err != nil {
					return false, err
				}
			}
			adding := operation == "PUSH" || operation == "INSERT"
			if index < 0 || index > length || (!adding && index == length) {
				return false, fmt.Errorf("индекс вне диапазона")
			}
			if operation == "DEL" {
				record.data.Erase(index)
			} else {
				value, err := number(parts[len(parts)-1])
				if err != nil {
					return false, err
				}
				if adding {
					record.data.Insert(index, value)
				} else {
					record.data.(*Array).Set(index, value)
				}
			}
		} else if kind == "S" || kind == "Q" {
			if operation != "PUSH" {
				return false, fmt.Errorf("неизвестная команда; HELP")
			}
			if err := arguments(3); err != nil {
				return false, err
			}
			value, err := number(parts[2])
			if err != nil {
				return false, err
			}
			if kind == "Q" {
				index = length
			}
			record.data.Insert(index, value)
		} else {
			if operation != "PUSH" && operation != "DEL" {
				return false, fmt.Errorf("неизвестная команда; HELP")
			}
			if len(parts) < 3 {
				return false, fmt.Errorf("нужен способ: HEAD, TAIL, BEFORE, AFTER, VALUE")
			}
			position := parts[2]
			adding := operation == "PUSH"
			if position == "HEAD" || position == "TAIL" {
				count := 3
				if adding {
					count = 4
				}
				if err := arguments(count); err != nil {
					return false, err
				}
				if position == "TAIL" {
					index = length
					if !adding {
						index--
					}
				}
			} else {
				if position != "BEFORE" && position != "AFTER" &&
					(adding || position != "VALUE") {
					return false, fmt.Errorf("неизвестный способ; HELP")
				}
				count := 4
				if adding {
					count = 5
				}
				if err := arguments(count); err != nil {
					return false, err
				}
				target, err := number(parts[3])
				if err != nil {
					return false, err
				}
				index = record.data.Find(target)
				if index < 0 {
					return false, fmt.Errorf("опорное значение не найдено")
				}
				if position == "AFTER" {
					index++
				}
				if position == "BEFORE" && !adding {
					index--
				}
			}
			if index < 0 || index > length || (!adding && index == length) {
				return false, fmt.Errorf("нет элемента в выбранной позиции")
			}
			if adding {
				value, err := number(parts[len(parts)-1])
				if err != nil {
					return false, err
				}
				record.data.Insert(index, value)
			} else {
				record.data.Erase(index)
			}
		}
	}
	fmt.Println("Выполнено")
	return true, nil
}
