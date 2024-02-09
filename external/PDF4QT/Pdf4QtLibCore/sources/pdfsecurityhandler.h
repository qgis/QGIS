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

#ifndef PDFSECURITYHANDLER_H
#define PDFSECURITYHANDLER_H

#include "pdfglobal.h"
#include "pdfobject.h"
#include "pdfcertificatestore.h"

#include <QByteArray>
#include <QSharedPointer>

#include <map>
#include <functional>

class QRandomGenerator;

namespace pdf
{
class PDFObjectFactory;

enum class EncryptionMode
{
    None,       ///< Document is not encrypted
    Standard,   ///< Document is encrypted and using standard security handler
    PublicKey,  ///< Document is encrypted and using public key security handler
    Custom      ///< Document is encrypted and using custom security handler. Custom handlers must return this value.
};

enum class CryptFilterType
{
    None,       ///< The application shall decrypt the data using the security handler
    V2,         ///< Use file encryption key for RC4 algorithm
    AESV2,      ///< Use file encryption key for AES algorithm
    AESV3,      ///< Use file encryption key for AES 256 bit algorithm
    Identity,   ///< Don't decrypt anything, use identity function
};

enum class AuthEvent
{
    DocOpen,    ///< Authorize on document open
    EFOpen      ///< Authorize when accessing embedded file stream
};

enum class CryptFilterApplication
{
    String,         ///< Apply filter to decrypt/encrypt strings
    Stream,         ///< Apply filter to decrypt/encrypt streams
    EmbeddedFile    ///< Apply filter to decrypt/encrypt embedded file streams
};

struct CryptFilter
{
    bool operator ==(const CryptFilter&) const = default;
    bool operator !=(const CryptFilter&) const = default;

    CryptFilterType type = CryptFilterType::None;
    AuthEvent authEvent = AuthEvent::DocOpen;
    int keyLength = 0; ///< Key length in bytes
    QByteArrayList recipients; ///< Recipients for public key security handler
    bool encryptMetadata = true; ///< Encrypt metadata (for public key encryption)
};

class PDFSecurityHandler;
using PDFSecurityHandlerPointer = QSharedPointer<PDFSecurityHandler>;

class PDFStandardSecurityHandler;

class PDFSecurityHandler
{
public:
    explicit PDFSecurityHandler() = default;
    virtual ~PDFSecurityHandler() = default;

    static constexpr const char* IDENTITY_FILTER_NAME = "Identity";
    static constexpr const char* OBJECT_REFERENCE_DICTIONARY_NAME = "Pdf4Qt_ObjectReference";

    enum class Permission : uint32_t
    {
        PrintLowResolution       = (1 <<  2),
        Modify                   = (1 <<  3),
        CopyContent              = (1 <<  4),
        ModifyInteractiveItems   = (1 <<  5),
        ModifyFormFields         = (1 <<  8),
        Accessibility            = (1 <<  9),
        Assemble                 = (1 << 10),
        PrintHighResolution      = (1 << 11)
    };

    enum class AuthorizationResult
    {
        NoAuthorizationRequired,
        UserAuthorized,
        OwnerAuthorized,
        Failed,
        Cancelled
    };

    enum class EncryptionScope
    {
        String,
        Stream,
        EmbeddedFile
    };

    /// Retrieve encryption mode (none/standard encryption/custom)
    virtual EncryptionMode getMode() const = 0;

    /// Creates a clone of this object
    virtual PDFSecurityHandler* clone() const = 0;

    /// Performs authentication of the document content access. First, algorithm should check,
    /// if empty password allows document access (so, for example, only owner password is provided).
    /// If this fails, function \p getPasswordCallback is called to retrieve user entered password.
    /// This callback function also has pointer to bool parameter, which sets to false, if user wants
    /// to cancel the authentication (and \p Cancelled authentication result is returned) or true,
    /// to try provided password.
    /// \param getPasswordCallback Callback to get user password
    /// \param authorizeOwnerOnly Authorize owner only
    /// \returns Result of authentication
    virtual AuthorizationResult authenticate(const std::function<QString(bool*)>& getPasswordCallback, bool authorizeOwnerOnly) = 0;

