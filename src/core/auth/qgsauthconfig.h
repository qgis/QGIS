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

#include "qgis_core.h"

#include <QHash>
#include <QString>
#include <QDomElement>
#include <QDomDocument>

#ifndef QT_NO_SSL
#include <QSslCertificate>
#include <QSslKey>
#include <QSslError>
#include <QSslSocket>
#endif

#include "qgis.h"


/**
 * \ingroup core
 * \brief Configuration storage class for authentication method configurations
 */
class CORE_EXPORT QgsAuthMethodConfig
{
  public:

    /**
     * Construct a configuration for an authentication method
     * \param method Textual key of the authentication method
     * \param version Version of the configuration (for updating previously saved configs later on)
     */
    QgsAuthMethodConfig( const QString &method = QString(), int version = 0 );

    // TODO c++20 - replace with = default

    bool operator==( const QgsAuthMethodConfig &other ) const;
    bool operator!=( const QgsAuthMethodConfig &other ) const;

    /**
     * Gets 'authcfg' 7-character alphanumeric ID of the config
     * \note This is set by QgsAuthManager when the config is initially stored
     */
    const QString id() const { return mId; }
    //! Sets auth config ID
    void setId( const QString &id ) { mId = id; }

    //! Gets name of configuration
    const QString name() const { return mName; }
    //! Sets name of configuration
    void setName( const QString &name ) { mName = name; }

    //! A URI to auto-select a config when connecting to a resource
    const QString uri() const { return mUri; }
    void setUri( const QString &uri ) { mUri = uri; }

    //! Textual key of the associated authentication method
    QString method() const { return mMethod; }
    void setMethod( const QString &method ) { mMethod = method; }

    //! Gets version of the configuration
    int version() const { return mVersion; }
    //! Sets version of the configuration
    void setVersion( int version ) { mVersion = version; }

    /**
     * Whether the configuration is valid
     * \param validateid Additionally verify the auth config ID is not empty
     */
    bool isValid( bool validateid = false ) const;

    /**
     * The extended configuration, as stored and retrieved from the authentication database
     * \note This is an internal construct used by QgsAuthManager that should generally not be set by client code
     */
    const QString configString() const;

    /**
     * Load existing extended configuration
     * \param configstr Configuration string to load
     */
    void loadConfigString( const QString &configstr );

    //! Gets extended configuration, mapped to key/value pairs of QStrings
    QgsStringMap configMap() const { return mConfigMap; }

    /**
     * Set extended configuration map
     * \param map Map to set
     */
    void setConfigMap( const QgsStringMap &map ) { mConfigMap = map; }

    /**
     * Set a single config value per key in the map
     * \note if key exists, it is replaced
     * \param key Config key
     * \param value Config value
     */
    void setConfig( const QString &key, const QString &value );

    /**
     * Set a multiple config values per key in the map
     * \note if key exists, it is replaced
     * \param key Config key
     * \param value Config value
     */
    void setConfigList( const QString &key, const QStringList &value );

    /**
     * Remove a config from map
     * \param key Config to remove
     * \returns Number of keys removed (should always be 1 or 0)
     */
    int removeConfig( const QString &key );

    /**
     * Returns a config's value
     * \param key Config key
     * \param defaultvalue Default value, if key not found
     */
    QString config( const QString &key, const QString &defaultvalue = QString() ) const;

    /**
     * Returns a config's list of values
     * \param key
     */
    QStringList configList( const QString &key ) const;

    /**
     * Whether a config key exists in config map
     * \param key
     */
    bool hasConfig( const QString &key ) const;

    //! Clear all configs
    void clearConfigMap() { mConfigMap.clear(); }

    /**
     * A utility function for generating a resource from a URL to be compared
     * against the config's uri() for auto-selecting authentication configs to use
     * \note Essentially strips the URL query variables, and by default, strips the path as well
     * \param accessurl A URL to process
     * \param resource Output variable for result
     * \param withpath Whether to include the URI's path in output
     */
    static bool uriToResource( const QString &accessurl, QString *resource, bool withpath = false );

    /**
     * Stores the configuration in a DOM
     * \param parentElement parent DOM element
     * \param document DOM document
     * \see readXml()
     * \since QGIS 3.20
     */
    bool writeXml( QDomElement &parentElement, QDomDocument &document );

    /**
     *  from a DOM element.
     * \param element is the DOM node corresponding to item (e.g. 'LayoutItem' element)
     * \since QGIS 3.20
     */
    bool readXml( const QDomElement &element );

  private:
    QString mId;
    QString mName;
    QString mUri;
    QString mMethod;
    int mVersion;

    QgsStringMap mConfigMap;

