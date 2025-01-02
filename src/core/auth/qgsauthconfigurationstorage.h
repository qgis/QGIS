/***************************************************************************
  qgsauthconfigurationstorage.h - QgsAuthConfigurationStorage

 ---------------------
 begin                : 20.6.2024
 copyright            : (C) 2024 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSAUTHCONFIGURATIONSTORAGE_H
#define QGSAUTHCONFIGURATIONSTORAGE_H


#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgis.h"
#include "qgsauthconfig.h"
#include "qgsauthcertutils.h"
#include "qgsexception.h"

#include <QString>
#include <QObject>


/**
 * \ingroup core
 *  Abstract class that defines the interface for all authentication configuration storage implementations.
 *  \since QGIS 3.40
 */
class CORE_EXPORT QgsAuthConfigurationStorage: public QObject
{
    Q_OBJECT

  public:

    /**
     * Structure that holds the (encrypted) master password elements.
     */
    struct CORE_EXPORT MasterPasswordConfig
    {
      QString salt;
      QString civ;
      QString hash;
    };

    /**
     * Storage configuration setting parameter.
     */
    struct CORE_EXPORT SettingParameter
    {
      QString name;
      QString description;
      QVariant::Type type;
    };

    /**
     * Creates a new authentication configuration storage.
     * \param settings Implementation-specific configuration settings.
     */
    QgsAuthConfigurationStorage( const QMap<QString, QVariant> &settings );

    virtual ~QgsAuthConfigurationStorage() {}

    /**
     * Returns a human readable localized short name of the storage implementation (e.g "SQLite").
     * This name is displayed to the user and used to identify the storage implementation.
     */
    virtual QString name() const = 0;

    /**
     * Returns the type of the storage implementation.
     * The type is used to identify the storage implementation internally (e.g. "sqlite").
     * A valid type must be ASCII alphanumeric and contain no spaces.
     */
    virtual QString type() const = 0;

    /**
     * Returns a human readable localized description of the storage implementation (e.g. "Store credentials in a local SQLite database").
     * This description is displayed to the user.
     */
    virtual QString description() const = 0;

    /**
     * Returns the unique identifier of the storage object.
     * The id is used to uniquely identify the storage object (e.g. the path or the connection URI to a storage configuration).
     */
    virtual QString id() const = 0;

    /**
     * Initializes the storage.
     * \returns TRUE if the storage was successfully initialized, FALSE otherwise.
     * If the storage is already initialized, this method does nothing and returns TRUE.
     *
     * \note The default implementation does nothing and returns TRUE.
     * This method is called by the authentication manager when the storage is added to the manager.
     */
    virtual bool initialize() { return true; }

    /**
     * Returns the last error message.
     */
    virtual QString lastError() const;

    /**
     * Returns TRUE is the storage is ready to be used.
     * \note This method should be called after the initialize() method to check whether the initialization was properly completed.
     */
    virtual bool isReady() const = 0;

    /**
     * Returns the capabilities of the storage.
     */
    Qgis::AuthConfigurationStorageCapabilities capabilities() const;

    /**
     * Returns the settings of the storage.
     */
    QMap<QString, QVariant> settings() const;

    /**
     * Returns a list of the settings accepted by the storage.
     */
    virtual QList<QgsAuthConfigurationStorage::SettingParameter> settingsParameters() const = 0;

    /**
     * Returns TRUE if the storage is encrypted.
     */
    bool isEncrypted() const;

    /**
     * Returns TRUE if the storage is enabled.
     */
    bool isEnabled() const;

    /**
     * Set the storage enabled status to \a enabled.
     * \note This is a user-controlled setting: the storage may be enabled but not ready to be used.
     */
    void setEnabled( bool enabled );


    /**
     * Utility method to unset all editing capabilities.
     * \note This method does not alter existing capabilities, make sure subclasses recompute capabilities if needed.
     */
    virtual void setReadOnly( bool readOnly );

    /**
     * Returns TRUE if the storage is read-only, FALSE otherwise.
     * \see setReadOnly()
     */
    virtual bool isReadOnly() const;

    /**
     * Returns a mapping of authentication configurations available from this storage.
     * \param allowedMethods Optional filter to return only configurations for specific authentication methods.
     * \throws QgsNotSupportedException if the operation is not supported by the storage.
     */
    virtual QgsAuthMethodConfigsMap authMethodConfigs( const QStringList &allowedMethods = QStringList() ) const SIP_THROW( QgsNotSupportedException ) = 0;

