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

#ifndef PDFPROGRESS_H
#define PDFPROGRESS_H

#include "pdfglobal.h"

#include <QObject>

#include <atomic>

namespace pdf
{

struct ProgressStartupInfo
{
    bool showDialog = false;
    QString text;
};

class PDF4QTLIBCORESHARED_EXPORT PDFProgress : public QObject
{
    Q_OBJECT

public:
    explicit PDFProgress(QObject* parent);

    void start(size_t stepCount, ProgressStartupInfo startupInfo);
    void step();
    void finish();

signals:
    // Namespace pdf:: must be here, due to queued signals calling
    void progressStarted(pdf::ProgressStartupInfo info);
    void progressStep(int percentage);
    void progressFinished();

private:
    std::atomic<size_t> m_currentStep = 0;
    std::atomic<size_t> m_stepCount = 0;
    std::atomic<int> m_percentage = 0;
};

}   // namespace pdf

Q_DECLARE_METATYPE(pdf::ProgressStartupInfo)

#endif // PDFPROGRESS_H
