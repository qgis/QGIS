//    Copyright (C) 2020-2022 Jakub Melka
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

#include "pdfsignaturehandler.h"
#include "pdfdocument.h"
#include "pdfencoding.h"
#include "pdfform.h"
#include "pdfutils.h"
#include "pdfsignaturehandler_impl.h"

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
#include <QMutex>
#include <QFileInfo>
#include <QLockFile>
#include <QDataStream>
#include <QMutexLocker>
#include <QStandardPaths>

#include "pdfdbgheap.h"

#include <array>
#ifdef Q_OS_UNIX
#include <time.h>
#endif

namespace pdf
{

template<typename T>
using openssl_ptr = std::unique_ptr<T, void(*)(T*)>;

PDFSignatureReference PDFSignatureReference::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDFSignatureReference result;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        PDFDocumentDataLoaderDecorator loader(storage);

        constexpr const std::array<std::pair<const char*, PDFSignatureReference::TransformMethod>, 3> types = {
            std::pair<const char*, PDFSignatureReference::TransformMethod>{ "DocMDP", PDFSignatureReference::TransformMethod::DocMDP },
            std::pair<const char*, PDFSignatureReference::TransformMethod>{ "UR", PDFSignatureReference::TransformMethod::UR },
            std::pair<const char*, PDFSignatureReference::TransformMethod>{ "FieldMDP", PDFSignatureReference::TransformMethod::FieldMDP }
        };

        // Jakub Melka: parse the signature reference dictionary
        result.m_transformMethod = loader.readEnumByName(dictionary->get("TransformMethod"), types.cbegin(), types.cend(), PDFSignatureReference::TransformMethod::Invalid);
        result.m_transformParams = dictionary->get("TransformParams");
        result.m_data = dictionary->get("Data");
        result.m_digestMethod = loader.readNameFromDictionary(dictionary, "DigestMethod");
    }

    return result;
}

PDFSignature PDFSignature::parse(const PDFObjectStorage* storage, PDFObject object)
{
    PDFSignature result;

    if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(object))
    {
        PDFDocumentDataLoaderDecorator loader(storage);

        constexpr const std::array<std::pair<const char*, Type>, 2> types = {
            std::pair<const char*, Type>{ "Sig", Type::Sig },
            std::pair<const char*, Type>{ "DocTimeStamp", Type::DocTimeStamp }
        };

        // Jakub Melka: parse the signature dictionary
        result.m_type = loader.readEnumByName(dictionary->get("Type"), types.cbegin(), types.cend(), Type::Sig);
        result.m_filter = loader.readNameFromDictionary(dictionary, "Filter");
        result.m_subfilter = loader.readNameFromDictionary(dictionary, "SubFilter");
        result.m_contents = loader.readStringFromDictionary(dictionary, "Contents");

        if (dictionary->hasKey("Cert"))
        {
            PDFObject certificates = storage->getObject(dictionary->get("Cert"));
            if (certificates.isString())
            {
                result.m_certificates = { loader.readString(certificates) };
            }
            else if (certificates.isArray())
            {
                result.m_certificates = loader.readStringArray(certificates);
            }
        }

        std::vector<PDFInteger> byteRangesArray = loader.readIntegerArrayFromDictionary(dictionary, "ByteRange");
        const size_t byteRangeCount = byteRangesArray.size() / 2;
        result.m_byteRanges.reserve(byteRangeCount);
        for (size_t i = 0; i < byteRangeCount; ++i)
        {
            ByteRange byteRange = { byteRangesArray[2 * i], byteRangesArray[2 * i + 1] };
            result.m_byteRanges.push_back(byteRange);
        }

        result.m_references = loader.readObjectList<PDFSignatureReference>(dictionary->get("Reference"));
        std::vector<PDFInteger> changes = loader.readIntegerArrayFromDictionary(dictionary, "Changes");

        if (changes.size() == 3)
        {
            result.m_changes = { changes[0], changes[1], changes[2] };
        }

        result.m_name = loader.readTextStringFromDictionary(dictionary, "Name", QString());
        result.m_signingDateTime = PDFEncoding::convertToDateTime(loader.readStringFromDictionary(dictionary, "M"));
        result.m_location = loader.readTextStringFromDictionary(dictionary, "Location", QString());
        result.m_reason = loader.readTextStringFromDictionary(dictionary, "Reason", QString());
        result.m_contactInfo = loader.readTextStringFromDictionary(dictionary, "ContactInfo", QString());
        result.m_R = loader.readIntegerFromDictionary(dictionary, "R", 0);
        result.m_V = loader.readIntegerFromDictionary(dictionary, "V", 0);
        result.m_propBuild = dictionary->get("Prop_Build");
        result.m_propTime = loader.readIntegerFromDictionary(dictionary, "Prop_AuthTime", 0);

        constexpr const std::array<std::pair<const char*, AuthentificationType>, 3> authentificationTypes = {
            std::pair<const char*, AuthentificationType>{ "PIN", AuthentificationType::PIN },
            std::pair<const char*, AuthentificationType>{ "Password", AuthentificationType::Password },
            std::pair<const char*, AuthentificationType>{ "Fingerprint", AuthentificationType::Fingerprint }
        };
        result.m_propType = loader.readEnumByName(dictionary->get("Prop_AuthType"), authentificationTypes.cbegin(), authentificationTypes.cend(), AuthentificationType::Invalid);
    }

    return result;
}

PDFSignatureHandler* PDFSignatureHandler::createHandler(const PDFFormFieldSignature* signatureField, const QByteArray& sourceData, const Parameters& parameters)
{
    Q_ASSERT(signatureField);

    const QByteArray& subfilter = signatureField->getSignature().getSubfilter();
    if (subfilter == "adbe.pkcs7.detached")
    {
        return new PDFSignatureHandler_adbe_pkcs7_detached(signatureField, sourceData, parameters);
    }
    else if (subfilter == "adbe.pkcs7.sha1")
    {
        return new PDFSignatureHandler_adbe_pkcs7_sha1(signatureField, sourceData, parameters);
    }
    else if (subfilter == "adbe.x509.rsa_sha1")
    {
        return new PDFSignatureHandler_adbe_pkcs7_rsa_sha1(signatureField, sourceData, parameters);
    }
    else if (subfilter == "ETSI.CAdES.detached")
    {
        return new PDFSignatureHandler_ETSI_CAdES_detached(signatureField, sourceData, parameters);
    }
    else if (subfilter == "ETSI.RFC3161")
    {
        return new PDFSignatureHandler_ETSI_RFC3161(signatureField, sourceData, parameters);
    }

    return nullptr;
}

