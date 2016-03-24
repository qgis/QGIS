class QgsAuthMethodConfig
{
%TypeHeaderCode
#include <qgsauthconfig.h>
%End
  public:

    QgsAuthMethodConfig( const QString& method = QString(), int version = 0 );

    QgsAuthMethodConfig( const QgsAuthMethodConfig& methodconfig );

    bool operator==( const QgsAuthMethodConfig& other ) const;

    bool operator!=( const QgsAuthMethodConfig& other ) const;

    ~QgsAuthMethodConfig();

    const QString id() const;
    void setId( const QString& id );

    const QString name() const;
    void setName( const QString& name );

    const QString uri() const;
    void setUri( const QString& uri );

    QString method() const;
    void setMethod( const QString& method );

    int version() const;
    void setVersion( int version );

    bool isValid( bool validateid = false ) const;

    const QString configString() const;
    void loadConfigString( const QString& configstr );

    QgsStringMap configMap() const;
    void setConfigMap( const QgsStringMap& map );

    void setConfig( const QString &key, const QString &value );
    void setConfigList( const QString &key, const QStringList &value );

    int removeConfig( const QString &key );

    QString config( const QString &key, const QString& defaultvalue = QString() ) const;

    QStringList configList( const QString &key ) const;

    bool hasConfig( const QString &key ) const;

    void clearConfigMap();

    static bool uriToResource( const QString &accessurl, QString *resource, bool withpath = false );
};

typedef QHash<QString, QgsAuthMethodConfig> QgsAuthMethodConfigsMap;


class QgsPkiBundle
{
%TypeHeaderCode
#include <qgsauthconfig.h>
%End
  public:
    QgsPkiBundle( const QSslCertificate &clientCert = QSslCertificate(),
                  const QSslKey &clientKey = QSslKey(),
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

    const QSslCertificate clientCert() const;
    void setClientCert( const QSslCertificate &cert );

    const QSslKey clientKey() const;
    void setClientKey( const QSslKey &certkey );

    const QList<QSslCertificate> caChain() const;
    void setCaChain( const QList<QSslCertificate> &cachain );
};


class QgsPkiConfigBundle
{
%TypeHeaderCode
#include <qgsauthconfig.h>
%End

  public:
    QgsPkiConfigBundle( const QgsAuthMethodConfig& config,
                  const QSslCertificate& cert,
                  const QSslKey& certkey );
    ~QgsPkiConfigBundle();

    bool isValid();

    const QgsAuthMethodConfig config() const;
    void setConfig( const QgsAuthMethodConfig& config );

    const QSslCertificate clientCert() const;
    void setClientCert( const QSslCertificate& cert );

    const QSslKey clientCertKey() const;
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


class QgsAuthConfigSslServer
{
%TypeHeaderCode
#include <qgsauthconfig.h>
%End
  public:
    QgsAuthConfigSslServer();

    ~QgsAuthConfigSslServer();

    const QSslCertificate sslCertificate() const;
    void setSslCertificate( const QSslCertificate& cert );

    const QString sslHostPort() const;
    void setSslHostPort( const QString& hostport );

    QSsl::SslProtocol sslProtocol() const;
    void setSslProtocol( QSsl::SslProtocol protocol );

    const QList<QSslError> sslIgnoredErrors() const;
    const QList<QSslError::SslError> sslIgnoredErrorEnums() const;
    void setSslIgnoredErrorEnums( const QList<QSslError::SslError>& errors );

    QSslSocket::PeerVerifyMode sslPeerVerifyMode() const;
    void setSslPeerVerifyMode( QSslSocket::PeerVerifyMode mode );

    int sslPeerVerifyDepth() const;
    void setSslPeerVerifyDepth( int depth );

    int version() const;
    void setVersion( int version );

    int qtVersion() const;
    void setQtVersion( int version );

    const QString configString() const;
    void loadConfigString( const QString& config = QString() );

    bool isNull() const;
};

