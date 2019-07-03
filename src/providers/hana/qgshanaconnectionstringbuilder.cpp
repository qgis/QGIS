/***************************************************************************
   qgsghanaconnectionstringbuilder.cpp
   --------------------------------------
   Date      : 31-05-2019
   Copyright : (C) SAP SE
   Author    : Maksim Rylov
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
  if (!uri.hasParam("encrypt"))
    return;

  mSslEnabled = (uri.param("encrypt") == "true") ? true : false;
  mSslCryptoProvider = uri.param("sslCryptoProvider");
  mSslValidateCertificate = uri.param("sslValidateCertificate") == "true" ? true : false;
  if (mSslValidateCertificate)
    mSslHostNameInCertificate = uri.param("sslHostNameInCertificate");
  mSslKeyStore = uri.param("sslKeyStore");
  mSslTrustStore = uri.param("sslTrustStore");
}

QString QgsHanaConnectionStringBuilder::toString()
{
  QString ret = QString("DRIVER={%1};SERVERNODE=%2:%3;DATABASENAME=%4;UID=%5;PWD=%6").arg(
      mDriver, mHost, mPort, mDatabase, mUserName, mPassword);
  if (!mSchema.isEmpty())
    ret += ";CURRENTSCHEMA=" + mSchema;
  if (mSslEnabled)
  {
    ret += ";encrypt=true";
    ret += ";sslCryptoProvider=" + mSslCryptoProvider;
    ret += ";sslValidateCertificate=" + QString(mSslValidateCertificate ? "true" : "false");
    if (mSslValidateCertificate)
      ret += ";sslHostNameInCertificate=" + mSslValidateCertificate;
    if (!mSslKeyStore.isEmpty())
      ret += ";sslKeyStore=" + mSslKeyStore;
    if (!mSslKeyStore.isEmpty())
      ret += ";sslTrustStore=" + mSslTrustStore;
  }
  return ret;
}
