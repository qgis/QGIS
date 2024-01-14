//    Copyright (C) 2022-2023 Jakub Melka
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

#include "pdfcertificatestore.h"
#include "pdfutils.h"

#if defined(PDF4QT_COMPILER_MINGW) || defined(PDF4QT_COMPILER_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#if defined(PDF4QT_COMPILER_MSVC)
#pragma warning(push)
#pragma warning(disable: 4996)
#endif

#include <openssl/err.h>
#include <openssl/sha.h>
#include <openssl/rsa.h>
#include <openssl/rsaerr.h>
#include <openssl/ts.h>
#include <openssl/tserr.h>

#include <QDir>
#include <QFileInfo>
#include <QLockFile>
#include <QDataStream>
#include <QStandardPaths>

#include "pdfdbgheap.h"

#include <array>
#ifdef Q_OS_UNIX
#include <time.h>
#endif

namespace pdf
{

QRecursiveMutex PDFOpenSSLGlobalLock::s_globalOpenSSLMutex;

PDFOpenSSLGlobalLock::PDFOpenSSLGlobalLock() :
    m_mutexLocker(&s_globalOpenSSLMutex)
{

}

void PDFCertificateEntry::serialize(QDataStream& stream) const
{
    stream << persist_version;
    stream << type;
    stream << info;
}

void PDFCertificateEntry::deserialize(QDataStream& stream)
{
    int persistVersionDeserialized = 0;
    stream >> persistVersionDeserialized;
    stream >> type;
    stream >> info;
}


void PDFCertificateInfo::serialize(QDataStream& stream) const
{
    stream << persist_version;
    stream << m_version;
    stream << m_keySize;
    stream << m_publicKey;
    stream << m_nameEntries;
    stream << m_notValidBefore;
    stream << m_notValidAfter;
    stream << m_keyUsage;
    stream << m_certificateData;
}

void PDFCertificateInfo::deserialize(QDataStream& stream)
{
    int persistVersionDeserialized = 0;
    stream >> persistVersionDeserialized;
    stream >> m_version;
    stream >> m_keySize;
    stream >> m_publicKey;
    stream >> m_nameEntries;
    stream >> m_notValidBefore;
    stream >> m_notValidAfter;
    stream >> m_keyUsage;
    stream >> m_certificateData;
}

QDateTime PDFCertificateInfo::getNotValidBefore() const
{
    return m_notValidBefore;
}

void PDFCertificateInfo::setNotValidBefore(const QDateTime& notValidBefore)
{
    m_notValidBefore = notValidBefore;
}

QDateTime PDFCertificateInfo::getNotValidAfter() const
{
    return m_notValidAfter;
}

void PDFCertificateInfo::setNotValidAfter(const QDateTime& notValidAfter)
{
    m_notValidAfter = notValidAfter;
}

int32_t PDFCertificateInfo::getVersion() const
{
    return m_version;
}

void PDFCertificateInfo::setVersion(int32_t version)
{
    m_version = version;
}

PDFCertificateInfo::PublicKey PDFCertificateInfo::getPublicKey() const
{
    return m_publicKey;
}

void PDFCertificateInfo::setPublicKey(const PublicKey& publicKey)
{
    m_publicKey = publicKey;
}

int PDFCertificateInfo::getKeySize() const
{
    return m_keySize;
}

void PDFCertificateInfo::setKeySize(int keySize)
{
    m_keySize = keySize;
}

PDFCertificateInfo::KeyUsageFlags PDFCertificateInfo::getKeyUsage() const
{
    return m_keyUsage;
}

void PDFCertificateInfo::setKeyUsage(KeyUsageFlags keyUsage)
{
    m_keyUsage = keyUsage;
}

std::optional<PDFCertificateInfo> PDFCertificateInfo::getCertificateInfo(const QByteArray& certificateData)
{
    std::optional<PDFCertificateInfo> result;

    PDFOpenSSLGlobalLock lock;
    const unsigned char* data = convertByteArrayToUcharPtr(certificateData);
    if (X509* certificate = d2i_X509(nullptr, &data, certificateData.length()))
    {
        result = getCertificateInfo(certificate);
        X509_free(certificate);
    }

    return result;
}

PDFCertificateInfo PDFCertificateInfo::getCertificateInfo(x509_st* certificate)
{
    PDFCertificateInfo info;

    auto getStringFromASN1_STRING = [](ASN1_STRING* string) -> QString
    {
        QString result;

        if (string)
        {
            // Jakub Melka: we must convert entry to UTF8 encoding using function ASN1_STRING_to_UTF8
            unsigned char* utf8Buffer = nullptr;
            int errorCodeOrLength = ASN1_STRING_to_UTF8(&utf8Buffer, string);
            if (errorCodeOrLength > 0)
            {
                result = QString::fromUtf8(reinterpret_cast<const char*>(utf8Buffer), errorCodeOrLength);
            }
            OPENSSL_free(utf8Buffer);
        }

        return result;
    };

    auto getStringFromX509Name = [&getStringFromASN1_STRING](X509_NAME* name, int nid) -> QString
    {
        QString result;

        const int stringLocation = X509_NAME_get_index_by_NID(name, nid, -1);
        X509_NAME_ENTRY* entry = X509_NAME_get_entry(name, stringLocation);
        return getStringFromASN1_STRING(X509_NAME_ENTRY_get_data(entry));
    };

    auto getDateTimeFromASN = [](const ASN1_TIME* time) -> QDateTime
    {
        QDateTime result;

        if (time)
        {
            tm internalTime = { };
            if (ASN1_TIME_to_tm(time, &internalTime) > 0)
            {
#if defined(Q_OS_WIN)
                time_t localTime = _mkgmtime(&internalTime);
#elif defined(Q_OS_UNIX)
                time_t localTime = timegm(&internalTime);
#else
                static_assert(false, "Implement this for another OS!");
#endif
                result = QDateTime::fromSecsSinceEpoch(localTime, Qt::UTC);
            }
        }

        return result;
    };

    if (X509_NAME* subjectName = X509_get_subject_name(certificate))
    {
        // List of these properties are in RFC 5280, section 4.1.2.4, these attributes
        // are standard and all implementations must be prepared to process them.
        QString countryName = getStringFromX509Name(subjectName, NID_countryName);
        QString organizationName = getStringFromX509Name(subjectName, NID_organizationName);
        QString organizationalUnitName = getStringFromX509Name(subjectName, NID_organizationalUnitName);
        QString distinguishedName = getStringFromX509Name(subjectName, NID_distinguishedName);
        QString stateOrProvinceName = getStringFromX509Name(subjectName, NID_stateOrProvinceName);
        QString commonName = getStringFromX509Name(subjectName, NID_commonName);
        QString serialNumber = getStringFromX509Name(subjectName, NID_serialNumber);

        // These attributes are defined also in section 4.1.2.4, they are not mandatory,
        // but application should be able to process them.
        QString localityName = getStringFromX509Name(subjectName, NID_localityName);
        QString title = getStringFromX509Name(subjectName, NID_title);
        QString surname = getStringFromX509Name(subjectName, NID_surname);
        QString givenName = getStringFromX509Name(subjectName, NID_givenName);
        QString initials = getStringFromX509Name(subjectName, NID_initials);
        QString pseudonym = getStringFromX509Name(subjectName, NID_pseudonym);
        QString generationQualifier = getStringFromX509Name(subjectName, NID_generationQualifier);

        // This entry is not defined in section 4.1.2.4, but is commonly used
        QString email = getStringFromX509Name(subjectName, NID_pkcs9_emailAddress);

        info.setName(PDFCertificateInfo::CountryName, qMove(countryName));
        info.setName(PDFCertificateInfo::OrganizationName, qMove(organizationName));
        info.setName(PDFCertificateInfo::OrganizationalUnitName, qMove(organizationalUnitName));
        info.setName(PDFCertificateInfo::DistinguishedName, qMove(distinguishedName));
        info.setName(PDFCertificateInfo::StateOrProvinceName, qMove(stateOrProvinceName));
        info.setName(PDFCertificateInfo::CommonName, qMove(commonName));
        info.setName(PDFCertificateInfo::SerialNumber, qMove(serialNumber));

        info.setName(PDFCertificateInfo::LocalityName, qMove(localityName));
        info.setName(PDFCertificateInfo::Title, qMove(title));
        info.setName(PDFCertificateInfo::Surname, qMove(surname));
        info.setName(PDFCertificateInfo::GivenName, qMove(givenName));
        info.setName(PDFCertificateInfo::Initials, qMove(initials));
        info.setName(PDFCertificateInfo::Pseudonym, qMove(pseudonym));
        info.setName(PDFCertificateInfo::GenerationalQualifier, qMove(generationQualifier));

        info.setName(PDFCertificateInfo::Email, qMove(email));

        const long version = X509_get_version(certificate);
        info.setVersion(version);

        const ASN1_TIME* notBeforeTime = X509_get0_notBefore(certificate);
        const ASN1_TIME* notAfterTime = X509_get0_notAfter(certificate);

        info.setNotValidBefore(getDateTimeFromASN(notBeforeTime));
        info.setNotValidAfter(getDateTimeFromASN(notAfterTime));

        X509_PUBKEY* publicKey = X509_get_X509_PUBKEY(certificate);
        EVP_PKEY* evpKey = X509_PUBKEY_get(publicKey);
        const int keyType = EVP_PKEY_type(EVP_PKEY_base_id(evpKey));

        PDFCertificateInfo::PublicKey key = PDFCertificateInfo::KeyUnknown;
        switch (keyType)
        {
        case EVP_PKEY_RSA:
            key = PDFCertificateInfo::KeyRSA;
            break;

        case EVP_PKEY_DSA:
            key = PDFCertificateInfo::KeyDSA;
            break;

        case EVP_PKEY_DH:
            key = PDFCertificateInfo::KeyDH;
            break;

        case EVP_PKEY_EC:
            key = PDFCertificateInfo::KeyEC;
            break;

        default:
            break;
        }
        info.setPublicKey(key);

        const int bits = EVP_PKEY_bits(evpKey);
        info.setKeySize(bits);

        uint32_t keyUsage = X509_get_key_usage(certificate);
        if (keyUsage != UINT32_MAX)
        {
            static_assert(PDFCertificateInfo::KeyUsageDigitalSignature    == KU_DIGITAL_SIGNATURE, "Fix this code!");
            static_assert(PDFCertificateInfo::KeyUsageNonRepudiation      == KU_NON_REPUDIATION, "Fix this code!");
            static_assert(PDFCertificateInfo::KeyUsageKeyEncipherment     == KU_KEY_ENCIPHERMENT, "Fix this code!");
            static_assert(PDFCertificateInfo::KeyUsageDataEncipherment    == KU_DATA_ENCIPHERMENT, "Fix this code!");
            static_assert(PDFCertificateInfo::KeyUsageAgreement           == KU_KEY_AGREEMENT, "Fix this code!");
            static_assert(PDFCertificateInfo::KeyUsageCertSign            == KU_KEY_CERT_SIGN, "Fix this code!");
            static_assert(PDFCertificateInfo::KeyUsageCrlSign             == KU_CRL_SIGN, "Fix this code!");
            static_assert(PDFCertificateInfo::KeyUsageEncipherOnly        == KU_ENCIPHER_ONLY, "Fix this code!");
            static_assert(PDFCertificateInfo::KeyUsageDecipherOnly        == KU_DECIPHER_ONLY, "Fix this code!");

            if (X509_get_extension_flags(certificate) & EXFLAG_XKUSAGE)
            {
                const uint32_t extendedKeyUsage = X509_get_extended_key_usage(certificate);
                Q_ASSERT(extendedKeyUsage != UINT32_MAX);

                static_assert(PDFCertificateInfo::KeyUsageExtended_SSL_SERVER  >> 16 == XKU_SSL_SERVER, "Fix this code!");
                static_assert(PDFCertificateInfo::KeyUsageExtended_SSL_CLIENT  >> 16 == XKU_SSL_CLIENT, "Fix this code!");
                static_assert(PDFCertificateInfo::KeyUsageExtended_SMIME       >> 16 == XKU_SMIME, "Fix this code!");
                static_assert(PDFCertificateInfo::KeyUsageExtended_CODE_SIGN   >> 16 == XKU_CODE_SIGN, "Fix this code!");
                static_assert(PDFCertificateInfo::KeyUsageExtended_SGC         >> 16 == XKU_SGC, "Fix this code!");
                static_assert(PDFCertificateInfo::KeyUsageExtended_OCSP_SIGN   >> 16 == XKU_OCSP_SIGN, "Fix this code!");
                static_assert(PDFCertificateInfo::KeyUsageExtended_TIMESTAMP   >> 16 == XKU_TIMESTAMP, "Fix this code!");
                static_assert(PDFCertificateInfo::KeyUsageExtended_DVCS        >> 16 == XKU_DVCS, "Fix this code!");
                static_assert(PDFCertificateInfo::KeyUsageExtended_ANYEKU      >> 16 == XKU_ANYEKU, "Fix this code!");

                keyUsage = keyUsage | (extendedKeyUsage << 16);
            }

            info.setKeyUsage(static_cast<PDFCertificateInfo::KeyUsageFlags>(keyUsage));
        }

        unsigned char* buffer = nullptr;
        int length = i2d_X509(certificate, &buffer);
        if (length >= 0)
        {
            Q_ASSERT(buffer);
            info.setCertificateData(QByteArray(reinterpret_cast<const char*>(buffer), length));
            OPENSSL_free(buffer);
        }
    }

    return info;
}

QByteArray PDFCertificateInfo::getCertificateData() const
{
    return m_certificateData;
}

void PDFCertificateInfo::setCertificateData(const QByteArray& certificateData)
{
    m_certificateData = certificateData;
}


void PDFCertificateStore::serialize(QDataStream& stream) const
{
    stream << persist_version;
    stream << m_certificates;
}

void PDFCertificateStore::deserialize(QDataStream& stream)
{
    int persistVersionDeserialized = 0;
    stream >> persistVersionDeserialized;
    stream >> m_certificates;
}

bool PDFCertificateStore::add(PDFCertificateEntry::EntryType type, const QByteArray& certificate)
{
    if (auto certificateInfo = PDFCertificateInfo::getCertificateInfo(certificate))
    {
        return add(type, qMove(*certificateInfo));
    }

    return false;
}

bool PDFCertificateStore::add(PDFCertificateEntry::EntryType type, PDFCertificateInfo info)
{
    auto it = std::find_if(m_certificates.cbegin(), m_certificates.cend(), [&info](const auto& entry) { return entry.info == info; });
    if (it == m_certificates.cend())
    {
        m_certificates.push_back({ type, qMove(info), QByteArray(), QString() });
    }

    return true;
}

bool PDFCertificateStore::contains(const PDFCertificateInfo& info)
{
    return std::find_if(m_certificates.cbegin(), m_certificates.cend(), [&info](const auto& entry) { return entry.info == info; }) != m_certificates.cend();
}

QString PDFCertificateStore::getDefaultCertificateStoreFileName() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + "/TrustedCertStorage.bin";
}

