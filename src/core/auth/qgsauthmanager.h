/***************************************************************************
    qgsauthmanager.h
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

#ifndef QGSAUTHMANAGER_H
#define QGSAUTHMANAGER_H

#include <QObject>
#include <QMutex>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>

#ifndef QT_NO_OPENSSL
#include <QSslCertificate>
#include <QSslKey>
#include <QtCrypto>
#include "qgsauthcertutils.h"
#endif

#include "qgsauthconfig.h"
#include "qgsauthmethod.h"
#include "qgsdatasourceuri.h"

namespace QCA
{
  class Initializer;
}
class QgsAuthMethod;
class QgsAuthMethodEdit;
class QgsAuthProvider;
class QTimer;


/** \ingroup core
 * Singleton offering an interface to manage the authentication configuration database
 * and to utilize configurations through various authentication method plugins
 */
class CORE_EXPORT QgsAuthManager : public QObject
{
    Q_OBJECT
    Q_ENUMS( MessageLevel )

  public:

    /** Message log level (mirrors that of QgsMessageLog, so it can also output there) */
    enum MessageLevel
    {
      INFO = 0,
      WARNING = 1,
      CRITICAL = 2
    };

    /** Enforce singleton pattern
     * @note To set up the manager instance and initialize everything use QgsAuthManager::instance()->init()
     */
    static QgsAuthManager *instance();

    ~QgsAuthManager();

    /** Set up the application instance of the authentication database connection */
    QSqlDatabase authDbConnection() const;

    /** Name of the authentication database table that stores configs */
    const QString authDbConfigTable() const { return smAuthConfigTable; }

    /** Name of the authentication database table that stores server exceptions/configs */
    const QString authDbServersTable() const { return smAuthServersTable; }

    /** Initialize QCA, prioritize qca-ossl plugin and optionally set up the authentication database */
    bool init( const QString& pluginPath = QString::null );

    /** Whether QCA has the qca-ossl plugin, which a base run-time requirement */
    bool isDisabled() const;

    /** Standard message for when QCA's qca-ossl plugin is missing and system is disabled */
    const QString disabledMessage() const;

    /** The standard authentication database file in ~/.qgis2/ or defined location
     * @see QgsApplication::qgisAuthDbFilePath
     */
    const QString authenticationDbPath() const { return mAuthDbPath; }

    /** Main call to initially set or continually check master password is set
     * @note If it is not set, the user is asked for its input
     * @param verify Whether password's hash was saved in authentication database
     */
    bool setMasterPassword( bool verify = false );

    /** Overloaded call to reset master password or set it initially without user interaction
     * @note Only use this in trusted reset functions, unit tests or user/app setup scripts!
     * @param pass Password to use
     * @param verify Whether password's hash was saved in authentication database
     */
    bool setMasterPassword( const QString& pass, bool verify = false );

    /** Verify the supplied master password against any existing hash in authentication database
     * @note Do not emit verification signals when only comparing
     * @param compare Password to compare against
     */
    bool verifyMasterPassword( const QString &compare = QString::null );

    /** Whether master password has be input and verified, i.e. authentication database is accessible */
    bool masterPasswordIsSet() const;

    /** Verify a password hash existing in authentication database */
    bool masterPasswordHashInDb() const;

    /** Clear supplied master password
     * @note This will not necessarily clear authenticated connections cached in network connection managers
     */
    void clearMasterPassword() { mMasterPass = QString(); }

    /** Check whether supplied password is the same as the one already set
     * @param pass Password to verify
     */
    bool masterPasswordSame( const QString& pass ) const;

    /** Reset the master password to a new one, then re-encrypt all previous
     * configs in a new database file, optionally backup curren database
     * @param newpass New master password to replace existing
     * @param oldpass Current master password to replace existing
     * @param keepbackup Whether to keep the generated backup of current database
     * @param backuppath Where the backup is located, if kept
     */
    bool resetMasterPassword( const QString& newpass, const QString& oldpass, bool keepbackup, QString *backuppath = nullptr );

    /** Whether there is a scheduled opitonal erase of authentication database.
     * @note not available in Python bindings
     */
    bool scheduledAuthDbErase() { return mScheduledDbErase; }

    /** Schedule an optional erase of authentication database, starting when mutex is lockable.
     * @note When an erase is scheduled, any attempt to set the master password,
     * e.g. password input dialog, is effectively cancelled.
     * For example: In a GUI app, this keeps excess password input dialogs from popping
     * up when a user has initiated an erase, from a password input dialog, because
     * they forgot their password.
     * The created schedule timer will emit a request to gain access to the user,
     * through the given application, to prompt the erase operation (e.g. via a dialog);
     * if no access to user interaction occurs wihtin 90 seconds, it cancels the schedule.
     * @note not available in Python bindings
     */
    void setScheduledAuthDbErase( bool scheduleErase );

