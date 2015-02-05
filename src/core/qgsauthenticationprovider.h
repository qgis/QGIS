/***************************************************************************
    qgsauthenticationprovider.h
    ---------------------
    begin                : October 5, 2014
    copyright            : (C) 2014 by Boundless Spatial, Inc. USA
    author               : Larry Shaffer
    email                : lshaffer at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAUTHENTICATIONPROVIDER_H
#define QGSAUTHENTICATIONPROVIDER_H

#include <QObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

#ifndef QT_NO_OPENSSL
#include <QSslCertificate>
#include <QSslKey>
#endif

#include "qgsauthenticationconfig.h"

/** \ingroup core
 * \brief Base authentication provider class (not meant to be directly used)
 * \since 2.8
 */
class CORE_EXPORT QgsAuthProvider
{

  public:

    explicit QgsAuthProvider( QgsAuthType::ProviderType providertype = QgsAuthType::None );

    virtual ~QgsAuthProvider();

    QgsAuthType::ProviderType providerType() const { return mType; }
    void setProviderType( QgsAuthType::ProviderType ptype ) { mType = ptype; }

    static bool urlToResource( const QString& accessurl, QString *resource, bool withpath = false );

    virtual bool updateNetworkRequest( QNetworkRequest &request, const QString& authid ) = 0;

    virtual bool updateNetworkReply( QNetworkReply *reply, const QString& authid ) = 0;

    virtual void clearCachedConfig( const QString& authid ) = 0;

  protected:
    static const QString authProviderTag() { return QObject::tr( "Authentication provider" ); }

  private:
    QgsAuthType::ProviderType mType;
};

/** \ingroup core
 * \brief Basic username/password authentication provider class
 * \since 2.8
 */
class CORE_EXPORT QgsAuthProviderBasic : public QgsAuthProvider
{
  public:
    QgsAuthProviderBasic();

    ~QgsAuthProviderBasic();

    // QgsAuthProvider interface
    bool updateNetworkRequest( QNetworkRequest &request, const QString &authid );
    bool updateNetworkReply( QNetworkReply *reply, const QString &authid );
    void clearCachedConfig( const QString& authid );

  private:

    QgsAuthConfigBasic getAuthBasicConfig( const QString& authid );

    void putAuthBasicConfig( const QString& authid, QgsAuthConfigBasic config );

    void removeAuthBasicConfig( const QString& authid );

    static QMap<QString, QgsAuthConfigBasic> mAuthBasicCache;
};


#ifndef QT_NO_OPENSSL
/** \ingroup core
 * \brief Storage set for constructed SSL certificate, key and optional certificate issuer
 * \since 2.6
 */
class CORE_EXPORT QgsPkiBundle
{
  public:
    QgsPkiBundle( const QgsAuthConfigPkiPaths& config,
                  const QSslCertificate& cert,
                  const QSslKey& certkey,
                  const QSslCertificate& issuer = QSslCertificate(),
                  bool issuerSeflSigned = false );
    ~QgsPkiBundle();

    bool isValid();

    const QgsAuthConfigPkiPaths config() const { return mConfig; }
    void setConfig( const QgsAuthConfigPkiPaths& config ) { mConfig = config; }

    const QSslCertificate clientCert() const { return mCert; }
    void setClientCert( const QSslCertificate& cert ) { mCert = cert; }

    const QSslKey clientCertKey() const { return mCertKey; }
    void setClientCertKey( const QSslKey& certkey ) { mCertKey = certkey; }

    const QSslCertificate issuerCert() const { return mIssuer; }
    void setIssuerCert( const QSslCertificate& issuer ) { mIssuer = issuer; }

    bool issuerSelfSigned() const { return mIssuerSelf; }
    void setIssuerSelfSigned( bool selfsigned ) { mIssuerSelf = selfsigned; }

  private:
    QgsAuthConfigBase mConfig;
    QSslCertificate mCert;
    QSslKey mCertKey;
    QSslCertificate mIssuer;
    bool mIssuerSelf;
};

/** \ingroup core
 * \brief PKI (PEM/DER paths only) authentication provider class
 * \since 2.8
 */
class CORE_EXPORT QgsAuthProviderPkiPaths : public QgsAuthProvider
{
  public:
    QgsAuthProviderPkiPaths();

    virtual ~QgsAuthProviderPkiPaths();

    // QgsAuthProvider interface
    bool updateNetworkRequest( QNetworkRequest &request, const QString &authid );
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

  private:

    static QMap<QString, QgsPkiBundle *> mPkiBundleCache;
};

/** \ingroup core
 * \brief PKI (.p12/.pfx and CA paths only) authentication provider class
 * \note Since this uses QCA's PKCS#12 support, signing CAs in the user's root OS cert store will also be queried.
 * \since 2.8
 */
class CORE_EXPORT QgsAuthProviderPkiPkcs12 : public QgsAuthProviderPkiPaths
{
  public:
    QgsAuthProviderPkiPkcs12();

    ~QgsAuthProviderPkiPkcs12();

    static const QString certAsPem( const QString &bundlepath, const QString &bundlepass );

    static const QString keyAsPem( const QString &bundlepath, const QString &bundlepass, bool reencrypt = true );

    static const QString issuerAsPem( const QString &bundlepath, const QString &bundlepass, const QString &issuerpath );

  protected:

    QgsPkiBundle * getPkiBundle( const QString &authid );

  private:

    static QMap<QString, QgsPkiBundle *> mPkiBundleCache;
};
#endif

#endif // QGSAUTHENTICATIONPROVIDER_H
