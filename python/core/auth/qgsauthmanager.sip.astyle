/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/auth/qgsauthmanager.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/








class QgsAuthManager : QObject
{
%Docstring
 Singleton offering an interface to manage the authentication configuration database
 and to utilize configurations through various authentication method plugins
%End

%TypeHeaderCode
#include "qgsauthmanager.h"
%End
  public:

    enum MessageLevel
    {
      INFO,
      WARNING,
      CRITICAL
    };

    static QgsAuthManager *instance();
%Docstring
 Enforce singleton pattern
.. note::

   To set up the manager instance and initialize everything use QgsAuthManager.instance()->init()
 :rtype: QgsAuthManager
%End

    ~QgsAuthManager();

    QSqlDatabase authDatabaseConnection() const;
%Docstring
Set up the application instance of the authentication database connection
 :rtype: QSqlDatabase
%End

    const QString authDatabaseConfigTable() const;
%Docstring
Name of the authentication database table that stores configs
 :rtype: str
%End

    const QString authDatabaseServersTable() const;
%Docstring
Name of the authentication database table that stores server exceptions/configs
 :rtype: str
%End

    bool init( const QString &pluginPath = QString() );
%Docstring
Initialize QCA, prioritize qca-ossl plugin and optionally set up the authentication database
 :rtype: bool
%End

    bool isDisabled() const;
%Docstring
Whether QCA has the qca-ossl plugin, which a base run-time requirement
 :rtype: bool
%End

    const QString disabledMessage() const;
%Docstring
Standard message for when QCA's qca-ossl plugin is missing and system is disabled
 :rtype: str
%End

    const QString authenticationDatabasePath() const;
%Docstring
 The standard authentication database file in ~/.qgis3/ or defined location
.. seealso:: QgsApplication.qgisAuthDatabaseFilePath
 :rtype: str
%End

    bool setMasterPassword( bool verify = false );
%Docstring
 Main call to initially set or continually check master password is set
.. note::

   If it is not set, the user is asked for its input
 \param verify Whether password's hash was saved in authentication database
 :rtype: bool
%End

    bool setMasterPassword( const QString &pass, bool verify = false );
%Docstring
 Overloaded call to reset master password or set it initially without user interaction
.. note::

   Only use this in trusted reset functions, unit tests or user/app setup scripts!
 \param pass Password to use
 \param verify Whether password's hash was saved in authentication database
 :rtype: bool
%End

    bool verifyMasterPassword( const QString &compare = QString() );
%Docstring
 Verify the supplied master password against any existing hash in authentication database
.. note::

   Do not emit verification signals when only comparing
 \param compare Password to compare against
 :rtype: bool
%End

    bool masterPasswordIsSet() const;
%Docstring
Whether master password has be input and verified, i.e. authentication database is accessible
 :rtype: bool
%End

    bool masterPasswordHashInDatabase() const;
%Docstring
Verify a password hash existing in authentication database
 :rtype: bool
%End

    void clearMasterPassword();
%Docstring
 Clear supplied master password
.. note::

   This will not necessarily clear authenticated connections cached in network connection managers
%End

    bool masterPasswordSame( const QString &pass ) const;
%Docstring
 Check whether supplied password is the same as the one already set
 \param pass Password to verify
 :rtype: bool
%End

    bool resetMasterPassword( const QString &newpass, const QString &oldpass, bool keepbackup, QString *backuppath /In,Out/ = 0 );
%Docstring
 Reset the master password to a new one, then re-encrypt all previous
 configs in a new database file, optionally backup curren database
 \param newpass New master password to replace existing
 \param oldpass Current master password to replace existing
 \param keepbackup Whether to keep the generated backup of current database
 \param backuppath Where the backup is located, if kept
 :rtype: bool
%End



    void setScheduledAuthDatabaseEraseRequestEmitted( bool emitted );
%Docstring
 Re-emit a signal to schedule an optional erase of authentication database.
.. note::

