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

#ifndef PDFCERTIFICATESTORE_H
#define PDFCERTIFICATESTORE_H

#include "pdfglobal.h"

#include <QFlags>
#include <QDateTime>
#include <QRecursiveMutex>
#include <QMutexLocker>

class QDataStream;
struct x509_st;

namespace pdf
{

/// OpenSSL is not thread safe.
class PDF4QTLIBCORESHARED_EXPORT PDFOpenSSLGlobalLock
{
public:
    explicit PDFOpenSSLGlobalLock();
    inline ~PDFOpenSSLGlobalLock() = default;

private:
    QMutexLocker<QRecursiveMutex> m_mutexLocker;
    static QRecursiveMutex s_globalOpenSSLMutex;
};

/// Info about certificate, various details etc.
class PDF4QTLIBCORESHARED_EXPORT PDFCertificateInfo
{
public:
    explicit inline PDFCertificateInfo() = default;

    void serialize(QDataStream& stream) const;
    void deserialize(QDataStream& stream);

    friend inline QDataStream& operator<<(QDataStream& stream, const PDFCertificateInfo& info) { info.serialize(stream); return stream; }
    friend inline QDataStream& operator>>(QDataStream& stream, PDFCertificateInfo& info) { info.deserialize(stream); return stream; }

    bool operator ==(const PDFCertificateInfo&) const = default;
    bool operator !=(const PDFCertificateInfo&) const = default;

    /// These entries are taken from RFC 5280, section 4.1.2.4,
    /// they are supported and loaded from the certificate, with
    /// exception of Email entry.
    enum NameEntry
    {
        CountryName,
        OrganizationName,
        OrganizationalUnitName,
        DistinguishedName,
        StateOrProvinceName,
        CommonName,
        SerialNumber,
        LocalityName,
        Title,
        Surname,
        GivenName,
        Initials,
        Pseudonym,
        GenerationalQualifier,
        Email,
        NameEnd
    };

    enum PublicKey
    {
        KeyRSA,
        KeyDSA,
        KeyEC,
        KeyDH,
        KeyUnknown
    };

    // This enum is defined in RFC 5280, chapter 4.2.1.3, Key Usage. Second part,
    // are defined in extended key usage entry, if it is defined.
    enum KeyUsageFlag : uint32_t
    {
        KeyUsageNone                = 0x00000000,
        KeyUsageDigitalSignature    = 0x00000080,
        KeyUsageNonRepudiation      = 0x00000040,
        KeyUsageKeyEncipherment     = 0x00000020,
        KeyUsageDataEncipherment    = 0x00000010,
        KeyUsageAgreement           = 0x00000008,
        KeyUsageCertSign            = 0x00000004,
        KeyUsageCrlSign             = 0x00000002,
        KeyUsageEncipherOnly        = 0x00000001,
        KeyUsageDecipherOnly        = 0x00008000,

        KeyUsageExtended_SSL_SERVER = 0x1    << 16,
        KeyUsageExtended_SSL_CLIENT = 0x2    << 16,
        KeyUsageExtended_SMIME      = 0x4    << 16,
        KeyUsageExtended_CODE_SIGN  = 0x8    << 16,
        KeyUsageExtended_SGC        = 0x10   << 16,
        KeyUsageExtended_OCSP_SIGN  = 0x20   << 16,
        KeyUsageExtended_TIMESTAMP  = 0x40   << 16,
        KeyUsageExtended_DVCS       = 0x80   << 16,
        KeyUsageExtended_ANYEKU     = 0x100  << 16,
    };
    Q_DECLARE_FLAGS(KeyUsageFlags, KeyUsageFlag)

    const QString& getName(NameEntry name) const { return m_nameEntries[name]; }
    void setName(NameEntry name, QString string) { m_nameEntries[name] = qMove(string); }

    QDateTime getNotValidBefore() const;
    void setNotValidBefore(const QDateTime& notValidBefore);

    QDateTime getNotValidAfter() const;
    void setNotValidAfter(const QDateTime& notValidAfter);

    int32_t getVersion() const;
    void setVersion(int32_t version);

    PublicKey getPublicKey() const;
    void setPublicKey(const PublicKey& publicKey);

    int getKeySize() const;
    void setKeySize(int keySize);