    /** Re-emit a signal to schedule an optional erase of authentication database.
     * @note This can be called from the slot connected to a previously emitted scheduling signal,
     * so that the slot can ask for another emit later, if the slot noticies the current GUI
     * processing state is not ready for interacting with the user, e.g. project is still loading
     * @param emitted Setting to false will cause signal to be emitted by the schedule timer.
     * Setting to true will stop any emitting, but will not stop the schedule timer.
     */
    void setScheduledAuthDbEraseRequestEmitted( bool emitted ) { mScheduledDbEraseRequestEmitted = emitted; }

    /** Simple text tag describing authentication system for message logs */
    QString authManTag() const { return smAuthManTag; }

    /** Instantiate and register existing C++ core authentication methods from plugins */
    bool registerCoreAuthMethods();

    /** Get mapping of authentication config ids and their base configs (not decrypted data) */
    QgsAuthMethodConfigsMap availableAuthMethodConfigs( const QString &dataprovider = QString() );

    /** Sync the confg/authentication method cache with what is in database */
    void updateConfigAuthMethods();

    /**
     * Get authentication method from the config/provider cache
     * @param authcfg Authentication config id
     */
    QgsAuthMethod *configAuthMethod( const QString& authcfg );

    /**
     * Get key of authentication method associated with config ID
     * @param authcfg
     */
    QString configAuthMethodKey( const QString& authcfg ) const;

    /**
     * Get keys of supported authentication methods
     */
    QStringList authMethodsKeys( const QString &dataprovider = QString() );

    /**
     * Get authentication method from the config/provider cache via its key
     * @param authMethodKey Authentication method key
     */
    QgsAuthMethod *authMethod( const QString &authMethodKey );

    /**
     * Get available authentication methods mapped to their key
     * @param dataprovider Provider key filter, returning only methods that support a particular provider
     * @note not available in Python bindings
     */
    QgsAuthMethodsMap authMethodsMap( const QString &dataprovider = QString() );

    /**
     * Get authentication method edit widget via its key
     * @param authMethodKey Authentication method key
     * @param parent Parent widget
     */
    QWidget *authMethodEditWidget( const QString &authMethodKey, QWidget *parent );

    /**
     * Get supported authentication method expansion(s), e.g. NetworkRequest | DataSourceURI, as flags
     * @param authcfg
     */
    QgsAuthMethod::Expansions supportedAuthMethodExpansions( const QString &authcfg );

    /** Get a unique generated 7-character string to assign to as config id */
    const QString uniqueConfigId() const;

    /**
     * Verify if provided authentication id is unique
     * @param id Id to check
     */
    bool configIdUnique( const QString &id ) const;

    /**
     * Return whether a string includes an authcfg ID token
     * @param txt String to check
     */
    bool hasConfigId( const QString &txt ) const;

    /** Return regular expression for authcfg=.{7} key/value token for authentication ids */
    QString configIdRegex() const { return smAuthCfgRegex;}

    /** Get list of authentication ids from database */
    QStringList configIds() const;

    /**
     * Store an authentication config in the database
     * @param mconfig Associated authentication config id
     * @return Whether operation succeeded
     */
    bool storeAuthenticationConfig( QgsAuthMethodConfig &mconfig );

    /**
     * Update an authentication config in the database
     * @param config Associated authentication config id
     * @return Whether operation succeeded
     */
    bool updateAuthenticationConfig( const QgsAuthMethodConfig& config );

    /**
     * Load an authentication config from the database into subclass
     * @param authcfg Associated authentication config id
     * @param mconfig Subclassed config to load into
     * @param full Whether to decrypt and populate all sensitive data in subclass
     * @return Whether operation succeeded
     */
    bool loadAuthenticationConfig( const QString& authcfg, QgsAuthMethodConfig &mconfig, bool full = false );

    /**
     * Remove an authentication config in the database
     * @param authcfg Associated authentication config id
     * @return Whether operation succeeded
     */
    bool removeAuthenticationConfig( const QString& authcfg );

    /**
     * Clear all authentication configs from table in database and from provider caches
     * @return Whether operation succeeded
     */
    bool removeAllAuthenticationConfigs();

    /**
     * Close connection to current authentication database and back it up
     * @return Path to backup
     */
    bool backupAuthenticationDatabase( QString *backuppath = nullptr );

