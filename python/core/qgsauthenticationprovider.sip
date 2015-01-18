class QgsAuthProvider
{
%TypeHeaderCode
#include <qgsauthenticationprovider.h>
%End

  public:

    explicit QgsAuthProvider( QgsAuthType::ProviderType providertype = QgsAuthType::None );

    virtual ~QgsAuthProvider();

    QgsAuthType::ProviderType providerType() const;

    static bool urlToResource( const QString& accessurl, QString *resource, bool withpath = false );

    virtual bool updateNetworkRequest( QNetworkRequest &request /In,Out/, const QString& authid ) = 0;

    virtual bool updateNetworkReply( QNetworkReply *reply, const QString& authid ) = 0;

    virtual void clearCachedConfig( const QString& authid ) = 0;

  protected:
    static const QString authProviderTag();
};


class QgsAuthProviderBasic : QgsAuthProvider
{
%TypeHeaderCode
#include <qgsauthenticationprovider.h>
%End

  public:
    QgsAuthProviderBasic();

    ~QgsAuthProviderBasic();

    // QgsAuthProvider interface
    bool updateNetworkRequest( QNetworkRequest &request /In,Out/, const QString &authid );
    bool updateNetworkReply( QNetworkReply *reply, const QString &authid );
    void clearCachedConfig( const QString& authid );
};


class QgsPkiBundle
{
%TypeHeaderCode
#include <qgsauthenticationprovider.h>
%End

  public:
    QgsPkiBundle( const QgsAuthConfigPkiPaths& config,
                  const QSslCertificate& cert,
                  const QSslKey& certkey,
                  const QSslCertificate& issuer = QSslCertificate(),
                  bool issuerSeflSigned = false );
    ~QgsPkiBundle();

    bool isValid();

    const QgsAuthConfigPkiPaths config() const;
    void setConfig( const QgsAuthConfigPkiPaths& config );

    const QSslCertificate clientCert() const;
    void setClientCert( const QSslCertificate& cert );

    const QSslKey clientCertKey() const;
    void setClientCertKey( const QSslKey& certkey );

    const QSslCertificate issuerCert() const;
    void setIssuerCert( const QSslCertificate& issuer );
    
    bool issuerSelfSigned() const;
    void setIssuerSelfSigned( bool selfsigned );

};


class  QgsAuthProviderPkiPaths : QgsAuthProvider
{
%TypeHeaderCode
#include <qgsauthenticationprovider.h>
%End

  public:
    QgsAuthProviderPkiPaths();

    virtual ~QgsAuthProviderPkiPaths();

    // QgsAuthProvider interface
    bool updateNetworkRequest( QNetworkRequest &request /In,Out/, const QString &authid );
    bool updateNetworkReply( QNetworkReply *reply, const QString &authid );
    void clearCachedConfig( const QString& authid );

    static const QByteArray certAsPem( const QString &certpath );

    static const QByteArray keyAsPem( const QString &keypath,
                                      const QString &keypass = QString(),
                                      QString *algtype = 0,
                                      bool reencrypt = true );

    static const QByteArray issuerAsPem( const QString &issuerpath );

  protected:

    virtual QgsPkiBundle * getPkiBundle( const QString &authid );

    virtual void putPkiBundle( const QString &authid, QgsPkiBundle * pkibundle );

    virtual void removePkiBundle( const QString &authid );
};

class QgsAuthProviderPkiPkcs12 : QgsAuthProviderPkiPaths
{
%TypeHeaderCode
#include <qgsauthenticationprovider.h>
%End

  public:
    QgsAuthProviderPkiPkcs12();

    ~QgsAuthProviderPkiPkcs12();

    static const QString certAsPem( const QString &bundlepath, const QString &bundlepass );

    static const QString keyAsPem( const QString &bundlepath, const QString &bundlepass, bool reencrypt = true );

    static const QString issuerAsPem( const QString &bundlepath, const QString &bundlepass, const QString &issuerpath );

  protected:

    QgsPkiBundle * getPkiBundle( const QString &authid );

};