    /// Decrypts the PDF object. This function works properly only (and only if)
    /// \p authenticate function returns user/owner authorization code.
    /// \param object Object to be decrypted
    /// \param reference Reference of indirect object (some algorithms require to generate key also from reference)
    /// \returns Decrypted object
    PDFObject decryptObject(const PDFObject& object, PDFObjectReference reference) const;

    /// Encrypts the PDF object. This function works properly only (and only if)
    /// \p authenticate function returns user/owner authorization code.
    /// \param object Object to be encrypted
    /// \param reference Reference of indirect object (some algorithms require to generate key also from reference)
    /// \returns Encrypted object
    PDFObject encryptObject(const PDFObject& object, PDFObjectReference reference) const;

    /// Decrypts the PDF object data. This function works properly only (and only if)
    /// \p authenticate function returns user/owner authorization code.
    /// \param data Data to be decrypted
    /// \param reference Reference of indirect object (some algorithms require to generate key also from reference)
    /// \param encryptionScope Scope of the encryption (if it is string/stream/...)
    /// \returns Decrypted object data
    virtual QByteArray decrypt(const QByteArray& data, PDFObjectReference reference, EncryptionScope encryptionScope) const = 0;

    /// Decrypts data using specified filter. Throws exception, if filter is not found.
    /// \param data Data to be decrypted
    /// \param filterName Filter name to be used to decrypt the data
    /// \param reference Reference object
    virtual QByteArray decryptByFilter(const QByteArray& data, const QByteArray& filterName, PDFObjectReference reference) const = 0;

    /// Encrypts the PDF object data. This function works properly only (and only if)
    /// \p authenticate function returns user/owner authorization code.
    /// \param data Data to be encrypted
    /// \param reference Reference of indirect object (some algorithms require to generate key also from reference)
    /// \param encryptionScope Scope of the encryption (if it is string/stream/...)
    /// \returns Encrypted object data
    virtual QByteArray encrypt(const QByteArray& data, PDFObjectReference reference, EncryptionScope encryptionScope) const = 0;

    /// Encrypts data using specified filter. Throws exception, if filter is not found.
    /// \param data Data to be encrypted
    /// \param filterName Filter name to be used to encrypt the data
    /// \param reference Reference object
    virtual QByteArray encryptByFilter(const QByteArray& data, const QByteArray& filterName, PDFObjectReference reference) const = 0;

    /// Returns true, if given permission is allowed in the current authorization context.
    /// If owner is authorized, then this function allways returns true.
    virtual bool isAllowed(Permission permission) const = 0;

    /// Returns true, if metadata are encrypted
    virtual bool isMetadataEncrypted() const = 0;

    /// Returns true, if encryption is allowed (and document
    /// can be encrypted, if it was decrypted)
    virtual bool isEncryptionAllowed() const = 0;

    /// Returns result of authorization process
    virtual AuthorizationResult getAuthorizationResult() const = 0;

    /// Creates encryption dictionary object
    virtual PDFObject createEncryptionDictionaryObject() const = 0;

    /// Returns version of the encryption
    int getVersion() const { return m_V; }

    /// Creates a security handler from the object. If object is null, then
    /// "None" security handler is created. If error occurs, then exception is thrown.
    /// \param encryptionDictionaryObject Encryption dictionary object
    /// \param id First part of the id of the document
    static PDFSecurityHandlerPointer createSecurityHandler(const PDFObject& encryptionDictionaryObject, const QByteArray& id);

protected:
    friend class PDFSecurityHandlerFactory;

    static bool parseBool(const PDFDictionary* dictionary, const char* key, bool required, bool defaultValue = true);
    static QByteArray parseName(const PDFDictionary* dictionary, const char* key, bool required, const char* defaultValue = nullptr);
    static PDFInteger parseInt(const PDFDictionary* dictionary, const char* key, bool required, PDFInteger defaultValue = -1);
    static CryptFilter parseCryptFilter(PDFInteger length, const PDFObject& object, bool publicKey);
    static PDFSecurityHandlerPointer createSecurityHandlerInstance(const PDFDictionary* dictionary);
    static QByteArrayList parseRecipients(const PDFDictionary* dictionary);

    static void parseCryptFilters(const PDFDictionary* dictionary, PDFSecurityHandler& handler, int Length, bool publicKey);
    static void parseDataStandardSecurityHandler(const PDFDictionary* dictionary, const QByteArray& id, int Length, PDFStandardSecurityHandler& handler);