    /**
     * Erase all rows from all tables in authentication database
     * @param backup Whether to backup of current database
     * @param backuppath Where the backup is locate
     * @return Whether operation succeeded
     */
    bool eraseAuthenticationDatabase( bool backup, QString *backuppath = nullptr );


    ////////////////// Auth Method calls ///////////////////////

    /**
     * Provider call to update a QNetworkRequest with an authentication config
     * @param request The QNetworkRequest
     * @param authcfg Associated authentication config id
     * @param dataprovider Provider key filter, offering logic branching in authentication method
     * @return Whether operation succeeded
     */
    bool updateNetworkRequest( QNetworkRequest &request, const QString& authcfg,
                               const QString &dataprovider = QString() );

    /**
     * Provider call to update a QNetworkReply with an authentication config (used to skip known SSL errors, etc.)
     * @param reply The QNetworkReply
     * @param authcfg Associated authentication config id
     * @param dataprovider Provider key filter, offering logic branching in authentication method
     * @return Whether operation succeeded
     */
    bool updateNetworkReply( QNetworkReply *reply, const QString& authcfg,
                             const QString &dataprovider = QString() );

    /**
     * Provider call to update a QgsDataSourceURI with an authentication config
     * @param connectionItems The connection items, e.g. username=myname, of QgsDataSourceURI
     * @param authcfg Associated authentication config id
     * @param dataprovider Provider key filter, offering logic branching in authentication method
     * @return Whether operation succeeded
     */
    bool updateDataSourceUriItems( QStringList &connectionItems, const QString& authcfg,
                                   const QString &dataprovider = QString() );

    ////////////////// Generic settings ///////////////////////

    /** Store an authentication setting (stored as string via QVariant( value ).toString() ) */
    bool storeAuthSetting( const QString& key, const QVariant& value, bool encrypt = false );

    /** Get an authentication setting (retrieved as string and returned as QVariant( QString )) */
    QVariant getAuthSetting( const QString& key, const QVariant& defaultValue = QVariant(), bool decrypt = false );

    /** Check if an authentication setting exists */
    bool existsAuthSetting( const QString& key );

    /** Remove an authentication setting */
    bool removeAuthSetting( const QString& key );

#ifndef QT_NO_OPENSSL
    ////////////////// Certificate calls ///////////////////////

    /** Initialize various SSL authentication caches */
    bool initSslCaches();

    /** Store a certificate identity */
    bool storeCertIdentity( const QSslCertificate& cert, const QSslKey& key );

    /** Get a certificate identity by id (sha hash) */
    const QSslCertificate getCertIdentity( const QString& id );

    /** Get a certificate identity bundle by id (sha hash).
     * @note not available in Python bindings
     */
    const QPair<QSslCertificate, QSslKey> getCertIdentityBundle( const QString& id );

    /** Get a certificate identity bundle by id (sha hash) returned as PEM text */
    const QStringList getCertIdentityBundleToPem( const QString& id );

    /** Get certificate identities */
    const QList<QSslCertificate> getCertIdentities();

    /** Get list of certificate identity ids from database */
    QStringList getCertIdentityIds() const;

    /** Check if a certificate identity exists */
    bool existsCertIdentity( const QString& id );

    /** Remove a certificate identity */
    bool removeCertIdentity( const QString& id );


    /** Store an SSL certificate custom config */
    bool storeSslCertCustomConfig( const QgsAuthConfigSslServer& config );

    /** Get an SSL certificate custom config by id (sha hash) and host:port */
    const QgsAuthConfigSslServer getSslCertCustomConfig( const QString& id, const QString &hostport );

    /** Get an SSL certificate custom config by host:port */
    const QgsAuthConfigSslServer getSslCertCustomConfigByHost( const QString& hostport );

    /** Get SSL certificate custom configs */
    const QList<QgsAuthConfigSslServer> getSslCertCustomConfigs();

    /** Check if SSL certificate custom config exists */
    bool existsSslCertCustomConfig( const QString& id, const QString &hostport );

    /** Remove an SSL certificate custom config */
    bool removeSslCertCustomConfig( const QString& id, const QString &hostport );

    /** Get ignored SSL error cache, keyed with cert/connection's sha:host:port.
     * @note not available in Python bindings
     */
    QHash<QString, QSet<QSslError::SslError> > getIgnoredSslErrorCache() { return mIgnoredSslErrorsCache; }

    /** Utility function to dump the cache for debug purposes */
    void dumpIgnoredSslErrorsCache_();

    /** Update ignored SSL error cache with possible ignored SSL errors, using server config */
    bool updateIgnoredSslErrorsCacheFromConfig( const QgsAuthConfigSslServer &config );

