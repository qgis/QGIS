/***************************************************************************
   qgsghanaconnectionstringbuilder.cpp
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
#include "qgshanaconnectionstringbuilder.h"

QgsHanaConnectionStringBuilder::QgsHanaConnectionStringBuilder(const QgsDataSourceUri& uri)
  : mDriver(uri.driver())
  , mHost(uri.host())
  , mPort(uri.port())
  , mDatabase(uri.database())
  , mUserName(uri.username())
  , mPassword(uri.password())
  , mSslEnabled(false)
{
  if (!uri.hasParam(QStringLiteral("encrypt")))
    return;

  mSslEnabled = (uri.param(QStringLiteral("encrypt")) == QStringLiteral("true")) ? true : false;
  mSslCryptoProvider = uri.param(QStringLiteral("sslCryptoProvider"));
  mSslValidateCertificate = uri.param(QStringLiteral("sslValidateCertificate")) == QStringLiteral("true") ? true : false;
  if (mSslValidateCertificate)
    mSslHostNameInCertificate = uri.param(QStringLiteral("sslHostNameInCertificate"));
  mSslKeyStore = uri.param(QStringLiteral("sslKeyStore"));
  mSslTrustStore = uri.param(QStringLiteral("sslTrustStore"));
}

QString QgsHanaConnectionStringBuilder::toString()
{
  QString ret = QStringLiteral("DRIVER={%1};SERVERNODE=%2:%3;DATABASENAME=%4;UID=%5;PWD=%6;CHAR_AS_UTF8=1").arg(
      mDriver, mHost, mPort, mDatabase, mUserName, mPassword);
  if (!mSchema.isEmpty())
    ret += QStringLiteral(";CURRENTSCHEMA=") + mSchema;
  if (mSslEnabled)
  {
    ret += QStringLiteral(";encrypt=true");
    ret += QStringLiteral(";sslCryptoProvider=") + mSslCryptoProvider;
    ret += QStringLiteral(";sslValidateCertificate=") + QString(mSslValidateCertificate ? QStringLiteral("true") : QStringLiteral("false"));
    if (mSslValidateCertificate)
      ret += QStringLiteral(";sslHostNameInCertificate=") + mSslValidateCertificate;
    if (!mSslKeyStore.isEmpty())
      ret += QStringLiteral(";sslKeyStore=") + mSslKeyStore;
    if (!mSslKeyStore.isEmpty())
      ret += QStringLiteral(";sslTrustStore=") + mSslTrustStore;
  }
  return ret;
}
