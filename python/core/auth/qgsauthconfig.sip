/** \ingroup core
 * \brief Configuration storage class for authentication method configurations
 */
class QgsAuthMethodConfig
{
%TypeHeaderCode
#include <qgsauthconfig.h>
%End
  public:

    /**
     * Construct a configuration for an authentication method
     * @param method Textual key of the authentication method
     * @param version Version of the configuration (for updating previously saved configs later on)
     */
    QgsAuthMethodConfig( const QString& method = QString(), int version = 0 );

    /** Operator used to compare configs' equality */
    bool operator==( const QgsAuthMethodConfig& other ) const;

    /** Operator used to compare configs' inequality */
    bool operator!=( const QgsAuthMethodConfig& other ) const;

    /**
     * Get 'authcfg' 7-character alphanumeric ID of the config
     * @note This is set by QgsAuthManager when the config is initially stored
     */
    const QString id() const;
    /** Set auth config ID */
    void setId( const QString& id );

    /** Get name of configuration */
    const QString name() const;
    /** Set name of configuration */
    void setName( const QString& name );

    /** A URI to auto-select a config when connecting to a resource */
    const QString uri() const;
    void setUri( const QString& uri );

    /** Textual key of the associated authentication method */
    QString method() const;
    void setMethod( const QString& method );

    /** Get version of the configuration */
    int version() const;
    /** Set version of the configuration */
    void setVersion( int version );

    /**
     * Whether the configuration is valid
     * @param validateid Additionally verify the auth config ID is not empty
     */
    bool isValid( bool validateid = false ) const;

    /**
     * The extended configuration, as stored and retrieved from the authentication database
     * @note This is an internal construct used by QgsAuthManager that should generally not be set by client code
     */
    const QString configString() const;
    /**
     * Load existing extended configuration
     * @param configstr Configuration string to load
     */
    void loadConfigString( const QString& configstr );

    /** Get extended configuration, mapped to key/value pairs of QStrings */
    QgsStringMap configMap() const;
    /**
     * Set extended configuration map
     * @param map Map to set
     */
    void setConfigMap( const QgsStringMap& map );

    /**
     * Set a single config value per key in the map
     * @note if key exists, it is replaced
     * @param key Config key
     * @param value Config value
     */
    void setConfig( const QString &key, const QString &value );
    /**
     * Set a multiple config values per key in the map
     * @note if key exists, it is replaced
     * @param key Config key
     * @param value Config value
     */
    void setConfigList( const QString &key, const QStringList &value );

    /**
     * Remove a config from map
     * @param key Config to remove
     * @return Number of keys removed (should always be 1 or 0)
     */
    int removeConfig( const QString &key );

    /**
     * Return a config's value
     * @param key Config key
     * @param defaultvalue Default value, if key not found
     */
    QString config( const QString &key, const QString& defaultvalue = QString() ) const;

    /**
     * Return a config's list of values
     * @param key
     */
    QStringList configList( const QString &key ) const;

    /**
     * Whether a config key exists in config map
     * @param key
     */
    bool hasConfig( const QString &key ) const;

    /** Clear all configs */
    void clearConfigMap();

    /**
     * A utility function for generating a resource from a URL to be compared
     * against the config's uri() for auto-selecting authentication configs to use
     * @note Essentially strips the URL query variables, and by default, strips the path as well
     * @param accessurl A URL to process
     * @param resource Ouput variable for result
     * @param withpath Whether to include the URI's path in output
     */
    static bool uriToResource( const QString &accessurl, QString *resource, bool withpath = false );
};

typedef QHash<QString, QgsAuthMethodConfig> QgsAuthMethodConfigsMap;


/** \ingroup core
 * \brief Storage set for PKI bundle: SSL certificate, key, optional CA cert chain
 * \note Useful for caching the bundle during application run sessions
 */
class QgsPkiBundle
{
%TypeHeaderCode
#include <qgsauthconfig.h>
%End
  public:
    /**
     * Construct a bundle from existing PKI components
     * @param clientCert Certificate to store in bundle
     * @param clientKey Private key to store in bundle
     * @param caChain Chain of Certificate Authorities for client certificate
     */
    QgsPkiBundle( const QSslCertificate &clientCert = QSslCertificate(),
                  const QSslKey &clientKey = QSslKey(),
                  const QList<QSslCertificate> &caChain = QList<QSslCertificate>() );

    /**
     * Construct a bundle of PKI components from PEM-formatted file paths
     * @param certPath Certificate file path
     * @param keyPath Private key path
     * @param keyPass Private key passphrase
     * @param caChain Chain of Certificate Authorities for client certificate
     */
    static const QgsPkiBundle fromPemPaths( const QString &certPath,
                                            const QString &keyPath,
                                            const QString &keyPass = QString::null,
                                            const QList<QSslCertificate> &caChain = QList<QSslCertificate>() );

    /**
     * Construct a bundle of PKI components from a PKCS#12 file path
     * @param bundlepath Bundle file path
     * @param bundlepass Optional bundle passphrase
     */
    static const QgsPkiBundle fromPkcs12Paths( const QString &bundlepath,
        const QString &bundlepass = QString::null );

    /** Whether the bundle, either its certificate or private key, is null */
    bool isNull() const;

    /** Whether the bundle is valid */
    bool isValid() const;

    /** The sha hash of the client certificate */
    const QString certId() const;