void PDFCertificateStore::loadDefaultUserCertificates()
{
    createDirectoryForDefaultUserCertificatesStore();
    QString trustedCertificateStoreFileName = getDefaultCertificateStoreFileName();
    QString trustedCertificateStoreLockFileName = trustedCertificateStoreFileName + ".lock";

    QLockFile lockFile(trustedCertificateStoreLockFileName);
    if (lockFile.lock())
    {
        QFile trustedCertificateStoreFile(trustedCertificateStoreFileName);
        if (trustedCertificateStoreFile.open(QFile::ReadOnly))
        {
            QDataStream stream(&trustedCertificateStoreFile);
            deserialize(stream);
            trustedCertificateStoreFile.close();
        }
        lockFile.unlock();
    }
}

void PDFCertificateStore::saveDefaultUserCertificates()
{
    createDirectoryForDefaultUserCertificatesStore();
    QString trustedCertificateStoreFileName = getDefaultCertificateStoreFileName();
    QString trustedCertificateStoreLockFileName = trustedCertificateStoreFileName + ".lock";

    QFileInfo fileInfo(trustedCertificateStoreFileName);
    QDir dir = fileInfo.dir();
    dir.mkpath(dir.path());

    QLockFile lockFile(trustedCertificateStoreLockFileName);
    if (lockFile.lock())
    {
        QFile trustedCertificateStoreFile(trustedCertificateStoreFileName);
        if (trustedCertificateStoreFile.open(QFile::WriteOnly | QFile::Truncate))
        {
            QDataStream stream(&trustedCertificateStoreFile);
            serialize(stream);
            trustedCertificateStoreFile.close();
        }
        lockFile.unlock();
    }
}

