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

#include "pdfcertificatemanager.h"

#include <QDir>
#include <QFile>
#include <QStandardPaths>

#include "pdfdbgheap.h"

#if defined(PDF4QT_COMPILER_MINGW) || defined(PDF4QT_COMPILER_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#if defined(PDF4QT_COMPILER_MSVC)
#pragma warning(push)
#pragma warning(disable: 4996)
#endif

#include <openssl/bio.h>
#include <openssl/rsa.h>
#include <openssl/rsaerr.h>
#include <openssl/x509.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/x509v3.h>
#include <openssl/pkcs12.h>

#include <memory>

namespace pdf
{

PDFCertificateManager::PDFCertificateManager()
{

}

template<typename T>
using openssl_ptr = std::unique_ptr<T, void(*)(T*)>;

void PDFCertificateManager::createCertificate(const NewCertificateInfo& info)
{
    openssl_ptr<BIO> pksBuffer(BIO_new(BIO_s_mem()), &BIO_free_all);

    if (pksBuffer)
    {
        openssl_ptr<BIGNUM> bignumber(BN_new(), &BN_free);
        openssl_ptr<RSA> rsaKey(RSA_new(), &RSA_free);

        BN_set_word(bignumber.get(), RSA_F4);
        const int rsaResult = RSA_generate_key_ex(rsaKey.get(), info.rsaKeyLength, bignumber.get(), nullptr);
        if (rsaResult)
        {
            openssl_ptr<X509> certificate(X509_new(), &X509_free);
            openssl_ptr<EVP_PKEY> privateKey(EVP_PKEY_new(), &EVP_PKEY_free);

            EVP_PKEY_set1_RSA(privateKey.get(), rsaKey.get());
            ASN1_INTEGER* serialNumber = X509_get_serialNumber(certificate.get());
            ASN1_INTEGER_set(serialNumber, info.serialNumber);

            // Set validity of the certificate
            X509_gmtime_adj(X509_getm_notBefore(certificate.get()), 0);
            X509_gmtime_adj(X509_getm_notAfter(certificate.get()), info.validityInSeconds);

            // Set name
            X509_NAME* name = X509_get_subject_name(certificate.get());

            auto addString = [name](const char* identifier, QString string)
            {
                if (string.isEmpty())
                {
                    return;
                }

                QByteArray stringUtf8 = string.toUtf8();
                X509_NAME_add_entry_by_txt(name, identifier, MBSTRING_UTF8, reinterpret_cast<const unsigned char*>(stringUtf8.constData()), stringUtf8.length(), -1, 0);
            };
            addString("C", info.certCountryCode);
            addString("O", info.certOrganization);
            addString("OU", info.certOrganizationUnit);
            addString("CN", info.certCommonName);
            addString("E", info.certEmail);

            X509_EXTENSION* extension = nullptr;
            X509V3_CTX context = { };
            X509V3_set_ctx_nodb(&context);
            X509V3_set_ctx(&context, certificate.get(), certificate.get(), nullptr, nullptr, 0);
            extension = X509V3_EXT_conf_nid (NULL, &context, NID_key_usage, "digitalSignature, keyAgreement");
            X509_add_ext(certificate.get(), extension, -1);
            X509_EXTENSION_free(extension);

            X509_set_issuer_name(certificate.get(), name);

            // Set public key
            X509_set_pubkey(certificate.get(), privateKey.get());
            X509_sign(certificate.get(), privateKey.get(), EVP_sha512());

            // Private key password
            QByteArray privateKeyPaswordUtf8 = info.privateKeyPasword.toUtf8();

            // Write the data
            openssl_ptr<PKCS12> pkcs12(PKCS12_create(privateKeyPaswordUtf8.constData(),
                                                     nullptr,
                                                     privateKey.get(),
                                                     certificate.get(),
                                                     nullptr,
                                                     0,
                                                     0,
                                                     PKCS12_DEFAULT_ITER,
                                                     PKCS12_DEFAULT_ITER,
                                                     0), &PKCS12_free);
            i2d_PKCS12_bio(pksBuffer.get(), pkcs12.get());

            BUF_MEM* pksMemoryBuffer = nullptr;
            BIO_get_mem_ptr(pksBuffer.get(), &pksMemoryBuffer);

            if (!info.fileName.isEmpty())
            {
                QFile file(info.fileName);
                if (file.open(QFile::WriteOnly | QFile::Truncate))
                {
                    file.write(pksMemoryBuffer->data, pksMemoryBuffer->length);
                    file.close();
                }
            }
        }
    }
}

PDFCertificateEntries PDFCertificateManager::getCertificates()
{
    PDFCertificateEntries entries = PDFCertificateStore::getPersonalCertificates();

    QDir directory(getCertificateDirectory());
    QFileInfoList pfxFiles = directory.entryInfoList(QStringList() << "*.pfx", QDir::Files | QDir::NoDotAndDotDot | QDir::Readable, QDir::Name);

    for (const QFileInfo& fileInfo : pfxFiles)
    {
        QFile file(fileInfo.absoluteFilePath());
        if (file.open(QFile::ReadOnly))
        {
            QByteArray data = file.readAll();

            openssl_ptr<BIO> pksBuffer(BIO_new(BIO_s_mem()), &BIO_free_all);
            BIO_write(pksBuffer.get(), data.constData(), data.length());

            openssl_ptr<PKCS12> pkcs12(d2i_PKCS12_bio(pksBuffer.get(), nullptr), &PKCS12_free);
            if (pkcs12)
            {
                X509* certificatePtr = nullptr;

                PDFCertificateEntry entry;

                // Parse PKCS12 with password
                bool isParsed = PKCS12_parse(pkcs12.get(), nullptr, nullptr, &certificatePtr, nullptr) == 1;
                if (isParsed)
                {
                    std::optional<PDFCertificateInfo> info = PDFCertificateInfo::getCertificateInfo(certificatePtr);
                    if (info)
                    {
                        entry.type = PDFCertificateEntry::EntryType::System;
                        entry.info = qMove(*info);
                    }
                }

                if (certificatePtr)
                {
                    X509_free(certificatePtr);
                }

                entry.pkcs12 = data;
                entry.pkcs12fileName = fileInfo.fileName();
                entries.emplace_back(qMove(entry));
            }

            file.close();
        }
    }

    return entries;
}

QString PDFCertificateManager::getCertificateDirectory()
{
    QString standardDataLocation = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation).front();
    QDir directory(standardDataLocation + "/certificates/");
    return directory.absolutePath();
}