    /// Fills encryption dictionary with basic data
    /// \param factory Factory
    void fillEncryptionDictionary(PDFObjectFactory& factory, bool publicKeyHandler) const;

    /// Version of the encryption, shall be a number from 1 to 5, according the
    /// PDF specification. Other values are invalid.
    int m_V = 0;

    /// Length of the key to encrypt/decrypt the document in bits.
    int m_keyLength = 40;

    /// Map containing crypt filters.
    std::map<QByteArray, CryptFilter> m_cryptFilters;

    /// Default filter
    CryptFilter m_filterDefault;

    /// Crypt filter for decrypting strings
    CryptFilter m_filterStrings;

    /// Crypt filter for decrypting streams
    CryptFilter m_filterStreams;

    /// Crypt filter for decrypting embedded files
    CryptFilter m_filterEmbeddedFiles;
};

/// Specifies the security of unencrypted document
class PDFNoneSecurityHandler : public PDFSecurityHandler
{
public:
    virtual EncryptionMode getMode() const override { return EncryptionMode::None; }
    virtual PDFSecurityHandler* clone() const override { return new PDFNoneSecurityHandler(); }
    virtual AuthorizationResult authenticate(const std::function<QString(bool*)>&, bool) override { return AuthorizationResult::OwnerAuthorized; }
    virtual QByteArray decrypt(const QByteArray& data, PDFObjectReference, EncryptionScope) const override { return data; }
    virtual QByteArray decryptByFilter(const QByteArray& data, const QByteArray&, PDFObjectReference) const override { return data; }
    virtual QByteArray encrypt(const QByteArray& data, PDFObjectReference, EncryptionScope) const override { return data; }
    virtual QByteArray encryptByFilter(const QByteArray& data, const QByteArray&, PDFObjectReference) const override { return data; }
    virtual bool isMetadataEncrypted() const override { return true; }
    virtual bool isAllowed(Permission) const override { return true; }
    virtual bool isEncryptionAllowed() const override { return true; }
    virtual AuthorizationResult getAuthorizationResult() const override { return AuthorizationResult::NoAuthorizationRequired; }
    virtual PDFObject createEncryptionDictionaryObject() const override { return PDFObject(); }
};

class PDFStandardOrPublicSecurityHandler : public PDFSecurityHandler
{
public:
    virtual QByteArray decrypt(const QByteArray& data, PDFObjectReference reference, EncryptionScope encryptionScope) const override;
    virtual QByteArray decryptByFilter(const QByteArray& data, const QByteArray& filterName, PDFObjectReference reference) const override;
    virtual QByteArray encrypt(const QByteArray& data, PDFObjectReference reference, EncryptionScope encryptionScope) const override;
    virtual QByteArray encryptByFilter(const QByteArray& data, const QByteArray& filterName, PDFObjectReference reference) const override;
    virtual AuthorizationResult getAuthorizationResult() const override { return m_authorizationData.authorizationResult; }
    virtual bool isEncryptionAllowed() const override { return m_authorizationData.isAuthorized(); }

    /// Adjusts the password according to the PDF specification
    static QByteArray adjustPassword(const QString& password, int revision);

    struct AuthorizationData
    {
        bool isAuthorized() const { return authorizationResult == AuthorizationResult::UserAuthorized || authorizationResult == AuthorizationResult::OwnerAuthorized; }

        AuthorizationResult authorizationResult = AuthorizationResult::Failed;
        QByteArray fileEncryptionKey;
    };

protected:
    friend class PDFSecurityHandlerFactory;

    /// Decrypts data using specified filter. This function can be called only, if authorization was successfull.
    /// \param data Data to be decrypted
    /// \param filter Filter to be used for decryption
    /// \param reference Object reference for key generation
    /// \returns Decrypted data
    QByteArray decryptUsingFilter(const QByteArray& data, CryptFilter filter, PDFObjectReference reference) const;

    /// Encrypts data using specified filter. This function can be called only, if authorization was successfull.
    /// \param data Data to be encrypted
    /// \param filter Filter to be used for encryption
    /// \param reference Object reference for key generation
    /// \returns Encrypted data
    QByteArray encryptUsingFilter(const QByteArray& data, CryptFilter filter, PDFObjectReference reference) const;

