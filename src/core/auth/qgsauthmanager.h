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

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QObject>
#include <QRecursiveMutex>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>

#ifndef QT_NO_SSL
#include <QSslCertificate>
#include <QSslKey>
#include <QtCrypto>
#include "qgsauthcertutils.h"
#endif

#include "qgsauthconfig.h"
#include "qgsauthmethod.h"

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <qt6keychain/keychain.h>
#else
#include <qt5keychain/keychain.h>
#endif

#ifndef SIP_RUN
namespace QCA
{
  class Initializer;
}
#endif
class QgsAuthMethod;
class QgsAuthMethodEdit;
class QgsAuthProvider;
class QgsAuthMethodMetadata;
class QTimer;


/**
 * \ingroup core
 * \brief Singleton offering an interface to manage the authentication configuration database
 * and to utilize configurations through various authentication method plugins
 *
 * QgsAuthManager should not usually be directly created, but rather accessed through
 * QgsApplication::authManager().
 */
class CORE_EXPORT QgsAuthManager : public QObject
{
    Q_OBJECT

  public:

    //! Message log level (mirrors that of QgsMessageLog, so it can also output there)
    enum MessageLevel
    {
      INFO = 0,
      WARNING = 1,
      CRITICAL = 2
    };
    Q_ENUM( MessageLevel )

    /**
     * \brief init initialize QCA, prioritize qca-ossl plugin and optionally set up the authentication database
     * \param pluginPath the plugin path
     * \param authDatabasePath the authentication DB path
     * \return TRUE on success
     * \see QgsApplication::pluginPath
     * \see QgsApplication::qgisAuthDatabaseFilePath
     * \deprecated Since QGIS 3.36, use setup() instead.
     */
    Q_DECL_DEPRECATED bool init( const QString &pluginPath = QString(),  const QString &authDatabasePath = QString() ) SIP_DEPRECATED;

    /**
     * Sets up the authentication manager configuration.
     *
     * This method does not initialize the authentication framework, instead that is deferred
     * to lazy-initialize when required.
     *
     * \param pluginPath the plugin path
     * \param authDatabasePath the authentication DB path
     */
    void setup( const QString &pluginPath = QString(),  const QString &authDatabasePath = QString() );

    ~QgsAuthManager() override;

    //! Sets up the application instance of the authentication database connection
    QSqlDatabase authDatabaseConnection() const;

    //! Name of the authentication database table that stores configs
    const QString authDatabaseConfigTable() const { return AUTH_CONFIG_TABLE; }

    //! Name of the authentication database table that stores server exceptions/configs
    const QString authDatabaseServersTable() const { return AUTH_SERVERS_TABLE; }


    //! Whether QCA has the qca-ossl plugin, which a base run-time requirement
    bool isDisabled() const;

    //! Standard message for when QCA's qca-ossl plugin is missing and system is disabled
    const QString disabledMessage() const;

    /**
     * The standard authentication database file in ~/.qgis3/ or defined location
     * \see QgsApplication::qgisAuthDatabaseFilePath
     */
    const QString authenticationDatabasePath() const { return mAuthDbPath; }

    /**
     * Main call to initially set or continually check master password is set
     * \note If it is not set, the user is asked for its input
     * \param verify Whether password's hash was saved in authentication database
     */
    bool setMasterPassword( bool verify = false );

    /**
     * Overloaded call to reset master password or set it initially without user interaction
     * \note Only use this in trusted reset functions, unit tests or user/app setup scripts!
     * \param pass Password to use
     * \param verify Whether password's hash was saved in authentication database
     */
    bool setMasterPassword( const QString &pass, bool verify = false );

    /**
     * Verify the supplied master password against any existing hash in authentication database
     * \note Do not emit verification signals when only comparing
     * \param compare Password to compare against
     */
    bool verifyMasterPassword( const QString &compare = QString() );

    //! Whether master password has be input and verified, i.e. authentication database is accessible
    bool masterPasswordIsSet() const;

    //! Verify a password hash existing in authentication database
    bool masterPasswordHashInDatabase() const;