    /**
     * Returns a mapping of authentication configurations available from this storage.
     * The encrypted payload is added to the configuration as "encrypted_payload" key.
     * \throws QgsNotSupportedException if the operation is not supported by the storage
     * \note This convenience method is used by the authentication manager to retrieve the configurations
     * and check if it can decrypt all of them, it is faster than retrieve all the configurations one
     * by one.
     */
    virtual QgsAuthMethodConfigsMap authMethodConfigsWithPayload( ) const SIP_THROW( QgsNotSupportedException ) = 0;

    /**
     * Load an authentication configuration from the database.
     * \param id Configuration id.
     * \param payload (possibly encrypted) payload.
     * \param full If TRUE, the full configuration is loaded and the (possibly encrypted) payload is populated, otherwise only the configuration metadata is loaded.
     * \returns Authentication configuration metadata.
     * \throws QgsNotSupportedException if the operation is not supported by the storage.
     */
    virtual QgsAuthMethodConfig loadMethodConfig( const QString &id, QString &payload SIP_OUT, bool full = false ) const SIP_THROW( QgsNotSupportedException ) = 0;

    /**
     * Store an authentication config in the database.
     * \param config Authentication configuration.
     * \param payload payload to store (possibly encrypted).
     * \returns Whether operation succeeded
     * \throws QgsNotSupportedException if the operation is not supported by the storage.
     */
    virtual bool storeMethodConfig( const QgsAuthMethodConfig &config, const QString &payload ) SIP_THROW( QgsNotSupportedException ) = 0;

    /**
     * Removes the authentication configuration with the specified \a id.
     * \returns TRUE if the configuration was removed, FALSE otherwise.
     * \throws QgsNotSupportedException if the operation is not supported by the storage.
     */
    virtual bool removeMethodConfig( const QString &id ) SIP_THROW( QgsNotSupportedException ) = 0;

    /**
     *  Check if an authentication configuration exists in the storage.
     *  \param id Configuration id.
     *  \returns TRUE if the configuration exists, FALSE otherwise.
     *  \throws QgsNotSupportedException if the operation is not supported by the storage.
     */
    virtual bool methodConfigExists( const QString &id ) const SIP_THROW( QgsNotSupportedException ) = 0;

    /**
     * Store an authentication setting in the storage.
     * \param key Setting key.
     * \param value Setting value.
     * \returns Whether operation succeeded
     * \throws QgsNotSupportedException if the operation is not supported by the storage.
     */
    virtual bool storeAuthSetting( const QString &key, const QString &value ) SIP_THROW( QgsNotSupportedException ) = 0;

    /**
     * Load an authentication setting from the storage.
     * \param key Setting key.
     * \returns Setting value.
     * \throws QgsNotSupportedException if the operation is not supported by the storage.
     */
    virtual QString loadAuthSetting( const QString &key ) const SIP_THROW( QgsNotSupportedException ) = 0;

    /**
     * Remove an authentication setting from the storage.
     * \param key Setting key.
     * \returns Whether operation succeeded
     * \throws QgsNotSupportedException if the operation is not supported by the storage.
     */
    virtual bool removeAuthSetting( const QString &key ) SIP_THROW( QgsNotSupportedException ) = 0;

    /**
     * Check if an authentication setting exists in the storage.
     * \param key Setting key.
     * \returns TRUE if the setting exists, FALSE otherwise.
     * \throws QgsNotSupportedException if the operation is not supported by the storage.
     */
    virtual bool authSettingExists( const QString &key ) const SIP_THROW( QgsNotSupportedException ) = 0;

#ifndef QT_NO_SSL

    /**
     * Store a certificate identity in the storage.
     * \param cert Certificate.
     * \param keyPem SSL key in PEM format.
     * \returns Whether operation succeeded
     * \throws QgsNotSupportedException if the operation is not supported by the storage.
     */
    virtual bool storeCertIdentity( const QSslCertificate &cert, const QString &keyPem ) SIP_THROW( QgsNotSupportedException ) = 0;

    /**
    * Remove a certificate identity from the storage.
    * \param cert Certificate.
    * \returns Whether operation succeeded
    * \throws QgsNotSupportedException if the operation is not supported by the storage.
    */
    virtual bool removeCertIdentity( const QSslCertificate &cert ) SIP_THROW( QgsNotSupportedException ) = 0;