std::vector<PDFSignatureVerificationResult> PDFSignatureHandler::verifySignatures(const PDFForm& form, const QByteArray& sourceData, const Parameters& parameters)
{
    std::vector<PDFSignatureVerificationResult> result;

    if (parameters.enableVerification && (form.isAcroForm() || form.isXFAForm()))
    {
        std::vector<const PDFFormFieldSignature*> signatureFields;
        auto getSignatureFields = [&signatureFields](const PDFFormField* field)
        {
            if (field->getFieldType() == PDFFormField::FieldType::Signature)
            {
                const PDFFormFieldSignature* signatureField = dynamic_cast<const PDFFormFieldSignature*>(field);
                Q_ASSERT(signatureField);
                signatureFields.push_back(signatureField);
            }
        };
        form.apply(getSignatureFields);
        result.reserve(signatureFields.size());

        for (const PDFFormFieldSignature* signatureField : signatureFields)
        {
            if (const PDFSignatureHandler* signatureHandler = createHandler(signatureField, sourceData, parameters))
            {
                result.emplace_back(signatureHandler->verify());
                delete signatureHandler;
            }
            else
            {
                PDFObjectReference signatureFieldReference = signatureField->getSelfReference();
                QString qualifiedName = signatureField->getName(PDFFormField::NameType::FullyQualified);
                PDFSignatureVerificationResult verificationResult(signatureField->getSignature().getType(), signatureFieldReference, qMove(qualifiedName));
                verificationResult.addNoHandlerError(signatureField->getSignature().getSubfilter());
                result.emplace_back(qMove(verificationResult));
            }
        }
    }

    return result;
}

void PDFSignatureVerificationResult::addNoHandlerError(const QByteArray& format)
{
    m_flags.setFlag(Error_NoHandler);
    m_errors << PDFTranslationContext::tr("No signature handler for signature format '%1'.").arg(QString::fromLatin1(format));
}

void PDFSignatureVerificationResult::addInvalidCertificateError()
{
    m_flags.setFlag(Error_Certificate_Invalid);
    m_errors << PDFTranslationContext::tr("Certificate format is invalid.");
}

void PDFSignatureVerificationResult::addNoSignaturesError()
{
    m_flags.setFlag(Error_Certificate_NoSignatures);
    m_errors << PDFTranslationContext::tr("No signatures in certificate data.");
}

void PDFSignatureVerificationResult::addCertificateMissingError()
{
    m_flags.setFlag(Error_Certificate_Missing);
    m_errors << PDFTranslationContext::tr("Certificate is missing.");
}

void PDFSignatureVerificationResult::addCertificateGenericError()
{
    m_flags.setFlag(Error_Certificate_Generic);
    m_errors << PDFTranslationContext::tr("Generic error occured during certificate validation.");
}

void PDFSignatureVerificationResult::addCertificateExpiredError()
{
    m_flags.setFlag(Error_Certificate_Expired);
    m_errors << PDFTranslationContext::tr("Certificate has expired.");
}

void PDFSignatureVerificationResult::addCertificateSelfSignedError()
{
    m_flags.setFlag(Error_Certificate_SelfSigned);
    m_errors << PDFTranslationContext::tr("Certificate is self-signed.");
}

void PDFSignatureVerificationResult::addCertificateSelfSignedInChainError()
{
    m_flags.setFlag(Error_Certificate_SelfSignedChain);
    m_errors << PDFTranslationContext::tr("Self-signed certificate in chain.");
}

void PDFSignatureVerificationResult::addCertificateTrustedNotFoundError()
{
    m_flags.setFlag(Error_Certificate_TrustedNotFound);
    m_errors << PDFTranslationContext::tr("Trusted certificate not found.");
}

void PDFSignatureVerificationResult::addCertificateRevokedError()
{
    m_flags.setFlag(Error_Certificate_Revoked);
    m_errors << PDFTranslationContext::tr("Certificate has been revoked.");
}

void PDFSignatureVerificationResult::addCertificateOtherError(int error)
{
    m_flags.setFlag(Error_Certificate_Other);
    m_errors << PDFTranslationContext::tr("Certificate validation failed with code %1.").arg(error);
}

void PDFSignatureVerificationResult::addInvalidSignatureError()
{
    m_flags.setFlag(Error_Signature_Invalid);
    m_errors << PDFTranslationContext::tr("Signature is invalid.");
}

void PDFSignatureVerificationResult::addSignatureNoSignaturesFoundError()
{
    m_flags.setFlag(Error_Signature_NoSignaturesFound);
    m_errors << PDFTranslationContext::tr("No signatures found in certificate.");
}

void PDFSignatureVerificationResult::addSignatureCertificateMissingError()
{
    m_flags.setFlag(Error_Signature_SourceCertificateMissing);
    m_errors << PDFTranslationContext::tr("Signature certificate is missing.");
}

void PDFSignatureVerificationResult::addSignatureDigestFailureError()
{
    m_flags.setFlag(Error_Signature_DigestFailure);
    m_errors << PDFTranslationContext::tr("Signed data has different hash function digest.");
}

void PDFSignatureVerificationResult::addSignatureDataOtherError()
{
    m_flags.setFlag(Error_Signature_DataOther);
    m_errors << PDFTranslationContext::tr("Signed data are invalid.");
}

void PDFSignatureVerificationResult::addSignatureDataCoveredBySignatureMissingError()
{
    m_flags.setFlag(Error_Signature_DataCoveredBySignatureMissing);
    m_errors << PDFTranslationContext::tr("Data covered by signature are not present.");
}

void PDFSignatureVerificationResult::addSignatureNotCoveredBytesWarning(PDFInteger count)
{
    if (!m_flags.testFlag(Warning_Signature_NotCoveredBytes))
    {
        m_flags.setFlag(Warning_Signature_NotCoveredBytes);
        m_warnings << PDFTranslationContext::tr("%1 bytes are not covered by signature.").arg(count);
    }
}

void PDFSignatureVerificationResult::addCertificateCRLValidityTimeExpiredWarning()
{
    if (!m_flags.testFlag(Warning_Certificate_CRLValidityTimeExpired))
    {
        m_flags.setFlag(Warning_Certificate_CRLValidityTimeExpired);
        m_warnings << PDFTranslationContext::tr("Certificate revocation list (CRL) not checked, validity time has expired.");
    }
}

void PDFSignatureVerificationResult::addCertificateQualifiedStatementNotVerifiedWarning()
{
    if (!m_flags.testFlag(Warning_Certificate_QualifiedStatement))
    {
        m_flags.setFlag(Warning_Certificate_QualifiedStatement);
        m_warnings << PDFTranslationContext::tr("Qualified certificate statement not verified.");
    }
}

void PDFSignatureVerificationResult::addCertificateUnableToGetCRLWarning()
{
    if (!m_flags.testFlag(Warning_Certificate_UnableToGetCRL))
    {
        m_flags.setFlag(Warning_Certificate_UnableToGetCRL);
        m_warnings << PDFTranslationContext::tr("Unable to get CRL.");
    }
}

void PDFSignatureVerificationResult::setSignatureFieldQualifiedName(const QString& signatureFieldQualifiedName)
{
    m_signatureFieldQualifiedName = signatureFieldQualifiedName;
}

void PDFSignatureVerificationResult::setSignatureFieldReference(PDFObjectReference signatureFieldReference)
{
    m_signatureFieldReference = signatureFieldReference;
}

void PDFSignatureVerificationResult::addHashAlgorithm(const QString& algorithm)
{
    if (!m_hashAlgorithms.contains(algorithm))
    {
        m_hashAlgorithms << algorithm;
    }
}

void PDFSignatureVerificationResult::validate()
{
    if (isCertificateValid() && isSignatureValid())
    {
        m_flags.setFlag(OK);
    }
}

