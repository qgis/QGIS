/***************************************************************************
   qgsghanaconnectionstringbuilder.cpp
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
#include "qgshanaconnectionstringbuilder.h"

QgsHanaConnectionStringBuilder::QgsHanaConnectionStringBuilder( const QgsDataSourceUri &uri )
  : mDriver( uri.driver() )
  , mHost( uri.host() )
  , mPort( uri.port() )
  , mDatabase( uri.database() )
  , mUserName( uri.username() )
  , mPassword( uri.password() )
{
  if ( uri.hasParam( QStringLiteral( "dsn" ) ) )
    mDsn = uri.param( QStringLiteral( "dsn" ) );

  // SSL parameters
  mSslEnabled = ( uri.param( QStringLiteral( "sslEnabled" ) ) == QLatin1String( "true" ) );
  if ( mSslEnabled )
  {
    mSslCryptoProvider = uri.param( QStringLiteral( "sslCryptoProvider" ) );
    mSslValidateCertificate = uri.param( QStringLiteral( "sslValidateCertificate" ) ) == QLatin1String( "true" );
    if ( mSslValidateCertificate )
      mSslHostNameInCertificate = uri.param( QStringLiteral( "sslHostNameInCertificate" ) );
    mSslKeyStore = uri.param( QStringLiteral( "sslKeyStore" ) );
    mSslTrustStore = uri.param( QStringLiteral( "sslTrustStore" ) );
  }
  // Proxy parameters
  mProxyEnabled = ( uri.param( QStringLiteral( "proxyEnabled" ) ) == QLatin1String( "true" ) );
  if ( mProxyEnabled )
  {
    mProxyHttp = ( uri.param( QStringLiteral( "proxyHttp" ) ) == QLatin1String( "true" ) );
    mProxyHost = uri.param( QStringLiteral( "proxyHost" ) );
    mProxyPort = QVariant( uri.param( QStringLiteral( "proxyPort" ) ) ).toUInt();
    mProxyUsername = uri.param( QStringLiteral( "proxyUsername" ) );
    mProxyPassword = uri.param( QStringLiteral( "proxyPassword" ) );
  }
}

QString QgsHanaConnectionStringBuilder::toString() const
{
  // For more details on how to escape special characters in passwords,
  // see https://stackoverflow.com/questions/55150362/maybe-illegal-character-in-odbc-sql-server-connection-string-pwd
  const QString pwd = QString( mPassword ).replace( '}', QLatin1String( "}}" ) );
  QString ret = !mDsn.isEmpty()
                ? QStringLiteral( "DSN=%1;UID={%2};PWD={%3};CHAR_AS_UTF8=1" ).arg(
                  mDsn, mUserName, pwd )
                : QStringLiteral( "DRIVER={%1};SERVERNODE=%2:%3;DATABASENAME=%4;UID={%5};PWD={%6};CHAR_AS_UTF8=1" ).arg(
                  mDriver, mHost, mPort, mDatabase, mUserName, pwd );
  if ( !mSchema.isEmpty() )
    ret += QStringLiteral( ";CURRENTSCHEMA=" ) + mSchema;
  if ( mSslEnabled )
  {
    ret += QLatin1String( ";encrypt=true" );
    if ( !mSslCryptoProvider.isEmpty() )
      ret += QStringLiteral( ";sslCryptoProvider=" ) + mSslCryptoProvider;
    ret += QStringLiteral( ";sslValidateCertificate=" ) + QString( mSslValidateCertificate ? QStringLiteral( "true" ) : QStringLiteral( "false" ) );
    if ( !mSslHostNameInCertificate.isEmpty() )
      ret += QStringLiteral( ";sslHostNameInCertificate=" ) + mSslHostNameInCertificate;
    if ( !mSslKeyStore.isEmpty() )
      ret += QStringLiteral( ";sslKeyStore=" ) + mSslKeyStore;
    if ( !mSslKeyStore.isEmpty() )
      ret += QStringLiteral( ";sslTrustStore=" ) + mSslTrustStore;
  }

  if ( mProxyEnabled )
  {
    if ( mProxyHttp )
      ret += QStringLiteral( ";proxyHttp=TRUE" );
    ret += QStringLiteral( ";proxyHostname=" ) + mProxyHost;
    ret += QStringLiteral( ";proxyPort=" ) + QString::number( mProxyPort );
    if ( !mProxyUsername.isEmpty() )
    {
      ret += QStringLiteral( ";proxyUserName=" ) + mProxyUsername;
      ret += QStringLiteral( ";proxyPassword=" ) + mProxyPassword;
    }
  }

  return ret;
}