    /** Client certificate object */
    const QSslCertificate clientCert() const;
    /** Set client certificate object */
    void setClientCert( const QSslCertificate &cert );

    /** Private key object */
    const QSslKey clientKey() const;
    /** Set private key object */
    void setClientKey( const QSslKey &certkey );

    /** Chain of Certificate Authorities for client certificate */
    const QList<QSslCertificate> caChain() const;
    /** Set chain of Certificate Authorities for client certificate */
    void setCaChain( const QList<QSslCertificate> &cachain );
};


/** \ingroup core
 * \brief Storage set for constructed SSL certificate, key, associated with an authentication config
 */
class QgsPkiConfigBundle
{
%TypeHeaderCode
#include <qgsauthconfig.h>
%End

  public:
    /**
     * Construct a bundle from existing PKI components and authentication method configuration
     * @param config Authentication method configuration
     * @param cert Certificate to store in bundle
     * @param certkey Private key to store in bundle
     */
    QgsPkiConfigBundle( const QgsAuthMethodConfig& config,
                        const QSslCertificate& cert,
                        const QSslKey& certkey );

    /** Whether the bundle is valid */
    bool isValid();

    /** Authentication method configuration */
    const QgsAuthMethodConfig config() const;
    /** Set authentication method configuration */
    void setConfig( const QgsAuthMethodConfig& config );

    /** Client certificate object */
    const QSslCertificate clientCert() const;
    /** Set client certificate object */
    void setClientCert( const QSslCertificate& cert );

    /** Private key object */
    const QSslKey clientCertKey() const;
    /** Set private key object */
    void setClientCertKey( const QSslKey& certkey );
};


%MappedType QList<QSslError::SslError>
{
%TypeHeaderCode
#include <QList>
%End

%ConvertFromTypeCode
  // Create the list.
  PyObject *l;

  if ((l = PyList_New(sipCpp->size())) == NULL)
    return NULL;

  // Set the list elements.
  QList<QSslError::SslError>::iterator it = sipCpp->begin();
  for (int i = 0; it != sipCpp->end(); ++it, ++i)
  {
    PyObject *tobj;

    if ((tobj = sipConvertFromEnum(*it, sipType_QSslError_SslError)) == NULL)
    {
      Py_DECREF(l);
      return NULL;
    }
    PyList_SET_ITEM(l, i, tobj);
  }

  return l;
%End

%ConvertToTypeCode
  // Check the type if that is all that is required.
  if (sipIsErr == NULL)
    return PyList_Check(sipPy);

  QList<QSslError::SslError> *qlist = new QList<QSslError::SslError>;

  for (int i = 0; i < PyList_GET_SIZE(sipPy); ++i)
  {
    *qlist << (QSslError::SslError)SIPLong_AsLong(PyList_GET_ITEM(sipPy, i));
  }

  *sipCppPtr = qlist;
  return sipGetState(sipTransferObj);
%End
};


/** \ingroup core
 * \brief Configuration container for SSL server connection exceptions or overrides
 */
class QgsAuthConfigSslServer
{
%TypeHeaderCode
#include <qgsauthconfig.h>
%End
  public:
    QgsAuthConfigSslServer();

    ~QgsAuthConfigSslServer();

    /** Server certificate object */
    const QSslCertificate sslCertificate() const;
    /** Set server certificate object */
    void setSslCertificate( const QSslCertificate& cert );

    /** Server host:port string */
    const QString sslHostPort() const;
    /** Set server host:port string */
    void setSslHostPort( const QString& hostport );

    /** SSL server protocol to use in connections */
    QSsl::SslProtocol sslProtocol() const;
    /** Set SSL server protocol to use in connections */
    void setSslProtocol( QSsl::SslProtocol protocol );

    /** SSL server errors to ignore in connections */
    const QList<QSslError> sslIgnoredErrors() const;
    /** SSL server errors (as enum list) to ignore in connections */
    const QList<QSslError::SslError> sslIgnoredErrorEnums() const;
    /** Set SSL server errors (as enum list) to ignore in connections */
    void setSslIgnoredErrorEnums( const QList<QSslError::SslError>& errors );

    /** SSL client's peer verify mode to use in connections */
    QSslSocket::PeerVerifyMode sslPeerVerifyMode() const;
    /** Set SSL client's peer verify mode to use in connections */
    void setSslPeerVerifyMode( QSslSocket::PeerVerifyMode mode );

    /** Number or SSL client's peer to verify in connections
     * @note When set to 0 = unlimited depth
     */
    int sslPeerVerifyDepth() const;
    /** Set number or SSL client's peer to verify in connections
     * @note When set to 0 = unlimited depth
     */
    void setSslPeerVerifyDepth( int depth );

    /** Version of the configuration (used for future upgrading) */
    int version() const;
    /** Set version of the configuration (used for future upgrading) */
    void setVersion( int version );

    /** Qt version when the configuration was made (SSL protocols may differ) */
    int qtVersion() const;
    /** Set Qt version when the configuration was made (SSL protocols may differ) */
    void setQtVersion( int version );

    /** Configuration as a concatenated string */
    const QString configString() const;
    /** Load concatenated string into configuration, e.g. from auth database */
    void loadConfigString( const QString& config = QString() );

    /** Whether configuration is null (missing components) */
    bool isNull() const;
};

