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
    INSTANCE_NUMBER = 0,
    PORT_NUMBER = 1
  };

  static bool isValid( uint ) noexcept;
  static Value fromInt( uint );
};

class QgsHanaSettings
{
  public:
    QgsHanaSettings( const QString &name, bool autoLoad = false );

    const QString &name() const { return mName; }

    const QString &driver() const { return mDriver; }
    void setDriver( const QString &driver ) { mDriver = driver; }

    const QString &host() const { return mHost; }
    void setHost( const QString &host ) { mHost = host; }

    uint identifierType() const { return mIdentifierType; }
    void setIdentifierType( uint identifierType ) { mIdentifierType = identifierType; }

    const QString &identifier() const { return mIdentifier; }
    void setIdentifier( const QString &identifier ) { mIdentifier = identifier; }

    const QString &database() const { return mDatabase; }
    void setDatabase( const QString &database ) { mDatabase = database; }

    bool multitenant() const { return mMultitenant; }
    void setMultitenant( bool value ) { mMultitenant = value; }

    const QString &schema() const { return mSchema; }
    void setSchema( const QString &schema ) { mSchema = schema; }

    const QString &authCfg() const { return mAuthcfg; }
    void setAuthCfg( const QString &authcfg ) { mAuthcfg = authcfg; }

    const QString &userName() const { return mUserName; }
    void setUserName( const QString &userName ) { mUserName = userName; }

    bool saveUserName() const { return mSaveUserName; }
    void setSaveUserName( bool saveUserName ) { mSaveUserName = saveUserName; }

    const QString &password() const { return mPassword; }
    void setPassword( const QString &password ) { mPassword = password; }

    bool savePassword() const { return mSavePassword; }
    void setSavePassword( bool savePassword ) { mSavePassword = savePassword; }

    bool userTablesOnly() const { return mUserTablesOnly; }
    void setUserTablesOnly( bool userTablesOnly ) { mUserTablesOnly = userTablesOnly; }

    bool allowGeometrylessTables() const { return mAllowGeometrylessTables; }
    void setAllowGeometrylessTables( bool allowGeometrylessTables )
    {
      mAllowGeometrylessTables = allowGeometrylessTables;
    }

    bool enableSsl() const { return mSslEnabled; }
    void setEnableSsl( bool value ) { mSslEnabled = value; }

    const QString &sslCryptoProvider() const { return mSslCryptoProvider; }
    void setSslCryptoProvider( const QString &value ) { mSslCryptoProvider = value; }

    const QString &sslKeyStore() const { return mSslKeyStore; }
    void setSslKeyStore( const QString &value ) { mSslKeyStore = value; }

    const QString &sslTrustStore() const { return mSslTrustStore; }
    void setSslTrustStore( const QString &value ) { mSslTrustStore = value; }

    bool sslValidateCertificate() const { return mSslValidateCertificate; }
    void setSslValidateCertificate( bool value ) { mSslValidateCertificate = value; }

    const QString &sslHostNameInCertificate() const
    {
      return mSslHostNameInCertificate;
    }
    void setSslHostNameInCertificate( const QString &value )
    {
      mSslHostNameInCertificate = value;
    }

    QString port() const;
    void setFromDataSourceUri( const QgsDataSourceUri &uri );
    QgsDataSourceUri toDataSourceUri() const ;

    void load();
    void save();

    static QStringList getConnectionNames();
    static QString getSelectedConnection();
    static void setSelectedConnection( const QString &name );
    static void removeConnection( const QString &name );

  private:
    QString path() const { return getBaseKey() + mName; }
    static QString getBaseKey() { return "/HANA/connections/"; }

  private:
    QString mName;
    QString mDriver;
    QString mHost;
    uint mIdentifierType;
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
    // Ssl parameters
    bool mSslEnabled = false;
    QString mSslCryptoProvider;
    QString mSslKeyStore;
    QString mSslTrustStore;
    bool mSslValidateCertificate = false;
    QString mSslHostNameInCertificate;
};

#endif // QGSHANAPSETTINGS_H