QDateTime PDFSignatureVerificationResult::getSignatureDate() const
{
    return m_signatureDate;
}

void PDFSignatureVerificationResult::setSignatureDate(const QDateTime& signatureDate)
{
    m_signatureDate = signatureDate;
}

QDateTime PDFSignatureVerificationResult::getTimestampDate() const
{
    return m_timestampDate;
}

void PDFSignatureVerificationResult::setTimestampDate(const QDateTime& timestampDate)
{
    m_timestampDate = timestampDate;
}

QByteArray PDFSignatureVerificationResult::getSignatureHandler() const
{
    return m_signatureHandler;
}

void PDFSignatureVerificationResult::setSignatureHandler(const QByteArray& signatureFilter)
{
    m_signatureHandler = signatureFilter;
}

PDFSignatureVerificationResult::Status PDFSignatureVerificationResult::getCertificateStatus() const
{
    if (hasCertificateError())
    {
        return Status::Error;
    }

    if (hasCertificateWarning())
    {
        return Status::Warning;
    }

    return Status::OK;
}

PDFSignatureVerificationResult::Status PDFSignatureVerificationResult::getSignatureStatus() const
{
    if (hasSignatureError())
    {
        return Status::Error;
    }

    if (hasSignatureWarning())
    {
        return Status::Warning;
    }

    return Status::OK;
}

QString PDFSignatureVerificationResult::getStatusText(Status status)
{
    switch (status)
    {
        case Status::OK:
            return PDFTranslationContext::tr("OK");

        case Status::Warning:
            return PDFTranslationContext::tr("Warning");

        case Status::Error:
            return PDFTranslationContext::tr("Error");

        default:
            break;
    }

    return QString();
}

const PDFClosedIntervalSet& PDFSignatureVerificationResult::getBytesCoveredBySignature() const
{
    return m_bytesCoveredBySignature;
}

void PDFSignatureVerificationResult::setBytesCoveredBySignature(const PDFClosedIntervalSet& bytesCoveredBySignature)
{
    m_bytesCoveredBySignature = bytesCoveredBySignature;
}

PDFSignature::Type PDFSignatureVerificationResult::getType() const
{
    return m_type;
}

void PDFSignatureVerificationResult::setType(const PDFSignature::Type& type)
{
    m_type = type;
}

void PDFPublicKeySignatureHandler::initializeResult(PDFSignatureVerificationResult& result) const
{
    PDFObjectReference signatureFieldReference = m_signatureField->getSelfReference();
    QString signatureFieldQualifiedName = m_signatureField->getName(PDFFormField::NameType::FullyQualified);
    result.setType(m_signatureField->getSignature().getType());
    result.setSignatureFieldReference(signatureFieldReference);
    result.setSignatureFieldQualifiedName(signatureFieldQualifiedName);
    result.setSignatureHandler(m_signatureField->getSignature().getSubfilter());
}

STACK_OF(X509)* PDFPublicKeySignatureHandler::getCertificates(PKCS7* pkcs7)
{
    if (!pkcs7)
    {
        return nullptr;
    }

    if (PKCS7_type_is_signed(pkcs7))
    {
        return pkcs7->d.sign->cert;
    }

    if (PKCS7_type_is_signedAndEnveloped(pkcs7))
    {
        return pkcs7->d.signed_and_enveloped->cert;
    }

    return nullptr;
}

void PDFPublicKeySignatureHandler::verifyCertificate(PDFSignatureVerificationResult& result) const
{
    PDFOpenSSLGlobalLock lock;

    OpenSSL_add_all_algorithms();

    const PDFSignature& signature = m_signatureField->getSignature();
    const QByteArray& content = signature.getContents();

    // Jakub Melka: we will try to get pkcs7 from signature, then
    // verify signer certificates.
    const unsigned char* data = convertByteArrayToUcharPtr(content);
    if (PKCS7* pkcs7 = d2i_PKCS7(nullptr, &data, content.size()))
    {
        X509_STORE* store = X509_STORE_new();
        X509_STORE_CTX* context = X509_STORE_CTX_new();

        // Above functions can fail only if not enough memory. But in this
        // case, this library will crash anyway.
        Q_ASSERT(store);
        Q_ASSERT(context);

        addTrustedCertificates(store);

        STACK_OF(PKCS7_SIGNER_INFO)* signerInfo = PKCS7_get_signer_info(pkcs7);
        const int signerInfoCount = sk_PKCS7_SIGNER_INFO_num(signerInfo);
        STACK_OF(X509)* certificates = getCertificates(pkcs7);
        if (signerInfo && signerInfoCount > 0 && certificates)
        {
            for (int i = 0; i < signerInfoCount; ++i)
            {
                PKCS7_SIGNER_INFO* signerInfoValue = sk_PKCS7_SIGNER_INFO_value(signerInfo, i);
                PKCS7_ISSUER_AND_SERIAL* issuerAndSerial = signerInfoValue->issuer_and_serial;
                X509* signer = X509_find_by_issuer_and_serial(certificates, issuerAndSerial->issuer, issuerAndSerial->serial);

                if (!signer)
                {
                    result.addCertificateMissingError();
                    break;
                }

                if (!X509_STORE_CTX_init(context, store, signer, certificates))
                {
                    result.addCertificateGenericError();
                    break;
                }

                if (!X509_STORE_CTX_set_purpose(context, X509_PURPOSE_SMIME_SIGN))
                {
                    result.addCertificateGenericError();
                    break;
                }

                unsigned long flags = X509_V_FLAG_TRUSTED_FIRST;
                if (m_parameters.ignoreExpirationDate)
                {
                    flags |= X509_V_FLAG_NO_CHECK_TIME;
                }
                X509_STORE_CTX_set_flags(context, flags);

                int verificationResult = X509_verify_cert(context);
                if (verificationResult <= 0)
                {
                    int error = X509_STORE_CTX_get_error(context);
                    switch (error)
                    {
                        case X509_V_OK:
                            // Strange, this should not occur... when X509_verify_cert fails
                            break;

                        case X509_V_ERR_CERT_HAS_EXPIRED:
                            result.addCertificateExpiredError();
                            break;

                        case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
                            result.addCertificateSelfSignedError();
                            break;

                        case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
                            result.addCertificateSelfSignedInChainError();
                            break;

                        case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
                        case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
                            result.addCertificateTrustedNotFoundError();
                            break;

                        case X509_V_ERR_CERT_REVOKED:
                            result.addCertificateRevokedError();
                            break;

                        default:
                            result.addCertificateOtherError(error);
                            break;
                    }

                    // We will add certificate info for all certificates
                    const int count = sk_X509_num(certificates);
                    for (int ii = 0; ii < count; ++ii)
                    {
                        result.addCertificateInfo(getCertificateInfo(sk_X509_value(certificates, ii)));
                    }
                }
                else
                {
                    STACK_OF(X509)* validChain = X509_STORE_CTX_get0_chain(context);
                    const int count = sk_X509_num(validChain);
                    for (int ii = 0; ii < count; ++ii)
                    {
                        result.addCertificateInfo(getCertificateInfo(sk_X509_value(validChain, ii)));
                    }
                }
                X509_STORE_CTX_cleanup(context);
            }
        }
        else
        {
            result.addNoSignaturesError();
        }

        X509_STORE_CTX_free(context);
        X509_STORE_free(store);

        PKCS7_free(pkcs7);
    }
    else
    {
        result.addInvalidCertificateError();
    }

    if (!result.hasCertificateError())
    {
        result.setFlag(PDFSignatureVerificationResult::Certificate_OK, true);
    }
}

