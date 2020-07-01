// grepWin - regex search and replace for Windows

// Copyright (C) 2007-2020 - Stefan Kueng

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
#include <thread>
#include <mutex>
#include <future>
#include <condition_variable>

// ---------------------------------------
// a semaphore class
// (until C++20  std::counting_semaphore)
// ---------------------------------------
class Semaphore
{
public:
    explicit Semaphore(unsigned int max_count = 1)
        : m_mtx()
        , m_cndvar()
        , m_max(max_count > 1 ? max_count : 1)
        , m_count(m_max)
    {
    }

    inline void setmaxcount(unsigned int max_count)
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        m_max = max_count > 1 ? max_count : 1;
        while (m_count < m_max)
        {
            ++m_count;
            m_cndvar.notify_one(); // notify the waiting thread
        }
    }

    inline void notify()
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        if (m_count < m_max)
        {
            ++m_count;
            m_cndvar.notify_one(); // notify the waiting thread
        }
    }

    inline void wait()
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        while (m_count == 0)
        {
            m_cndvar.wait(lock); // wait on the mutex until notify is called
        }
        --m_count;
    }

    inline void reset()
    {
        std::unique_lock<std::mutex> lock(m_mtx);
        m_count = m_max;
        m_cndvar.notify_all();
    }

private:
    mutable std::mutex      m_mtx;
    std::condition_variable m_cndvar;
    unsigned int            m_max;
    unsigned int            m_count;
};


// ---------------------------------------
// a thread-safe log of backup-file paths
// ---------------------------------------
class BackupAndTempFilesLog
{
public:
    inline void insert(const std::wstring& backupFilePath)
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_set.insert(backupFilePath);
    }
    // auto unlock (lock_guard, RAII)

    inline void clear()
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_set.clear();
    }
    // auto unlock (lock_guard, RAII)

    inline bool contains(const std::wstring& filePath)
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        return (m_set.find(filePath) != m_set.end());
    }
    // auto unlock (lock_guard, RAII)

private:
    mutable std::mutex     m_mtx;
    std::set<std::wstring> m_set;
};


// ---------------------------------------
// a thread-safe map of running search threads (futures)
// ---------------------------------------
class SearchThreadMap
{
public:

    explicit SearchThreadMap(unsigned int max_count = 1) : m_semaphore(max_count) { }

    inline void set_max_worker(unsigned int max_count)
    {
        m_semaphore.setmaxcount(max_count);
    }

    inline void insert_ready(std::shared_ptr<CSearchInfo> sInfoPtr, int nFound)
    {
        m_semaphore.wait();  // wait for registration
        std::lock_guard<std::mutex> lock(m_mtx);
        m_threadReadyMap.insert(std::make_pair(sInfoPtr, nFound));
    }

    inline void insert_future(std::shared_ptr<CSearchInfo> sInfoPtr, std::shared_future<int> sFuturePtr)
    {
        m_semaphore.wait(); // wait for registration
        std::lock_guard<std::mutex> lock(m_mtx);
        m_threadFutureMap.insert(std::make_pair(sInfoPtr, sFuturePtr));
    }

    inline bool empty()
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        return (m_threadReadyMap.empty() && m_threadFutureMap.empty());
    }

    // get next ready thread result
    inline std::shared_ptr<CSearchInfo> retrieve(int& nFound_out)
    {
        std::lock_guard<std::mutex> lock(m_mtx);

        if (!m_threadReadyMap.empty())
        {
            auto const head     = m_threadReadyMap.cbegin();
            auto const sInfoPtr = head->first;
            nFound_out          = head->second;
            m_threadReadyMap.erase(head);  // done
            m_semaphore.notify();          // allow register next thread
            return sInfoPtr;
        }

        for (auto it = m_threadFutureMap.cbegin(); it != m_threadFutureMap.cend() /* not hoisted */; /* no increment */)
        {
            std::future_status const status = (it->second).wait_for(std::chrono::milliseconds(1));

            if (status == std::future_status::ready)
            {
                auto const sInfoPtr  = it->first;
                nFound_out           = (it->second).get();
                m_threadFutureMap.erase(it);  // done
                m_semaphore.notify();         // allow register next thread
                return sInfoPtr;
            }
            else if (status == std::future_status::timeout)
            {
                ++it; // still running
            }
            else //if (status == std::future_status::deferred)
            {
                assert(status != std::future_status::ready);  // should not happen
                it = m_threadFutureMap.erase(it);             // no result
                m_semaphore.notify();                         // allow register next thread
            }
        }

        return nullptr;  // no thread ready yet
    }

    inline void clear()
    {
        m_semaphore.reset();

        std::lock_guard<std::mutex> lock(m_mtx);
        m_threadReadyMap.clear();
        m_threadFutureMap.clear();
    }

private:
    mutable std::mutex                                                        m_mtx;
    Semaphore                                                                 m_semaphore;
    std::unordered_map<std::shared_ptr<CSearchInfo>, int>                     m_threadReadyMap;
    std::unordered_map<std::shared_ptr<CSearchInfo>, std::shared_future<int>> m_threadFutureMap;
};

