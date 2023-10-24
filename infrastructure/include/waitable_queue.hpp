/*******************************************************************************
 * Project:     Waitable Queue
 * Version:     1.0 - 24/10/2023
*******************************************************************************/
#ifndef __WAITABLE_QUEUE_HPP__
#define __WAITABLE_QUEUE_HPP__

#include <chrono>               //std::chrono::milliseconds
#include <queue>
#include <mutex>                //std::mutex, lock_guard
#include <condition_variable>   //std::condition_variable

#include "utils.hpp"            //ThrowIfBad

namespace Infrastructure
{
// QUEUE must be a queue from the stl containers library,
// and value_type must be copyable
// if QUEUE is a priority queue, use PQueue instead of std::priority_queue
template <typename QUEUE>
class WaitableQueue
{
public:
    explicit WaitableQueue() =default;
    ~WaitableQueue() noexcept =default;

    WaitableQueue(const WaitableQueue&) =delete;
    WaitableQueue& operator=(const WaitableQueue&) =delete;

    void Push(const typename QUEUE::value_type& data_);

    // Pop() might fail, perform checks on "out_"
    void Pop(typename QUEUE::value_type *out_);
    // returns false if "timeout_" expired
    bool Pop(typename QUEUE::value_type *out_,
                                    const std::chrono::milliseconds& timeout_);
    
    bool IsEmpty() const;

private:
    QUEUE m_queue;

    mutable std::mutex m_queue_guard;
    mutable std::condition_variable m_can_op;
};

template <typename QUEUE>
void WaitableQueue<QUEUE>::Push(const typename QUEUE::value_type& data_)
{
    const std::lock_guard<std::mutex> lock(m_queue_guard);

    m_queue.push(data_);

    m_can_op.notify_one();
}

template <typename QUEUE>
void WaitableQueue<QUEUE>::Pop(typename QUEUE::value_type *out_)
{
    std::unique_lock<std::mutex> lock(m_queue_guard);
    m_can_op.wait(lock, [&]{return !m_queue.empty();});

    *out_ = m_queue.front();
    m_queue.pop();

    m_can_op.notify_one();
}

template <typename QUEUE>
bool WaitableQueue<QUEUE>::Pop(typename QUEUE::value_type *out_,
                                    const std::chrono::milliseconds& timeout_)
{
    std::unique_lock<std::mutex> lock(m_queue_guard);
    
    bool status = m_can_op.wait_for(lock, timeout_, [&]{return !m_queue.empty();});
    if (true == status)
    {
        *out_ = m_queue.front();
        m_queue.pop();
    }

    m_can_op.notify_one();

    return status;
}

template <typename QUEUE>
bool WaitableQueue<QUEUE>::IsEmpty() const
{
    const std::lock_guard<std::mutex> lock(m_queue_guard);
    
    m_can_op.notify_one();

    return m_queue.empty();
}
} // Infrastructure

#endif // __WAITABLE_QUEUE_HPP__