BIO* PDFPublicKeySignatureHandler::getSignedDataBuffer(pdf::PDFSignatureVerificationResult& result, QByteArray& outputBuffer) const
{
    const PDFSignature& signature = m_signatureField->getSignature();
    const QByteArray& contents = signature.getContents();
    const QByteArray& sourceData = m_sourceData;

    PDFInteger size = 0;
    const PDFSignature::ByteRanges& byteRanges = signature.getByteRanges();
    for (const PDFSignature::ByteRange& byteRange : byteRanges)
    {
        size += byteRange.size;
    }

    // Sanity checks
    if (size > sourceData.size())
    {
        result.addSignatureDataCoveredBySignatureMissingError();
        return nullptr;
    }

    PDFClosedIntervalSet bytesCoveredBySignature;

    outputBuffer.reserve(size);
    for (const PDFSignature::ByteRange& byteRange : byteRanges)
    {
        PDFInteger startOffset = byteRange.offset; // Offset to the first data byte
        PDFInteger endOffset = byteRange.offset + byteRange.size; // Offset to the byte following last data byte

        if (startOffset == endOffset)
        {
            // This means byte range is zero
            continue;
        }

        if (startOffset > endOffset || startOffset < 0 || endOffset < 0 || startOffset >= m_sourceData.size() || endOffset > m_sourceData.size())
        {
            result.addSignatureDataCoveredBySignatureMissingError();
            return nullptr;
        }

        const int length = endOffset - startOffset;
        outputBuffer.append(sourceData.constData() + startOffset, length);
        bytesCoveredBySignature.addInterval(startOffset, endOffset - 1);
    }

    // Jakub Melka: We must find byte string, which corresponds to signature.
    // We find only first occurence, because second one should not exist - because
    // it will mean that signature must be covered by itself.
    QByteArray hexContents = contents.toHex();
    int index = sourceData.indexOf(hexContents);
    if (index == -1)
    {
        index = sourceData.indexOf(hexContents.toUpper());
    }
    if (index != -1)
    {
        int firstByteIndex = index;
        int lastByteIndex = index + hexContents.size() - 1;

        if (firstByteIndex > 0 && sourceData[firstByteIndex - 1] == '<')
        {
            --firstByteIndex;
        }
        if (lastByteIndex + 1 < sourceData.size() && sourceData[lastByteIndex + 1] == '>')
        {
            ++lastByteIndex;
        }
        bytesCoveredBySignature.addInterval(firstByteIndex, lastByteIndex);
    }

    // We add a warning, that this signature doesn't cover whole source byte range
    if (!bytesCoveredBySignature.isCovered(0, sourceData.size() - 1))
    {
        const PDFInteger notCoveredBytes = sourceData.size() - int(bytesCoveredBySignature.getTotalLength());
        result.addSignatureNotCoveredBytesWarning(notCoveredBytes);
    }

    result.setBytesCoveredBySignature(qMove(bytesCoveredBySignature));

    return BIO_new_mem_buf(outputBuffer.data(), outputBuffer.length());
}

void PDFPublicKeySignatureHandler::verifySignature(PDFSignatureVerificationResult& result) const
{
    PDFOpenSSLGlobalLock lock;

    OpenSSL_add_all_algorithms();

    const PDFSignature& signature = m_signatureField->getSignature();
    const QByteArray& content = signature.getContents();

    // Jakub Melka: we will try to get pkcs7 from signature, then
    // verify signer certificates.
    const unsigned char* data = convertByteArrayToUcharPtr(content);
    if (PKCS7* pkcs7 = d2i_PKCS7(nullptr, &data, content.size()))
    {
        QByteArray buffer;
        if (BIO* inputBuffer = getSignedDataBuffer(result, buffer))
        {
            if (BIO* dataBio = PKCS7_dataInit(pkcs7, inputBuffer))
            {
                // Now, we must read from bio to calculate digests (digest is returned)
                std::array<char, 16384> bioReadBuffer = { };
                int bytesRead = 0;
                do
                {
                    bytesRead = BIO_read(dataBio, bioReadBuffer.data(), int(bioReadBuffer.size()));
                } while (bytesRead > 0);

                STACK_OF(PKCS7_SIGNER_INFO)* signerInfo = PKCS7_get_signer_info(pkcs7);
                addHashAlgorithmFromSignerInfoStack(signerInfo, result);
                addSignatureDateFromSignerInfoStack(signerInfo, result);
                const int signerInfoCount = sk_PKCS7_SIGNER_INFO_num(signerInfo);
                STACK_OF(X509)* certificates = getCertificates(pkcs7);
                if (signerInfo && signerInfoCount > 0 && certificates)
                {
                    for (int i = 0; i < signerInfoCount; ++i)
                    {
                        PKCS7_SIGNER_INFO* signerInfoValue = sk_PKCS7_SIGNER_INFO_value(signerInfo, i);
                        PKCS7_ISSUER_AND_SERIAL* issuerAndSerial = signerInfoValue->issuer_and_serial;
                        X509* signer = X509_find_by_issuer_and_serial(certificates, issuerAndSerial->issuer, issuerAndSerial->serial);

                        if (!signer)
                        {
                            result.addSignatureCertificateMissingError();
                            break;
                        }

                        const int verification = PKCS7_signatureVerify(dataBio, pkcs7, signerInfoValue, signer);
                        if (verification <= 0)
                        {
                            const int reason = ERR_GET_REASON(ERR_get_error());
                            switch (reason)
                            {
                                case PKCS7_R_DIGEST_FAILURE:
                                    result.addSignatureDigestFailureError();
                                    break;

                                default:
                                    result.addSignatureDataOtherError();
                                    break;
                            }
                        }
                    }
                }
                else
                {
                    result.addSignatureNoSignaturesFoundError();
                }

                // According to the documentation, we should not call PKCS7_dataFinal
                // at the end, when pkcs7 is populated.

                BIO_free(dataBio);
            }
            else
            {
                result.addInvalidSignatureError();
            }

            BIO_free(inputBuffer);
        }
        else
        {
            // There is no need for adding error, error is in this case added by getSignedDataBuffer function
        }

        PKCS7_free(pkcs7);
    }
    else
    {
        result.addInvalidSignatureError();
    }

    if (!result.hasSignatureError())
    {
        result.setFlag(PDFSignatureVerificationResult::Signature_OK, true);
    }
}

PDFSignatureVerificationResult PDFSignatureHandler_adbe_pkcs7_detached::verify() const
{
    PDFSignatureVerificationResult result;
    initializeResult(result);
    verifyCertificate(result);
    verifySignature(result);
    result.validate();
    return result;
}