    /**
     * Clear supplied master password
     * \note This will not necessarily clear authenticated connections cached in network connection managers
     */
    void clearMasterPassword() { mMasterPass = QString(); }

    /**
     * Check whether supplied password is the same as the one already set
     * \param pass Password to verify
     */
    bool masterPasswordSame( const QString &pass ) const;

    /**
     * Reset the master password to a new one, then re-encrypt all previous
     * configs in a new database file, optionally backup current database
     * \param newpass New master password to replace existing
     * \param oldpass Current master password to replace existing
     * \param keepbackup Whether to keep the generated backup of current database
     * \param backuppath Where the backup is located, if kept
     */
    bool resetMasterPassword( const QString &newpass, const QString &oldpass, bool keepbackup, QString *backuppath SIP_INOUT = nullptr );

    /**
     * Whether there is a scheduled opitonal erase of authentication database.
     * \note not available in Python bindings
     */
    bool scheduledAuthDatabaseErase() { return mScheduledDbErase; } SIP_SKIP

    /**
     * Schedule an optional erase of authentication database, starting when mutex is lockable.
     * \note When an erase is scheduled, any attempt to set the master password,
     * e.g. password input dialog, is effectively canceled.
     * For example: In a GUI app, this keeps excess password input dialogs from popping
     * up when a user has initiated an erase, from a password input dialog, because
     * they forgot their password.
     * The created schedule timer will emit a request to gain access to the user,
     * through the given application, to prompt the erase operation (e.g. via a dialog);
     * if no access to user interaction occurs within 90 seconds, it cancels the schedule.
     * \note not available in Python bindings
     */
    void setScheduledAuthDatabaseErase( bool scheduleErase ) SIP_SKIP;

    /**
     * Re-emit a signal to schedule an optional erase of authentication database.
     * \note This can be called from the slot connected to a previously emitted scheduling signal,
     * so that the slot can ask for another emit later, if the slot noticies the current GUI
     * processing state is not ready for interacting with the user, e.g. project is still loading
     * \param emitted Setting to FALSE will cause signal to be emitted by the schedule timer.
     * Setting to TRUE will stop any emitting, but will not stop the schedule timer.
     */
    void setScheduledAuthDatabaseEraseRequestEmitted( bool emitted ) { mScheduledDbEraseRequestEmitted = emitted; }

    //! Simple text tag describing authentication system for message logs
    QString authManTag() const { return AUTH_MAN_TAG; }

    //! Instantiate and register existing C++ core authentication methods from plugins
    bool registerCoreAuthMethods();

    //! Gets mapping of authentication config ids and their base configs (not decrypted data)
    QgsAuthMethodConfigsMap availableAuthMethodConfigs( const QString &dataprovider = QString() );

    //! Sync the confg/authentication method cache with what is in database
    void updateConfigAuthMethods();

    /**
     * Gets authentication method from the config/provider cache
     * \param authcfg Authentication config id
     */
    QgsAuthMethod *configAuthMethod( const QString &authcfg );

    /**
     * Gets key of authentication method associated with config ID
     * \param authcfg
     */
    QString configAuthMethodKey( const QString &authcfg ) const;

    /**
     * Gets keys of supported authentication methods
     */
    QStringList authMethodsKeys( const QString &dataprovider = QString() );

    /**
     * Gets authentication method from the config/provider cache via its key
     * \param authMethodKey Authentication method key
     */
    QgsAuthMethod *authMethod( const QString &authMethodKey );

    /**
     * Gets authentication method metadata via its key
     * \param authMethodKey Authentication method key
     * \since QGIS 3.22
     */
    const QgsAuthMethodMetadata *authMethodMetadata( const QString &authMethodKey ) SIP_SKIP;

    /**
     * Gets available authentication methods mapped to their key
     * \param dataprovider Provider key filter, returning only methods that support a particular provider
     * \note not available in Python bindings
     */
    QgsAuthMethodsMap authMethodsMap( const QString &dataprovider = QString() ) SIP_SKIP;

#ifdef HAVE_GUI
    SIP_IF_FEATURE( HAVE_GUI )