    static const QString CONFIG_SEP;
    static const QString CONFIG_KEY_SEP;
    static const QString CONFIG_LIST_SEP;

    static const int CONFIG_VERSION;
};

typedef QHash<QString, QgsAuthMethodConfig> QgsAuthMethodConfigsMap;


#ifndef QT_NO_SSL

/**
 * \ingroup core
 * \brief Storage set for PKI bundle: SSL certificate, key, optional CA cert chain
 * \note Useful for caching the bundle during application run sessions
 */
class CORE_EXPORT QgsPkiBundle
{
  public:

    /**
     * Construct a bundle from existing PKI components
     * \param clientCert Certificate to store in bundle
     * \param clientKey Private key to store in bundle
     * \param caChain Chain of Certificate Authorities for client certificate
     */
    QgsPkiBundle( const QSslCertificate &clientCert = QSslCertificate(),
                  const QSslKey &clientKey = QSslKey(),
                  const QList<QSslCertificate> &caChain = QList<QSslCertificate>() );

    /**
     * Construct a bundle of PKI components from PEM-formatted file paths
     * \param certPath Certificate file path
     * \param keyPath Private key path
     * \param keyPass Private key passphrase
     * \param caChain Chain of Certificate Authorities for client certificate
     */
    static const QgsPkiBundle fromPemPaths( const QString &certPath,
                                            const QString &keyPath,
                                            const QString &keyPass = QString(),
                                            const QList<QSslCertificate> &caChain = QList<QSslCertificate>() );

    /**
     * Construct a bundle of PKI components from a PKCS#12 file path
     * \param bundlepath Bundle file path
     * \param bundlepass Optional bundle passphrase
     */
    static const QgsPkiBundle fromPkcs12Paths( const QString &bundlepath,
        const QString &bundlepass = QString() );

    //! Whether the bundle, either its certificate or private key, is null
    bool isNull() const;

    //! Whether the bundle is valid
    bool isValid() const;

    //! The sha hash of the client certificate
    const QString certId() const;

    //! Client certificate object
    const QSslCertificate clientCert() const { return mCert; }
    //! Sets client certificate object
    void setClientCert( const QSslCertificate &cert );

    //! Private key object
    const QSslKey clientKey() const { return mCertKey; }
    //! Sets private key object
    void setClientKey( const QSslKey &certkey );

    //! Chain of Certificate Authorities for client certificate
    const QList<QSslCertificate> caChain() const { return mCaChain; }
    //! Sets chain of Certificate Authorities for client certificate
    void setCaChain( const QList<QSslCertificate> &cachain ) { mCaChain = cachain; }

  private:
    QSslCertificate mCert;
    QSslKey mCertKey;
    QList<QSslCertificate> mCaChain;
};


/**
 * \ingroup core
 * \brief Storage set for constructed SSL certificate, key, associated with an authentication config
 */
class CORE_EXPORT QgsPkiConfigBundle
{
  public:

    /**
     * Construct a bundle from existing PKI components and authentication method configuration
     * \param config Authentication method configuration
     * \param cert Certificate to store in bundle
     * \param certkey Private key to store in bundle
     * \param cachain list of CA certificates
     */
    QgsPkiConfigBundle( const QgsAuthMethodConfig &config,
                        const QSslCertificate &cert,
                        const QSslKey &certkey,
                        const QList<QSslCertificate> &cachain = QList<QSslCertificate>( ) );

    //! Whether the bundle is valid
    bool isValid();

    //! Authentication method configuration
    const QgsAuthMethodConfig config() const { return mConfig; }

    //! Sets authentication method configuration
    void setConfig( const QgsAuthMethodConfig &config ) { mConfig = config; }

    //! Client certificate object
    const QSslCertificate clientCert() const { return mCert; }

    //! Sets client certificate object
    void setClientCert( const QSslCertificate &cert ) { mCert = cert; }

    //! Private key object
    const QSslKey clientCertKey() const { return mCertKey; }

    //! Sets private key object
    void setClientCertKey( const QSslKey &certkey ) { mCertKey = certkey; }

    /**
     * \brief caChain return the CA chain
     * \return list of CA certificates
     */
    QList<QSslCertificate> caChain() const { return mCaChain; }

    /**
     * \brief setCaChain set the CA chain
     * \param caChain
     */
    void setCaChain( const QList<QSslCertificate> &caChain ) { mCaChain = caChain; }

  private:
    QgsAuthMethodConfig mConfig;
    QSslCertificate mCert;
    QSslKey mCertKey;
    QList<QSslCertificate> mCaChain;
};



