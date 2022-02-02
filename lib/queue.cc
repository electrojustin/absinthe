#include "lib/queue.h"
#include "io/stdio.h"

namespace lib {

namespace {

using io::printk;
using io::print_stack_trace;

} // namespace

template <class T>
Queue<T>::Queue() {
	head = nullptr;
	tail = nullptr;
}

template <class T>
Queue<T>::Queue(Queue& to_copy) {
	if (!to_copy.head) {
		head = nullptr;
		tail = nullptr;
		return;
	}

	QueueNode<T>* curr = to_copy.head;
	while(curr) {
		enqueue(curr.data);
		curr = curr->next;
	}
}

template <class T>
Queue<T>::Queue(Queue&& to_move) {
	head = to_move.head;
	tail = to_move.tail;
	to_move.head = nullptr;
	to_move.tail = nullptr;
}

template <class T>
void Queue<T>::enqueue(T to_enqueue) {
	if (!tail) {
		tail = new QueueNode<T>();
		tail.data = to_enqueue;
		tail.next = nullptr;
		head = tail;
		return;
	}

	tail->next = new QueueNode<T>();
	tail->next->data = to_enqueue;
	tail->next->next = nullptr;
	tail = tail->next;
}

template <class T>
T Queue<T>::dequeue() {
	T ret;

	if (!head) {
		printk("Error! Attempted to dequeue empty queue!\n");
		print_stack_trace();
		return ret;
	}

	ret = head.data;
	QueueNode<T>* next = head->next;
	delete head;
	head = next;

	return ret;
}

} // namespace lib
