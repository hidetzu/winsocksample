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

	void enqueue(T t);
	T dequeue(void);

private:
	std::queue<T> q_;
	mutable std::mutex m_;
	std::condition_variable c_;
};
