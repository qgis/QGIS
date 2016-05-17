class QgsAuthCrypto
{
%TypeHeaderCode
#include <qgsauthcrypto.h>
%End

  public:
    /** Whether QCA has the qca-ossl plugin, which a base run-time requirement */
    static bool isDisabled();

    /** Encrypt data using master password */
    static const QString encrypt( const QString& pass, const QString& cipheriv, const QString& text );

    /** Decrypt data using master password */
    static const QString decrypt( const QString& pass, const QString& cipheriv, const QString& text );

    /** Generate SHA256 hash for master password, with iterations and salt */
    static void passwordKeyHash( const QString &pass,
                                 QString *salt,
                                 QString *hash,
                                 QString *cipheriv = 0 );

    /** Verify existing master password hash to a re-generated one */
    static bool verifyPasswordKeyHash( const QString& pass,
                                       const QString& salt,
                                       const QString& hash,
                                       QString *hashderived = 0 );
};
