package main

// Array хранит элементы в собственном динамическом массиве.
type Array struct {
	data []int
	size int
}

func (array *Array) Len() int             { return array.size }
func (array *Array) Get(index int) int    { return array.data[index] }
func (array *Array) Set(index, value int) { array.data[index] = value }

func (array *Array) Insert(index, value int) {
	if array.size == len(array.data) {
		capacity := len(array.data) * 2
		if capacity == 0 {
			capacity = 4
		}
		expanded := make([]int, capacity)
		copy(expanded, array.data)
		array.data = expanded
	}
	for i := array.size; i > index; i-- {
		array.data[i] = array.data[i-1]
	}
	array.data[index] = value
	array.size++
}

func (array *Array) Erase(index int) {
	for i := index; i+1 < array.size; i++ {
		array.data[i] = array.data[i+1]
	}
	array.size--
}

func (array *Array) Find(value int) int {
	for i := 0; i < array.size; i++ {
		if array.data[i] == value {
			return i
		}
	}
	return -1
}

func (array *Array) Values(reverse bool) []int {
	result := make([]int, array.size)
	copy(result, array.data[:array.size])
	return result
}
