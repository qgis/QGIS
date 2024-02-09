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

#ifndef PDFSIGNATUREHANDLER_IMPL_H
#define PDFSIGNATUREHANDLER_IMPL_H

#include "pdfsignaturehandler.h"

#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pkcs7.h>

namespace pdf
{

/// PKCS7 public key signature handler
class PDFPublicKeySignatureHandler : public PDFSignatureHandler
{
protected:
    explicit PDFPublicKeySignatureHandler(const PDFFormFieldSignature* signatureField, const QByteArray& sourceData, const Parameters& parameters) :
        m_signatureField(signatureField),
        m_sourceData(sourceData),
        m_parameters(parameters)
    {

    }

    void initializeResult(PDFSignatureVerificationResult& result) const;
    void verifyCertificate(PDFSignatureVerificationResult& result) const;
    void verifySignature(PDFSignatureVerificationResult& result) const;
    void addTrustedCertificates(X509_STORE* store) const;

    virtual BIO* getSignedDataBuffer(PDFSignatureVerificationResult& result, QByteArray& outputBuffer) const;

public:
    /// Return a list of certificates from PKCS7 object
    static STACK_OF(X509)* getCertificates(PKCS7* pkcs7);

    /// Return certificate info for given certificate
    static PDFCertificateInfo getCertificateInfo(X509* certificate);

    /// Returns name converted to QString
    static QString getStringFromX509Name(X509_NAME* name, int nid);

    /// Returns ASN1 string converted to QString
    static QString getStringFromASN1_STRING(ASN1_STRING* string);

    /// Converts ASN time to QDateTime. If conversion fails, then invalid
    /// datetime is returned.
    static QDateTime getDateTimeFromASN(const ASN1_TIME* time);

protected:
    /// Add hash algorithm from signer info stack. If \p signerInfoStack is nullptr,
    /// then nothing happens. If multiple signer hash algorithms are present,
    /// then they are all added.
    /// \param signerInfoStack Signer stack
    /// \param result Result, to which algorithm is added
    static void addHashAlgorithmFromSignerInfoStack(STACK_OF(PKCS7_SIGNER_INFO)* signerInfoStack, PDFSignatureVerificationResult& result);

    /// Add signing date/time from signer info stack. If there are multiple signature
    /// infos, nothing is added (because we can't decide, which one is right).
    /// \param signerInfoStack Signer info stack
    /// \param result Verification, to which signature date is being set
    static void addSignatureDateFromSignerInfoStack(STACK_OF(PKCS7_SIGNER_INFO)* signerInfoStack, PDFSignatureVerificationResult& result);

protected:
    const PDFFormFieldSignature* m_signatureField;
    QByteArray m_sourceData;
    Parameters m_parameters;
};

class PDFSignatureHandler_adbe_pkcs7_detached : public PDFPublicKeySignatureHandler
{
public:
    explicit PDFSignatureHandler_adbe_pkcs7_detached(const PDFFormFieldSignature* signatureField, const QByteArray& sourceData, const Parameters& parameters) :
        PDFPublicKeySignatureHandler(signatureField, sourceData, parameters)
    {

    }

    virtual PDFSignatureVerificationResult verify() const override;
};

class PDFSignatureHandler_adbe_pkcs7_rsa_sha1 : public PDFPublicKeySignatureHandler
{
public:
    explicit PDFSignatureHandler_adbe_pkcs7_rsa_sha1(const PDFFormFieldSignature* signatureField, const QByteArray& sourceData, const Parameters& parameters) :
        PDFPublicKeySignatureHandler(signatureField, sourceData, parameters)
    {

    }

    virtual PDFSignatureVerificationResult verify() const override;

private:
    X509* createCertificate(size_t index) const;
    bool getMessageDigest(const QByteArray& message, ASN1_OCTET_STRING* encryptedString, RSA* rsa, int& algorithmNID, QByteArray& digest) const;
    bool getMessageDigestAlgorithm(ASN1_OCTET_STRING* encryptedString, RSA* rsa, int& algorithmNID) const;

    void verifyRSACertificate(PDFSignatureVerificationResult& result) const;
    void verifyRSASignature(PDFSignatureVerificationResult& result) const;
};

class PDFSignatureHandler_adbe_pkcs7_sha1 : public PDFPublicKeySignatureHandler
{
public:
    explicit PDFSignatureHandler_adbe_pkcs7_sha1(const PDFFormFieldSignature* signatureField, const QByteArray& sourceData, const Parameters& parameters) :
        PDFPublicKeySignatureHandler(signatureField, sourceData, parameters)
    {

    }

    virtual PDFSignatureVerificationResult verify() const override;

protected:
    virtual BIO* getSignedDataBuffer(PDFSignatureVerificationResult& result, QByteArray& outputBuffer) const override;
};

class PDFSignatureHandler_ETSI_base : public PDFPublicKeySignatureHandler
{
public:
    explicit PDFSignatureHandler_ETSI_base(const PDFFormFieldSignature* signatureField, const QByteArray& sourceData, const Parameters& parameters) :
        PDFPublicKeySignatureHandler(signatureField, sourceData, parameters)
    {

    }

protected:
    void verifyCertificateCAdES(PDFSignatureVerificationResult& result, int purpose) const;
    static int verifyCallback(int ok, X509_STORE_CTX* context);
};

class PDFSignatureHandler_ETSI_CAdES_detached : public PDFSignatureHandler_ETSI_base
{
public:
    explicit PDFSignatureHandler_ETSI_CAdES_detached(const PDFFormFieldSignature* signatureField, const QByteArray& sourceData, const Parameters& parameters) :
        PDFSignatureHandler_ETSI_base(signatureField, sourceData, parameters)
    {

    }

    virtual PDFSignatureVerificationResult verify() const override;
};

class PDFSignatureHandler_ETSI_RFC3161: public PDFSignatureHandler_ETSI_base
{
public:
    explicit PDFSignatureHandler_ETSI_RFC3161(const PDFFormFieldSignature* signatureField, const QByteArray& sourceData, const Parameters& parameters) :
        PDFSignatureHandler_ETSI_base(signatureField, sourceData, parameters)
    {

    }

    virtual PDFSignatureVerificationResult verify() const override;

private:
    void verifySignatureTimestamp(PDFSignatureVerificationResult& result) const;
};

}   // namespace pdf

#endif // PDFSIGNATUREHANDLER_IMPL_H