    /**
     * Gets authentication method edit widget via its key
     * \param authMethodKey Authentication method key
     * \param parent Parent widget
     */
    QWidget *authMethodEditWidget( const QString &authMethodKey, QWidget *parent );
    SIP_END
#endif

    /**
     * Gets supported authentication method expansion(s), e.g. NetworkRequest | DataSourceURI, as flags
     * \param authcfg
     */
    QgsAuthMethod::Expansions supportedAuthMethodExpansions( const QString &authcfg );

    //! Gets a unique generated 7-character string to assign to as config id
    const QString uniqueConfigId() const;

    /**
     * Verify if provided authentication id is unique
     * \param id Id to check
     */
    bool configIdUnique( const QString &id ) const;

    /**
     * Returns whether a string includes an authcfg ID token
     * \param txt String to check
     */
    static bool hasConfigId( const QString &txt );

    //! Returns the regular expression for authcfg=.{7} key/value token for authentication ids
    QString configIdRegex() const { return AUTH_CFG_REGEX;}

    //! Gets list of authentication ids from database
    QStringList configIds() const;

    /**
     * Store an authentication config in the database
     * \param mconfig Associated authentication config id
     * \param overwrite If set to TRUE, pre-existing authentication configurations will be overwritten
     * \returns Whether operation succeeded
     */
    bool storeAuthenticationConfig( QgsAuthMethodConfig &mconfig SIP_INOUT, bool overwrite = false );

    /**
     * Update an authentication config in the database
     * \param config Associated authentication config id
     * \returns Whether operation succeeded
     */
    bool updateAuthenticationConfig( const QgsAuthMethodConfig &config );

    /**
     * Load an authentication config from the database into subclass
     * \param authcfg Associated authentication config id
     * \param mconfig Subclassed config to load into
     * \param full Whether to decrypt and populate all sensitive data in subclass
     * \returns Whether operation succeeded
     */
    bool loadAuthenticationConfig( const QString &authcfg, QgsAuthMethodConfig &mconfig SIP_INOUT, bool full = false );

    /**
     * Remove an authentication config in the database
     * \param authcfg Associated authentication config id
     * \returns Whether operation succeeded
     */
    bool removeAuthenticationConfig( const QString &authcfg );

    /**
     * Export authentication configurations to an XML file
     * \param filename The file path to save the XML content to
     * \param authcfgs The list of configuration IDs to export
     * \param password A password string to encrypt the XML content
     * \since QGIS 3.20
     */
    bool exportAuthenticationConfigsToXml( const QString &filename, const QStringList &authcfgs, const QString &password = QString() );

    /**
     * Import authentication configurations from an XML file
     * \param filename The file path from which the XML content will be read
     * \param password A password string to decrypt the XML content
     * \param overwrite If set to TRUE, pre-existing authentication configurations will be overwritten
     * \since QGIS 3.20
     */
    bool importAuthenticationConfigsFromXml( const QString &filename, const QString &password = QString(), bool overwrite = false );

    /**
     * Clear all authentication configs from table in database and from provider caches
     * \returns Whether operation succeeded
     */
    bool removeAllAuthenticationConfigs();

    /**
     * Close connection to current authentication database and back it up
     * \returns Path to backup
     */
    bool backupAuthenticationDatabase( QString *backuppath SIP_INOUT = nullptr );

    /**
     * Erase all rows from all tables in authentication database
     * \param backup Whether to backup of current database
     * \param backuppath Where the backup is locate
     * \returns Whether operation succeeded
     */
    bool eraseAuthenticationDatabase( bool backup, QString *backuppath SIP_INOUT = nullptr );


    ////////////////// Auth Method calls ///////////////////////

    /**
     * Provider call to update a QNetworkRequest with an authentication config
     * \param request The QNetworkRequest
     * \param authcfg Associated authentication config id
     * \param dataprovider Provider key filter, offering logic branching in authentication method
     * \returns Whether operation succeeded
     */
    bool updateNetworkRequest( QNetworkRequest &request SIP_INOUT, const QString &authcfg,
                               const QString &dataprovider = QString() );