QString PDFCertificateManager::generateCertificateFileName()
{
    QString directoryString = getCertificateDirectory();
    QDir directory(directoryString);

    int certificateIndex = 1;
    while (true)
    {
        QString fileName = directory.absoluteFilePath(QString("cert_%1.pfx").arg(certificateIndex++));
        if (!QFile::exists(fileName))
        {
            return fileName;
        }
    }

    return QString();
}

bool PDFCertificateManager::isCertificateValid(const PDFCertificateEntry& certificateEntry, QString password)
{
    QByteArray pkcs12data = certificateEntry.pkcs12;

    openssl_ptr<BIO> pksBuffer(BIO_new(BIO_s_mem()), &BIO_free_all);
    BIO_write(pksBuffer.get(), pkcs12data.constData(), pkcs12data.length());

    openssl_ptr<PKCS12> pkcs12(d2i_PKCS12_bio(pksBuffer.get(), nullptr), &PKCS12_free);
    if (pkcs12)
    {
        const char* passwordPointer = nullptr;
        QByteArray passwordByteArray = password.isEmpty() ? QByteArray() : password.toUtf8();
        if (!passwordByteArray.isEmpty())
        {
            passwordPointer = passwordByteArray.constData();
        }

        return PKCS12_parse(pkcs12.get(), passwordPointer, nullptr, nullptr, nullptr) == 1;
    }

    return pkcs12data.isEmpty();
}

bool PDFSignatureFactory::sign(const PDFCertificateEntry& certificateEntry,
                               QString password,
                               QByteArray data,
                               QByteArray& result)
{
    QByteArray pkcs12Data = certificateEntry.pkcs12;

    if (!pkcs12Data.isEmpty())
    {
        openssl_ptr<BIO> pkcs12Buffer(BIO_new(BIO_s_mem()), &BIO_free_all);
        BIO_write(pkcs12Buffer.get(), pkcs12Data.constData(), pkcs12Data.length());

        openssl_ptr<PKCS12> pkcs12(d2i_PKCS12_bio(pkcs12Buffer.get(), nullptr), &PKCS12_free);
        if (pkcs12)
        {
            const char* passwordPointer = nullptr;
            QByteArray passwordByteArray = password.isEmpty() ? QByteArray() : password.toUtf8();
            if (!passwordByteArray.isEmpty())
            {
                passwordPointer = passwordByteArray.constData();
            }

            EVP_PKEY* key = nullptr;
            X509* certificate = nullptr;
            STACK_OF(X509)* certificates = nullptr;
            if (PKCS12_parse(pkcs12.get(), passwordPointer, &key, &certificate, &certificates) == 1)
            {
                openssl_ptr<BIO> signedDataBuffer(BIO_new(BIO_s_mem()), &BIO_free_all);
                BIO_write(signedDataBuffer.get(), data.constData(), data.length());

                PKCS7* signature = PKCS7_sign(certificate, key, certificates, signedDataBuffer.get(), PKCS7_DETACHED | PKCS7_BINARY);
                if (signature)
                {
                    openssl_ptr<BIO> outputBuffer(BIO_new(BIO_s_mem()), &BIO_free_all);
                    i2d_PKCS7_bio(outputBuffer.get(), signature);

                    BUF_MEM* pksMemoryBuffer = nullptr;
                    BIO_get_mem_ptr(outputBuffer.get(), &pksMemoryBuffer);

                    result = QByteArray(pksMemoryBuffer->data, int(pksMemoryBuffer->length));

                    EVP_PKEY_free(key);
                    X509_free(certificate);
                    sk_X509_free(certificates);
                    return true;
                }

                EVP_PKEY_free(key);
                X509_free(certificate);
                sk_X509_free(certificates);
                return false;
            }
        }
    }
#ifdef Q_OS_WIN
    else
    {
        return signImpl_Win(certificateEntry, password, data, result);
    }
#endif

    return false;
}

}   // namespace pdf