#ifdef SIP_RUN
% MappedType QList<QSslError::SslError>
{
  % TypeHeaderCode
#include <QList>
  % End

  % ConvertFromTypeCode
  // Create the list.
  PyObject *l;

  if ( ( l = PyList_New( sipCpp->size() ) ) == NULL )
    return NULL;

  // Set the list elements.
  QList<QSslError::SslError>::iterator it = sipCpp->begin();
  for ( int i = 0; it != sipCpp->end(); ++it, ++i )
  {
    PyObject *tobj;

    if ( ( tobj = sipConvertFromEnum( *it, sipType_QSslError_SslError ) ) == NULL )
    {
      Py_DECREF( l );
      return NULL;
    }
    PyList_SET_ITEM( l, i, tobj );
  }

  return l;
  % End

  % ConvertToTypeCode
  // Check the type if that is all that is required.
  if ( sipIsErr == NULL )
    return PyList_Check( sipPy );

  QList<QSslError::SslError> *qlist = new QList<QSslError::SslError>;

  for ( int i = 0; i < PyList_GET_SIZE( sipPy ); ++i )
  {
    *qlist << ( QSslError::SslError )SIPLong_AsLong( PyList_GET_ITEM( sipPy, i ) );
  }

  *sipCppPtr = qlist;
  return sipGetState( sipTransferObj );
  % End
};
#endif



/**
 * \ingroup core
 * \brief Configuration container for SSL server connection exceptions or overrides
 */
class CORE_EXPORT QgsAuthConfigSslServer
{
  public:
    //! Construct a default SSL server configuration
    QgsAuthConfigSslServer();

    //! Server certificate object
    const QSslCertificate sslCertificate() const { return mSslCert; }
    //! Sets server certificate object
    void setSslCertificate( const QSslCertificate &cert ) { mSslCert = cert; }

    //! Server host:port string
    const QString sslHostPort() const  { return mSslHostPort; }
    //! Sets server host:port string
    void setSslHostPort( const QString &hostport ) { mSslHostPort = hostport; }

    //! SSL server protocol to use in connections
    QSsl::SslProtocol sslProtocol() const { return mSslProtocol; }
    //! Sets SSL server protocol to use in connections
    void setSslProtocol( QSsl::SslProtocol protocol ) { mSslProtocol = protocol; }

    //! SSL server errors to ignore in connections
    const QList<QSslError> sslIgnoredErrors() const;
    //! SSL server errors (as enum list) to ignore in connections
    const QList<QSslError::SslError> sslIgnoredErrorEnums() const { return mSslIgnoredErrors; }
    //! Sets SSL server errors (as enum list) to ignore in connections
    void setSslIgnoredErrorEnums( const QList<QSslError::SslError> &errors ) { mSslIgnoredErrors = errors; }

    //! SSL client's peer verify mode to use in connections
    QSslSocket::PeerVerifyMode sslPeerVerifyMode() const { return mSslPeerVerifyMode; }
    //! Sets SSL client's peer verify mode to use in connections
    void setSslPeerVerifyMode( QSslSocket::PeerVerifyMode mode ) { mSslPeerVerifyMode = mode; }

    /**
     * Number or SSL client's peer to verify in connections
     * \note When set to 0 = unlimited depth
     */
    int sslPeerVerifyDepth() const { return mSslPeerVerifyDepth; }

    /**
     * Set number or SSL client's peer to verify in connections
     * \note When set to 0 = unlimited depth
     */
    void setSslPeerVerifyDepth( int depth ) { mSslPeerVerifyDepth = depth; }

    //! Version of the configuration (used for future upgrading)
    int version() const { return mVersion; }
    //! Sets version of the configuration (used for future upgrading)
    void setVersion( int version ) { mVersion = version; }

    //! Qt version when the configuration was made (SSL protocols may differ)
    int qtVersion() const { return mQtVersion; }
    //! Sets Qt version when the configuration was made (SSL protocols may differ)
    void setQtVersion( int version ) { mQtVersion = version; }

    //! Configuration as a concatenated string
    const QString configString() const;
    //! Load concatenated string into configuration, e.g. from auth database
    void loadConfigString( const QString &config = QString() );

    //! Whether configuration is null (missing components)
    bool isNull() const;

  private:

    QString mSslHostPort;
    QSslCertificate mSslCert;

    static QSsl::SslProtocol decodeSslProtocol( const QString &protocol );
    static QString encodeSslProtocol( QSsl::SslProtocol protocol );

    QSsl::SslProtocol mSslProtocol;
    int mQtVersion;
    QList<QSslError::SslError> mSslIgnoredErrors;
    QSslSocket::PeerVerifyMode mSslPeerVerifyMode = QSslSocket::VerifyPeer;
    int mSslPeerVerifyDepth = 0;
    int mVersion = 1;

    static const QString CONF_SEP;
};
#endif

#endif // QGSAUTHCONFIG_H
