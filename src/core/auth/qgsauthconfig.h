/***************************************************************************
    qgsauthconfig.h
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

#ifndef QGSAUTHCONFIG_H
#define QGSAUTHCONFIG_H

#include <QHash>
#include <QString>

#ifndef QT_NO_OPENSSL
#include <QSslCertificate>
#include <QSslKey>
#include <QSslError>
#include <QSslSocket>
#endif

#include "qgis.h"


/** \ingroup core
 * \brief Configuration storage class for authentication method configurations
 */
class CORE_EXPORT QgsAuthMethodConfig
{
  public:

    /**
     * Constructor
     * @param method The textual key of the authentication method
     * @param version Version of the configuration (for updating previously saved configs later on)
     */
    QgsAuthMethodConfig( QString method = QString(), int version = 0 );

    /** Full clone of config */
    QgsAuthMethodConfig( const QgsAuthMethodConfig& methodconfig );

    ~QgsAuthMethodConfig() {}

    /**
     * The 'authcfg' 7-character alphanumeric ID of the config
     * @note This is set by QgsAuthManager when the config is initially stored
     */
    const QString id() const { return mId; }
    void setId( const QString& id ) { mId = id; }

    const QString name() const { return mName; }
    void setName( const QString& name ) { mName = name; }

    /**
     * A URI to auto-select a config when connecting to a resource
     */
    const QString uri() const { return mUri; }
    void setUri( const QString& uri ) { mUri = uri; }

    /**
     * The textual key of the associated authentication method
     */
    QString method() const { return mMethod; }
    void setMethod( QString method ) { mMethod = method; }

    /**
     * Version of the configuration
     */
    int version() const { return mVersion; }
    void setVersion( int version ) { mVersion = version; }

    bool isValid( bool validateid = false ) const;

    /**
     * The extended configuration, as stored and retrieved from the authentication database
     * @note This is an internal construct used by QgsAuthManager that should generally not be set by client code
     */
    const QString configString() const;
    void loadConfigString( const QString& configstr );

    /**
     * The extended configuration, mapped to key/value pairs of QStrings
     */
    QgsStringMap configMap() const { return mConfigMap; }
    void setConfigMap( QgsStringMap map ) { mConfigMap = map; }

    /** @note if key exists, it is replaced */
    void setConfig( const QString &key, const QString &value );
    void setConfigList( const QString &key, const QStringList &value );

    int removeConfig( const QString &key );

    QString config( const QString &key , const QString defaultvalue = QString() ) const;

    QStringList configList( const QString &key ) const;

    bool hasConfig( const QString &key ) const;

    void clearConfigMap() { mConfigMap.clear(); }

    /**
     * A utility function for generating a resource from a URL to be compared
     * against the config's uri() for auto-selecting authentication configs to use
     * @note Essentially strips the URL query variables, and by default, strips the path as well
     * @param accessurl A URL to process
     * @param resource Ouput variable for result
     * @param withpath Whether to include the
     */
    static bool uriToResource( const QString &accessurl, QString *resource, bool withpath = false );

  private:
    QString mId;
    QString mName;
    QString mUri;
    QString mMethod;
    int mVersion;

    QgsStringMap mConfigMap;

    static const QString mConfigSep;
    static const QString mConfigKeySep;
    static const QString mConfigListSep;

    static const int mConfigVersion;
};

typedef QHash<QString, QgsAuthMethodConfig> QgsAuthMethodConfigsMap;


#ifndef QT_NO_OPENSSL

/** \ingroup core
 * \brief Storage set for PKI bundle: SSL certificate, key, optional CA cert chain
 * \note Useful for caching the bundle during application run sessions
 */
class CORE_EXPORT QgsPkiBundle
{
  public:
    QgsPkiBundle( const QSslCertificate &clientCert = QSslCertificate(),
                  const QSslKey &clientKey = QSslKey(),
                  const QString &keyPassphrase = QString::null ,
                  const QList<QSslCertificate> &caChain = QList<QSslCertificate>() );
    ~QgsPkiBundle();

    static const QgsPkiBundle fromPemPaths( const QString &certPath,
                                            const QString &keyPath,
                                            const QString &keyPass = QString::null,
                                            const QList<QSslCertificate> &caChain = QList<QSslCertificate>() );

