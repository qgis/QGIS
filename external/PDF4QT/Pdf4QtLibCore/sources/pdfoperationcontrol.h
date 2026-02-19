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
