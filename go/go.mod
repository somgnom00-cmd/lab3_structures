package main

type DoubleNode struct {
	value int
	next  *DoubleNode
	prev  *DoubleNode
}

type DoublyList struct {
	head *DoubleNode
	tail *DoubleNode
	size int
}

func (list *DoublyList) Len() int { return list.size }

func (list *DoublyList) nodeAt(index int) *DoubleNode {
	current := list.head
	for i := 0; i < index; i++ {
		current = current.next
	}
	return current
}

func (list *DoublyList) Get(index int) int {
	return list.nodeAt(index).value
}

func (list *DoublyList) Insert(index, value int) {
	added := &DoubleNode{value: value}
	var before *DoubleNode
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
	added.prev = before
	if added.next != nil {
		added.next.prev = added
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

func (list *DoublyList) Erase(index int) {
	var removed *DoubleNode
	if index == list.size-1 {
		removed = list.tail
	} else {
		removed = list.nodeAt(index)
	}
	before := removed.prev
	if removed.next != nil {
		removed.next.prev = before
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

func (list *DoublyList) Find(value int) int {
	index := 0
	for current := list.head; current != nil; current = current.next {
		if current.value == value {
			return index
		}
		index++
	}
	return -1
}

func (list *DoublyList) Values(reverse bool) []int {
	result := make([]int, 0, list.size)
	current := list.head
	if reverse {
		current = list.tail
	}
	for current != nil {
		result = append(result, current.value)
		if reverse {
			current = current.prev
		} else {
			current = current.next
		}
	}
	return result
}
