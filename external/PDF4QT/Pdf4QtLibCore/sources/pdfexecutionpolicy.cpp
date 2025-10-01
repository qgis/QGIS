// MIT License
//
// Copyright (c) 2018-2025 Jakub Melka and Contributors
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "pdfexecutionpolicy.h"

#include <QThread>
#include <QCoreApplication>

#include "pdfdbgheap.h"

namespace pdf
{

struct PDFExecutionPolicyHolder
{
    PDFExecutionPolicyHolder()
    {
        qAddPostRoutine(&PDFExecutionPolicy::finalize);
    }
    ~PDFExecutionPolicyHolder()
    {
        auxiliary.waitForDone();
        primary.waitForDone();
    }

    PDFExecutionPolicy policy;
    QThreadPool primary;
    QThreadPool auxiliary;
} s_execution_policy;

void PDFExecutionPolicy::setStrategy(Strategy strategy)
{
    s_execution_policy.policy.m_strategy.store(strategy, std::memory_order_relaxed);
}

bool PDFExecutionPolicy::isParallelizing(Scope scope)
{
    const Strategy strategy = s_execution_policy.policy.m_strategy.load(std::memory_order_relaxed);
    switch (strategy)
    {
        case Strategy::SingleThreaded:
            return false;

        case Strategy::PageMultithreaded:
        {
            switch (scope)
            {
                case Scope::Page:
                case Scope::Unknown:
                    return true; // We are parallelizing pages...

                case Scope::Content:
                    return false;
            }

            break;
        }

        case Strategy::AlwaysMultithreaded:
            return true;
    }

    // It should never go here...
    Q_ASSERT(false);
    return false;
}

int PDFExecutionPolicy::getActiveThreadCount(Scope scope)
{
    return getThreadPool(scope)->activeThreadCount();
}

int PDFExecutionPolicy::getMaxThreadCount(Scope scope)
{
    return getThreadPool(scope)->maxThreadCount();
}

void PDFExecutionPolicy::setMaxThreadCount(Scope scope, int count)
{
    // Sanitize value!
    count = qMax(count, 1);
    getThreadPool(scope)->setMaxThreadCount(count);
}

int PDFExecutionPolicy::getIdealThreadCount(Scope scope)
{
    Q_UNUSED(scope);
    return QThread::idealThreadCount();
}

int PDFExecutionPolicy::getContentStreamCount()
{
    return s_execution_policy.policy.m_contentStreamsCount.load(std::memory_order_relaxed);
}

void PDFExecutionPolicy::startProcessingContentStream()
{
    ++s_execution_policy.policy.m_contentStreamsCount;
}

void PDFExecutionPolicy::endProcessingContentStream()
{
    --s_execution_policy.policy.m_contentStreamsCount;
}

void PDFExecutionPolicy::finalize()
{
    s_execution_policy.auxiliary.waitForDone();
    s_execution_policy.primary.waitForDone();
}

QThreadPool* PDFExecutionPolicy::getThreadPool(PDFExecutionPolicy::Scope scope)
{
    switch (scope)
    {
        case Scope::Page:
        case Scope::Unknown:
            return &s_execution_policy.primary;

        case Scope::Content:
            return &s_execution_policy.auxiliary;

        default:
            Q_ASSERT(false);
            break;
    }

    return nullptr;
}

PDFExecutionPolicy::PDFExecutionPolicy() :
    m_contentStreamsCount(0),
    m_strategy(Strategy::PageMultithreaded)
{

}

}   // namespace pdf