void PDFCertificateStore::createDirectoryForDefaultUserCertificatesStore()
{
    QFileInfo fileInfo(getDefaultCertificateStoreFileName());
    QString path = fileInfo.path();
    QDir().mkpath(path);
}

}   // namespace pdf

#ifdef Q_OS_WIN
#include <Windows.h>
#include <wincrypt.h>
#if defined(PDF4QT_USE_PRAGMA_LIB)
#pragma comment(lib, "crypt32.lib")
#endif
#endif

pdf::PDFCertificateEntries pdf::PDFCertificateStore::getSystemCertificates()
{
    PDFCertificateEntries result;

#ifdef Q_OS_WIN
    HCERTSTORE certStore = CertOpenSystemStore(0, L"ROOT");
    PCCERT_CONTEXT context = nullptr;
    if (certStore)
    {
        while (context = CertEnumCertificatesInStore(certStore, context))
        {
            const unsigned char* pointer = context->pbCertEncoded;
            QByteArray data(reinterpret_cast<const char*>(pointer), context->cbCertEncoded);
            std::optional<PDFCertificateInfo> info = PDFCertificateInfo::getCertificateInfo(data);
            if (info)
            {
                PDFCertificateEntry entry;
                entry.type = PDFCertificateEntry::EntryType::System;
                entry.info = qMove(*info);
                result.emplace_back(qMove(entry));
            }
        }

        CertCloseStore(certStore, CERT_CLOSE_STORE_FORCE_FLAG);
    }
#endif

    return result;
}