PDFSignatureVerificationResult PDFSignatureHandler_ETSI_CAdES_detached::verify() const
{
    PDFSignatureVerificationResult result;
    initializeResult(result);
    verifyCertificateCAdES(result, X509_PURPOSE_SMIME_SIGN);
    verifySignature(result);
    result.validate();
    return result;
}

PDFSignatureVerificationResult PDFSignatureHandler_ETSI_RFC3161::verify() const
{
    PDFSignatureVerificationResult result;
    initializeResult(result);
    verifyCertificateCAdES(result, X509_PURPOSE_TIMESTAMP_SIGN);
    verifySignatureTimestamp(result);
    result.validate();
    return result;
}

void PDFSignatureHandler_ETSI_RFC3161::verifySignatureTimestamp(PDFSignatureVerificationResult& result) const
{
    PDFOpenSSLGlobalLock lock;

    OpenSSL_add_all_algorithms();

    const PDFSignature& signature = m_signatureField->getSignature();
    const QByteArray& content = signature.getContents();

    // Jakub Melka: we will try to get pkcs7 from signature, then
    // verify signer certificates.
    const unsigned char* data = convertByteArrayToUcharPtr(content);
    if (PKCS7* pkcs7 = d2i_PKCS7(nullptr, &data, content.size()))
    {
        QByteArray buffer;
        if (BIO* inputBuffer = getSignedDataBuffer(result, buffer))
        {
            X509_STORE* store = X509_STORE_new();

            // Above function can fail only if not enough memory. But in this
            // case, this library will crash anyway.
            Q_ASSERT(store);

            addTrustedCertificates(store);

            // Add certificates from DSS store
            STACK_OF(X509)* certificatesFromPkcs7 = getCertificates(pkcs7);
            STACK_OF(X509)* usedCertificates = sk_X509_new_null();

            // First, add all certificates from pkcs7
            for (int i = 0; i < sk_X509_num(certificatesFromPkcs7); ++i)
            {
                X509* certificate = sk_X509_value(certificatesFromPkcs7, i);
                sk_X509_push(usedCertificates, certificate);
                X509_up_ref(certificate);
            }

            if (m_parameters.dss && !m_parameters.dss->getMasterItem()->Cert.empty())
            {
                // Second, add all certificates from document's security store
                for (const QByteArray& certificateData : m_parameters.dss->getMasterItem()->Cert)
                {
                    const unsigned char* certificateDataBuffer = convertByteArrayToUcharPtr(certificateData);
                    if (X509* certificate = d2i_X509(nullptr, &certificateDataBuffer, certificateData.size()))
                    {
                        sk_X509_push(usedCertificates, certificate);
                    }
                }
            }

            // Initialization of verification context
            TS_VERIFY_CTX* ts_context = TS_VERIFY_CTX_new();
            TS_VERIFY_CTX_init(ts_context);
            TS_VERIFY_CTX_set_data(ts_context, inputBuffer);
            TS_VERIFY_CTX_set_flags(ts_context, TS_VFY_ALL_DATA & ~TS_VFY_POLICY & ~TS_VFY_NONCE & ~TS_VFY_TSA_NAME);
            TS_VERIFY_CTX_set_store(ts_context, store);
            TS_VERIFY_CTS_set_certs(ts_context, usedCertificates);

            // Get timestamp and hash algorithm
            if (TS_TST_INFO* info = PKCS7_to_TS_TST_INFO(pkcs7))
            {
                // Date/time of timestamp
                const ASN1_GENERALIZEDTIME* time = TS_TST_INFO_get_time(info);
                result.setTimestampDate(getDateTimeFromASN(time));
            }

            STACK_OF(PKCS7_SIGNER_INFO)* signerInfos = PKCS7_get_signer_info(pkcs7);
            addHashAlgorithmFromSignerInfoStack(signerInfos, result);
            addSignatureDateFromSignerInfoStack(signerInfos, result);

            const int verifyValue = TS_RESP_verify_token(ts_context, pkcs7);
            if (verifyValue <= 0)
            {
                const int reason = ERR_GET_REASON(ERR_get_error());
                switch (reason)
                {
                    case TS_R_MESSAGE_IMPRINT_MISMATCH:
                        result.addSignatureDigestFailureError();
                        break;

                    default:
                        result.addSignatureDataOtherError();
                        break;
                }
            }

            // Finalization of verification context. Function TS_VERIFY_CTX_cleanup also
            // frees all data, such as context, store, etc.
            TS_VERIFY_CTX_cleanup(ts_context);
            TS_VERIFY_CTX_free(ts_context);
        }
        else
        {
            // There is no need for adding error, error is in this case added by getSignedDataBuffer function
        }

        PKCS7_free(pkcs7);
    }
    else
    {
        result.addInvalidSignatureError();
    }

    if (!result.hasSignatureError())
    {
        result.setFlag(PDFSignatureVerificationResult::Signature_OK, true);
    }
}

// This is protected by global mutex, but it is ugly
static PDFSignatureVerificationResult* s_ETSI_currentResult = nullptr;

int PDFSignatureHandler_ETSI_base::verifyCallback(int ok, X509_STORE_CTX* context)
{
    const int errorCode = X509_STORE_CTX_get_error(context);

    switch (errorCode)
    {
        case X509_V_ERR_CRL_NOT_YET_VALID:
        case X509_V_ERR_CRL_HAS_EXPIRED:
        {
            // We will treat this as only warning
            s_ETSI_currentResult->addCertificateCRLValidityTimeExpiredWarning();
            X509_STORE_CTX_set_error(context, X509_V_OK);
            return 1;
        }

        case X509_V_ERR_UNABLE_TO_GET_CRL:
        {
            // We will treat this as only warning. It means that
            // CRL cannot be downloaded or other error occured.
            s_ETSI_currentResult->addCertificateUnableToGetCRLWarning();
            X509_STORE_CTX_set_error(context, X509_V_OK);
            return 1;
        }

        case X509_V_ERR_UNHANDLED_CRITICAL_EXTENSION:
        {
            // We must handle all critical extensions manually
            X509* certificate = X509_STORE_CTX_get_current_cert(context);
            const STACK_OF(X509_EXTENSION)* extensions = X509_get0_extensions(certificate);
            for (int i = 0, extensionsCount = sk_X509_EXTENSION_num(extensions); i < extensionsCount; ++i)
            {
                X509_EXTENSION* extension = sk_X509_EXTENSION_value(extensions, i);

                // Skip non-critical extensions
                if (!X509_EXTENSION_get_critical(extension))
                {
                    continue;
                }

                const ASN1_OBJECT* object = X509_EXTENSION_get_object(extension);
                const int nid = OBJ_obj2nid(object);

                switch (nid)
                {
                    case NID_basic_constraints:
                    case NID_key_usage:
                        // These are handled by OpenSSL
                        continue;

                    case NID_qcStatements:
                    {
                        // We will treat this as only warning
                        s_ETSI_currentResult->addCertificateQualifiedStatementNotVerifiedWarning();
                        X509_STORE_CTX_set_error(context, X509_V_OK);
                        continue;
                    }

                    default:
                        return ok;
                }
            }

            X509_STORE_CTX_set_error(context, X509_V_OK);
            return 1;
        }

        default:
            break;
    }

    return ok;
}