    /** Update ignored SSL error cache with possible ignored SSL errors, using sha:host:port key */
    bool updateIgnoredSslErrorsCache( const QString &shahostport, const QList<QSslError> &errors );

    /** Rebuild ignoredSSL error cache */
    bool rebuildIgnoredSslErrorCache();


    /** Store multiple certificate authorities */
    bool storeCertAuthorities( const QList<QSslCertificate>& certs );

    /** Store a certificate authority */
    bool storeCertAuthority( const QSslCertificate& cert );

    /** Get a certificate authority by id (sha hash) */
    const QSslCertificate getCertAuthority( const QString& id );

    /** Check if a certificate authority exists */
    bool existsCertAuthority( const QSslCertificate& cert );

    /** Remove a certificate authority */
    bool removeCertAuthority( const QSslCertificate& cert );

    /** Get root system certificate authorities */
    const QList<QSslCertificate> getSystemRootCAs();

    /** Get extra file-based certificate authorities */
    const QList<QSslCertificate> getExtraFileCAs();

    /** Get database-stored certificate authorities */
    const QList<QSslCertificate> getDatabaseCAs();

    /** Get sha1-mapped database-stored certificate authorities */
    const QMap<QString, QSslCertificate> getMappedDatabaseCAs();

    /** Get all CA certs mapped to their sha1 from cache.
     * @note not available in Python bindings
     */
    const QMap<QString, QPair<QgsAuthCertUtils::CaCertSource , QSslCertificate> > getCaCertsCache()
    {
      return mCaCertsCache;
    }

    /** Rebuild certificate authority cache */
    bool rebuildCaCertsCache();

    /** Store user trust value for a certificate */
    bool storeCertTrustPolicy( const QSslCertificate& cert, QgsAuthCertUtils::CertTrustPolicy policy );

    /** Get a whether certificate is trusted by user
        @return DefaultTrust if certificate sha not in trust table, i.e. follows default trust policy
     */
    QgsAuthCertUtils::CertTrustPolicy getCertTrustPolicy( const QSslCertificate& cert );

    /** Remove a group certificate authorities */
    bool removeCertTrustPolicies( const QList<QSslCertificate>& certs );

    /** Remove a certificate authority */
    bool removeCertTrustPolicy( const QSslCertificate& cert );

    /** Get trust policy for a particular certificate */
    QgsAuthCertUtils::CertTrustPolicy getCertificateTrustPolicy( const QSslCertificate& cert );

    /** Set the default certificate trust policy perferred by user */
    bool setDefaultCertTrustPolicy( QgsAuthCertUtils::CertTrustPolicy policy );

    /** Get the default certificate trust policy perferred by user */
    QgsAuthCertUtils::CertTrustPolicy defaultCertTrustPolicy();

    /** Get cache of certificate sha1s, per trust policy */
    const QMap<QgsAuthCertUtils::CertTrustPolicy, QStringList > getCertTrustCache() { return mCertTrustCache; }

    /** Rebuild certificate authority cache */
    bool rebuildCertTrustCache();

    /** Get list of all trusted CA certificates */
    const QList<QSslCertificate> getTrustedCaCerts( bool includeinvalid = false );

    /** Get list of all untrusted CA certificates */
    const QList<QSslCertificate> getUntrustedCaCerts( QList<QSslCertificate> trustedCAs = QList<QSslCertificate>() );

    /** Rebuild trusted certificate authorities cache */
    bool rebuildTrustedCaCertsCache();

    /** Get cache of trusted certificate authorities, ready for network connections */
    const QList<QSslCertificate> getTrustedCaCertsCache() { return mTrustedCaCertsCache; }

    /** Get concatenated string of all trusted CA certificates */
    const QByteArray getTrustedCaCertsPemText();

#endif

    /** Return pointer to mutex */
    QMutex *mutex() { return mMutex; }

  signals:
    /**
     * Custom logging signal to relay to console output and QgsMessageLog
     * @see QgsMessageLog
     * @param message Message to send
     * @param tag Associated tag (title)
     * @param level Message log level
     */
    void messageOut( const QString& message, const QString& tag = smAuthManTag, QgsAuthManager::MessageLevel level = INFO ) const;

    /**
     * Emitted when a password has been verify (or not)
     * @param verified The state of password's verification
     */
    void masterPasswordVerified( bool verified ) const;

    /** Emitted when a user has indicated they may want to erase the authentication db. */
    void authDatabaseEraseRequested() const;