    /**
     * Provider call to update a QNetworkReply with an authentication config (used to skip known SSL errors, etc.)
     * \param reply The QNetworkReply
     * \param authcfg Associated authentication config id
     * \param dataprovider Provider key filter, offering logic branching in authentication method
     * \returns Whether operation succeeded
     */
    bool updateNetworkReply( QNetworkReply *reply, const QString &authcfg,
                             const QString &dataprovider = QString() );

    /**
     * Provider call to update a QgsDataSourceUri with an authentication config
     * \param connectionItems The connection items, e.g. username=myname, of QgsDataSourceUri
     * \param authcfg Associated authentication config id
     * \param dataprovider Provider key filter, offering logic branching in authentication method
     * \returns Whether operation succeeded
     */
    bool updateDataSourceUriItems( QStringList &connectionItems SIP_INOUT, const QString &authcfg,
                                   const QString &dataprovider = QString() );

    /**
     * Provider call to update a QNetworkProxy with an authentication config
     * \param proxy the QNetworkProxy
     * \param authcfg Associated authentication config id
     * \param dataprovider Provider key filter, offering logic branching in authentication method
     * \returns Whether operation succeeded
     */
    bool updateNetworkProxy( QNetworkProxy &proxy SIP_INOUT, const QString &authcfg,
                             const QString &dataprovider = QString() );

    ////////////////// Generic settings ///////////////////////

    //! Store an authentication setting (stored as string via QVariant( value ).toString() )
    bool storeAuthSetting( const QString &key, const QVariant &value, bool encrypt = false );

    /**
     * \brief authSetting get an authentication setting (retrieved as string and returned as QVariant( QString ))
     * \param key setting key
     * \param defaultValue
     * \param decrypt if the value needs decrypted
     * \return QVariant( QString ) authentication setting
     */
    QVariant authSetting( const QString &key, const QVariant &defaultValue = QVariant(), bool decrypt = false );

    //! Check if an authentication setting exists
    bool existsAuthSetting( const QString &key );

    //! Remove an authentication setting
    bool removeAuthSetting( const QString &key );

#ifndef QT_NO_SSL
    ////////////////// Certificate calls ///////////////////////

    //! Initialize various SSL authentication caches
    bool initSslCaches();

    //! Store a certificate identity
    bool storeCertIdentity( const QSslCertificate &cert, const QSslKey &key );

    /**
     * \brief certIdentity get a certificate identity by \a id (sha hash)
     * \param id sha hash of the cert
     * \return the certificate
     */
    const QSslCertificate certIdentity( const QString &id );

    /**
     * Gets a certificate identity bundle by \a id (sha hash).
     * \param id sha shash
     * \return a pair with the certificate and its SSL key
     * \note not available in Python bindings
     */
    const QPair<QSslCertificate, QSslKey> certIdentityBundle( const QString &id ) SIP_SKIP;

    /**
     * \brief certIdentityBundleToPem get a certificate identity bundle by \a id (sha hash) returned as PEM text
     * \param id sha hash
     * \return a list of strings
     */
    const QStringList certIdentityBundleToPem( const QString &id );

    /**
     * \brief certIdentities get certificate identities
     * \return list of certificates
     */
    const QList<QSslCertificate> certIdentities();

    //!

    /**
     * \brief certIdentityIds get list of certificate identity ids from database
     * \return list of certificate ids
     */
    QStringList certIdentityIds() const;

    //! Check if a certificate identity exists
    bool existsCertIdentity( const QString &id );

    //! Remove a certificate identity
    bool removeCertIdentity( const QString &id );


    //! Store an SSL certificate custom config
    bool storeSslCertCustomConfig( const QgsAuthConfigSslServer &config );

    /**
     * \brief sslCertCustomConfig get an SSL certificate custom config by \a id (sha hash) and \a hostport (host:port)
     * \param id sha hash
     * \param hostport string host:port
     * \return a SSL certificate custom config
     */
    const QgsAuthConfigSslServer sslCertCustomConfig( const QString &id, const QString &hostport );

