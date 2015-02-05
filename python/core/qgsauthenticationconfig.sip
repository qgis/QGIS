class QgsAuthType
{
%TypeHeaderCode
#include <qgsauthenticationconfig.h>
%End
  public:
    enum ProviderType
    {
      None = 0,
      Basic = 1,
      PkiPaths = 2,
      PkiPkcs12 = 3,
      Unknown = 20 // padding for more standard auth types
    };

    static const QHash<QgsAuthType::ProviderType, QString> typeNameHash();

    static QgsAuthType::ProviderType providerTypeFromInt( int itype );

    static const QString typeToString( QgsAuthType::ProviderType providertype = None );

    static QgsAuthType::ProviderType stringToType( const QString& name );

    static const QString typeDescription( QgsAuthType::ProviderType providertype = None );
};

/**
 * @brief Base class for configs
 */
class QgsAuthConfigBase
{
%TypeHeaderCode
#include <qgsauthenticationconfig.h>
%End
  public:

    QgsAuthConfigBase( QgsAuthType::ProviderType type = QgsAuthType::None, int version = 0 );

    QgsAuthConfigBase( const QgsAuthConfigBase& config );

    virtual ~QgsAuthConfigBase();

    const QString id() const;
    void setId( const QString& id );

    const QString name() const;
    void setName( const QString& name );

    const QString uri() const;
    void setUri( const QString& uri );

    QgsAuthType::ProviderType type() const;
    void setType( QgsAuthType::ProviderType ptype );

    int version() const;
    void setVersion( int version );

    const QString typeToString() const;

    virtual bool isValid( bool validateid = false ) const;

    virtual const QString configString() const;
    virtual void loadConfigString( const QString& config );

    const QgsAuthConfigBase toBaseConfig();

};


class QgsAuthConfigBasic : QgsAuthConfigBase
{
%TypeHeaderCode
#include <qgsauthenticationconfig.h>
%End
  public:
    QgsAuthConfigBasic();

    QgsAuthConfigBasic( const QgsAuthConfigBase& config );

    ~QgsAuthConfigBasic();

    const QString realm() const;
    void setRealm( const QString& realm );

    const QString username() const;
    void setUsername( const QString& name );

    const QString password() const;
    void setPassword( const QString& pass );

    bool isValid( bool validateid = false ) const;

    const QString configString() const;
    void loadConfigString( const QString& config = QString() );

};

class QgsAuthConfigPkiPaths : QgsAuthConfigBase
{
%TypeHeaderCode
#include <qgsauthenticationconfig.h>
%End
  public:
    QgsAuthConfigPkiPaths();

    QgsAuthConfigPkiPaths( const QgsAuthConfigBase& config );

    ~QgsAuthConfigPkiPaths();

    const QString certId() const;
    void setCertId( const QString& id );

    const QString keyId() const;
    void setKeyId( const QString& id );

    const QString keyPassphrase() const;
    void setKeyPassphrase( const QString& passphrase );

    const QString issuerId() const;
    void setIssuerId( const QString& id );

    bool issuerSelfSigned() const;
    void setIssuerSelfSigned( bool selfsigned );
    
    const QString certAsPem() const;

    const QStringList keyAsPem( bool reencrypt = true ) const;

    const QString issuerAsPem() const;

    bool isValid( bool validateid = false ) const;

    const QString configString() const;
    void loadConfigString( const QString& config = QString() );

};

class QgsAuthConfigPkiPkcs12: QgsAuthConfigBase
{
%TypeHeaderCode
#include <qgsauthenticationconfig.h>
%End
  public:
    QgsAuthConfigPkiPkcs12();

    QgsAuthConfigPkiPkcs12( const QgsAuthConfigBase& config );

    ~QgsAuthConfigPkiPkcs12();

    const QString bundlePath() const;
    void setBundlePath( const QString& path );

    const QString bundlePassphrase() const;
    void setBundlePassphrase( const QString& passphrase );

    const QString issuerPath() const;
    void setIssuerPath( const QString& id );

    bool issuerSelfSigned() const;
    void setIssuerSelfSigned( bool selfsigned );

    const QString certAsPem() const;

    const QStringList keyAsPem( bool reencrypt = true ) const;

    const QString issuerAsPem() const;

    bool isValid( bool validateid = false ) const;

    const QString configString() const;
    void loadConfigString( const QString& config = QString() );

};

