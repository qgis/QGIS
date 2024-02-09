//    Copyright (C) 2019-2021 Jakub Melka
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
