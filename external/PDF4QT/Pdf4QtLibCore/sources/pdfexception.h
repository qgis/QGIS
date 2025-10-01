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

#ifndef PDFEXCEPTION_H
#define PDFEXCEPTION_H

#include <QString>

namespace pdf
{

class PDFException : public std::exception
{
public:
    PDFException(const QString& message) :
        m_message(message)
    {

    }

    /// Returns error message
    const QString& getMessage() const { return m_message; }

private:
    QString m_message;
};

enum RenderErrorType
{
    Error,
    Warning,
    NotImplemented,
    NotSupported,
    Information
};

struct PDFRenderError
{
    explicit PDFRenderError() = default;
    explicit PDFRenderError(RenderErrorType type, QString message) :
        type(type),
        message(std::move(message))
    {

    }

    RenderErrorType type = RenderErrorType::Error;
    QString message;
};

class PDFRendererException : public std::exception
{
public:
    explicit PDFRendererException(RenderErrorType type, QString message) :
        m_error(type, std::move(message))
    {

    }

    const PDFRenderError& getError() const { return m_error; }

private:
    PDFRenderError m_error;
};

/// Abstract class for reporting render errors
class PDFRenderErrorReporter
{
public:
    explicit PDFRenderErrorReporter() = default;
    virtual ~PDFRenderErrorReporter() = default;

    /// Reports render errors
    /// \param type Error type
    /// \param message Error message
    virtual void reportRenderError(RenderErrorType type, QString message) = 0;

    /// Reports render error, but only once - if same error was already reported,
    /// then no new error is reported.
    /// \param type Error type
    /// \param message Error message
    virtual void reportRenderErrorOnce(RenderErrorType type, QString message) = 0;
};

/// Dummy class for reporting render errors
class PDFRenderErrorReporterDummy : public PDFRenderErrorReporter
{
public:
    virtual void reportRenderError(RenderErrorType type, QString message) override
    {
        Q_UNUSED(type);
        Q_UNUSED(message);
    }

    virtual void reportRenderErrorOnce(RenderErrorType type, QString message) override
    {
        Q_UNUSED(type);
        Q_UNUSED(message);
    }
};

}   // namespace pdf

#endif // PDFEXCEPTION_H