   This can be called from the slot connected to a previously emitted scheduling signal,
 so that the slot can ask for another emit later, if the slot noticies the current GUI
 processing state is not ready for interacting with the user, e.g. project is still loading
 \param emitted Setting to false will cause signal to be emitted by the schedule timer.
 Setting to true will stop any emitting, but will not stop the schedule timer.
%End

    QString authManTag() const;
%Docstring
Simple text tag describing authentication system for message logs
 :rtype: str
%End

    bool registerCoreAuthMethods();
%Docstring
Instantiate and register existing C++ core authentication methods from plugins
 :rtype: bool
%End

    QgsAuthMethodConfigsMap availableAuthMethodConfigs( const QString &dataprovider = QString() );
%Docstring
Get mapping of authentication config ids and their base configs (not decrypted data)
 :rtype: QgsAuthMethodConfigsMap
%End

    void updateConfigAuthMethods();
%Docstring
Sync the confg/authentication method cache with what is in database
%End

    QgsAuthMethod *configAuthMethod( const QString &authcfg );
%Docstring
 Get authentication method from the config/provider cache
 \param authcfg Authentication config id
 :rtype: QgsAuthMethod
%End

    QString configAuthMethodKey( const QString &authcfg ) const;
%Docstring
 Get key of authentication method associated with config ID
 \param authcfg
 :rtype: str
%End

    QStringList authMethodsKeys( const QString &dataprovider = QString() );
%Docstring
 Get keys of supported authentication methods
 :rtype: list of str
%End

    QgsAuthMethod *authMethod( const QString &authMethodKey );
%Docstring
 Get authentication method from the config/provider cache via its key
 \param authMethodKey Authentication method key
 :rtype: QgsAuthMethod
%End


    QWidget *authMethodEditWidget( const QString &authMethodKey, QWidget *parent );
%Docstring
 Get authentication method edit widget via its key
 \param authMethodKey Authentication method key
 \param parent Parent widget
 :rtype: QWidget
%End

    QgsAuthMethod::Expansions supportedAuthMethodExpansions( const QString &authcfg );
%Docstring
 Get supported authentication method expansion(s), e.g. NetworkRequest | DataSourceURI, as flags
 \param authcfg
 :rtype: QgsAuthMethod.Expansions
%End

    const QString uniqueConfigId() const;
%Docstring
Get a unique generated 7-character string to assign to as config id
 :rtype: str
%End

    bool configIdUnique( const QString &id ) const;
%Docstring
 Verify if provided authentication id is unique
 \param id Id to check
 :rtype: bool
%End

    bool hasConfigId( const QString &txt ) const;
%Docstring
 Return whether a string includes an authcfg ID token
 \param txt String to check
 :rtype: bool
%End

    QString configIdRegex() const;
%Docstring
Return regular expression for authcfg=.{7} key/value token for authentication ids
 :rtype: str
%End

    QStringList configIds() const;
%Docstring
Get list of authentication ids from database
 :rtype: list of str
%End

    bool storeAuthenticationConfig( QgsAuthMethodConfig &mconfig /In,Out/ );
%Docstring
 Store an authentication config in the database
 \param mconfig Associated authentication config id
 :return: Whether operation succeeded
 :rtype: bool
%End

    bool updateAuthenticationConfig( const QgsAuthMethodConfig &config );
%Docstring
 Update an authentication config in the database
 \param config Associated authentication config id
 :return: Whether operation succeeded
 :rtype: bool
%End

    bool loadAuthenticationConfig( const QString &authcfg, QgsAuthMethodConfig &mconfig /In,Out/, bool full = false );
%Docstring
 Load an authentication config from the database into subclass
 \param authcfg Associated authentication config id
 \param mconfig Subclassed config to load into
 \param full Whether to decrypt and populate all sensitive data in subclass
 :return: Whether operation succeeded
 :rtype: bool
%End

    bool removeAuthenticationConfig( const QString &authcfg );
%Docstring
 Remove an authentication config in the database
 \param authcfg Associated authentication config id
 :return: Whether operation succeeded
 :rtype: bool
%End