void PDFSignatureHandler_ETSI_base::verifyCertificateCAdES(PDFSignatureVerificationResult& result, int purpose) const
{
    PDFOpenSSLGlobalLock lock;

    s_ETSI_currentResult = &result;

    OpenSSL_add_all_algorithms();

    const PDFSignature& signature = m_signatureField->getSignature();
    const QByteArray& content = signature.getContents();

    // Jakub Melka: we will try to get pkcs7 from signature, then
    // verify signer certificates.
    const unsigned char* data = convertByteArrayToUcharPtr(content);
    if (PKCS7* pkcs7 = d2i_PKCS7(nullptr, &data, content.size()))
    {
        X509_STORE* store = X509_STORE_new();
        X509_STORE_CTX* context = X509_STORE_CTX_new();

        // Above functions can fail only if not enough memory. But in this
        // case, this library will crash anyway.
        Q_ASSERT(store);
        Q_ASSERT(context);

        addTrustedCertificates(store);

        STACK_OF(PKCS7_SIGNER_INFO)* signerInfo = PKCS7_get_signer_info(pkcs7);
        const int signerInfoCount = sk_PKCS7_SIGNER_INFO_num(signerInfo);
        STACK_OF(X509)* certificates = getCertificates(pkcs7);
        if (signerInfo && signerInfoCount > 0 && certificates)
        {
            STACK_OF(X509)* allCertificates = nullptr;
            if (m_parameters.dss && !m_parameters.dss->getMasterItem()->Cert.empty())
            {
                allCertificates = sk_X509_new_null();

                // First, add all certificates from pkcs7
                for (int i = 0; i < sk_X509_num(certificates); ++i)
                {
                    sk_X509_push(allCertificates, sk_X509_value(certificates, i));
                }

                // Second, add all certificates from document's security store
                for (const QByteArray& certificateData : m_parameters.dss->getMasterItem()->Cert)
                {
                    const unsigned char* certificateDataBuffer = convertByteArrayToUcharPtr(certificateData);
                    if (X509* certificate = d2i_X509(nullptr, &certificateDataBuffer, certificateData.size()))
                    {
                        sk_X509_push(allCertificates, certificate);
                    }
                }
            }
            STACK_OF(X509)* usedCertificates = allCertificates ? allCertificates : certificates;

            // Jakub Melka: add certificate revocation lists
            if (m_parameters.dss && !m_parameters.dss->getMasterItem()->CRL.empty())
            {
                for (const QByteArray& crlData : m_parameters.dss->getMasterItem()->CRL)
                {
                    const unsigned char* crlDataBuffer = convertByteArrayToUcharPtr(crlData);
                    if (X509_CRL* crl = d2i_X509_CRL(nullptr, &crlDataBuffer, crlData.size()))
                    {
                        X509_STORE_add_crl(store, crl);
                        X509_CRL_free(crl);
                    }
                }
            }

            for (int i = 0; i < signerInfoCount; ++i)
            {
                PKCS7_SIGNER_INFO* signerInfoValue = sk_PKCS7_SIGNER_INFO_value(signerInfo, i);
                PKCS7_ISSUER_AND_SERIAL* issuerAndSerial = signerInfoValue->issuer_and_serial;
                X509* signer = X509_find_by_issuer_and_serial(usedCertificates, issuerAndSerial->issuer, issuerAndSerial->serial);

                if (!signer)
                {
                    result.addCertificateMissingError();
                    break;
                }

                if (!X509_STORE_CTX_init(context, store, signer, usedCertificates))
                {
                    result.addCertificateGenericError();
                    break;
                }

                if (!X509_STORE_CTX_set_purpose(context, purpose))
                {
                    result.addCertificateGenericError();
                    break;
                }

                unsigned long flags = X509_V_FLAG_TRUSTED_FIRST | X509_V_FLAG_CRL_CHECK | X509_V_FLAG_CRL_CHECK_ALL | X509_V_FLAG_EXTENDED_CRL_SUPPORT;
                if (m_parameters.ignoreExpirationDate)
                {
                    flags |= X509_V_FLAG_NO_CHECK_TIME;
                }
                X509_STORE_CTX_set_flags(context, flags);
                X509_STORE_CTX_set_verify_cb(context, &PDFSignatureHandler_ETSI_CAdES_detached::verifyCallback);

                int verificationResult = X509_verify_cert(context);
                if (verificationResult <= 0)
                {
                    int error = X509_STORE_CTX_get_error(context);
                    switch (error)
                    {
                        case X509_V_OK:
                            // Strange, this should not occur... when X509_verify_cert fails
                            break;

                        case X509_V_ERR_CERT_HAS_EXPIRED:
                            result.addCertificateExpiredError();
                            break;

                        case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
                            result.addCertificateSelfSignedError();
                            break;

                        case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
                            result.addCertificateSelfSignedInChainError();
                            break;

                        case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
                        case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
                            result.addCertificateTrustedNotFoundError();
                            break;

                        case X509_V_ERR_CERT_REVOKED:
                            result.addCertificateRevokedError();
                            break;

                        default:
                            result.addCertificateOtherError(error);
                            break;
                    }

                    // We will add certificate info for all certificates
                    const int count = sk_X509_num(usedCertificates);
                    for (int ii = 0; ii < count; ++ii)
                    {
                        result.addCertificateInfo(getCertificateInfo(sk_X509_value(usedCertificates, ii)));
                    }
                }
                else
                {
                    STACK_OF(X509)* validChain = X509_STORE_CTX_get0_chain(context);
                    const int count = sk_X509_num(validChain);
                    for (int ii = 0; ii < count; ++ii)
                    {
                        result.addCertificateInfo(getCertificateInfo(sk_X509_value(validChain, ii)));
                    }
                }
                X509_STORE_CTX_cleanup(context);
            }

            if (allCertificates)
            {
                for (int i = sk_X509_num(certificates); i < sk_X509_num(allCertificates); ++i)
                {
                    X509_free(sk_X509_value(allCertificates, i));
                }

                sk_X509_free(allCertificates);
            }
        }
        else
        {
            result.addNoSignaturesError();
        }

        X509_STORE_CTX_free(context);
        X509_STORE_free(store);

        PKCS7_free(pkcs7);
    }
    else
    {
        result.addInvalidCertificateError();
    }

    if (!result.hasCertificateError())
    {
        result.setFlag(PDFSignatureVerificationResult::Certificate_OK, true);
    }
}

PDFSignatureVerificationResult PDFSignatureHandler_adbe_pkcs7_rsa_sha1::verify() const
{
    PDFSignatureVerificationResult result;
    initializeResult(result);

    verifyRSACertificate(result);
    verifyRSASignature(result);

    result.validate();
    return result;
}

X509* PDFSignatureHandler_adbe_pkcs7_rsa_sha1::createCertificate(size_t index) const
{
    const PDFSignature& signature = m_signatureField->getSignature();
    const std::vector<QByteArray>* certificates = signature.getCertificates();
    if (certificates && index < certificates->size())
    {
        const QByteArray& certificateSize = (*certificates)[index];
        const unsigned char* data = convertByteArrayToUcharPtr(certificateSize);
        return d2i_X509(nullptr, &data, certificateSize.size());
    }

    return nullptr;
}

