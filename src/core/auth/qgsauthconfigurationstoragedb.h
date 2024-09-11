/***************************************************************************
  qgsauthconfigurationstoragedb.h - QgsAuthConfigurationStorageDb

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
#ifndef QGSAUTHCONFIGURATIONSTORAGEDB_H
#define QGSAUTHCONFIGURATIONSTORAGEDB_H

#include "qgis_core.h"
#include "qgsauthconfigurationstorage.h"

#include <QObject>
#include <QRecursiveMutex>
#include <QSqlDatabase>


/**
 * \ingroup core
 * QSqlDatabase based implementation of QgsAuthConfigurationStorage.
 * \since QGIS 3.40
 */
class CORE_EXPORT QgsAuthConfigurationStorageDb : public QgsAuthConfigurationStorage
{
    Q_OBJECT
  public:

    /**
     * Creates a new QgsAuthConfigurationStorageDb instance from the specified \a settings.
     * Settings can contain the following keys:
     *
     * Mandatory settings:
     *
     * - driver: the database driver to use (default is "QSQLITE", see: https://doc.qt.io/qt-6/sql-driver.html)
     * - database: the database name (or path in case of QSQLITE)
     *
     * Optional settings:
     *
     * - host: the database host
     * - user: the database user
     * - password: the database password
     * - port: the database port
     * - schema: the database schema for all the tables (ignored if not supported)
     * - options: the database connection options (see: https://doc.qt.io/qt-6/qsqldatabase.html#setConnectOptions)
     *
     * \note Even if this generic storage also works with a pre-existing SQLite DB,
     *       a convenience subclass QgsAuthConfigurationStorageSqlite is provided for
     *       SQLite DBs.
     */
    QgsAuthConfigurationStorageDb( const QMap<QString, QVariant> &settings );

    /**
     * Creates a new QgsAuthConfigurationStorageDb instance from the specified \a uri.
     * The URI should be in the format: \verbatim<DRIVER>://<USER>:<PASSWORD>@<HOST>:<PORT>/<DATABASE>[?OPTIONS]\endverbatim
     * \note It is not possible to set the schema in the URI, pass SCHEMA=\verbatim<SCHEMA>\endverbatim in the options instead.
     */
    QgsAuthConfigurationStorageDb( const QString &uri );


    ~QgsAuthConfigurationStorageDb() override;


    /**
     * Returns the database connection used by this storage.
     */
    QSqlDatabase authDatabaseConnection() const;

    // QgsAuthConfigurationStorage interface
  public:
    QString name() const override;
    QString type() const override;
    QString description() const override;
    QString id() const override;

    QgsAuthMethodConfigsMap authMethodConfigs( const QStringList &allowedMethods = QStringList() ) const override;
    QgsAuthMethodConfigsMap authMethodConfigsWithPayload( ) const override;
    QgsAuthMethodConfig loadMethodConfig( const QString &id, QString &payload SIP_OUT, bool full = false ) const override;
    bool storeMethodConfig( const QgsAuthMethodConfig &mconfig, const QString &payload ) override;
    bool removeMethodConfig( const QString &id ) override;
    bool methodConfigExists( const QString &id ) const override;
    bool storeAuthSetting( const QString &key, const QString &value ) override;
    QString loadAuthSetting( const QString &key ) const override;
    bool removeAuthSetting( const QString &key ) override;
    bool authSettingExists( const QString &key ) const override;
    bool clearMethodConfigs() override;
    bool erase() override;
    bool isReady() const override;
    bool initialize() override;
    QList<QgsAuthConfigurationStorage::SettingParameter> settingsParameters() const override;

    bool storeCertIdentity( const QSslCertificate &cert, const QString &keyPem ) override;
    bool removeCertIdentity( const QSslCertificate &cert ) override;
    const QSslCertificate loadCertIdentity( const QString &id ) const override;
    const QPair<QSslCertificate, QString> loadCertIdentityBundle( const QString &id ) const override;
    const QList<QSslCertificate> certIdentities() const override;
    QStringList certIdentityIds() const override;
    bool certIdentityExists( const QString &id ) const override;
    bool removeCertIdentity( const QString &id ) override;
    bool storeSslCertCustomConfig( const QgsAuthConfigSslServer &config ) override;
    QStringList sslCertCustomConfigIds() const override;
    const QgsAuthConfigSslServer loadSslCertCustomConfig( const QString &id, const QString &hostport ) const override;
    const QgsAuthConfigSslServer loadSslCertCustomConfigByHost( const QString &hostport ) const override;
    const QList<QgsAuthConfigSslServer> sslCertCustomConfigs() const override;
    bool sslCertCustomConfigExists( const QString &id, const QString &hostport ) override;
    bool removeSslCertCustomConfig( const QString &id, const QString &hostport ) override;

