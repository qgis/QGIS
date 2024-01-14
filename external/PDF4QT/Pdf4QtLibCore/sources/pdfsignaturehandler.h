//    Copyright (C) 2020-2021 Jakub Melka
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

#ifndef PDFSIGNATUREHANDLER_H
#define PDFSIGNATUREHANDLER_H

#include "pdfglobal.h"
#include "pdfobject.h"
#include "pdfutils.h"
#include "pdfcertificatestore.h"

#include <QString>
#include <QDateTime>

#include <optional>

class QDataStream;

namespace pdf
{
class PDFForm;
class PDFObjectStorage;
class PDFCertificateStore;
class PDFFormFieldSignature;
class PDFDocumentSecurityStore;

/// Signature reference dictionary.
class PDFSignatureReference
{
public:

    enum class TransformMethod
    {
        Invalid,
        DocMDP,
        UR,
        FieldMDP
    };

    TransformMethod getTransformMethod() const { return m_transformMethod; }
    const PDFObject& getTransformParams() const { return m_transformParams; }
    const PDFObject& getData() const { return m_data; }
    const QByteArray& getDigestMethod() const { return m_digestMethod; }

    /// Tries to parse the signature reference. No exception is thrown, in case of error,
    /// invalid signature reference object is returned.
    /// \param storage Object storage
    /// \param object Object containing the signature
    static PDFSignatureReference parse(const PDFObjectStorage* storage, PDFObject object);

private:
    TransformMethod m_transformMethod = TransformMethod::Invalid;
    PDFObject m_transformParams;
    PDFObject m_data;
    QByteArray m_digestMethod;
};

/// Signature dictionary. Contains digital signature. This signature can be validated by signature validator.
/// This object contains certificates, digital signatures, and other information about signature.
class PDFSignature
{
public:

    enum class Type
    {
        Invalid,
        Sig,
        DocTimeStamp
    };

    struct ByteRange
    {
        PDFInteger offset = 0;
        PDFInteger size = 0;
    };

    using ByteRanges = std::vector<ByteRange>;

    struct Changes
    {
        PDFInteger pagesAltered = 0;
        PDFInteger fieldsAltered = 0;
        PDFInteger fieldsFilled = 0;
    };

    enum class AuthentificationType
    {
        Invalid,
        PIN,
        Password,
        Fingerprint
    };

    /// Tries to parse the signature. No exception is thrown, in case of error,
    /// invalid signature object is returned.
    /// \param storage Object storage
    /// \param object Object containing the signature
    static PDFSignature parse(const PDFObjectStorage* storage, PDFObject object);

    Type getType() const { return m_type; }
    const QByteArray& getFilter() const { return m_filter; }
    const QByteArray& getSubfilter() const { return m_subfilter; }
    const QByteArray& getContents() const { return m_contents; }
    const std::vector<QByteArray>* getCertificates() const { return m_certificates.has_value() ? &m_certificates.value() : nullptr; }
    const ByteRanges& getByteRanges() const { return m_byteRanges; }
    const std::vector<PDFSignatureReference>& getReferences() const { return m_references; }
    const Changes* getChanges() const { return m_changes.has_value() ? &m_changes.value() : nullptr; }

    const QString& getName() const { return m_name; }
    const QDateTime& getSigningDateTime() const { return m_signingDateTime; }
    const QString& getLocation() const { return m_location; }
    const QString& getReason() const { return m_reason; }
    const QString& getContactInfo() const { return m_contactInfo; }
    PDFInteger getR() const { return m_R; }
    PDFInteger getV() const { return m_V; }
    const PDFObject& getPropBuild() const { return m_propBuild; }
    PDFInteger getPropTime() const { return m_propTime; }
    AuthentificationType getAuthentificationType() const { return m_propType; }

private:
    Type m_type = Type::Invalid;
    QByteArray m_filter;    ///< Preferred signature handler name
    QByteArray m_subfilter; ///< Describes encoding of signature
    QByteArray m_contents;
    std::optional<std::vector<QByteArray>> m_certificates; ///< Certificate chain (only for adbe.x509.rsa_sha1)
    ByteRanges m_byteRanges;
    std::vector<PDFSignatureReference> m_references;
    std::optional<Changes> m_changes;

    QString m_name; ///< Name of signer. Should rather be extracted from signature.
    QDateTime m_signingDateTime; ///< Signing date and time. Should be extracted from signature, if possible.
    QString m_location; ///< CPU hostname or physical location of signing
    QString m_reason; ///< Reason for signing
    QString m_contactInfo; ///< Contact info for verifying the signature
    PDFInteger m_R; ///< Version of signature handler. Obsolete.
    PDFInteger m_V; ///< Version of signature dictionary format. 1 if References should be used.
    PDFObject m_propBuild;
    PDFInteger m_propTime = 0;
    AuthentificationType m_propType = AuthentificationType::Invalid;
};

class PDF4QTLIBCORESHARED_EXPORT PDFSignatureVerificationResult
{
public:
    explicit PDFSignatureVerificationResult() = default;
    explicit PDFSignatureVerificationResult(PDFSignature::Type type, PDFObjectReference signatureFieldReference, QString qualifiedName) :
        m_type(type),
        m_signatureFieldReference(signatureFieldReference),
        m_signatureFieldQualifiedName(qualifiedName)
    {

    }

