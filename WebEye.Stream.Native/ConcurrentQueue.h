#ifndef WEBEYE_CONCURRENTQUEUE_H
#define WEBEYE_CONCURRENTQUEUE_H

#include <atomic>
#include <queue>

#include <boost/thread/mutex.hpp>

namespace WebEye
{
	/// <summary>
	/// A ConcurrentQueue class implements a thread-safe first in-first out (FIFO) collection. 
	/// </summary>
	template <typename T>
	class ConcurrentQueue
	{
	public:
		ConcurrentQueue(const ConcurrentQueue&) = delete;
		ConcurrentQueue& operator=(const ConcurrentQueue&) = delete;

		/// <summary>
		/// Initializes a new instance of the ConcurrentQueue class. 
		/// </summary>
		ConcurrentQueue() : stopRequested_(false) { }

		/// <summary>
		/// Adds an object to the end of the ConcurrentQueue. 
		/// </summary>
		void Push(const T& value)
		{
			{
				boost::unique_lock<boost::mutex> lock(mutex_);
				queue_.push(value);
			}

			queueNotEmpty_.notify_one();
		}

		/// <summary>
		/// Tries to remove and retrieve the object at the beginning of the ConcurrentQueue. 
		/// </summary>
		bool TryPop(T& value)
		{
			boost::unique_lock<boost::mutex> lock(mutex_);
			if (queue_.empty())
			{
				return false;
			}

			value = queue_.front();
			queue_.pop();

			return true;
		}

		/// <summary>
		/// Removes and retrieves the object at the beginning of the ConcurrentQueue,
		/// waiting if necessary until an object becomes available.
		/// </summary>
		bool WaitAndPop(T &value)
		{
			boost::unique_lock<boost::mutex> lock(mutex_);
			while (queue_.empty() && !stopRequested_)
			{
				queueNotEmpty_.wait(lock);
			}

			if (stopRequested_)
			{
				return false;
			}

			value = queue_.front();
			queue_.pop();

			return true;
		}

		/// <summary>
		/// Interrupts waiting for a new object. 
		/// </summary>
		void StopWait()
		{
			stopRequested_ = true;
			queueNotEmpty_.notify_all();
		}

		/// <summary>
		/// Returns the number of elements in the ConcurrentQueue. 
		/// </summary>
		size_t Size()
		{
			boost::unique_lock<boost::mutex> lock(mutex_);
			return queue_.size();
		}

	private:
		std::atomic<bool> stopRequested_;
		std::queue<T> queue_;
		boost::mutex mutex_;
		boost::condition_variable queueNotEmpty_;
	};
}

#endif // WEBEYE_CONCURRENTQUEUE_H