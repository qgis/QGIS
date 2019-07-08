/***************************************************************************
   qgshanasettings.h
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maksim Rylov
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

  static bool isValid(uint8_t) noexcept;
  static Value fromInt(uint8_t);
};

class QgsHanaSettings
{
  public:
    QgsHanaSettings(const QString& name, bool autoLoad = false);

    const QString& getName() const { return mName; }

    const QString& getDriver() const { return mDriver; }
    void setDriver(const QString& driver) { mDriver = driver; }

    const QString& getHost() const { return mHost; }
    void setHost(const QString& host) { mHost = host; }

    uint getIdentifierType() const { return mIdentifierType; }
    void setIdentifierType(int identifierType) { mIdentifierType = identifierType; }

    const QString& getIdentifier() const { return mIdentifier; }
    void setIdentifier(const QString& identifier) { mIdentifier = identifier; }

    const QString& getDatabase() const { return mDatabase; }
    void setDatabase(const QString& database) { mDatabase = database; }

    bool getMultitenant() const { return mMultitenant; }
    void setMultitenant(bool value) { mMultitenant = value; }

    const QString& getSchema() const { return mSchema; }
    void setSchema(const QString& schema) { mSchema = schema; }

    const QString& getAuthCfg() const { return mAuthcfg; }
    void setAuthCfg(const QString& authcfg) { mAuthcfg = authcfg; }

    const QString& getUserName() const { return mUserName; }
    void setUserName(const QString& userName) { mUserName = userName; }

    bool getSaveUserName() const { return mSaveUserName; }
    void setSaveUserName(bool saveUserName) { mSaveUserName = saveUserName; }

    const QString& getPassword() const { return mPassword; }
    void setPassword(const QString& password) { mPassword = password; }

    bool getSavePassword() const { return mSavePassword; }
    void setSavePassword(bool savePassword) { mSavePassword = savePassword; }

    bool getUserTablesOnly() const { return mUserTablesOnly; }
    void setUserTablesOnly(bool userTablesOnly) { mUserTablesOnly = userTablesOnly; }

    bool getAllowGeometrylessTables() const { return mAllowGeometrylessTables; }
    void setAllowGeometrylessTables(bool allowGeometrylessTables) {
      mAllowGeometrylessTables = allowGeometrylessTables; }

    bool getEnableSsl() const { return mSslEnabled; }
    void setEnableSsl(bool value) { mSslEnabled = value; }

    const QString& getSslCryptoProvider() const { return mSslCryptoProvider; }
    void setSslCryptoProvider(const QString& value) { mSslCryptoProvider = value; }

    const QString& getSslKeyStore() const { return mSslKeyStore; }
    void setSslKeyStore(const QString& value) { mSslKeyStore = value; }

    const QString& getSslTrustStore() const { return mSslTrustStore; }
    void setSslTrustStore(const QString& value) { mSslTrustStore = value; }

    bool getSslValidateCertificate() const { return mSslValidateCertificate; }
    void setSslValidateCertificate(bool value) { mSslValidateCertificate = value; }

    const QString& getSslHostNameInCertificate() const {
      return mSslHostNameInCertificate; }
    void setSslHostNameInCertificate(const QString& value) {
      mSslHostNameInCertificate = value; }

    QString getPort() const;
    QgsDataSourceUri toDataSourceUri();

    void load();
    void save();

    static QStringList getConnectionNames();
    static QString getSelectedConnection();
    static void setSelectedConnection(const QString& name);
    static void removeConnection(const QString& name);

  private:
    QString getPath() const { return getBaseKey() + mName; }
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
