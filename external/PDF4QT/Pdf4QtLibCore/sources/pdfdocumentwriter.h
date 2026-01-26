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

#ifndef PDFDOCUMENTWRITER_H
#define PDFDOCUMENTWRITER_H

#include "pdfdocument.h"
#include "pdfprogress.h"
#include "pdfutils.h"

#include <QIODevice>

namespace pdf
{

/// Class used for writing PDF documents to the desired target device (or file,
/// buffer, etc.). If writing is not successful, then error message is returned.
class PDF4QTLIBCORESHARED_EXPORT PDFDocumentWriter
{
    Q_DECLARE_TR_FUNCTIONS(pdf::PDFDocumentWriter)

public:
    explicit inline PDFDocumentWriter(PDFProgress* progress)
    {
        Q_UNUSED(progress);
    }

    /// Writes document to the file. If \p safeWrite is true, then document is first
    /// written to the temporary file, and then renamed to original file name atomically,
    /// so no data can be lost on, for example, power failure. If it is not possible to
    /// create temporary file, then writing operation will attempt to write to the file
    /// directly.
    /// \param fileName File name
    /// \param document Document
    /// \param safeWrite Write document to the temporary file and then rename
    PDFOperationResult write(const QString& fileName, const PDFDocument* document, bool safeWrite);

    /// Write document to the output device. Device must be writable (i.e. opened
    /// for writing).
    /// \param device Output device
    /// \param document Document
    PDFOperationResult write(QIODevice* device, const PDFDocument* document);

    /// Calculates document file size, as if it is written to the disk.
    /// No file is accessed by this function; document is written
    /// to fake stream, which counts operations. If error occurs, and
    /// size can't be determined, then -1 is returned.
    /// \param document Document
    static qint64 getDocumentFileSize(const PDFDocument* document);

    /// Calculates size estimate of an object. If object is null, then zero is returned.
    /// \param document Document
    /// \param reference Reference
    static qint64 getObjectSize(const PDFDocument* document, PDFObjectReference reference);

    /// Writes an object to byte array, without object header/footer
    /// \param object Object to be written
    static QByteArray getSerializedObject(const PDFObject& object);

private:
    static void writeCRLF(QIODevice* device);
    static void writeObjectHeader(QIODevice* device, PDFObjectReference reference);
    static void writeObjectFooter(QIODevice* device);
};

}   // namespace pdf

#endif // PDFDOCUMENTWRITER_H
