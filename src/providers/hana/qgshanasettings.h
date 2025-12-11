/***************************************************************************
   qgshanasettings.h
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maxim Rylov
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#ifndef QGSHANAPSETTINGS_H
#define QGSHANAPSETTINGS_H

#include "qgsdatasourceuri.h"

#include <QString>

struct QgsHanaIdentifierType
{
    enum Value
    {
      InstanceNumber = 0,
      PortNumber = 1
    };

    static bool isValid( uint ) noexcept;
    static Value fromInt( uint );
};

enum class QgsHanaConnectionType : uint
{
  HostPort = 0,
  Dsn = 1
};

class QgsHanaSettings
{
  public:
    QgsHanaSettings( const QString &name, bool autoLoad = false );

    /**
     * The connection name.
     */
    [[nodiscard]] const QString &name() const { return mName; }

    /**
     * The connection type of the driver.
     */
    [[nodiscard]] QgsHanaConnectionType connectionType() const { return mConnectionType; }
    void setConnectionType( QgsHanaConnectionType connType ) { mConnectionType = connType; }

    /**
     * The Data Source Name.
     */
    [[nodiscard]] const QString &dsn() const { return mDsn; }
    void setDsn( const QString &dsn ) { mDsn = dsn; }

    /**
     * The name/path of/to the driver. For example,
     * HDBODBC/HDBODBC32 on Windows or /usr/sap/hdbclient/libodbcHDB.so on Linux.
     */
    [[nodiscard]] const QString &driver() const { return mDriver; }
    void setDriver( const QString &driver ) { mDriver = driver; }

    /**
     * The server host name.
     */
    [[nodiscard]] const QString &host() const { return mHost; }
    void setHost( const QString &host ) { mHost = host; }

    /**
     * The identifier type that specifies whether the port number depends
     * on the instance number of a database or not. Possible values are
     * 0 - InstanceNumber, 1 - PortNumber.
     */
    [[nodiscard]] uint identifierType() const { return mIdentifierType; }
    void setIdentifierType( uint identifierType ) { mIdentifierType = identifierType; }

    /**
     * The identifier that specifies either the instance number of a database or
     * the port number.
     */
    [[nodiscard]] const QString &identifier() const { return mIdentifier; }
    void setIdentifier( const QString &identifier ) { mIdentifier = identifier; }

    /**
     * The name of a database.
     */
    [[nodiscard]] const QString &database() const { return mDatabase; }
    void setDatabase( const QString &database ) { mDatabase = database; }

    /**
     * Specifies whether the HANA runs multitenant containers or not.
     */
    [[nodiscard]] bool multitenant() const { return mMultitenant; }
    void setMultitenant( bool value ) { mMultitenant = value; }

    /**
     * The schema name.
     */
    [[nodiscard]] const QString &schema() const { return mSchema; }
    void setSchema( const QString &schema ) { mSchema = schema; }

    /**
     * The authentication configuration id.
     */
    [[nodiscard]] const QString &authCfg() const { return mAuthcfg; }
    void setAuthCfg( const QString &authcfg ) { mAuthcfg = authcfg; }

    /**
     * The user name.
     */
    [[nodiscard]] const QString &userName() const { return mUserName; }
    void setUserName( const QString &userName ) { mUserName = userName; }

    /**
     * Specifies whether the user name is stored or not.
     */
    [[nodiscard]] bool saveUserName() const { return mSaveUserName; }
    void setSaveUserName( bool saveUserName ) { mSaveUserName = saveUserName; }

    /**
     * The user password.
     */
    [[nodiscard]] const QString &password() const { return mPassword; }
    void setPassword( const QString &password ) { mPassword = password; }

    /**
     * Specifies whether the user password is stored or not.
     */
    [[nodiscard]] bool savePassword() const { return mSavePassword; }
    void setSavePassword( bool savePassword ) { mSavePassword = savePassword; }

    /**
     * Specifies whether only user tables are returned or not.
     */
    [[nodiscard]] bool userTablesOnly() const { return mUserTablesOnly; }
    void setUserTablesOnly( bool userTablesOnly ) { mUserTablesOnly = userTablesOnly; }

    /**
     * Specifies whether tables without geometries are returned or not.
     */
    [[nodiscard]] bool allowGeometrylessTables() const { return mAllowGeometrylessTables; }
    void setAllowGeometrylessTables( bool allowGeometrylessTables )
    {
      mAllowGeometrylessTables = allowGeometrylessTables;
    }

    /**
     * Specifies whether estimated metadata from e.g. an index can be used or not.
     */
    [[nodiscard]] bool useEstimatedMetadata() const { return mUseEstimatedMetadata; }
    void setUseEstimatedMetadata( bool useEstimatedMetadata )
    {
      mUseEstimatedMetadata = useEstimatedMetadata;
    }

    /**
     * Enables or disables TLS 1.1 – TLS1.2 encryption.
     */
    [[nodiscard]] bool enableSsl() const { return mSslEnabled; }
    void setEnableSsl( bool value ) { mSslEnabled = value; }

    /**
     * Specifies the cryptographic library provider used for SSL communication.
     */
    [[nodiscard]] const QString &sslCryptoProvider() const { return mSslCryptoProvider; }
    void setSslCryptoProvider( const QString &value ) { mSslCryptoProvider = value; }

    /**
     * Specifies the path to the keystore file that contains the client’s
     * private key and, if using CommonCryptoLib, the server’s public certificates.
     */
    [[nodiscard]] const QString &sslKeyStore() const { return mSslKeyStore; }
    void setSslKeyStore( const QString &value ) { mSslKeyStore = value; }

    /**
     * Specifies the path to a trust store file that contains the server’s
     * public certificates if using OpenSSL.
     */
    [[nodiscard]] const QString &sslTrustStore() const { return mSslTrustStore; }
    void setSslTrustStore( const QString &value ) { mSslTrustStore = value; }

    /**
     * Specifies whether to validate the server's certificate.
     */
    [[nodiscard]] bool sslValidateCertificate() const { return mSslValidateCertificate; }
    void setSslValidateCertificate( bool value ) { mSslValidateCertificate = value; }

    /**
     * Specifies the host name used to verify server’s identity.
     */
    [[nodiscard]] const QString &sslHostNameInCertificate() const
    {
      return mSslHostNameInCertificate;
    }
    void setSslHostNameInCertificate( const QString &value )
    {
      mSslHostNameInCertificate = value;
    }

    /**
     * Enables proxy.
     */
    [[nodiscard]] bool enableProxy() const { return mProxyEnabled; }
    void setEnableProxy( bool value ) { mProxyEnabled = value; }

    /**
     * Enables HTTP proxy authentication.
     */
    [[nodiscard]] bool enableProxyHttp() const { return mProxyHttp; }
    void setEnableProxyHttp( bool value ) { mProxyHttp = value; }

    /**
     * Specifies the host name of the proxy server.
     */
    [[nodiscard]] const QString &proxyHost() const
    {
      return mProxyHost;
    }
    void setProxyHost( const QString &value )
    {
      mProxyHost = value;
    }

    /**
     * Specifies the port of the proxy server.
     */
    [[nodiscard]] uint proxyPort() const
    {
      return mProxyPort;
    }
    void setProxyPort( uint value )
    {
      mProxyPort = value;
    }

    /**
     * Specifies the user name for Basic HTTP Authentication or METHOD 02 SOCKS authentication.
     */
    [[nodiscard]] const QString &proxyUsername() const
    {
      return mProxyUsername;
    }
    void setProxyUsername( const QString &value )
    {
      mProxyUsername = value;
    }

    /**
     * Specifies the password for Basic HTTP Authentication or METHOD 02 SOCKS authentication.
     */
    [[nodiscard]] const QString &proxyPassword() const
    {
      return mProxyPassword;
    }
    void setProxyPassword( const QString &value )
    {
      mProxyPassword = value;
    }

    /**
     * Gets the server port.
     */
    [[nodiscard]] QString port() const;

    /**
     * Gets the key columns for the given database object.
     */
    [[nodiscard]] QStringList keyColumns( const QString &schemaName, const QString &objectName ) const;

    /**
     * Sets the key columns for the given database object.
     */
    void setKeyColumns( const QString &schemaName, const QString &objectName, const QStringList &columnNames );

    /**
     * Sets values from a RDBMS data source URI.
     */
    void setFromDataSourceUri( const QgsDataSourceUri &uri );

    /**
     * Constructs an instance of QgsDataSourceUri with values of the current object.
     */
    [[nodiscard]] QgsDataSourceUri toDataSourceUri() const;

    /**
     * Loads HANA connection settings from /HANA/connections/{connection_name}.
     */
    void load();

    /**
     * Saves HANA connection settings to /HANA/connections/{connection_name}.
     */
    void save();

    /**
     * Duplicates \a src connection settings to a new \a dst connection.
     * \since QGIS 3.40
     */
    static void duplicateConnection( const QString &src, const QString &dst );

    static QStringList getConnectionNames();
    static QString getSelectedConnection();
    static void setSelectedConnection( const QString &name );
    static void removeConnection( const QString &name );

  private:
    [[nodiscard]] QString path() const { return getBaseKey() + mName; }
    static QString getBaseKey() { return "/HANA/connections/"; }

  private:
    QString mName;
    QgsHanaConnectionType mConnectionType = QgsHanaConnectionType::HostPort;
    QString mDsn;
    QString mDriver;
    QString mHost;
    uint mIdentifierType = QgsHanaIdentifierType::InstanceNumber;
    QString mIdentifier;
    QString mDatabase;
    bool mMultitenant = false;
    QString mSchema;
    QString mAuthcfg;
    QString mUserName;
    QString mPassword;
    bool mSaveUserName = false;
    bool mSavePassword = false;
    bool mUserTablesOnly = true;
    bool mAllowGeometrylessTables = false;
    bool mUseEstimatedMetadata = false;
    QMap<QString, QMap<QString, QStringList>> mKeyColumns;
    // SSL parameters
    bool mSslEnabled = false;
    QString mSslCryptoProvider;
    QString mSslKeyStore;
    QString mSslTrustStore;
    bool mSslValidateCertificate = false;
    QString mSslHostNameInCertificate;
    // Proxy parameters
    bool mProxyEnabled = false;
    bool mProxyHttp = false;
    QString mProxyHost;
    uint mProxyPort = 1080;
    QString mProxyUsername;
    QString mProxyPassword;
};

#endif // QGSHANAPSETTINGS_H