    /**
     * \brief sslCertCustomConfigByHost get an SSL certificate custom config by \a hostport (host:port)
     * \param hostport host:port
     * \return a SSL certificate custom config
     */
    const QgsAuthConfigSslServer sslCertCustomConfigByHost( const QString &hostport );

    /**
     * \brief sslCertCustomConfigs get SSL certificate custom configs
     * \return list of SSL certificate custom config
     */
    const QList<QgsAuthConfigSslServer> sslCertCustomConfigs();

    //! Check if SSL certificate custom config exists
    bool existsSslCertCustomConfig( const QString &id, const QString &hostport );

    //! Remove an SSL certificate custom config
    bool removeSslCertCustomConfig( const QString &id, const QString &hostport );

    /**
     * \brief ignoredSslErrorCache Get ignored SSL error cache, keyed with cert/connection's sha:host:port.
     * \return hash keyed with cert/connection's sha:host:port.
     * \note not available in Python bindings
     */
    QHash<QString, QSet<QSslError::SslError> > ignoredSslErrorCache() { return mIgnoredSslErrorsCache; } SIP_SKIP

    //! Utility function to dump the cache for debug purposes
    void dumpIgnoredSslErrorsCache_();

    //! Update ignored SSL error cache with possible ignored SSL errors, using server config
    bool updateIgnoredSslErrorsCacheFromConfig( const QgsAuthConfigSslServer &config );

    //! Update ignored SSL error cache with possible ignored SSL errors, using sha:host:port key
    bool updateIgnoredSslErrorsCache( const QString &shahostport, const QList<QSslError> &errors );

    //! Rebuild ignoredSSL error cache
    bool rebuildIgnoredSslErrorCache();


    //! Store multiple certificate authorities
    bool storeCertAuthorities( const QList<QSslCertificate> &certs );

    //! Store a certificate authority
    bool storeCertAuthority( const QSslCertificate &cert );

    //! Gets a certificate authority by id (sha hash)

    /**
     * \brief certAuthority get a certificate authority by \a id (sha hash)
     * \param id sha hash
     * \return a certificate
     */
    const QSslCertificate certAuthority( const QString &id );

    //! Check if a certificate authority exists
    bool existsCertAuthority( const QSslCertificate &cert );

    //! Remove a certificate authority
    bool removeCertAuthority( const QSslCertificate &cert );

    /**
     * \brief systemRootCAs get root system certificate authorities
     * \return list of certificate authorities
     */
    static const QList<QSslCertificate> systemRootCAs();

    /**
     * \brief extraFileCAs extra file-based certificate authorities
     * \return list of certificate authorities
     */
    const QList<QSslCertificate> extraFileCAs();

    /**
     * \brief databaseCAs get database-stored certificate authorities
     * \return list of certificate authorities
     */
    const QList<QSslCertificate> databaseCAs();

    /**
     * \brief mappedDatabaseCAs get sha1-mapped database-stored certificate authorities
     * \return sha1-mapped certificate authorities
     */
    const QMap<QString, QSslCertificate> mappedDatabaseCAs();

    /**
     * \brief caCertsCache get all CA certs mapped to their sha1 from cache.
     * \return map of sha1 <source, certificates>
     * \note not available in Python bindings
     */
    const QMap<QString, QPair<QgsAuthCertUtils::CaCertSource, QSslCertificate> > caCertsCache() SIP_SKIP
    {
      return mCaCertsCache;
    }

    //! Rebuild certificate authority cache
    bool rebuildCaCertsCache();

    //! Store user trust value for a certificate
    bool storeCertTrustPolicy( const QSslCertificate &cert, QgsAuthCertUtils::CertTrustPolicy policy );

    /**
     * \brief certTrustPolicy get whether certificate \a cert is trusted by user
     * \param cert
     * \return DefaultTrust if certificate sha not in trust table, i.e. follows default trust policy
     */
    QgsAuthCertUtils::CertTrustPolicy certTrustPolicy( const QSslCertificate &cert );

    //! Remove a group certificate authorities
    bool removeCertTrustPolicies( const QList<QSslCertificate> &certs );

    //! Remove a certificate authority
    bool removeCertTrustPolicy( const QSslCertificate &cert );