#ifdef Q_OS_WIN
#include <Windows.h>
#include <wincrypt.h>
#include <ncrypt.h>
#if defined(PDF4QT_USE_PRAGMA_LIB)
#pragma comment(lib, "crypt32.lib")
#endif
#endif

bool pdf::PDFSignatureFactory::signImpl_Win(const pdf::PDFCertificateEntry& certificateEntry, QString password, QByteArray data, QByteArray& result)
{
    bool success = false;

#ifdef Q_OS_WIN
    Q_UNUSED(password);

    HCERTSTORE certStore = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, NULL, CERT_SYSTEM_STORE_CURRENT_USER, L"MY");
    if (certStore)
    {
        PCCERT_CONTEXT pCertContext = nullptr;

        while (pCertContext = CertEnumCertificatesInStore(certStore, pCertContext))
        {
            const unsigned char* pointer = pCertContext->pbCertEncoded;
            QByteArray testData(reinterpret_cast<const char*>(pointer), pCertContext->cbCertEncoded);

            if (testData == certificateEntry.info.getCertificateData())
            {
                break;
            }
        }

        if (pCertContext)
        {
            CRYPT_SIGN_MESSAGE_PARA SignParams{};
            BYTE* pbSignedBlob = nullptr;
            DWORD cbSignedBlob = 0;
            PCCERT_CONTEXT pCertContextArray[1] = { pCertContext };

            const BYTE* pbDataToBeSigned = (const BYTE*)data.constData();
            DWORD cbDataToBeSigned = (DWORD)data.size();

            // Nastavení parametrů pro podpis
            SignParams.cbSize = sizeof(CRYPT_SIGN_MESSAGE_PARA);
            SignParams.dwMsgEncodingType = PKCS_7_ASN_ENCODING | X509_ASN_ENCODING;
            SignParams.pSigningCert = pCertContext;
            SignParams.HashAlgorithm.pszObjId = (LPSTR)szOID_RSA_SHA256RSA;
            SignParams.HashAlgorithm.Parameters.cbData = 0;
            SignParams.HashAlgorithm.Parameters.pbData = NULL;
            SignParams.cMsgCert = 1;
            SignParams.rgpMsgCert = pCertContextArray;
            pCertContextArray[0] = pCertContext;

            const BYTE* rgpbToBeSigned[1] = {pbDataToBeSigned};
            DWORD rgcbToBeSigned[1] = {cbDataToBeSigned};

            // Retrieve signed message size
            CryptSignMessage(
                &SignParams,
                TRUE,
                1,
                rgpbToBeSigned,
                rgcbToBeSigned,
                NULL,
                &cbSignedBlob
                );

            pbSignedBlob = new BYTE[cbSignedBlob];

            // Create digital signature
            if (CryptSignMessage(
                    &SignParams,
                    TRUE,
                    1,
                    rgpbToBeSigned,
                    rgcbToBeSigned,
                    pbSignedBlob,
                    &cbSignedBlob
                    ))
            {
                result = QByteArray((const char*)pbSignedBlob, cbSignedBlob);
                success = true;
            }

            delete[] pbSignedBlob;

            CertFreeCertificateContext(pCertContext);
        }

        CertCloseStore(certStore, CERT_CLOSE_STORE_FORCE_FLAG);
    }
#else
    Q_UNUSED(certificateEntry);
    Q_UNUSED(password);
    Q_UNUSED(data);
    Q_UNUSED(result);
#endif

    return success;
}

#if defined(PDF4QT_COMPILER_MINGW) || defined(PDF4QT_COMPILER_GCC)
#pragma GCC diagnostic pop
#endif

#if defined(PDF4QT_COMPILER_MSVC)
#pragma warning(pop)
#endif
