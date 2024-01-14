//    Copyright (C) 2019-2022 Jakub Melka
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
