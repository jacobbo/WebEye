#ifndef FFMPEG_FACADE_CONCURRENTQUEUE_H
#define FFMPEG_FACADE_CONCURRENTQUEUE_H

#include <queue>

#pragma warning( push )
#pragma warning( disable : 4100 )

#include <boost/thread.hpp>

#pragma warning( pop )

namespace FFmpeg
{
    namespace Facade
    {
        template <typename T>
        class ConcurrentQueue
        {
        public:
            ConcurrentQueue() : stopRequested_(false) { }

            ConcurrentQueue(ConcurrentQueue const& other) = delete;
            ConcurrentQueue& operator=(ConcurrentQueue const&) = delete;

            void Push(T value)
            {
                boost::unique_lock<mutex> lock(mutex_);
                queue_.push(value);
                lock.unlock();
                queueNotEmpty_.notify_one();
            }

            bool WaitAndPop(T &value)
            {
                unique_lock<mutex> lock(mutex_);
                queueNotEmpty_.wait(lock,
                    [this] { return !queue_.empty() || stopRequested_; });

                if (stopRequested_)
                    return false;

                value = queue_.front();
                queue_.pop();

                return true;
            }

            void Stop()
            {
                unique_lock<std::mutex> lock(mutex_);
                stopRequested_ = true;
                lock.unlock();
                queueNotEmpty_.notify_all();
            }

        private:

            bool stopRequested_;
            mutable boost::mutex mutex_;
            std::queue<T> queue_;
            boost::condition_variable queueNotEmpty_;
        };
    }
}

#endif // FFMPEG_FACADE_CONCURRENTQUEUE_H