    bool storeCertAuthority( const QSslCertificate &cert ) override;
    QStringList certAuthorityIds() const override;
    const QSslCertificate loadCertAuthority( const QString &id ) const override;
    bool certAuthorityExists( const QSslCertificate &cert ) const override;
    bool removeCertAuthority( const QSslCertificate &cert ) override;
    const QMap<QString, QgsAuthCertUtils::CertTrustPolicy> caCertsPolicy() const override;
    const QList<QSslCertificate> caCerts() const override;

    bool storeCertTrustPolicy( const QSslCertificate &cert, QgsAuthCertUtils::CertTrustPolicy policy ) override;
    QgsAuthCertUtils::CertTrustPolicy loadCertTrustPolicy( const QSslCertificate &cert ) const override;
    bool removeCertTrustPolicy( const QSslCertificate &cert ) override;
    bool certTrustPolicyExists( const QSslCertificate &cert ) const override;

    const QList<QgsAuthConfigurationStorage::MasterPasswordConfig> masterPasswords( ) const override;
    bool storeMasterPassword( const QgsAuthConfigurationStorage::MasterPasswordConfig &config ) override;
    bool clearMasterPasswords() override;

    // DB specific methods

    /**
     * Returns the name of the table used to store the method configurations.
     */
    virtual QString methodConfigTableName() const;

    /**
     * Returns the name of the table used to store the auth settings.
     */
    virtual QString authSettingsTableName() const;

    /**
     * Returns the name of the table used to store the certificate identities.
     */
    virtual QString certIdentityTableName() const;

    /**
     * Returns the name of the table used to store the certificate authorities.
     */
    virtual QString certAuthorityTableName() const;

    /**
     * Returns the name of the table used to store the SSL custom configurations.
     */
    virtual QString sslCertCustomConfigTableName() const;

    /**
     * Returns the name of the table used to store the certificate trust policies.
     */
    virtual QString certTrustPolicyTableName() const;

    /**
     * Returns the name of the table used to store the master passwords.
     */
    virtual QString masterPasswordTableName() const;

    /**
     * Returns TRUE if the specified \a table exists in the database, FALSE otherwise.
     * \note The schema is automatically prepended to the table name.
     */
    virtual bool tableExists( const QString &table ) const;

    /**
     * Returns the quoted identifier, prefixed with the schema
     * (if not null), ready for the insertion into a SQL query.
     * \param identifier the identifier to quote.
     * \param isIndex if TRUE, the identifier is treated as an index name.
     */
    virtual QString quotedQualifiedIdentifier( const QString &identifier, bool isIndex = false ) const;

  private:

    bool clearTables( const QStringList &tables );

    static const QMap<QString, QVariant> uriToSettings( const QString &uri );
    mutable QMap<QThread *, QMetaObject::Connection> mConnectedThreads;

  protected:

    /**
     * Opens the connction to the database.
     * \returns TRUE if the connection was opened successfully, FALSE otherwise.
     */
    bool authDbOpen() const;

    /**
     * Runs the specified \a query on the database. Optional \a sql can be provided.
     * \returns TRUE if the query was executed successfully, FALSE otherwise.
     */
    bool authDbQuery( QSqlQuery *query, const QString &sql = QString() ) const;

    /**
     * Executes the specified \a query on the database using a transaction. Optional \a sql can be provided.
     * \returns TRUE if the query was executed successfully, FALSE otherwise.
     */
    bool authDbTransactionQuery( QSqlQuery *query );

    /**
     * Creates the configuration tables in the database.
     * \returns TRUE if the tables were created successfully, FALSE otherwise.
     */
    bool createConfigTables();

    /**
     * Creates the certificate tables in the database.
     * \returns TRUE if the tables were created successfully, FALSE otherwise.
     */
    bool createCertTables();

    /**
     * Checks the capabilities of the storage.
     */
    virtual void checkCapabilities();

    // Storage ID
    mutable QString mId;

    // From https://doc.qt.io/qt-6/sql-driver.html
    QString mDriver;
    QString mDatabase;
    QString mHost;
    QString mUser;
    QString mPassword;
    int mPort;
    // Driver specific options
    QString mConnectOptions;

    bool mIsReady = false;

    mutable QRecursiveMutex mMutex;

};

#endif // QGSAUTHCONFIGURATIONSTORAGEDB_H
