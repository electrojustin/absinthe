#ifndef LIB_QUEUE_H
#define LIB_QUEUE_H

#include "io/stdio.h"
#include "memory/heap.h"
#include "thread/lock.h"

namespace lib {

template <class T>
struct QueueNode {
	T data;
	QueueNode<T>* next;
};

template <class T>
class Queue {
	public:
	Queue();
	Queue(Queue& to_copy);
	Queue(Queue&& to_move);
	~Queue();

	void enqueue(T to_enqueue);
	bool is_empty();
	bool dequeue(T& ret);

	private:
	QueueNode<T>* head;
	QueueNode<T>* tail;

	thread::Lock queue_mutex;
};

template <class T>
Queue<T>::Queue() {
	queue_mutex.lock();
	head = nullptr;
	tail = nullptr;
	queue_mutex.unlock();
}

template <class T>
Queue<T>::Queue(Queue& to_copy) {
	to_copy.queue_mutex.lock();
	queue_mutex.lock();

	if (!to_copy.head) {
		head = nullptr;
		tail = nullptr;
		to_copy.queue_mutex.unlock();
		queue_mutex.unlock();
		return;
	}

	QueueNode<T>* curr = to_copy.head;
	while(curr) {
		enqueue(curr.data);
		curr = curr->next;
	}

	to_copy.queue_mutex.unlock();
	queue_mutex.unlock();
}

template <class T>
Queue<T>::Queue(Queue&& to_move) {
	to_move.queue_mutex.lock();
	queue_mutex.lock();

	head = to_move.head;
	tail = to_move.tail;
	to_move.head = nullptr;
	to_move.tail = nullptr;

	to_move.queue_mutex.unlock();
	queue_mutex.unlock();
}

template <class T>
Queue<T>::~Queue() {
	T discard;
	while(dequeue(discard));
}

template <class T>
void Queue<T>::enqueue(T to_enqueue) {
	queue_mutex.lock();

	if (!tail) {
		tail = new QueueNode<T>();
		tail->data = to_enqueue;
		tail->next = nullptr;
		head = tail;
		queue_mutex.unlock();
		return;
	}

	tail->next = new QueueNode<T>();
	tail->next->data = to_enqueue;
	tail->next->next = nullptr;
	tail = tail->next;

	queue_mutex.unlock();
}

template <class T>
bool Queue<T>::dequeue(T& ret) {
	queue_mutex.lock();
	if (!head) {
		queue_mutex.unlock();
		return false;
	}

	ret = head->data;
	QueueNode<T>* next = head->next;
	delete head;
	head = next;
	if (!head) {
		tail = nullptr;
	}

	queue_mutex.unlock();
	return true;
}

template <class T>
bool Queue<T>::is_empty() {
	queue_mutex.lock();
	return head == nullptr;
	queue_mutex.unlock();
}

} // namespace lib

#endif