    enum class Status
    {
        OK,
        Warning,
        Error
    };

    enum VerificationFlag
    {
        None                                            = 0x00000000,  ///< Used only for initialization
        OK                                              = 0x00000001,  ///< Both certificate and signature is OK
        Certificate_OK                                  = 0x00000002,  ///< Certificate is OK
        Signature_OK                                    = 0x00000004,  ///< Signature is OK
        Error_NoHandler                                 = 0x00000008,  ///< No signature handler for given signature
        Error_Generic                                   = 0x00000010,  ///< Generic error (uknown general error)

        Error_Certificate_Invalid                       = 0x00000020,  ///< Certificate is invalid
        Error_Certificate_NoSignatures                  = 0x00000040,  ///< No signature found in certificate data
        Error_Certificate_Missing                       = 0x00000080,  ///< Certificate is missing
        Error_Certificate_Generic                       = 0x00000100,  ///< Generic error during certificate verification
        Error_Certificate_Expired                       = 0x00000200,  ///< Certificate has expired
        Error_Certificate_SelfSigned                    = 0x00000400,  ///< Self signed certificate
        Error_Certificate_SelfSignedChain               = 0x00000800,  ///< Self signed certificate in chain
        Error_Certificate_TrustedNotFound               = 0x00001000,  ///< No trusted certificate was found
        Error_Certificate_Revoked                       = 0x00002000,  ///< Certificate has been revoked
        Error_Certificate_Other                         = 0x00004000,  ///< Other certificate error. See OpenSSL code for details.

        Error_Signature_Invalid                         = 0x00008000,  ///< Signature is invalid for some reason
        Error_Signature_SourceCertificateMissing        = 0x00010000,  ///< Source certificate of signature is missing
        Error_Signature_NoSignaturesFound               = 0x00020000,  ///< No signatures found
        Error_Signature_DigestFailure                   = 0x00040000,  ///< Digest failure
        Error_Signature_DataOther                       = 0x00080000,  ///< Signed data were not verified
        Error_Signature_DataCoveredBySignatureMissing   = 0x00100000,  ///< Data covered by signature are not present

        Warning_Signature_NotCoveredBytes               = 0x00200000,  ///< Some bytes in source data are not covered by signature
        Warning_Certificate_CRLValidityTimeExpired      = 0x00400000,  ///< Certificate revocation list was not checked, because it's validity expired
        Warning_Certificate_QualifiedStatement          = 0x00800000,  ///< Qualified certificate statement not verified
        Warning_Certificate_UnableToGetCRL              = 0x01000000,  ///< Unable to get CRL

        Error_Certificates_Mask = Error_Certificate_Invalid | Error_Certificate_NoSignatures | Error_Certificate_Missing | Error_Certificate_Generic |
                                  Error_Certificate_Expired | Error_Certificate_SelfSigned | Error_Certificate_SelfSignedChain | Error_Certificate_TrustedNotFound |
                                  Error_Certificate_Revoked | Error_Certificate_Other,

        Error_Signatures_Mask = Error_Signature_Invalid | Error_Signature_SourceCertificateMissing | Error_Signature_NoSignaturesFound |
                                Error_Signature_DigestFailure | Error_Signature_DataOther | Error_Signature_DataCoveredBySignatureMissing,

        Warning_Certificates_Mask = Warning_Certificate_CRLValidityTimeExpired | Warning_Certificate_QualifiedStatement | Warning_Certificate_UnableToGetCRL,
        Warning_Signatures_Mask = Warning_Signature_NotCoveredBytes,

        Warnings_Mask = Warning_Certificates_Mask | Warning_Signatures_Mask
    };
    Q_DECLARE_FLAGS(VerificationFlags, VerificationFlag)

    PDFSignature::Type getType() const;
    void setType(const PDFSignature::Type& type);

    /// Adds no handler error for given signature format
    /// \param format Signature format
    void addNoHandlerError(const QByteArray& format);

    void addInvalidCertificateError();
    void addNoSignaturesError();
    void addCertificateMissingError();
    void addCertificateGenericError();
    void addCertificateExpiredError();
    void addCertificateSelfSignedError();
    void addCertificateSelfSignedInChainError();
    void addCertificateTrustedNotFoundError();
    void addCertificateRevokedError();
    void addCertificateOtherError(int error);
    void addInvalidSignatureError();
    void addSignatureNoSignaturesFoundError();
    void addSignatureCertificateMissingError();
    void addSignatureDigestFailureError();
    void addSignatureDataOtherError();
    void addSignatureDataCoveredBySignatureMissingError();