    /**
     * \brief certificateTrustPolicy get trust policy for a particular certificate \a cert
     * \param cert
     * \return DefaultTrust if certificate sha not in trust table, i.e. follows default trust policy
     */
    QgsAuthCertUtils::CertTrustPolicy certificateTrustPolicy( const QSslCertificate &cert );

    //! Sets the default certificate trust policy preferred by user
    bool setDefaultCertTrustPolicy( QgsAuthCertUtils::CertTrustPolicy policy );

    //! Gets the default certificate trust policy preferred by user
    QgsAuthCertUtils::CertTrustPolicy defaultCertTrustPolicy();

    /**
     * \brief certTrustCache get cache of certificate sha1s, per trust policy
     * \return trust-policy-mapped certificate sha1s
     */
    const QMap<QgsAuthCertUtils::CertTrustPolicy, QStringList > certTrustCache() { return mCertTrustCache; }

    //! Rebuild certificate authority cache
    bool rebuildCertTrustCache();

    /**
     * \brief trustedCaCerts get list of all trusted CA certificates
     * \param includeinvalid whether invalid certs needs to be returned
     * \return list of certificates
     */
    const QList<QSslCertificate> trustedCaCerts( bool includeinvalid = false );

    /**
     * \brief untrustedCaCerts get list of untrusted certificate authorities
     * \return list of certificates
     */
    const QList<QSslCertificate> untrustedCaCerts( QList<QSslCertificate> trustedCAs = QList<QSslCertificate>() );

    //! Rebuild trusted certificate authorities cache
    bool rebuildTrustedCaCertsCache();

    /**
     * \brief trustedCaCertsCache cache of trusted certificate authorities, ready for network connections
     * \return list of certificates
     */
    const QList<QSslCertificate> trustedCaCertsCache() { return mTrustedCaCertsCache; }

    /**
     * \brief trustedCaCertsPemText get concatenated string of all trusted CA certificates
     * \return bye array with all PEM encoded trusted CAs
     */
    const QByteArray trustedCaCertsPemText();

#endif

    /**
     * Error message getter
     * \note not available in Python bindings
     */
    const QString passwordHelperErrorMessage() { return mPasswordHelperErrorMessage; } SIP_SKIP

    /**
     * Delete master password from wallet
     * \note not available in Python bindings
     */
    bool passwordHelperDelete() SIP_SKIP;

    /**
     * Password helper enabled getter
     * \note Available in Python bindings since QGIS 3.8.0
     */
    static bool passwordHelperEnabled();

    /**
     * Password helper enabled setter
     * \note Available in Python bindings since QGIS 3.8.0
     */
    void setPasswordHelperEnabled( bool enabled );

    /**
     * Password helper logging enabled getter
     * \note not available in Python bindings
     */
    static bool passwordHelperLoggingEnabled() SIP_SKIP;

    /**
     * Password helper logging enabled setter
     * \note not available in Python bindings
     */
    static void setPasswordHelperLoggingEnabled( bool enabled ) SIP_SKIP;

    /**
     * Store the password manager into the wallet
     * \note Available in Python bindings since QGIS 3.8.0
     */
    bool passwordHelperSync();

    //! The display name of the password helper (platform dependent)
    static const QString AUTH_PASSWORD_HELPER_DISPLAY_NAME;

    //! The display name of the Authentication Manager
    static const QString AUTH_MAN_TAG;

  signals:

    /**
     * Signals emitted on password helper failure,
     * mainly used in the tests to exit main application loop
     */
    void passwordHelperFailure();

    /**
     * Signals emitted on password helper success,
     * mainly used in the tests to exit main application loop
     */
    void passwordHelperSuccess();

    /**
     * Custom logging signal to relay to console output and QgsMessageLog
     * \param message Message to send
     * \param tag Associated tag (title)
     * \param level Message log level
     * \see QgsMessageLog
     */
    void messageOut( const QString &message, const QString &tag = QgsAuthManager::AUTH_MAN_TAG, QgsAuthManager::MessageLevel level = QgsAuthManager::INFO ) const;

