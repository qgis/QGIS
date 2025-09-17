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

#include "pdfprogress.h"
#include "pdfdbgheap.h"

namespace pdf
{

PDFProgress::PDFProgress(QObject* parent) :
    QObject(parent)
{
    qRegisterMetaType<pdf::ProgressStartupInfo>();
}

void PDFProgress::start(size_t stepCount, ProgressStartupInfo startupInfo)
{
    Q_ASSERT(stepCount > 0);

    m_currentStep = 0;
    m_stepCount = stepCount;
    m_percentage = 0;

    Q_EMIT progressStarted(qMove(startupInfo));
}

void PDFProgress::step()
{
    // Atomically increment by one. We must add + 1 to the current step,
    // because fetch_add will return old value. Then we must test, if percentage
    // has been changed.
    size_t currentStep = m_currentStep.fetch_add(1) + 1;

    int newPercentage = int((size_t(100) * currentStep) / m_stepCount);
    int oldPercentage = m_percentage.load(std::memory_order_acquire);
    bool emitSignal = oldPercentage < newPercentage;
    do
    {
        emitSignal = oldPercentage < newPercentage;
    } while (oldPercentage < newPercentage && !m_percentage.compare_exchange_weak(oldPercentage, newPercentage, std::memory_order_release, std::memory_order_relaxed));

    if (emitSignal)
    {
        Q_EMIT progressStep(newPercentage);
    }
}

void PDFProgress::finish()
{
    Q_EMIT progressFinished();
}

} // namespace pdf
