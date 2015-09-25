class QgsAuthCrypto
{
%TypeHeaderCode
#include <qgsauthcrypto.h>
%End

  public:
    static bool isDisabled();

    static const QString encrypt( QString pass, QString cipheriv, QString text );

    static const QString decrypt( QString pass, QString cipheriv, QString text );

    static void passwordKeyHash( const QString &pass,
                                 QString *salt,
                                 QString *hash,
                                 QString *cipheriv = 0 );

    static bool verifyPasswordKeyHash( const QString& pass,
                                       const QString& salt,
                                       const QString& hash,
                                       QString *hashderived = 0 );
};