    /**
     * Custom logging signal to inform the user about master password <-> password manager interactions
     * \param message Message to send
     * \param tag Associated tag (title)
     * \param level Message log level
     * \see QgsMessageLog
     */
    void passwordHelperMessageOut( const QString &message, const QString &tag = QgsAuthManager::AUTH_MAN_TAG, QgsAuthManager::MessageLevel level = QgsAuthManager::INFO );


    /**
     * Emitted when a password has been verify (or not)
     * \param verified The state of password's verification
     */
    void masterPasswordVerified( bool verified );

    //! Emitted when a user has indicated they may want to erase the authentication db.
    void authDatabaseEraseRequested();

    //! Emitted when the authentication db is significantly changed, e.g. large record removal, erased, etc.
    void authDatabaseChanged();

  public slots:
    //! Clear all authentication configs from authentication method caches
    void clearAllCachedConfigs();

    //! Clear an authentication config from its associated authentication method cache
    void clearCachedConfig( const QString &authcfg );

  private slots:
    void writeToConsole( const QString &message, const QString &tag = QString(), QgsAuthManager::MessageLevel level = INFO );

    /**
     * This slot emits the authDatabaseEraseRequested signal, instead of attempting
     * the erase. It relies upon a slot connected to the signal in calling application
     * (the one that initiated the erase) to initiate the erase, when it is ready.
     * Upon activation, a receiving slot should get confimation from the user, then
     * IMMEDIATELY call setScheduledAuthDatabaseErase( FALSE ) to stop the scheduling timer.
     * If receiving slot is NOT ready to initiate the erase, e.g. project is still loading,
     * it can skip the confirmation and request another signal emit from the scheduling timer.
     */
    void tryToStartDbErase();

  protected:

    /**
     * Enforce singleton pattern
     * \note To set up the manager instance and initialize everything use QgsAuthManager::instance()->init()
     */
    static QgsAuthManager *instance() SIP_SKIP;


#ifdef Q_OS_WIN
  public:
    explicit QgsAuthManager() SIP_SKIP;
#else
  protected:
    explicit QgsAuthManager() SIP_SKIP;
#endif

  private:

    /**
     * Performs lazy initialization of the authentication framework, if it has
     * not already been done.
     */
    bool ensureInitialized() const;

    bool initPrivate( const QString &pluginPath,  const QString &authDatabasePath );

    //////////////////////////////////////////////////////////////////////////////
    // Password Helper methods

    //! Returns the name for logging
    QString passwordHelperName() const;

    //! Print a debug message in QGIS
    void passwordHelperLog( const QString &msg ) const;

    //! Read Master password from the wallet
    QString passwordHelperRead();

    //! Store Master password in the wallet
    bool passwordHelperWrite( const QString &password );

    //! Error message setter
    void passwordHelperSetErrorMessage( const QString &errorMessage ) { mPasswordHelperErrorMessage = errorMessage; }

    //! Clear error code and message
    void passwordHelperClearErrors();

    /**
     * Process the error: show it and/or disable the password helper system in case of
     * access denied or no backend, reset error flags at the end
     */
    void passwordHelperProcessError();

    bool createConfigTables();

    bool createCertTables();

    bool masterPasswordInput();

    bool masterPasswordRowsInDb( int *rows ) const;

    bool masterPasswordCheckAgainstDb( const QString &compare = QString() ) const;

    bool masterPasswordStoreInDb() const;

    bool masterPasswordClearDb();

    const QString masterPasswordCiv() const;

    bool verifyPasswordCanDecryptConfigs() const;

    bool reencryptAllAuthenticationConfigs( const QString &prevpass, const QString &prevciv );

    bool reencryptAuthenticationConfig( const QString &authcfg, const QString &prevpass, const QString &prevciv );

    bool reencryptAllAuthenticationSettings( const QString &prevpass, const QString &prevciv );

    bool reencryptAllAuthenticationIdentities( const QString &prevpass, const QString &prevciv );

    bool reencryptAuthenticationIdentity( const QString &identid, const QString &prevpass, const QString &prevciv );

    bool authDbOpen() const;

    bool authDbQuery( QSqlQuery *query ) const;

