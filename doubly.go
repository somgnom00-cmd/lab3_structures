package main

import (
	"bufio"
	"fmt"
	"os"
	"sort"
	"strconv"
	"strings"
)

// Общие операции массива и списков.
type Sequence interface {
	Len() int
	Get(int) int
	Insert(int, int)
	Erase(int)
	Find(int) int
	Values(bool) []int
}

type Record struct {
	kind, name string
	data       Sequence
	tree       *CompleteTree
}

type Database map[string]*Record

func number(token string) (int, error) {
	value, err := strconv.ParseInt(token, 10, 32)
	if err != nil {
		return 0, fmt.Errorf("требуется целое число int: %s", token)
	}
	return int(value), nil
}

func checkName(name string) error {
	if name == "" {
		return fmt.Errorf("не указано имя структуры")
	}
	for _, symbol := range name {
		if !(symbol >= 'a' && symbol <= 'z') &&
			!(symbol >= 'A' && symbol <= 'Z') &&
			!(symbol >= '0' && symbol <= '9') && symbol != '_' {
			return fmt.Errorf("имя: латинские буквы, цифры и _")
		}
	}
	return nil
}

func (database Database) Create(kind, name string) *Record {
	record := &Record{kind: kind, name: name}
	switch kind {
	case "M":
		record.data = &Array{}
	case "L":
		record.data = &DoublyList{}
	case "T":
		record.tree = &CompleteTree{}
	default:
		record.data = &SinglyList{}
	}
	database[kind+":"+name] = record
	return record
}

func (record *Record) Values(reverse bool) []int {
	if record.kind == "T" {
		return record.tree.Values()
	}
	return record.data.Values(reverse)
}

func (database Database) Load(file string) error {
	input, err := os.Open(file)
	if os.IsNotExist(err) {
		return nil
	}
	if err != nil {
		return err
	}
	defer input.Close()
	scanner := bufio.NewScanner(input)
	scanner.Buffer(make([]byte, 4096), 64*1024*1024)
	for scanner.Scan() {
		fields := strings.Fields(scanner.Text())
		if len(fields) == 0 {
			continue
		}
		if len(fields) < 2 || len(fields[0]) != 1 ||
			!strings.Contains("MFLSQT", fields[0]) {
			return fmt.Errorf("неверный формат файла")
		}
		kind, name := fields[0], fields[1]
		if err := checkName(name); err != nil {
			return err
		}
		if database[kind+":"+name] != nil {
			return fmt.Errorf("повтор имени в файле")
		}
		loaded := []int{}
		for _, token := range fields[2:] {
			value, err := number(token)
			if err != nil {
				return err
			}
			loaded = append(loaded, value)
		}
		record := database.Create(kind, name)
		for _, value := range loaded {
			if kind == "T" {
				record.tree.Insert(value)
			} else {
				record.data.Insert(record.data.Len(), value)
			}
		}
	}
	return scanner.Err()
}

func (database Database) Save(file string) error {
	temporary := file + ".tmp"
	output, err := os.OpenFile(temporary, os.O_WRONLY|os.O_CREATE|os.O_EXCL, 0600)
	if err != nil {
		return err
	}
	defer os.Remove(temporary)
	writer := bufio.NewWriter(output)
	keys := make([]string, 0, len(database))
	for key := range database {
		keys = append(keys, key)
	}
	sort.Strings(keys)
	for _, key := range keys {
		record := database[key]
		fmt.Fprint(writer, record.kind, " ", record.name)
		for _, value := range record.Values(false) {
			fmt.Fprint(writer, " ", value)
		}
		fmt.Fprintln(writer)
	}
	flushError := writer.Flush()
	closeError := output.Close()
	if flushError != nil {
		return flushError
	}
	if closeError != nil {
		return closeError
	}
	return os.Rename(temporary, file)
}
