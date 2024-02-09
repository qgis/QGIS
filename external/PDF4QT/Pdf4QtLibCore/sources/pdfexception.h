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