    /**
    * \brief certIdentity get a certificate identity by \a id (sha hash)
    * \param id sha hash of the cert
    * \return the certificate
    * \throws QgsNotSupportedException if the operation is not supported by the storage.
    */
    virtual const QSslCertificate loadCertIdentity( const QString &id ) const SIP_THROW( QgsNotSupportedException ) = 0;

    /**
     * Returns a certificate identity bundle by \a id (sha hash).
     * \param id sha shash
     * \return a pair with the certificate and its SSL key as an encrypted string
     * \throws QgsNotSupportedException if the operation is not supported by the storage.
     */
    virtual const QPair<QSslCertificate, QString> loadCertIdentityBundle( const QString &id ) const SIP_THROW( QgsNotSupportedException ) = 0;

    /**
     * \brief certIdentities get certificate identities
     * \return list of certificates
     * \throws QgsNotSupportedException if the operation is not supported by the storage.
     */
    virtual const QList<QSslCertificate> certIdentities() const SIP_THROW( QgsNotSupportedException ) = 0;

    /**
     * \brief certIdentityIds get list of certificate identity ids from database
     * \return list of certificate ids
     * \throws QgsNotSupportedException if the operation is not supported by the storage.
     */
    virtual QStringList certIdentityIds() const SIP_THROW( QgsNotSupportedException ) = 0;

    /**
     *  Check if the certificate identity exists
     *  \param id Certificate identity id
     *  \returns TRUE if the certificate identity exists, FALSE otherwise
     *  \throws QgsNotSupportedException if the operation is not supported by the storage.
     */
    virtual bool certIdentityExists( const QString &id ) const SIP_THROW( QgsNotSupportedException ) = 0;

    /**
     * Remove a certificate identity from the storage.
     * \param id Certificate identity id
     * \returns Whether operation succeeded
     * \throws QgsNotSupportedException if the operation is not supported by the storage.
     */
    virtual bool removeCertIdentity( const QString &id ) SIP_THROW( QgsNotSupportedException ) = 0;

    /**
     *  Store an SSL certificate custom config
     *  \param config SSL certificate custom config
     *  \returns Whether operation succeeded
     *  \throws QgsNotSupportedException if the operation is not supported by the storage.
     */
    virtual bool storeSslCertCustomConfig( const QgsAuthConfigSslServer &config ) SIP_THROW( QgsNotSupportedException ) = 0;

    /**
     * Loads an SSL certificate custom config by \a id (sha hash) and \a hostport (host:port)
     * \param id sha hash
     * \param hostport string host:port
     * \return a SSL certificate custom config
     * \throws QgsNotSupportedException if the operation is not supported by the storage.
     */
    virtual const QgsAuthConfigSslServer loadSslCertCustomConfig( const QString &id, const QString &hostport ) const SIP_THROW( QgsNotSupportedException ) = 0;

    /**
     * Loads an SSL certificate custom config by \a hostport (host:port)
     * \param hostport host:port
     * \return a SSL certificate custom config
     * \throws QgsNotSupportedException if the operation is not supported by the storage.
     */
    virtual const QgsAuthConfigSslServer loadSslCertCustomConfigByHost( const QString &hostport ) const SIP_THROW( QgsNotSupportedException ) = 0;

    /**
     * \brief sslCertCustomConfigs get SSL certificate custom configs
     * \return list of SSL certificate custom config
     * \throws QgsNotSupportedException if the operation is not supported by the storage.
     */
    virtual const QList<QgsAuthConfigSslServer> sslCertCustomConfigs() const SIP_THROW( QgsNotSupportedException ) = 0;

    /**
     * Returns the list of SSL certificate custom config ids.
     * \return list of SSL certificate custom config ids
     * \throws QgsNotSupportedException if the operation is not supported by the storage.
     */
    virtual QStringList sslCertCustomConfigIds() const SIP_THROW( QgsNotSupportedException ) = 0;

    /**
     *  Check if SSL certificate custom config exists
     *  \param id sha hash
     *  \param hostport host:port
     *  \returns TRUE if the SSL certificate custom config exists, FALSE otherwise
     *  \throws QgsNotSupportedException if the operation is not supported by the storage.
     */
    virtual bool sslCertCustomConfigExists( const QString &id, const QString &hostport ) SIP_THROW( QgsNotSupportedException ) = 0;