    KeyUsageFlags getKeyUsage() const;
    void setKeyUsage(KeyUsageFlags keyUsage);

    QByteArray getCertificateData() const;
    void setCertificateData(const QByteArray& certificateData);

    /// Creates certificate info from binary data. Binary data must
    /// contain DER-encoded certificate.
    /// \param certificateData Data
    static std::optional<PDFCertificateInfo> getCertificateInfo(const QByteArray& certificateData);

    /// Creates certificate info from certificate
    static PDFCertificateInfo getCertificateInfo(x509_st* certificate);

private:
    static constexpr int persist_version = 1;

    int32_t m_version = 0;
    int m_keySize = 0;
    PublicKey m_publicKey = KeyUnknown;
    std::array<QString, NameEnd> m_nameEntries;
    QDateTime m_notValidBefore;
    QDateTime m_notValidAfter;
    KeyUsageFlags m_keyUsage;
    QByteArray m_certificateData;
};

using PDFCertificateInfos = std::vector<PDFCertificateInfo>;

struct PDFCertificateEntry
{
    enum class EntryType : int
    {
        User,       ///< Certificate has been added manually by the user
        System,     ///< System certificate
        AATL,       ///< Trusted list
    };

    void serialize(QDataStream& stream) const;
    void deserialize(QDataStream& stream);

    friend inline QDataStream& operator<<(QDataStream& stream, const PDFCertificateEntry& entry) { entry.serialize(stream); return stream; }
    friend inline QDataStream& operator>>(QDataStream& stream, PDFCertificateEntry& entry) { entry.deserialize(stream); return stream; }

    static constexpr int persist_version = 1;
    EntryType type = EntryType::User;
    PDFCertificateInfo info;
    QByteArray pkcs12;
    QString pkcs12fileName;
};

using PDFCertificateEntries = std::vector<PDFCertificateEntry>;

/// Trusted certificate store. Contains list of trusted certificates. Store
/// can be persisted to the persistent storage trough serialization/deserialization.
/// Persisting method is versioned.
class PDF4QTLIBCORESHARED_EXPORT PDFCertificateStore
{
public:
    explicit inline PDFCertificateStore() = default;

    void serialize(QDataStream& stream) const;
    void deserialize(QDataStream& stream);

    /// Tries to add new certificate to the certificate store. If certificate
    /// is already here, then nothing happens and function returns true.
    /// Otherwise data are checked, if they really contains a certificate,
    /// and if yes, then it is added. Function returns false if, and only if,
    /// error occured and certificate was not added.
    /// \param type Type
    /// \param certificate Certificate
    bool add(PDFCertificateEntry::EntryType type, const QByteArray& certificate);

    /// Tries to add new certificate to the certificate store. If certificate
    /// is already here, then nothing happens and function returns true.
    /// Otherwise data are checked, if they really contains a certificate,
    /// and if yes, then it is added. Function returns false if, and only if,
    /// error occured and certificate was not added.
    /// \param type Type
    /// \param info Certificate info
    bool add(PDFCertificateEntry::EntryType type, PDFCertificateInfo info);

    /// Returns true, if storage contains given certificate
    /// \param info Certificate info
    bool contains(const PDFCertificateInfo& info);

    /// Get certificates stored in the store
    const PDFCertificateEntries& getCertificates() const { return m_certificates; }

    /// Set certificates
    void setCertificates(PDFCertificateEntries certificates) { m_certificates = qMove(certificates); }

    /// Returns default certificate store file name
    QString getDefaultCertificateStoreFileName() const;

    /// Load from default user certificate storage
    void loadDefaultUserCertificates();

    /// Save to default user certificate storage
    void saveDefaultUserCertificates();

    /// Creates default directory for certificate store
    void createDirectoryForDefaultUserCertificatesStore();

    /// Returns a list of aatl certificates
    static PDFCertificateEntries getAATLCertificates();

    /// Returns a list of system certificates
    static PDFCertificateEntries getSystemCertificates();

    /// Returns a list of personal certificates (usually used for signing documents)
    static PDFCertificateEntries getPersonalCertificates();

private:
    static constexpr int persist_version = 1;

    PDFCertificateEntries m_certificates;
};

}   // namespace pdf

#endif // PDFCERTIFICATESTORE_H