    /// Returns true, if character with unicode code is non-ascii space character
    /// according the RFC 3454, section C.1.2
    /// \param unicode Unicode code to be tested
    static bool isUnicodeNonAsciiSpaceCharacter(ushort unicode);

    /// Returns true, if character with unicode code is mapped to nothing,
    /// according the RFC 3454, section B.1
    /// \param unicode Unicode code to be tested
    static bool isUnicodeMappedToNothing(ushort unicode);

    std::vector<uint8_t> createV2_ObjectEncryptionKey(PDFObjectReference reference, CryptFilter filter) const;
    std::vector<uint8_t> createAESV2_ObjectEncryptionKey(PDFObjectReference reference) const;
    CryptFilter getCryptFilter(EncryptionScope encryptionScope) const;

    /// Authorization data
    AuthorizationData m_authorizationData;
};

/// Specifies the security using standard security handler (see PDF specification
/// for details).
class PDFStandardSecurityHandler : public PDFStandardOrPublicSecurityHandler
{
public:
    virtual EncryptionMode getMode() const override { return EncryptionMode::Standard; }
    virtual PDFSecurityHandler* clone() const override;
    virtual AuthorizationResult authenticate(const std::function<QString(bool*)>& getPasswordCallback, bool authorizeOwnerOnly) override;
    virtual bool isMetadataEncrypted() const override { return m_encryptMetadata; }
    virtual bool isAllowed(Permission permission) const override { return m_authorizationData.authorizationResult == AuthorizationResult::OwnerAuthorized || (m_permissions & static_cast<uint32_t>(permission)); }
    virtual PDFObject createEncryptionDictionaryObject() const override;

private:
    friend class PDFSecurityHandler;
    friend class PDFSecurityHandlerFactory;
    friend PDFSecurityHandlerPointer PDFSecurityHandler::createSecurityHandler(const PDFObject& encryptionDictionaryObject, const QByteArray& id);

    struct UserOwnerData_r6
    {
        QByteArray hash;
        QByteArray validationSalt;
        QByteArray keySalt;
    };

    /// Creates file encryption key from passed password, based on the revision
    /// \param password Password to be used to create file encryption key
    /// \note Password must be in PDFDocEncoding for revision 4 or earlier,
    ///       otherwise it must be encoded in UTF-8.
    QByteArray createFileEncryptionKey(const QByteArray& password) const;

    /// Creates entry value U based on the file encryption key. This function
    /// is valid only for revisions 2, 3 and 4.
    /// \param fileEncryptionKey File encryption key
    QByteArray createEntryValueU_r234(const QByteArray& fileEncryptionKey) const;

    /// Creates user password from the owner password. User password must be then
    /// authenticated.
    QByteArray createUserPasswordFromOwnerPassword(const QByteArray& password) const;

    /// Creates 32-byte padded password from the passed password. If password is empty,
    /// then padding password is returned.
    std::array<uint8_t, 32>  createPaddedPassword32(const QByteArray& password) const;

    /// Creates hash using algorithm 2.B for revision 6. Input is input of the hash,
    /// and if \p useUserKey is used, user key is considered in the hash.
    /// \param input Input of the hash
    /// \param inputPassword Input password
    /// \param useUserKey Use user key in the hash computation
    QByteArray createHash_r6(const QByteArray& input, const QByteArray& inputPassword, bool useUserKey) const;

    /// Parses parts of the user/owner data (U/O values of the encryption dictionary)
    UserOwnerData_r6 parseParts(const QByteArray& data) const;

    /// Revision number of standard security number
    int m_R = 0;

    /// 32 byte string if revision number is 4 or less, or 48 byte string,
    /// if revision number is 6, based on both owner and user passwords,
    /// used for authenticate owner, and create a file encryption key.
    QByteArray m_O;

    /// 32 byte string if revision number is 4 or less, or 48 byte string,
    /// if revision number is 6, based on both owner and user passwords,
    /// used for authenticate owner, and create a file encryption key.
    QByteArray m_U;

    /// For revision number 6 only. 32 bytes string based on both owner
    /// and user password, that shall be used to compute file encryption key.
    QByteArray m_OE;