    void addSignatureNotCoveredBytesWarning(PDFInteger count);
    void addCertificateCRLValidityTimeExpiredWarning();
    void addCertificateQualifiedStatementNotVerifiedWarning();
    void addCertificateUnableToGetCRLWarning();

    bool isValid() const { return hasFlag(OK); }
    bool isCertificateValid() const { return hasFlag(Certificate_OK); }
    bool isSignatureValid() const { return hasFlag(Signature_OK); }
    bool hasError() const { return !isValid(); }
    bool hasWarning() const { return m_flags & Warnings_Mask; }
    bool hasCertificateError() const { return m_flags & Error_Certificates_Mask; }
    bool hasSignatureError() const { return m_flags & Error_Signatures_Mask; }
    bool hasCertificateWarning() const { return m_flags & Warning_Certificates_Mask; }
    bool hasSignatureWarning() const { return m_flags & Warning_Signatures_Mask; }
    bool hasFlag(VerificationFlag flag) const { return m_flags.testFlag(flag); }
    void setFlag(VerificationFlag flag, bool value) { m_flags.setFlag(flag, value); }

    PDFObjectReference getSignatureFieldReference() const { return m_signatureFieldReference; }
    const QString& getSignatureFieldQualifiedName() const { return m_signatureFieldQualifiedName; }
    const QStringList& getErrors() const { return m_errors; }
    const QStringList& getWarnings() const { return m_warnings; }
    const QStringList& getHashAlgorithms() const { return m_hashAlgorithms; }
    const PDFCertificateInfos& getCertificateInfos() const { return m_certificateInfos; }

    void setSignatureFieldQualifiedName(const QString& signatureFieldQualifiedName);
    void setSignatureFieldReference(PDFObjectReference signatureFieldReference);

    void addCertificateInfo(PDFCertificateInfo info) { m_certificateInfos.emplace_back(qMove(info)); }
    void addHashAlgorithm(const QString& algorithm);

    /// Adds OK flag, if both certificate and signature are valid
    void validate();

    QDateTime getSignatureDate() const;
    void setSignatureDate(const QDateTime& signatureDate);

    QDateTime getTimestampDate() const;
    void setTimestampDate(const QDateTime& timestampDate);

    QByteArray getSignatureHandler() const;
    void setSignatureHandler(const QByteArray& signatureFilter);

    Status getCertificateStatus() const;
    Status getSignatureStatus() const;

    QString getCertificateStatusText() const { return getStatusText(getCertificateStatus()); }
    QString getSignatureStatusText() const { return getStatusText(getSignatureStatus()); }

    static QString getStatusText(Status status);

    const PDFClosedIntervalSet& getBytesCoveredBySignature() const;
    void setBytesCoveredBySignature(const PDFClosedIntervalSet& bytesCoveredBySignature);

private:
    PDFSignature::Type m_type = PDFSignature::Type::Invalid;
    VerificationFlags m_flags = None;
    PDFObjectReference m_signatureFieldReference;
    QString m_signatureFieldQualifiedName;
    QDateTime m_signatureDate;
    QDateTime m_timestampDate;
    QStringList m_errors;
    QStringList m_warnings;
    QStringList m_hashAlgorithms;
    QByteArray m_signatureHandler;
    PDFCertificateInfos m_certificateInfos;
    PDFClosedIntervalSet m_bytesCoveredBySignature;
};

/// Signature handler. Can verify both certificate and signature validity.
class PDF4QTLIBCORESHARED_EXPORT PDFSignatureHandler
{
public:
    explicit PDFSignatureHandler() = default;
    virtual ~PDFSignatureHandler() = default;

    virtual PDFSignatureVerificationResult verify() const = 0;

    struct Parameters
    {
        const PDFCertificateStore* store = nullptr;
        const PDFDocumentSecurityStore* dss = nullptr;
        bool enableVerification = true;
        bool ignoreExpirationDate = false;
        bool useSystemCertificateStore = true;
    };

    /// Tries to verify all signatures in the form. If form is invalid, then
    /// empty vector is returned.
    /// \param form Form
    /// \param sourceData Source data
    /// \param parameters Verification settings
    static std::vector<PDFSignatureVerificationResult> verifySignatures(const PDFForm& form, const QByteArray& sourceData, const Parameters& parameters);

private:

    /// Creates signature handler using format specified by signature in signature field.
    /// If signature format is unknown, then nullptr is returned.
    /// \param signatureField Signature field
    /// \param sourceData
    /// \param parameters Verification settings
    static PDFSignatureHandler* createHandler(const PDFFormFieldSignature* signatureField, const QByteArray& sourceData, const Parameters& parameters);
};

} // namespace pdf

#endif // PDFSIGNATUREHANDLER_H
