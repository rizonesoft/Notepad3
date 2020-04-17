// sktoolslib - common files for SK tools

// Copyright (C) 2020 - Stefan Kueng

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
#include <iostream>
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

    unsigned int getProcessed() const { return processed; }

private:
    std::vector<std::thread>          workers;
    std::deque<std::function<void()>> tasks;
    std::mutex                        queue_mutex;
    std::condition_variable           cv_task;
    std::condition_variable           cv_finished;
    std::atomic_uint                  processed;
    unsigned int                      busy;
    bool                              stop;

    void thread_proc();
};

ThreadPool::ThreadPool(unsigned int n)
    : busy(0)
    , processed(0)
    , stop(false)
{
    for (unsigned int i = 0; i < n; ++i)
        workers.emplace_back(std::bind(&ThreadPool::thread_proc, this));
}

ThreadPool::~ThreadPool()
{
    // set stop-condition
    std::unique_lock<std::mutex> latch(queue_mutex);
    stop = true;
    cv_task.notify_all();
    latch.unlock();

    // all threads terminate, then we're done.
    for (auto& t : workers)
        t.join();
}

void ThreadPool::thread_proc()
{
    while (true)
    {
        std::unique_lock<std::mutex> latch(queue_mutex);
        cv_task.wait(latch, [this]() { return stop || !tasks.empty(); });
        if (!tasks.empty())
        {
            ++busy;
            auto fn = tasks.front();
            tasks.pop_front();

            // release lock. run async
            latch.unlock();
            // run function outside context
            fn();
            ++processed;

            // lock again
            latch.lock();
            --busy;
            cv_finished.notify_one();
        }
        else if (stop)
        {
            break;
        }
    }
}

template <class F>
void ThreadPool::enqueue(F&& f)
{
    std::unique_lock<std::mutex> lock(queue_mutex);
    tasks.emplace_back(std::forward<F>(f));
    cv_task.notify_one();
}

template <class F>
void ThreadPool::enqueueWait(F&& f)
{
    waitForFreeSlot();
    std::unique_lock<std::mutex> lock(queue_mutex);
    tasks.emplace_back(std::forward<F>(f));
    cv_task.notify_one();
}

// waits until the queue is empty and all threads are idle.
void ThreadPool::waitFinished()
{
    std::unique_lock<std::mutex> lock(queue_mutex);
    cv_finished.wait(lock, [this]() { return tasks.empty() && (busy == 0); });
}

// waits until there's at least one thread free in the pool.
void ThreadPool::waitForFreeSlot()
{
    std::unique_lock<std::mutex> lock(queue_mutex);
    if ((busy < workers.size()) && (tasks.size() < workers.size()))
        return;
    cv_finished.wait(lock, [this]() { return ((busy < workers.size()) && (tasks.size() < workers.size())); });
}