    static const QgsPkiBundle fromPkcs12Paths( const QString &bundlepath,
        const QString &bundlepass = QString::null );

    bool isNull() const;
    bool isValid() const;

    const QString certId() const;

    const QSslCertificate clientCert() const { return mCert; }
    void setClientCert( const QSslCertificate &cert );

    const QSslKey clientKey( bool reencrypt = true ) const;
    void setClientKey( const QSslKey &certkey );

    const QString keyPassphrase() const { return mKeyPassphrase; }
    void setKeyPassphrase( const QString &pass ) { mKeyPassphrase = pass; }

    const QList<QSslCertificate> caChain() const { return mCaChain; }
    void setCaChain( const QList<QSslCertificate> &cachain ) { mCaChain = cachain; }

  private:
    QSslCertificate mCert;
    QSslKey mCertKey;
    QString mKeyPassphrase;
    QList<QSslCertificate> mCaChain;
};


/** \ingroup core
 * \brief Storage set for constructed SSL certificate, key, associated with an authentication config
 */
class CORE_EXPORT QgsPkiConfigBundle
{
  public:
    QgsPkiConfigBundle( const QgsAuthMethodConfig& config,
                        const QSslCertificate& cert,
                        const QSslKey& certkey );
    ~QgsPkiConfigBundle();

    bool isValid();

    const QgsAuthMethodConfig config() const { return mConfig; }
    void setConfig( const QgsAuthMethodConfig& config ) { mConfig = config; }

    const QSslCertificate clientCert() const { return mCert; }
    void setClientCert( const QSslCertificate& cert ) { mCert = cert; }

    const QSslKey clientCertKey() const { return mCertKey; }
    void setClientCertKey( const QSslKey& certkey ) { mCertKey = certkey; }

  private:
    QgsAuthMethodConfig mConfig;
    QSslCertificate mCert;
    QSslKey mCertKey;
};


/** \ingroup core
 * \brief Configuration container for SSL server connection exceptions or overrides
 */
class CORE_EXPORT QgsAuthConfigSslServer
{
  public:
    QgsAuthConfigSslServer();

    ~QgsAuthConfigSslServer() {}

    const QSslCertificate sslCertificate() const { return mSslCert; }
    void setSslCertificate( const QSslCertificate& cert ) { mSslCert = cert; }

    const QString sslHostPort() const  { return mSslHostPort; }
    void setSslHostPort( const QString& hostport ) { mSslHostPort = hostport; }

    QSsl::SslProtocol sslProtocol() const { return mSslProtocol; }
    void setSslProtocol( QSsl::SslProtocol protocol ) { mSslProtocol = protocol; }

    const QList<QSslError> sslIgnoredErrors() const;
    const QList<QSslError::SslError> sslIgnoredErrorEnums() const { return mSslIgnoredErrors; }
    void setSslIgnoredErrorEnums( const QList<QSslError::SslError>& errors ) { mSslIgnoredErrors = errors; }

    QSslSocket::PeerVerifyMode sslPeerVerifyMode() const { return mSslPeerVerifyMode; }
    void setSslPeerVerifyMode( QSslSocket::PeerVerifyMode mode ) { mSslPeerVerifyMode = mode; }

    int sslPeerVerifyDepth() const { return mSslPeerVerifyDepth; }
    void setSslPeerVerifyDepth( int depth ) { mSslPeerVerifyDepth = depth; }

    int version() const { return mVersion; }
    void setVersion( int version ) { mVersion = version; }

    int qtVersion() const { return mQtVersion; }
    void setQtVersion( int version ) { mQtVersion = version; }

    const QString configString() const;
    void loadConfigString( const QString& config = QString() );

    bool isNull() const;

  private:

    QString mSslHostPort;
    QSslCertificate mSslCert;

    QSsl::SslProtocol mSslProtocol;
    int mQtVersion;
    QList<QSslError::SslError> mSslIgnoredErrors;
    QSslSocket::PeerVerifyMode mSslPeerVerifyMode;
    int mSslPeerVerifyDepth;
    int mVersion;

    static const QString mConfSep;
};
#endif

#endif // QGSAUTHCONFIG_H