    /** Emitted when the authentication db is significantly changed, e.g. large record removal, erased, etc. */
    void authDatabaseChanged() const;

  public slots:
    /** Clear all authentication configs from authentication method caches */
    void clearAllCachedConfigs();

    /** Clear an authentication config from its associated authentication method cache */
    void clearCachedConfig( const QString& authcfg );

  private slots:
    void writeToConsole( const QString& message, const QString& tag = QString(), QgsAuthManager::MessageLevel level = INFO );

    /** This slot emits the authDatabaseEraseRequested signal, instead of attempting
     * the erase. It relies upon a slot connected to the signal in calling application
     * (the one that initiated the erase) to initiate the erase, when it is ready.
     * Upon activation, a receiving slot should get confimation from the user, then
     * IMMEDIATELY call setScheduledAuthDbErase( false ) to stop the scheduling timer.
     * If receiving slot is NOT ready to initiate the erase, e.g. project is still loading,
     * it can skip the confirmation and request another signal emit from the scheduling timer.
     */
    void tryToStartDbErase();

  protected:
    explicit QgsAuthManager();

  private:

    bool createConfigTables();

    bool createCertTables();

    bool masterPasswordInput();

    bool masterPasswordRowsInDb( int *rows ) const;

    bool masterPasswordCheckAgainstDb( const QString &compare = QString::null ) const;

    bool masterPasswordStoreInDb() const;

    bool masterPasswordClearDb();

    const QString masterPasswordCiv() const;

    bool verifyPasswordCanDecryptConfigs() const;

    bool reencryptAllAuthenticationConfigs( const QString& prevpass, const QString& prevciv );

    bool reencryptAuthenticationConfig( const QString& authcfg, const QString& prevpass, const QString& prevciv );

    bool reencryptAllAuthenticationSettings( const QString& prevpass, const QString& prevciv );

    bool reencryptAllAuthenticationIdentities( const QString& prevpass, const QString& prevciv );

    bool reencryptAuthenticationIdentity( const QString& identid, const QString& prevpass, const QString& prevciv );

    bool authDbOpen() const;

    bool authDbQuery( QSqlQuery *query ) const;

    bool authDbStartTransaction() const;

    bool authDbCommit() const;

    bool authDbTransactionQuery( QSqlQuery *query ) const;

#ifndef QT_NO_OPENSSL
    void insertCaCertInCache( QgsAuthCertUtils::CaCertSource source, const QList<QSslCertificate> &certs );
#endif

    const QString authDbPassTable() const { return smAuthPassTable; }

    const QString authDbSettingsTable() const { return smAuthSettingsTable; }

    const QString authDbIdentitiesTable() const { return smAuthIdentitiesTable; }

    const QString authDbAuthoritiesTable() const { return smAuthAuthoritiesTable; }

    const QString authDbTrustTable() const { return smAuthTrustTable; }

    static QgsAuthManager* smInstance;
    static const QString smAuthConfigTable;
    static const QString smAuthPassTable;
    static const QString smAuthSettingsTable;
    static const QString smAuthIdentitiesTable;
    static const QString smAuthServersTable;
    static const QString smAuthAuthoritiesTable;
    static const QString smAuthTrustTable;
    static const QString smAuthManTag;
    static const QString smAuthCfgRegex;

    bool mAuthInit;
    QString mAuthDbPath;

    QCA::Initializer * mQcaInitializer;

    QHash<QString, QString> mConfigAuthMethods;
    QHash<QString, QgsAuthMethod*> mAuthMethods;

    QString mMasterPass;
    int mPassTries;
    bool mAuthDisabled;
    QString mAuthDisabledMessage;
    QTimer *mScheduledDbEraseTimer;
    bool mScheduledDbErase;
    int mScheduledDbEraseRequestWait; // in seconds
    bool mScheduledDbEraseRequestEmitted;
    int mScheduledDbEraseRequestCount;
    QMutex *mMutex;

#ifndef QT_NO_OPENSSL
    // mapping of sha1 digest and cert source and cert
    // appending removes duplicates
    QMap<QString, QPair<QgsAuthCertUtils::CaCertSource , QSslCertificate> > mCaCertsCache;
    // list of sha1 digests per policy
    QMap<QgsAuthCertUtils::CertTrustPolicy, QStringList > mCertTrustCache;
    // cache of certs ready to be utilized in network connections
    QList<QSslCertificate> mTrustedCaCertsCache;
    // cache of SSL errors to be ignored in network connections, per sha-hostport
    QHash<QString, QSet<QSslError::SslError> > mIgnoredSslErrorsCache;
#endif
};

#endif // QGSAUTHMANAGER_H
