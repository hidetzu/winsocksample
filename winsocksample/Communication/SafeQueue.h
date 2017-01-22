#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

template <class T>
class SafeQueue
{
public:
	SafeQueue(void)
		: q_()
		, m_()
		, c_()
	{}

	~SafeQueue(void)
	{}

	void enqueue(T t) {
		std::lock_guard<std::mutex> lock(m_);
		q_.push(t);
		c_.notify_one();
	}

	T dequeue(void) {
		std::unique_lock<std::mutex> lock(m_);
		while (q_.empty()) {
			c_.wait(lock);
		}
		T val = q_.front();
		q_.pop();
		return val;
	}

private:
	std::queue<T> q_;
	mutable std::mutex m_;
	std::condition_variable c_;
};
