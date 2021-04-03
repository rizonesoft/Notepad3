// sktoolslib - common files for SK tools

// Copyright (C) 2020-2021 - Stefan Kueng

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//

#pragma once
#include <deque>
#include <functional>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>

//thread pool
class ThreadPool
{
public:
    ThreadPool(unsigned int n = std::thread::hardware_concurrency());

    /// add a new task to the pool.
    /// the task is added to a queue and worked on as soon
    /// as there's a thread ready.
    template <class F>
    void enqueue(F&& f);

    /// add a new task to be worked on.
    /// this method waits until a thread in the pool
    /// is not busy anymore, so the queue will not
    /// grow bigger than the thread pool is.
    template <class F>
    void enqueueWait(F&& f);

    /// waits for all threads to be finished
    void waitFinished();

    /// waits until the thread pool has at least
    /// one thread not busy
    void waitForFreeSlot();
    ~ThreadPool();

    unsigned int getProcessed() const { return m_processed; }

private:
    std::vector<std::thread>          m_workers;
    std::deque<std::function<void()>> m_tasks;
    std::mutex                        m_queueMutex;
    std::condition_variable           m_cvTask;
    std::condition_variable           m_cvFinished;
    std::atomic_uint                  m_processed;
    unsigned int                      m_busy;
    bool                              m_stop;

    void thread_proc();
};

inline ThreadPool::ThreadPool(unsigned int n)
    : m_processed(0)
    , m_busy(0)
    , m_stop(false)
{
    for (unsigned int i = 0; i < n; ++i)
        m_workers.emplace_back(std::bind(&ThreadPool::thread_proc, this));
}

inline ThreadPool::~ThreadPool()
{
    // set stop-condition
    std::unique_lock<std::mutex> latch(m_queueMutex);
    m_stop = true;
    m_cvTask.notify_all();
    latch.unlock();

    // all threads terminate, then we're done.
    for (auto& t : m_workers)
        t.join();
}

inline void ThreadPool::thread_proc()
{
    while (true)
    {
        std::unique_lock<std::mutex> latch(m_queueMutex);
        m_cvTask.wait(latch, [this]() { return m_stop || !m_tasks.empty(); });
        if (!m_tasks.empty())
        {
            ++m_busy;
            auto fn = m_tasks.front();
            m_tasks.pop_front();

            // release lock. run async
            latch.unlock();
            // run function outside context
            fn();
            ++m_processed;

            // lock again
            latch.lock();
            --m_busy;
            m_cvFinished.notify_one();
        }
        else if (m_stop)
        {
            break;
        }
    }
}

template <class F>
void ThreadPool::enqueue(F&& f)
{
    std::unique_lock<std::mutex> lock(m_queueMutex);
    m_tasks.emplace_back(std::forward<F>(f));
    m_cvTask.notify_one();
}

template <class F>
void ThreadPool::enqueueWait(F&& f)
{
    waitForFreeSlot();
    std::unique_lock<std::mutex> lock(m_queueMutex);
    m_tasks.emplace_back(std::forward<F>(f));
    m_cvTask.notify_one();
}

// waits until the queue is empty and all threads are idle.
inline void ThreadPool::waitFinished()
{
    std::unique_lock<std::mutex> lock(m_queueMutex);
    m_cvFinished.wait(lock, [this]() { return m_tasks.empty() && (m_busy == 0); });
}

// waits until there's at least one thread free in the pool.
inline void ThreadPool::waitForFreeSlot()
{
    std::unique_lock<std::mutex> lock(m_queueMutex);
    if ((m_busy < m_workers.size()) && (m_tasks.size() < m_workers.size()))
        return;
    m_cvFinished.wait(lock, [this]() { return ((m_busy < m_workers.size()) && (m_tasks.size() < m_workers.size())); });
}
