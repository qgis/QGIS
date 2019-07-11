/***************************************************************************
   qgshanasettings.cpp
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
#include "qgshanasettings.h"
#include "qgssettings.h"

bool QgsHanaIdentifierType::isValid(uint8_t i) noexcept
{
  return (QgsHanaIdentifierType::fromInt(i) >= INSTANCE_NUMBER) && (QgsHanaIdentifierType::fromInt(i) <= PORT_NUMBER);
}

QgsHanaIdentifierType::Value QgsHanaIdentifierType::fromInt(uint8_t i)
{
  Q_ASSERT(isValid(i));
  return static_cast<Value>(i);
}

QgsHanaSettings::QgsHanaSettings(const QString& name, bool autoLoad)
 : mName(name)
{
  if (autoLoad)
    load();
}

QString QgsHanaSettings::getPort() const
{
  if (QgsHanaIdentifierType::fromInt(mIdentifierType) == QgsHanaIdentifierType::INSTANCE_NUMBER)
  {
    if (mMultitenant)
    {
      if (mDatabase == QStringLiteral("SYSTEMDB"))
        return QString("3" + mIdentifier + "13");
      else
        return QString("3" + mIdentifier + "41");
    }
    else
      return QString("3" + mIdentifier + "15");
  }
  else
    return mIdentifier;
}

QgsDataSourceUri QgsHanaSettings::toDataSourceUri()
{
  QgsDataSourceUri uri;
  uri.setConnection(mHost, getPort(), mDatabase, mUserName, mPassword);
  uri.setDriver(mDriver);
  uri.setSchema(mSchema);
  if (mSslEnabled)
  {
    uri.setParam(QStringLiteral("encrypt"), QStringLiteral("true"));
    uri.setParam(QStringLiteral("sslCryptoProvider"), mSslCryptoProvider);
    uri.setParam(QStringLiteral("sslValidateCertificate"), mSslValidateCertificate ? QStringLiteral("true") : QStringLiteral("false"));
    if (!mSslHostNameInCertificate.isEmpty())
      uri.setParam(QStringLiteral("sslHostNameInCertificate"), mSslHostNameInCertificate);
    if (!mSslKeyStore.isEmpty())
      uri.setParam(QStringLiteral("sslKeyStore"), mSslKeyStore);
    if (!mSslTrustStore.isEmpty())
      uri.setParam(QStringLiteral("sslTrustStore"), mSslTrustStore);
  }

  return uri;
}

void QgsHanaSettings::load()
{
  QgsSettings settings;
  QString key = getPath();
  mDriver = settings.value(key + "/driver").toString();
  mHost = settings.value(key + "/host").toString();
  mIdentifierType = settings.value(key + "/identifierType").toUInt();
  mIdentifier = settings.value(key + "/identifier").toString();
  mDatabase = settings.value(key + "/database").toString();
  mSchema = settings.value(key + "/schema").toString();
  mAuthcfg = settings.value(key + "/authcfg").toString();
  mSaveUserName = settings.value(key + "/saveUsername", false).toBool();
  if (mSaveUserName)
    mUserName = settings.value(key + "/username").toString();
  mSavePassword = settings.value(key + "/savePassword", false).toBool();
  if (mSavePassword)
    mPassword = settings.value(key + "/password").toString();
  mUserTablesOnly = settings.value(key + "/userTablesOnly", true).toBool();
  mAllowGeometrylessTables = settings.value(key + "/allowGeometrylessTables", false).toBool();
  mSslEnabled = settings.value(key + "/sslEnabled", false).toBool();
  mSslCryptoProvider = settings.value(key + "/sslCryptoProvider").toString();
  mSslKeyStore = settings.value(key + "/sslKeyStore").toString();
  mSslTrustStore = settings.value(key + "/sslTrustStore").toString();
  mSslValidateCertificate = settings.value(key + "/sslValidateCertificate", true).toBool();
  mSslHostNameInCertificate = settings.value(key + "/sslHostNameInCertificate").toString();
}

void QgsHanaSettings::save()
{
  QString key(getPath());
  QgsSettings settings;
  settings.setValue(key + "/driver", mDriver);
  settings.setValue(key + "/host", mHost);
  settings.setValue(key + "/identifierType", mIdentifierType);
  settings.setValue(key + "/identifier", mIdentifier);
  settings.setValue(key + "/database", mDatabase);
  settings.setValue(key + "/schema", mSchema);
  settings.setValue(key + "/authcfg", mAuthcfg);
  settings.setValue(key + "/saveUsername", mSaveUserName);
  settings.setValue(key + "/username", mSaveUserName ? mUserName : QLatin1String(""));
  settings.setValue(key + "/savePassword", mSavePassword);
  settings.setValue(key + "/password", mSavePassword ? mPassword : QLatin1String(""));
  settings.setValue(key + "/userTablesOnly", mUserTablesOnly);
  settings.setValue(key + "/allowGeometrylessTables", mAllowGeometrylessTables);
  settings.setValue(key + "/sslEnabled", mSslEnabled);
  settings.setValue(key + "/sslCryptoProvider", mSslCryptoProvider);
  settings.setValue(key + "/sslKeyStore", mSslKeyStore);
  settings.setValue(key + "/sslTrustStore", mSslTrustStore);
  settings.setValue(key + "/sslValidateCertificate", mSslValidateCertificate);
  settings.setValue(key + "/sslHostNameInCertificate", mSslHostNameInCertificate);
  settings.sync();
}

void QgsHanaSettings::removeConnection(const QString& name)
{
  QString key(getBaseKey() + name);
  QgsSettings settings;
  settings.remove(key + "/driver");
  settings.remove(key + "/host");
  settings.remove(key + "/identifierType");
  settings.remove(key + "/identifier");
  settings.remove(key + "/database");
  settings.remove(key + "/schema");
  settings.remove(key + "/userTablesOnly");
  settings.remove(key + "/allowGeometrylessTables");
  settings.remove(key + "/username");
  settings.remove(key + "/password");
  settings.remove(key + "/saveUsername");
  settings.remove(key + "/savePassword");
  settings.remove(key + "/authcfg");
  settings.remove(key + "/sslEnabled");
  settings.remove(key + "/sslCryptoProvider");
  settings.remove(key + "/sslKeyStore");
  settings.remove(key + "/sslTrustStore");
  settings.remove(key + "/sslValidateCertificate");
  settings.remove(key + "/sslHostNameInCertificate");
  settings.remove(key);
  settings.sync();
}

QStringList QgsHanaSettings::getConnectionNames()
{
  QgsSettings settings;
  settings.beginGroup(getBaseKey());
  return settings.childGroups();
}

QString QgsHanaSettings::getSelectedConnection()
{
  QgsSettings settings;
  return settings.value(getBaseKey() + "selected").toString();
}

void QgsHanaSettings::setSelectedConnection(const QString& name)
{
  QgsSettings settings;
  settings.setValue(getBaseKey() + "selected", name);
}
