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

#ifndef PDFCERTIFICATEMANAGER_H
#define PDFCERTIFICATEMANAGER_H

#include "pdfglobal.h"
#include "pdfcertificatestore.h"

#include <QString>
#include <QFileInfoList>

namespace pdf
{

class PDF4QTLIBCORESHARED_EXPORT PDFCertificateManager
{
public:
    PDFCertificateManager();

    struct NewCertificateInfo
    {
        QString fileName;
        QString privateKeyPasword;

        QString certCountryCode;
        QString certOrganization;
        QString certOrganizationUnit;
        QString certCommonName;
        QString certEmail;

        int rsaKeyLength = 1024;
        int validityInSeconds = 2 * 365 * 24 * 3600;
        long serialNumber = 1;
    };

    void createCertificate(const NewCertificateInfo& info);

    static PDFCertificateEntries getCertificates();
    static QString getCertificateDirectory();
    static QString generateCertificateFileName();
    static bool isCertificateValid(const PDFCertificateEntry& certificateEntry, QString password);
};

class PDF4QTLIBCORESHARED_EXPORT PDFSignatureFactory
{
public:
    static bool sign(const PDFCertificateEntry& certificateEntry,
                     QString password,
                     QByteArray data,
                     QByteArray& result);

private:
    static bool signImpl_Win(const PDFCertificateEntry& certificateEntry,
                             QString password,
                             QByteArray data,
                             QByteArray& result);
};

} // namespace pdf

#endif // PDFCERTIFICATEMANAGER_H