    /// For revision number 6 only. 32 bytes string based on both owner
    /// and user password, that shall be used to compute file encryption key.
    QByteArray m_UE;

    /// What operations shall be permitted, when document is opened with user access.
    uint32_t m_permissions = 0;

    /// For revision number 6 only. 16 byte encrypted version of permissions.
    QByteArray m_Perms;

    /// Optional, meaningfull only if revision number is 4 or 5.
    bool m_encryptMetadata = true;

    /// First part of the id of the document
    QByteArray m_ID;
};

/// Specifies the security using public key security handler (see PDF specification
/// for details).
class PDFPublicKeySecurityHandler : public PDFStandardOrPublicSecurityHandler
{
public:
    virtual EncryptionMode getMode() const override { return EncryptionMode::PublicKey; }
    virtual PDFSecurityHandler* clone() const override;
    virtual AuthorizationResult authenticate(const std::function<QString(bool*)>& getPasswordCallback, bool authorizeOwnerOnly) override;
    virtual bool isMetadataEncrypted() const override;
    virtual bool isAllowed(Permission permission) const override;
    virtual PDFObject createEncryptionDictionaryObject() const override;

private:
    friend class PDFSecurityHandler;
    friend class PDFSecurityHandlerFactory;

    enum class PKCS7_Type
    {
        Unknown,
        PKCS7_S3,
        PKCS7_S4,
        PKCS7_S5
    };

    enum PermissionFlag : uint32_t
    {
        PKSH_Owner = 1 << 1,
        PKSH_PrintLowResolution = 1 << 2,
        PKSH_Modify = 1 << 3,
        PKSH_CopyContent = 1 << 4,
        PKSH_ModifyAnnotationsFillFormFields = 1 << 5,
        PKSH_FillFormFields = 1 << 8,
        PKSH_Assemble = 1 << 10,
        PKSH_PrintHighResolution = 1 << 11
    };

    /// What operations shall be permitted, when document is opened with user access.
    uint32_t m_permissions = 0;

    /// Type of the PKCS7 subfilter
    PKCS7_Type m_pkcs7Type = PKCS7_Type::Unknown;
};

/// Factory, which creates security handler based on settings.
class PDF4QTLIBCORESHARED_EXPORT PDFSecurityHandlerFactory
{
    Q_DECLARE_TR_FUNCTIONS(pdf::PDFSecurityHandlerFactory)

public:

    enum Algorithm
    {
        None,
        RC4,
        AES_128,
        AES_256,
        Certificate
    };

    enum EncryptContents
    {
        All,
        AllExceptMetadata,
        EmbeddedFiles
    };

    struct SecuritySettings
    {
        Algorithm algorithm = None;
        EncryptContents encryptContents = All;
        QString userPassword;
        QString ownerPassword;
        uint32_t permissions = 0;
        QByteArray id;
        PDFCertificateEntry certificate;
    };

    /// Creates security handler based on given settings. If security handler cannot
    /// be created, then nullptr is returned.
    /// \param settings Security handler settings
    static PDFSecurityHandlerPointer createSecurityHandler(const SecuritySettings& settings);

    /// Returns optimal number of bits (entropy) for strong password
    static int getPasswordOptimalEntropy();

    /// Calculates password entropy (number of bits), can be used
    /// for ranking the password security. Encryption algorithm must be also
    /// considered, because password is adjusted according to the specification
    /// before it's entropy is being computed.
    /// \param password Password to be scored
    /// \param algorithm Encryption algorithm
    static int getPasswordEntropy(const QString& password, Algorithm algorithm);

    /// Returns revision number of standard security handler for a given
    /// algorithm. If algorithm is invalid or None, zero is returned.
    /// \param algorithm Algorithm
    static int getRevisionFromAlgorithm(Algorithm algorithm);

    /// Generates array of random bytes with given size, using random number generator.
    /// \param generator Random number generator
    /// \param size Target size
    static QByteArray generateRandomByteArray(QRandomGenerator& generator, int size);

    /// Validates security settings
    /// \param settings Settings
    /// \param[out] errorMessage Error message
    static bool validate(const SecuritySettings& settings, QString* errorMessage);
};

}   // namespace pdf

#endif // PDFSECURITYHANDLER_H