pdf::PDFCertificateEntries pdf::PDFCertificateStore::getPersonalCertificates()
{
    PDFCertificateEntries result;

#ifdef Q_OS_WIN
    HCERTSTORE certStore = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, NULL, CERT_SYSTEM_STORE_CURRENT_USER, L"MY");
    PCCERT_CONTEXT context = nullptr;
    if (certStore)
    {
        while (context = CertEnumCertificatesInStore(certStore, context))
        {
            const unsigned char* pointer = context->pbCertEncoded;
            QByteArray data(reinterpret_cast<const char*>(pointer), context->cbCertEncoded);
            std::optional<PDFCertificateInfo> info = PDFCertificateInfo::getCertificateInfo(data);
            if (info)
            {
                PDFCertificateEntry entry;
                entry.type = PDFCertificateEntry::EntryType::System;
                entry.info = qMove(*info);
                result.emplace_back(qMove(entry));
            }
        }

        CertCloseStore(certStore, CERT_CLOSE_STORE_FORCE_FLAG);
    }
#endif

    return result;
}

#if defined(PDF4QT_COMPILER_MINGW) || defined(PDF4QT_COMPILER_GCC)
#pragma GCC diagnostic pop
#endif

#if defined(PDF4QT_COMPILER_MSVC)
#pragma warning(pop)
#endif