    /**
     *  Remove an SSL certificate custom config
     *  \param id sha hash
     *  \param hostport host:port
     *  \returns Whether operation succeeded
     *  \throws QgsNotSupportedException if the operation is not supported by the storage.
     */
    virtual bool removeSslCertCustomConfig( const QString &id, const QString &hostport ) SIP_THROW( QgsNotSupportedException ) = 0;

    /**
     *  Store a certificate authority
     *  \param cert Certificate authority
     *  \returns Whether operation succeeded
     *  \throws QgsNotSupportedException if the operation is not supported by the storage.
     */
    virtual bool storeCertAuthority( const QSslCertificate &cert ) SIP_THROW( QgsNotSupportedException ) = 0;

    /**
     * Returns the list of certificate authority IDs in the storage.
     * \return list of certificate authority IDs
     * \throws QgsNotSupportedException if the operation is not supported by the storage.
     */
    virtual QStringList certAuthorityIds() const SIP_THROW( QgsNotSupportedException ) = 0;

    /**
     * \brief certAuthority get a certificate authority by \a id (sha hash)
     * \param id sha hash
     * \return a (possibly empty) certificate
     * \throws QgsNotSupportedException if the operation is not supported by the storage.
     */
    virtual const QSslCertificate loadCertAuthority( const QString &id ) const SIP_THROW( QgsNotSupportedException ) = 0;

    /**
     *  Check if a certificate authority exists
     *  \param cert Certificate authority
     *  \returns TRUE if the certificate authority exists, FALSE otherwise
     *  \throws QgsNotSupportedException if the operation is not supported by the storage.
     */
    virtual bool certAuthorityExists( const QSslCertificate &cert ) const SIP_THROW( QgsNotSupportedException ) = 0;

    /**
     *  Remove a certificate authority
     *  \param cert Certificate authority
     *  \returns Whether operation succeeded
     *  \throws QgsNotSupportedException if the operation is not supported by the storage.
     */
    virtual bool removeCertAuthority( const QSslCertificate &cert ) SIP_THROW( QgsNotSupportedException ) = 0;

    /**
     *  Returns the map of CA certificates hashes in the storages and their trust policy.
     *  \returns map of CA certificates hashes and their trust policy
     *  \throws QgsNotSupportedException if the operation is not supported by the storage.
     */
    virtual const QMap<QString, QgsAuthCertUtils::CertTrustPolicy> caCertsPolicy() const SIP_THROW( QgsNotSupportedException ) = 0;

    /**
     *  Returns the list of CA certificates in the storage
     *  \returns list of CA certificates
     *  \throws QgsNotSupportedException if the operation is not supported by the storage.
     */
    virtual const  QList<QSslCertificate> caCerts() const SIP_THROW( QgsNotSupportedException ) = 0;

    /**
     *  Store certificate trust policy
     *  \param cert Certificate
     *  \param policy Trust policy
     *  \returns Whether operation succeeded
     *  \throws QgsNotSupportedException if the operation is not supported by the storage.
     */
    virtual bool storeCertTrustPolicy( const QSslCertificate &cert, QgsAuthCertUtils::CertTrustPolicy policy ) SIP_THROW( QgsNotSupportedException ) = 0;

    /**
     *  Load certificate trust policy
     *  \param cert Certificate
     *  \returns Trust policy
     *  \throws QgsNotSupportedException if the operation is not supported by the storage.
     */
    virtual QgsAuthCertUtils::CertTrustPolicy loadCertTrustPolicy( const QSslCertificate &cert ) const SIP_THROW( QgsNotSupportedException ) = 0;

    /**
     *  Remove certificate trust policy
     *  \param cert Certificate
     *  \returns Whether operation succeeded
     *  \throws QgsNotSupportedException if the operation is not supported by the storage.
     */
    virtual bool removeCertTrustPolicy( const QSslCertificate &cert ) SIP_THROW( QgsNotSupportedException ) = 0;

    /**
     *  Check if certificate trust policy exists
     *  \param cert Certificate
     *  \returns TRUE if the certificate trust policy exists, FALSE otherwise
     *  \throws QgsNotSupportedException if the operation is not supported by the storage.
     */
    virtual bool certTrustPolicyExists( const QSslCertificate &cert ) const SIP_THROW( QgsNotSupportedException ) = 0;

#endif

    /**
     * Returns the list of (encrypted) master passwords stored in the database.
     * \returns list of master passwords
     * \throws QgsNotSupportedException if the operation is not supported by the storage.
     */
    virtual const QList<QgsAuthConfigurationStorage::MasterPasswordConfig> masterPasswords( ) const SIP_THROW( QgsNotSupportedException ) = 0;

