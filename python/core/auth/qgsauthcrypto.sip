class QgsAuthCrypto
{
%TypeHeaderCode
#include <qgsauthcrypto.h>
%End

  public:
    static bool isDisabled();

    static const QString encrypt( const QString& pass, const QString& cipheriv, const QString& text );

    static const QString decrypt( const QString& pass, const QString& cipheriv, const QString& text );

    static void passwordKeyHash( const QString &pass,
                                 QString *salt,
                                 QString *hash,
                                 QString *cipheriv = 0 );

    static bool verifyPasswordKeyHash( const QString& pass,
                                       const QString& salt,
                                       const QString& hash,
                                       QString *hashderived = 0 );
};
