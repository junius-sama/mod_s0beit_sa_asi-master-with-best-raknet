#ifndef T_QUEUE_H
#define T_QUEUE_H

#include <mutex>
#include <queue>

/** Queue class safe for multi-threading */

template<class T>
class TQueue {
private:
	std::queue<T*> queue;
	std::mutex mtx;

public:
	TQueue() { /* Default constructor */ }

	void push(T* element)
	{
		std::lock_guard<std::mutex> lock(mtx);

		if (element == nullptr) return;
		queue.push(element);
	}

	bool next(T** target)
	{
		std::lock_guard<std::mutex> lock(mtx);

		if (queue.empty()) return false;
		*target = queue.front();
		queue.pop();
		return true;
	}
};

#endif