    /**
     * Store a master password in the database.
     * \param config Master password configuration.
     * \returns TRUE if operation succeeded
     * \throws QgsNotSupportedException if the operation is not supported by the storage.
     */
    virtual bool storeMasterPassword( const QgsAuthConfigurationStorage::MasterPasswordConfig &config ) SIP_THROW( QgsNotSupportedException ) = 0;

    /**
     * Remove all master passwords from the database.
     * \returns TRUE if operation succeeded
     * \throws QgsNotSupportedException if the operation is not supported by the storage.
     */
    virtual bool clearMasterPasswords() SIP_THROW( QgsNotSupportedException ) = 0;

    /**
     * Completely erase the storage removing all configurations/certs/settings etc.
     * \returns TRUE if storage was completely erased, FALSE if any error occurred.
     * \throws QgsNotSupportedException if the operation is not supported by the storage (e.g. the storage is read-only).
     */
    virtual bool erase() SIP_THROW( QgsNotSupportedException ) = 0;

    /**
     * Remove all authentications configurations from the storage.
     * \returns TRUE if authentications configurations were removed, FALSE otherwise.
     * \note This method does not remove certificate and other assets.
     * \throws QgsNotSupportedException if the operation is not supported by the storage.
     */
    virtual bool clearMethodConfigs() SIP_THROW( QgsNotSupportedException ) = 0;


  signals:

    /**
     * Custom logging signal to relay to console output and QgsMessageLog
     * \param message Message to send
     * \param tag Associated tag (title)
     * \param level Message log level
     * \see QgsMessageLog
     */
    void messageLog( const QString &message, const QString &tag = QStringLiteral( "Authentication" ), Qgis::MessageLevel level = Qgis::MessageLevel::Info );

    /**
     * Emitted when the storage was updated.
     * \param id The storage id
     * \note This is a generic changed signal and it is normally
     * emitted together with the dedicated signals which are
     * provided for specific changes on the individual tables.
     */
    void storageChanged( const QString &id );

    /**
     * Emitted when the storage method config table was changed.
     */
    void methodConfigChanged( );

    /**
     * Emitted when the storage master password table was changed.
     */
    void masterPasswordChanged();

    /**
     * Emitted when the storage auth settings table was changed.
     */
    void authSettingsChanged();

    /**
     * Emitted when the storage read-only status was changed.
     */
    void readOnlyChanged( bool readOnly );


#ifndef QT_NO_SSL

    /**
     * Emitted when the storage cert identity table was changed.
     */
    void certIdentityChanged();

    /**
     * Emitted when the storage cert authority table was changed.
     */
    void certAuthorityChanged();

    /**
     * Emitted when the storage ssl cert custom config table was changed.
     */
    void sslCertCustomConfigChanged();

    /**
     * Emitted when the storage ssl cert trust policy table was changed.
     */
    void sslCertTrustPolicyChanged();

#endif

  protected:

    /**
     * Set the capabilities of the storage to \a capabilities.
     */
    void setCapabilities( Qgis::AuthConfigurationStorageCapabilities capabilities );

    /**
     * Set the last error message to \a error with message level \a level.
     */
    void setError( const QString &error, Qgis::MessageLevel level = Qgis::MessageLevel::Critical );

    /**
     * Utility to check \a capability and throw QgsNotSupportedException if not supported.
     * \note Not available in SIP bindings.
     */
    void checkCapability( Qgis::AuthConfigurationStorageCapability capability ) const SIP_SKIP;

    /**
     * Returns the logger tag for the storage.
     * The default implementation returns the literal "Auth storage" followed by the storage name.
     */
    virtual QString loggerTag() const;

    /**
     * Store the implementation-specific configuration.
     */
    QMap<QString, QVariant> mConfiguration;

    /**
     * Store the capabilities of the storage.
     */
    Qgis::AuthConfigurationStorageCapabilities mCapabilities;

    /**
     * Store the last error message.
     */
    mutable QString mLastError;

    /**
     * Store whether the storage is encrypted.
     */
    bool mIsEncrypted = true;

    /**
     * Store whether the storage is enabled.
     */
    bool mIsEnabled = true;

    /**
     * Store whether the storage is read-only.
     */
    bool mIsReadOnly = false;

};

#endif // QGSAUTHCONFIGURATIONSTORAGE_H