    bool removeAllAuthenticationConfigs();
%Docstring
 Clear all authentication configs from table in database and from provider caches
 :return: Whether operation succeeded
 :rtype: bool
%End

    bool backupAuthenticationDatabase( QString *backuppath /In,Out/ = 0 );
%Docstring
 Close connection to current authentication database and back it up
 :return: Path to backup
 :rtype: bool
%End

    bool eraseAuthenticationDatabase( bool backup, QString *backuppath /In,Out/ = 0 );
%Docstring
 Erase all rows from all tables in authentication database
 \param backup Whether to backup of current database
 \param backuppath Where the backup is locate
 :return: Whether operation succeeded
 :rtype: bool
%End



    bool updateNetworkRequest( QNetworkRequest &request /In,Out/, const QString &authcfg,
                               const QString &dataprovider = QString() );
%Docstring
 Provider call to update a QNetworkRequest with an authentication config
 \param request The QNetworkRequest
 \param authcfg Associated authentication config id
 \param dataprovider Provider key filter, offering logic branching in authentication method
 :return: Whether operation succeeded
 :rtype: bool
%End

    bool updateNetworkReply( QNetworkReply *reply, const QString &authcfg,
                             const QString &dataprovider = QString() );
%Docstring
 Provider call to update a QNetworkReply with an authentication config (used to skip known SSL errors, etc.)
 \param reply The QNetworkReply
 \param authcfg Associated authentication config id
 \param dataprovider Provider key filter, offering logic branching in authentication method
 :return: Whether operation succeeded
 :rtype: bool
%End

    bool updateDataSourceUriItems( QStringList &connectionItems /In,Out/, const QString &authcfg,
                                   const QString &dataprovider = QString() );
%Docstring
 Provider call to update a QgsDataSourceUri with an authentication config
 \param connectionItems The connection items, e.g. username=myname, of QgsDataSourceUri
 \param authcfg Associated authentication config id
 \param dataprovider Provider key filter, offering logic branching in authentication method
 :return: Whether operation succeeded
 :rtype: bool
%End


    bool storeAuthSetting( const QString &key, const QVariant &value, bool encrypt = false );
%Docstring
Store an authentication setting (stored as string via QVariant( value ).toString() )
 :rtype: bool
%End

    QVariant getAuthSetting( const QString &key, const QVariant &defaultValue = QVariant(), bool decrypt = false );
%Docstring
Get an authentication setting (retrieved as string and returned as QVariant( QString ))
 :rtype: QVariant
%End

    bool existsAuthSetting( const QString &key );
%Docstring
Check if an authentication setting exists
 :rtype: bool
%End

    bool removeAuthSetting( const QString &key );
%Docstring
Remove an authentication setting
 :rtype: bool
%End


    bool initSslCaches();
%Docstring
Initialize various SSL authentication caches
 :rtype: bool
%End

    bool storeCertIdentity( const QSslCertificate &cert, const QSslKey &key );
%Docstring
Store a certificate identity
 :rtype: bool
%End

    const QSslCertificate getCertIdentity( const QString &id );
%Docstring
Get a certificate identity by id (sha hash)
 :rtype: QSslCertificate
%End


    const QStringList getCertIdentityBundleToPem( const QString &id );
%Docstring
Get a certificate identity bundle by id (sha hash) returned as PEM text
 :rtype: list of str
%End

    const QList<QSslCertificate> getCertIdentities();
%Docstring
Get certificate identities
 :rtype: list of QSslCertificate
%End

    QStringList getCertIdentityIds() const;
%Docstring
Get list of certificate identity ids from database
 :rtype: list of str
%End

    bool existsCertIdentity( const QString &id );
%Docstring
Check if a certificate identity exists
 :rtype: bool
%End

    bool removeCertIdentity( const QString &id );
%Docstring
Remove a certificate identity
 :rtype: bool
%End


    bool storeSslCertCustomConfig( const QgsAuthConfigSslServer &config );
%Docstring
Store an SSL certificate custom config
 :rtype: bool
%End

