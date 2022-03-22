/******************************************************************************
 * Copyright (c) 2018, Connor Manning
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following
 * conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of the Martin Isenburg or Iowa Department
 *       of Natural Resources nor the names of its contributors may be
 *       used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 ****************************************************************************/

#include <iostream>

#include "ThreadPool.hpp"

namespace untwine
{

void ThreadPool::go()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_running)
        return;

    m_running = true;

    for (std::size_t i(0); i < m_numThreads; ++i)
    {
        m_threads.emplace_back([this]() { work(); });
    }
}

void ThreadPool::work()
{
    while (true)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_consumeCv.wait(lock, [this]()
        {
            return m_tasks.size() || !m_running;
        });

        if (m_tasks.size())
        {
            ++m_outstanding;
            auto task(std::move(m_tasks.front()));
            m_tasks.pop();

            lock.unlock();

            // Notify add(), which may be waiting for a spot in the queue.
            m_produceCv.notify_all();

            std::string err;

            if (m_trap)
            {
                try
                {
                    task();
                }
                catch (std::exception& e)
                {
                    err = e.what();
                }
                catch (...)
                {
                    err = m_catchall;
                }
            }
            else
                task();

            lock.lock();
            --m_outstanding;
            if (err.size())
            {
                if (m_verbose)
                    std::cout << "Exception in pool task: " << err << std::endl;
                m_errors.push_back(err);
            }
            lock.unlock();

            // Notify await(), which may be waiting for a running task.
            m_produceCv.notify_all();
        }
        else if (!m_running)
        {
            return;
        }
    }
}

} // namespace untwine

