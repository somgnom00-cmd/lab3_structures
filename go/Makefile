package main

import (
	"bufio"
	"fmt"
	"os"
)

func run() int {
	file, query := "data.txt", ""
	singleQuery := false
	for index := 1; index < len(os.Args); index += 2 {
		if index+1 >= len(os.Args) {
			fmt.Println("Ошибка: у ключа нет значения")
			return 1
		}
		switch os.Args[index] {
		case "--file":
			file = os.Args[index+1]
		case "--query":
			query = os.Args[index+1]
			singleQuery = true
		default:
			fmt.Println("Ошибка: ключи --file и --query")
			return 1
		}
	}
	database := Database{}
	if err := database.Load(file); err != nil {
		fmt.Println("Ошибка:", err)
		return 1
	}
	scanner := bufio.NewScanner(os.Stdin)
	scanner.Buffer(make([]byte, 4096), 1024*1024)
	if !singleQuery {
		fmt.Println("Структуры данных. HELP — справка, EXIT — выход.")
	}
	for {
		if !singleQuery {
			fmt.Print("> ")
			if !scanner.Scan() {
				break
			}
			query = scanner.Text()
		}
		if query == "EXIT" {
			break
		}
		changed, err := false, error(nil)
		if query == "SAVE" {
			changed = true
		} else {
			changed, err = execute(database, query)
		}
		if err == nil && changed {
			if saveError := database.Save(file); saveError != nil {
				err = fmt.Errorf("%v; данные в памяти сохранены, повторите SAVE", saveError)
			}
		}
		if err != nil {
			fmt.Println("Ошибка:", err)
			if singleQuery {
				return 1
			}
		}
		if singleQuery {
			break
		}
	}
	if err := scanner.Err(); err != nil {
		fmt.Println("Ошибка чтения:", err)
		return 1
	}
	return 0
}

func main() { os.Exit(run()) }
