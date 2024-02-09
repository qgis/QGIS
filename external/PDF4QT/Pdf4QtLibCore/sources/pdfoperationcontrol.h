//    Copyright (C) 2022 Jakub Melka
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

#ifndef PDFOPERATIONCONTROL_H
#define PDFOPERATIONCONTROL_H

#include "pdfglobal.h"

namespace pdf
{

/// Operation controller. This interface can be used for
/// long operation interruption - so operation is cancelled
/// immediately.
class PDFOperationControl
{
public:
    constexpr PDFOperationControl() = default;
    virtual ~PDFOperationControl() = default;

    /// Returns true, if operation is cancelled and
    /// processed data should be abandoned. It is safe
    /// to call this function in another thread, implementators
    /// of this interface must ensure thread safety.
    /// If this function returns true, it must return true until
    /// all operations are stopped and correctly handled.
    virtual bool isOperationCancelled() const = 0;

    static inline bool isOperationCancelled(const PDFOperationControl* operationControl)
    {
        return operationControl && operationControl->isOperationCancelled();
    }
};

}

#endif // PDFOPERATIONCONTROL_H
