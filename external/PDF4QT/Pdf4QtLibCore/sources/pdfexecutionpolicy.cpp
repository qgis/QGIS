//    Copyright (C) 2020-2023 Jakub Melka
//
//    This file is part of PDF4QT.
//
//    PDF4QT is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    with the written consent of the copyright owner, any later version.
//
//    PDF4QT is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public License
//    along with PDF4QT.  If not, see <https://www.gnu.org/licenses/>.

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