bool PDFSignatureHandler_adbe_pkcs7_rsa_sha1::getMessageDigest(const QByteArray& message,
                                                               ASN1_OCTET_STRING* encryptedString,
                                                               RSA* rsa,
                                                               int& algorithmNID,
                                                               QByteArray& digest) const
{
    if (!getMessageDigestAlgorithm(encryptedString, rsa, algorithmNID))
    {
        return false;
    }

    if (const EVP_MD* md = EVP_get_digestbynid(algorithmNID))
    {
        unsigned int messageDigestSize = EVP_MD_size(md);
        digest.resize(messageDigestSize);

        EVP_MD_CTX* context = EVP_MD_CTX_new();
        Q_ASSERT(context);

        EVP_DigestInit(context, md);
        EVP_DigestUpdate(context, message.constData(), message.size());
        EVP_DigestFinal(context, convertByteArrayToUcharPtr(digest), &messageDigestSize);

        EVP_MD_CTX_free(context);
        return true;
    }

    return false;
}

bool PDFSignatureHandler_adbe_pkcs7_rsa_sha1::getMessageDigestAlgorithm(ASN1_OCTET_STRING* encryptedString,
                                                                        RSA* rsa,
                                                                        int& algorithmNID) const
{
    algorithmNID = 0;

    int size = RSA_size(rsa);
    std::vector<unsigned char> decryptedBuffer(size, 0);
    const int signatureSize = RSA_public_decrypt(encryptedString->length, encryptedString->data, decryptedBuffer.data(), rsa, RSA_PKCS1_PADDING);

    if (signatureSize <= 0)
    {
        return false;
    }

    Q_ASSERT(static_cast<std::size_t>(signatureSize) < decryptedBuffer.size());

    const unsigned char* decryptedBufferPtr = decryptedBuffer.data();
    if (X509_SIG* x509_sig = d2i_X509_SIG(nullptr, &decryptedBufferPtr, signatureSize))
    {
        const X509_ALGOR* algorithm = nullptr;
        const ASN1_OBJECT* algorithmDescriptor = nullptr;

        X509_SIG_get0(x509_sig, &algorithm, nullptr);
        X509_ALGOR_get0(&algorithmDescriptor, nullptr, nullptr, algorithm);
        algorithmNID = OBJ_obj2nid(algorithmDescriptor);

        X509_SIG_free(x509_sig);
        return true;
    }

    return false;
}

void PDFSignatureHandler_adbe_pkcs7_rsa_sha1::verifyRSACertificate(PDFSignatureVerificationResult& result) const
{
    if (X509* certificate = createCertificate(0))
    {
        STACK_OF(X509)* certificates = sk_X509_new_null();
        sk_X509_push(certificates, certificate);

        for (size_t i = 1;; ++i)
        {
            if (X509* currentCertificate = createCertificate(i))
            {
                sk_X509_push(certificates, currentCertificate);
            }
            else
            {
                break;
            }
        }

        X509_STORE* store = X509_STORE_new();
        X509_STORE_CTX* context = X509_STORE_CTX_new();

        // Above functions can fail only if not enough memory. But in this
        // case, this library will crash anyway.
        Q_ASSERT(store);
        Q_ASSERT(context);

        addTrustedCertificates(store);

        X509* signer = certificate;
        if (!X509_STORE_CTX_init(context, store, signer, certificates))
        {
            result.addCertificateGenericError();
        }

        if (!X509_STORE_CTX_set_purpose(context, X509_PURPOSE_SMIME_SIGN))
        {
            result.addCertificateGenericError();
        }

        if (!result.hasCertificateError())
        {
            unsigned long flags = X509_V_FLAG_TRUSTED_FIRST;
            if (m_parameters.ignoreExpirationDate)
            {
                flags |= X509_V_FLAG_NO_CHECK_TIME;
            }
            X509_STORE_CTX_set_flags(context, flags);

            int verificationResult = X509_verify_cert(context);
            if (verificationResult <= 0)
            {
                int error = X509_STORE_CTX_get_error(context);
                switch (error)
                {
                    case X509_V_OK:
                        // Strange, this should not occur... when X509_verify_cert fails
                        break;

                    case X509_V_ERR_CERT_HAS_EXPIRED:
                        result.addCertificateExpiredError();
                        break;

                    case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
                        result.addCertificateSelfSignedError();
                        break;

                    case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
                        result.addCertificateSelfSignedInChainError();
                        break;

                    case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
                    case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
                        result.addCertificateTrustedNotFoundError();
                        break;

                    case X509_V_ERR_CERT_REVOKED:
                        result.addCertificateRevokedError();
                        break;

                    default:
                        result.addCertificateOtherError(error);
                        break;
                }

                // We will add certificate info for all certificates
                const int count = sk_X509_num(certificates);
                for (int i = 0; i < count; ++i)
                {
                    result.addCertificateInfo(getCertificateInfo(sk_X509_value(certificates, i)));
                }
            }
            else
            {
                STACK_OF(X509)* validChain = X509_STORE_CTX_get0_chain(context);
                const int count = sk_X509_num(validChain);
                for (int i = 0; i < count; ++i)
                {
                    result.addCertificateInfo(getCertificateInfo(sk_X509_value(validChain, i)));
                }
            }

            X509_STORE_CTX_cleanup(context);
        }

        X509_STORE_CTX_free(context);
        X509_STORE_free(store);

        sk_X509_pop_free(certificates, X509_free);
    }
    else
    {
        result.addInvalidCertificateError();
    }

    if (!result.hasCertificateError())
    {
        result.setFlag(PDFSignatureVerificationResult::Certificate_OK, true);
    }
}

void PDFSignatureHandler_adbe_pkcs7_rsa_sha1::verifyRSASignature(PDFSignatureVerificationResult& result) const
{
    // Jakub Melka: we will use first certificate to validate signature
    openssl_ptr<X509> certificate(createCertificate(0), X509_free);
    if (!certificate)
    {
        result.addSignatureCertificateMissingError();
        return;
    }

    EVP_PKEY* evpKey = X509_get0_pubkey(certificate.get());
    if (!evpKey)
    {
        result.addSignatureCertificateMissingError();
        return;
    }

    openssl_ptr<RSA> rsa(EVP_PKEY_get1_RSA(evpKey), RSA_free);
    if (!rsa)
    {
        result.addSignatureCertificateMissingError();
        return;
    }

    QByteArray outputBuffer;
    openssl_ptr<BIO> bio(this->getSignedDataBuffer(result, outputBuffer), BIO_free_all);
    if (bio)
    {
        const PDFSignature& signature = m_signatureField->getSignature();
        const QByteArray& signKey = signature.getContents();

        const unsigned char* encryptedSign = convertByteArrayToUcharPtr(signKey);
        const unsigned int encryptedSignLength = signKey.length();

        openssl_ptr<ASN1_OCTET_STRING> encryptedString(d2i_ASN1_OCTET_STRING(nullptr, &encryptedSign, encryptedSignLength), ASN1_OCTET_STRING_free);
        if (encryptedString)
        {
            int algorithmNID = NID_undef;
            QByteArray digestBuffer;
            if (!getMessageDigest(outputBuffer, encryptedString.get(), rsa.get(), algorithmNID, digestBuffer))
            {
                result.addSignatureDataOtherError();
                return;
            }

            const unsigned char* digest = convertByteArrayToUcharPtr(digestBuffer);
            const unsigned int digestLength = digestBuffer.length();

            std::array<char, 64> buffer = { };
            OBJ_obj2txt(buffer.data(), int(buffer.size() - 1), OBJ_nid2obj(algorithmNID), 0);
            result.addHashAlgorithm(QString::fromLatin1(buffer.data()));

            const int verifyValue = RSA_verify(algorithmNID, digest, digestLength, encryptedString->data, encryptedString->length, rsa.get());

            if (verifyValue == 0)
            {
                // We have failed, probably due to invalid signature
                const unsigned long errorCode = ERR_GET_REASON(ERR_get_error());

                switch (errorCode)
                {
                    case RSA_R_DIGEST_DOES_NOT_MATCH:
                        result.addSignatureDigestFailureError();
                        break;

                    default:
                        result.addSignatureDataOtherError();
                        break;
                }
            }
        }
        else
        {
            result.addSignatureDataOtherError();
        }
    }

    if (!result.hasSignatureError())
    {
        result.setFlag(PDFSignatureVerificationResult::Signature_OK, true);
    }
}

