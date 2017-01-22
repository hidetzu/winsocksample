#pragma once

#include "SafeQueue.h"

template <class T>
void SafeQueue<T>::enqueue(T t) {
	std::lock_guard<std::mutex> lock(m_);
	q_.push(t);
	c_.notify_one();
}

template <class T>
T SafeQueue<T>::dequeue(void) {
	std::unique_lock<std::mutex> lock(m_);
	while (q_.empty()) {
		c_.wait(lock);
	}
	T val = q_.front();
	q_.pop();
	return val;
}
