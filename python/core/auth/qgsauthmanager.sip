class QgsAuthManager : QObject
{
%TypeHeaderCode
#include <qgsauthmanager.h>
#include <qgsauthmethod.h>
%End

  public:

    enum MessageLevel
    {
      INFO = 0,
      WARNING = 1,
      CRITICAL = 2
    };

    static QgsAuthManager *instance();

    ~QgsAuthManager();

    QSqlDatabase authDbConnection() const;

    const QString authDbConfigTable() const;

    const QString authDbServersTable() const;

    bool init( const QString& pluginPath = QString::null );

    bool isDisabled() const;

    const QString disabledMessage() const;

    const QString authenticationDbPath() const;

    bool setMasterPassword( bool verify = false );

    bool setMasterPassword( const QString& pass, bool verify = false );

    bool verifyMasterPassword();

    bool masterPasswordIsSet() const;

    bool masterPasswordHashInDb() const;

    void clearMasterPassword();

    bool masterPasswordSame( const QString& pass ) const;

    bool resetMasterPassword( const QString& newpassword, const QString& oldpass, bool keepbackup, QString *backuppath /In,Out/ = 0 );

    const QString authManTag() const;

    bool registerCoreAuthMethods();

    QgsAuthMethodConfigsMap availableAuthMethodConfigs( const QString &dataprovider = QString() );

    void updateConfigAuthMethods();

    QgsAuthMethod *configAuthMethod( const QString& authcfg );

    QString configAuthMethodKey( const QString& authcfg ) const;

    QStringList authMethodsKeys( const QString &dataprovider = QString() );

    QgsAuthMethod *authMethod( const QString& authMethodKey );

    // "unsupported function return type - provide %MethodCode and a C++ signature"
    //QgsAuthMethodsMap authMethodsMap( const QString &dataprovider = QString() );

    QWidget *authMethodEditWidget( const QString& authMethodKey , QWidget *parent );

    QgsAuthMethod::Expansions supportedAuthMethodExpansions( const QString& authcfg );

    const QString uniqueConfigId() const;

    bool configIdUnique( const QString &id ) const;

    bool hasConfigId( const QString &txt ) const;

    QString configIdRegex() const;

    QStringList configIds() const;


    bool storeAuthenticationConfig( QgsAuthMethodConfig &mconfig /In,Out/ );

    bool updateAuthenticationConfig( const QgsAuthMethodConfig& config );

    bool loadAuthenticationConfig( const QString& authcfg, QgsAuthMethodConfig &mconfig /In,Out/, bool full = false );

    bool removeAuthenticationConfig( const QString& authcfg );

    bool removeAllAuthenticationConfigs();

    bool backupAuthenticationDatabase( QString *backuppath /In,Out/ = 0 );

    bool eraseAuthenticationDatabase( bool backup, QString *backuppath /In,Out/ = 0 );

    bool updateNetworkRequest( QNetworkRequest &request /In,Out/, const QString& authcfg,
                               const QString &dataprovider = QString() );

    bool updateNetworkReply( QNetworkReply *reply, const QString& authcfg,
                             const QString &dataprovider = QString() );

    bool updateDataSourceUriItems( QStringList &connectionItems /In,Out/, const QString& authcfg,
                                   const QString &dataprovider = QString() );

    ////////////////// Generic settings ///////////////////////

    bool storeAuthSetting( const QString& key, const QVariant& value, bool encrypt = false );

    QVariant getAuthSetting( const QString& key, const QVariant& defaultValue = QVariant(), bool decrypt = false );

    bool existsAuthSetting( const QString& key );

    bool removeAuthSetting( const QString& key );

    ////////////////// Certificate calls ///////////////////////

    bool initSslCaches();

    bool storeCertIdentity( const QSslCertificate& cert, const QSslKey& key );

    const QSslCertificate getCertIdentity( const QString& id );

    // This should be a restricted function
    //const QPair<QSslCertificate, QSslKey> getCertIdentityBundle( const QString& id );

    // This should be a restricted function
    const QStringList getCertIdentityBundleToPem( const QString& id );

    const QList<QSslCertificate> getCertIdentities();

    bool existsCertIdentity( const QString& id );

    bool removeCertIdentity( const QString& id );


    bool storeSslCertCustomConfig( const QgsAuthConfigSslServer& config );

    const QgsAuthConfigSslServer getSslCertCustomConfig( const QString& id, const QString &hostport );

    const QgsAuthConfigSslServer getSslCertCustomConfigByHost( const QString& hostport );

    const QList<QgsAuthConfigSslServer> getSslCertCustomConfigs();

    bool existsSslCertCustomConfig( const QString& id, const QString &hostport );

    bool removeSslCertCustomConfig( const QString& id, const QString &hostport );

    //QHash<QString, QSet<QSslError::SslError>> getIgnoredSslErrorCache();

    void dumpIgnoredSslErrorsCache_();

    bool updateIgnoredSslErrorsCacheFromConfig( const QgsAuthConfigSslServer &config );

    bool updateIgnoredSslErrorsCache( const QString &shahostport, const QList<QSslError> &errors );

    bool rebuildIgnoredSslErrorCache();


    bool storeCertAuthorities( const QList<QSslCertificate>& certs );

    bool storeCertAuthority( const QSslCertificate& cert );

    const QSslCertificate getCertAuthority( const QString& id );

    bool existsCertAuthority( const QSslCertificate& cert );

    bool removeCertAuthority( const QSslCertificate& cert );

    const QList<QSslCertificate> getSystemRootCAs();

    const QList<QSslCertificate> getExtraFileCAs();

    const QList<QSslCertificate> getDatabaseCAs();

    const QMap<QString, QSslCertificate> getMappedDatabaseCAs();

    //const QMap<QString, QPair<QgsAuthCertUtils::CaCertSource , QSslCertificate> > getCaCertsCache();

    bool rebuildCaCertsCache();


    bool storeCertTrustPolicy( const QSslCertificate& cert, QgsAuthCertUtils::CertTrustPolicy policy );

    QgsAuthCertUtils::CertTrustPolicy getCertTrustPolicy( const QSslCertificate& cert );

    bool removeCertTrustPolicies( const QList<QSslCertificate>& certs );

    bool removeCertTrustPolicy( const QSslCertificate& cert );

    QgsAuthCertUtils::CertTrustPolicy getCertificateTrustPolicy( const QSslCertificate& cert );

    bool setDefaultCertTrustPolicy( QgsAuthCertUtils::CertTrustPolicy policy );

    QgsAuthCertUtils::CertTrustPolicy defaultCertTrustPolicy();

    const QMap<QgsAuthCertUtils::CertTrustPolicy, QStringList > getCertTrustCache();

    bool rebuildCertTrustCache();

    const QList<QSslCertificate> getTrustedCaCerts( bool includeinvalid = false );

    const QList<QSslCertificate> getUntrustedCaCerts( QList<QSslCertificate> trustedCAs = QList<QSslCertificate>() );

    bool rebuildTrustedCaCertsCache();

    const QList<QSslCertificate> getTrustedCaCertsCache();

    const QByteArray getTrustedCaCertsPemText();

    QMutex *mutex();

  signals:
    void messageOut( const QString& message, const QString& tag = smAuthManTag, QgsAuthManager::MessageLevel level = INFO ) const;

    void masterPasswordVerified( bool verified ) const;

    void authDatabaseChanged() const;

  public slots:
    void clearAllCachedConfigs();

    void clearCachedConfig( const QString& authcfg );

  protected:
    explicit QgsAuthManager();
};