    const QgsAuthConfigSslServer getSslCertCustomConfig( const QString &id, const QString &hostport );
%Docstring
Get an SSL certificate custom config by id (sha hash) and host:port
 :rtype: QgsAuthConfigSslServer
%End

    const QgsAuthConfigSslServer getSslCertCustomConfigByHost( const QString &hostport );
%Docstring
Get an SSL certificate custom config by host:port
 :rtype: QgsAuthConfigSslServer
%End

    const QList<QgsAuthConfigSslServer> getSslCertCustomConfigs();
%Docstring
Get SSL certificate custom configs
 :rtype: list of QgsAuthConfigSslServer
%End

    bool existsSslCertCustomConfig( const QString &id, const QString &hostport );
%Docstring
Check if SSL certificate custom config exists
 :rtype: bool
%End

    bool removeSslCertCustomConfig( const QString &id, const QString &hostport );
%Docstring
Remove an SSL certificate custom config
 :rtype: bool
%End


    void dumpIgnoredSslErrorsCache_();
%Docstring
Utility function to dump the cache for debug purposes
%End

    bool updateIgnoredSslErrorsCacheFromConfig( const QgsAuthConfigSslServer &config );
%Docstring
Update ignored SSL error cache with possible ignored SSL errors, using server config
 :rtype: bool
%End

    bool updateIgnoredSslErrorsCache( const QString &shahostport, const QList<QSslError> &errors );
%Docstring
Update ignored SSL error cache with possible ignored SSL errors, using sha:host:port key
 :rtype: bool
%End

    bool rebuildIgnoredSslErrorCache();
%Docstring
Rebuild ignoredSSL error cache
 :rtype: bool
%End


    bool storeCertAuthorities( const QList<QSslCertificate> &certs );
%Docstring
Store multiple certificate authorities
 :rtype: bool
%End

    bool storeCertAuthority( const QSslCertificate &cert );
%Docstring
Store a certificate authority
 :rtype: bool
%End

    const QSslCertificate getCertAuthority( const QString &id );
%Docstring
Get a certificate authority by id (sha hash)
 :rtype: QSslCertificate
%End

    bool existsCertAuthority( const QSslCertificate &cert );
%Docstring
Check if a certificate authority exists
 :rtype: bool
%End

    bool removeCertAuthority( const QSslCertificate &cert );
%Docstring
Remove a certificate authority
 :rtype: bool
%End

    const QList<QSslCertificate> getSystemRootCAs();
%Docstring
Get root system certificate authorities
 :rtype: list of QSslCertificate
%End

    const QList<QSslCertificate> getExtraFileCAs();
%Docstring
Get extra file-based certificate authorities
 :rtype: list of QSslCertificate
%End

    const QList<QSslCertificate> getDatabaseCAs();
%Docstring
Get database-stored certificate authorities
 :rtype: list of QSslCertificate
%End

    const QMap<QString, QSslCertificate> getMappedDatabaseCAs();
%Docstring
Get sha1-mapped database-stored certificate authorities
 :rtype: QMap<str, QSslCertificate>
%End


    bool rebuildCaCertsCache();
%Docstring
Rebuild certificate authority cache
 :rtype: bool
%End

    bool storeCertTrustPolicy( const QSslCertificate &cert, QgsAuthCertUtils::CertTrustPolicy policy );
%Docstring
Store user trust value for a certificate
 :rtype: bool
%End

    QgsAuthCertUtils::CertTrustPolicy getCertTrustPolicy( const QSslCertificate &cert );
%Docstring
 Get a whether certificate is trusted by user
:return: DefaultTrust if certificate sha not in trust table, i.e. follows default trust policy
 :rtype: QgsAuthCertUtils.CertTrustPolicy
%End

    bool removeCertTrustPolicies( const QList<QSslCertificate> &certs );
%Docstring
Remove a group certificate authorities
 :rtype: bool
%End

    bool removeCertTrustPolicy( const QSslCertificate &cert );
%Docstring
Remove a certificate authority
 :rtype: bool
%End

    QgsAuthCertUtils::CertTrustPolicy getCertificateTrustPolicy( const QSslCertificate &cert );
%Docstring
Get trust policy for a particular certificate
 :rtype: QgsAuthCertUtils.CertTrustPolicy
%End

