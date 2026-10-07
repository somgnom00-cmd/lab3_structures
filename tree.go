package main

type SingleNode struct {
	value int
	next  *SingleNode
}

type SinglyList struct {
	head *SingleNode
	tail *SingleNode
	size int
}

func (list *SinglyList) Len() int { return list.size }

func (list *SinglyList) nodeAt(index int) *SingleNode {
	current := list.head
	for i := 0; i < index; i++ {
		current = current.next
	}
	return current
}

func (list *SinglyList) Get(index int) int {
	return list.nodeAt(index).value
}

func (list *SinglyList) Insert(index, value int) {
	added := &SingleNode{value: value}
	var before *SingleNode
	if index == list.size {
		before = list.tail
	} else if index > 0 {
		before = list.nodeAt(index - 1)
	}
	if before == nil {
		added.next = list.head
	} else {
		added.next = before.next
	}
	if before == nil {
		list.head = added
	} else {
		before.next = added
	}
	if added.next == nil {
		list.tail = added
	}
	list.size++
}

func (list *SinglyList) Erase(index int) {
	var before *SingleNode
	if index > 0 {
		before = list.nodeAt(index - 1)
	}
	removed := list.head
	if before != nil {
		removed = before.next
	}
	if before == nil {
		list.head = removed.next
	} else {
		before.next = removed.next
	}
	if removed == list.tail {
		list.tail = before
	}
	list.size--
}

func (list *SinglyList) Find(value int) int {
	index := 0
	for current := list.head; current != nil; current = current.next {
		if current.value == value {
			return index
		}
		index++
	}
	return -1
}

func (list *SinglyList) Values(reverse bool) []int {
	result := make([]int, 0, list.size)
	for current := list.head; current != nil; current = current.next {
		result = append(result, current.value)
	}
	if reverse {
		for i := 0; i < len(result)/2; i++ {
			other := len(result) - 1 - i
			result[i], result[other] = result[other], result[i]
		}
	}
	return result
}