    bool authDbStartTransaction() const;

    bool authDbCommit() const;

    bool authDbTransactionQuery( QSqlQuery *query ) const;

#ifndef QT_NO_SSL
    void insertCaCertInCache( QgsAuthCertUtils::CaCertSource source, const QList<QSslCertificate> &certs );
#endif

    const QString authDbPassTable() const { return AUTH_PASS_TABLE; }

    const QString authDbSettingsTable() const { return AUTH_SETTINGS_TABLE; }

    const QString authDbIdentitiesTable() const { return AUTH_IDENTITIES_TABLE; }

    const QString authDbAuthoritiesTable() const { return AUTH_AUTHORITIES_TABLE; }

    const QString authDbTrustTable() const { return AUTH_TRUST_TABLE; }

    QString authPasswordHelperKeyName() const;

    static QgsAuthManager *sInstance;
    static const QString AUTH_CONFIG_TABLE;
    static const QString AUTH_PASS_TABLE;
    static const QString AUTH_SETTINGS_TABLE;
    static const QString AUTH_IDENTITIES_TABLE;
    static const QString AUTH_SERVERS_TABLE;
    static const QString AUTH_AUTHORITIES_TABLE;
    static const QString AUTH_TRUST_TABLE;
    static const QString AUTH_CFG_REGEX;

    QString mPluginPath;
    QString mAuthDatabasePath;
    mutable bool mLazyInitResult = false;

    bool mAuthInit = false;
    QString mAuthDbPath;

    std::unique_ptr<QCA::Initializer> mQcaInitializer;

    QHash<QString, QString> mConfigAuthMethods;
    QHash<QString, QgsAuthMethod *> mAuthMethods;

    QString mMasterPass;
    int mPassTries = 0;
    bool mAuthDisabled = false;
    QString mAuthDisabledMessage;
    QTimer *mScheduledDbEraseTimer = nullptr;
    bool mScheduledDbErase = false;
    int mScheduledDbEraseRequestWait = 3 ; // in seconds
    bool mScheduledDbEraseRequestEmitted = false;
    int mScheduledDbEraseRequestCount = 0;

    std::unique_ptr<QRecursiveMutex> mMutex;
    std::unique_ptr<QRecursiveMutex> mMasterPasswordMutex;
#ifndef QT_NO_SSL
    // mapping of sha1 digest and cert source and cert
    // appending removes duplicates
    QMap<QString, QPair<QgsAuthCertUtils::CaCertSource, QSslCertificate> > mCaCertsCache;
    // list of sha1 digests per policy
    QMap<QgsAuthCertUtils::CertTrustPolicy, QStringList > mCertTrustCache;
    // cache of certs ready to be utilized in network connections
    QList<QSslCertificate> mTrustedCaCertsCache;
    // cache of SSL errors to be ignored in network connections, per sha-hostport
    QHash<QString, QSet<QSslError::SslError> > mIgnoredSslErrorsCache;

    bool mHasCustomConfigByHost = false;
    bool mHasCheckedIfCustomConfigByHostExists = false;
    QMap< QString, QgsAuthConfigSslServer > mCustomConfigByHostCache;
#endif

    //////////////////////////////////////////////////////////////////////////////
    // Password Helper Variables

    //! Master password verification has failed
    bool mPasswordHelperVerificationError = false;

    //! Store last error message
    QString mPasswordHelperErrorMessage;

    //! Store last error code (enum)
    QKeychain::Error mPasswordHelperErrorCode = QKeychain::NoError;

    //! Enable logging
    bool mPasswordHelperLoggingEnabled = false;

    //! Whether the keychain bridge failed to initialize
    bool mPasswordHelperFailedInit = false;

    //! Master password name in the wallets
    static const QLatin1String AUTH_PASSWORD_HELPER_KEY_NAME_BASE;

    //! password helper folder in the wallets
    static const QLatin1String AUTH_PASSWORD_HELPER_FOLDER_NAME;

    mutable QMap<QThread *, QMetaObject::Connection> mConnectedThreads;

    friend class QgsApplication;

};

#endif // QGSAUTHMANAGER_H