    bool setDefaultCertTrustPolicy( QgsAuthCertUtils::CertTrustPolicy policy );
%Docstring
Set the default certificate trust policy perferred by user
 :rtype: bool
%End

    QgsAuthCertUtils::CertTrustPolicy defaultCertTrustPolicy();
%Docstring
Get the default certificate trust policy perferred by user
 :rtype: QgsAuthCertUtils.CertTrustPolicy
%End

    const QMap<QgsAuthCertUtils::CertTrustPolicy, QStringList > getCertTrustCache();
%Docstring
Get cache of certificate sha1s, per trust policy
 :rtype: QMap<QgsAuthCertUtils.CertTrustPolicy, list of str >
%End

    bool rebuildCertTrustCache();
%Docstring
Rebuild certificate authority cache
 :rtype: bool
%End

    const QList<QSslCertificate> getTrustedCaCerts( bool includeinvalid = false );
%Docstring
Get list of all trusted CA certificates
 :rtype: list of QSslCertificate
%End

    const QList<QSslCertificate> getUntrustedCaCerts( QList<QSslCertificate> trustedCAs = QList<QSslCertificate>() );
%Docstring
Get list of all untrusted CA certificates
 :rtype: list of QSslCertificate
%End

    bool rebuildTrustedCaCertsCache();
%Docstring
Rebuild trusted certificate authorities cache
 :rtype: bool
%End

    const QList<QSslCertificate> getTrustedCaCertsCache();
%Docstring
Get cache of trusted certificate authorities, ready for network connections
 :rtype: list of QSslCertificate
%End

    const QByteArray getTrustedCaCertsPemText();
%Docstring
Get concatenated string of all trusted CA certificates
 :rtype: QByteArray
%End


    QMutex *mutex();
%Docstring
Return pointer to mutex
 :rtype: QMutex
%End








    static const QString AUTH_PASSWORD_HELPER_DISPLAY_NAME;
%Docstring
The display name of the password helper (platform dependent)
%End

    static const QString AUTH_MAN_TAG;
%Docstring
The display name of the Authentication Manager
%End

  signals:

    void passwordHelperFailure();
%Docstring
 Signals emitted on password helper failure,
 mainly used in the tests to exit main application loop
%End

    void passwordHelperSuccess();
%Docstring
 Signals emitted on password helper success,
 mainly used in the tests to exit main application loop
%End

    void messageOut( const QString &message, const QString &tag = QgsAuthManager::AUTH_MAN_TAG, QgsAuthManager::MessageLevel level = QgsAuthManager::INFO ) const;
%Docstring
 Custom logging signal to relay to console output and QgsMessageLog
.. seealso:: QgsMessageLog
 \param message Message to send
 \param tag Associated tag (title)
 \param level Message log level
%End

    void passwordHelperMessageOut( const QString &message, const QString &tag = QgsAuthManager::AUTH_MAN_TAG, QgsAuthManager::MessageLevel level = QgsAuthManager::INFO ) const;
%Docstring
 Custom logging signal to inform the user about master password <-> password manager interactions
 @see QgsMessageLog
 @param message Message to send
 @param tag Associated tag (title)
 @param level Message log level
%End


    void masterPasswordVerified( bool verified ) const;
%Docstring
 Emitted when a password has been verify (or not)
 \param verified The state of password's verification
%End

    void authDatabaseEraseRequested() const;
%Docstring
Emitted when a user has indicated they may want to erase the authentication db.
%End

    void authDatabaseChanged() const;
%Docstring
Emitted when the authentication db is significantly changed, e.g. large record removal, erased, etc.
%End

  public slots:
    void clearAllCachedConfigs();
%Docstring
Clear all authentication configs from authentication method caches
%End

    void clearCachedConfig( const QString &authcfg );
%Docstring
Clear an authentication config from its associated authentication method cache
%End

  protected:
    explicit QgsAuthManager();

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/auth/qgsauthmanager.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