PDFSignatureVerificationResult PDFSignatureHandler_adbe_pkcs7_sha1::verify() const
{
    PDFSignatureVerificationResult result;
    initializeResult(result);
    verifyCertificate(result);
    verifySignature(result);
    result.validate();
    return result;
}

BIO* PDFSignatureHandler_adbe_pkcs7_sha1::getSignedDataBuffer(PDFSignatureVerificationResult& result, QByteArray& outputBuffer) const
{
    QByteArray temporaryBuffer;
    if (BIO* bio = PDFPublicKeySignatureHandler::getSignedDataBuffer(result, temporaryBuffer))
    {
        // Calculate SHA1
        outputBuffer.resize(SHA_DIGEST_LENGTH);
        SHA1(convertByteArrayToUcharPtr(temporaryBuffer), temporaryBuffer.length(), convertByteArrayToUcharPtr(outputBuffer));
        BIO_free(bio);

        return BIO_new_mem_buf(outputBuffer.data(), outputBuffer.length());
    }

    return nullptr;
}

PDFCertificateInfo PDFPublicKeySignatureHandler::getCertificateInfo(X509* certificate)
{
    return PDFCertificateInfo::getCertificateInfo(certificate);
}

QString PDFPublicKeySignatureHandler::getStringFromX509Name(X509_NAME* name, int nid)
{
    QString result;

    const int stringLocation = X509_NAME_get_index_by_NID(name, nid, -1);
    X509_NAME_ENTRY* entry = X509_NAME_get_entry(name, stringLocation);
    return getStringFromASN1_STRING(X509_NAME_ENTRY_get_data(entry));
}

QString pdf::PDFPublicKeySignatureHandler::getStringFromASN1_STRING(ASN1_STRING* string)
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
}

QDateTime PDFPublicKeySignatureHandler::getDateTimeFromASN(const ASN1_TIME* time)
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
}

void PDFPublicKeySignatureHandler::addHashAlgorithmFromSignerInfoStack(STACK_OF(PKCS7_SIGNER_INFO)* signerInfoStack, PDFSignatureVerificationResult& result)
{
    if (!signerInfoStack)
    {
        // No signature info provided
        return;
    }

    const int count = sk_PKCS7_SIGNER_INFO_num(signerInfoStack);
    for (int i = 0; i < count; ++i)
    {
        PKCS7_SIGNER_INFO* signerInfoValue = sk_PKCS7_SIGNER_INFO_value(signerInfoStack, i);
        if (signerInfoValue && signerInfoValue->digest_alg && signerInfoValue->digest_alg->algorithm)
        {
            std::array<char, 64> buffer = { };
            OBJ_obj2txt(buffer.data(), int(buffer.size() - 1), signerInfoValue->digest_alg->algorithm, 0);
            result.addHashAlgorithm(QString::fromLatin1(buffer.data()));
        }
    }
}

void PDFPublicKeySignatureHandler::addSignatureDateFromSignerInfoStack(STACK_OF(PKCS7_SIGNER_INFO)* signerInfoStack, PDFSignatureVerificationResult& result)
{
    if (!signerInfoStack)
    {
        // No signature info provided
        return;
    }

    if (sk_PKCS7_SIGNER_INFO_num(signerInfoStack) != 1)
    {
        // Multiple signature infos, or no signature info
        return;
    }

    // Jakub Melka: We will get signed attribute from signer info.
    PKCS7_SIGNER_INFO* signerInfo = sk_PKCS7_SIGNER_INFO_value(signerInfoStack, 0);
    ASN1_TYPE* attribute = PKCS7_get_signed_attribute(signerInfo, NID_pkcs9_signingTime);

    if (!attribute)
    {
        return;
    }

    switch (attribute->type)
    {
        case V_ASN1_UTCTIME:
            result.setSignatureDate(getDateTimeFromASN(attribute->value.utctime));
            break;

        case V_ASN1_GENERALIZEDTIME:
            result.setSignatureDate(getDateTimeFromASN(attribute->value.generalizedtime));
            break;

        default:
            break;
    }
}


}   // namespace pdf

#ifdef Q_OS_WIN
#include <Windows.h>
#include <wincrypt.h>
#if defined(PDF4QT_USE_PRAGMA_LIB)
#pragma comment(lib, "crypt32.lib")
#endif
#endif

void pdf::PDFPublicKeySignatureHandler::addTrustedCertificates(X509_STORE* store) const
{
    if (m_parameters.store)
    {
        const PDFCertificateEntries& certificates = m_parameters.store->getCertificates();
        for (const auto& entry : certificates)
        {
            QByteArray certificateData = entry.info.getCertificateData();
            const unsigned char* pointer = convertByteArrayToUcharPtr(certificateData);
            X509* certificate = d2i_X509(nullptr, &pointer, certificateData.length());
            if (certificate)
            {
                X509_STORE_add_cert(store, certificate);
                X509_free(certificate);
            }
        }
    }

#ifdef Q_OS_WIN
    if (m_parameters.useSystemCertificateStore)
    {
        HCERTSTORE certStore = CertOpenSystemStore(0, L"ROOT");
        PCCERT_CONTEXT context = nullptr;
        if (certStore)
        {
            while (context = CertEnumCertificatesInStore(certStore, context))
            {
                const unsigned char* pointer = context->pbCertEncoded;
                X509* certificate = d2i_X509(nullptr, &pointer, context->cbCertEncoded);
                if (certificate)
                {
                    X509_STORE_add_cert(store, certificate);
                    X509_free(certificate);
                }
            }

            CertCloseStore(certStore, CERT_CLOSE_STORE_FORCE_FLAG);
        }
    }
#endif
}

#if defined(PDF4QT_COMPILER_MINGW) || defined(PDF4QT_COMPILER_GCC)
#pragma GCC diagnostic pop
#endif

#if defined(PDF4QT_COMPILER_MSVC)
#pragma warning(pop)
#endif
