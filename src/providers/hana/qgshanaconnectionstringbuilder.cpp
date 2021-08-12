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
  if ( !uri.hasParam( QStringLiteral( "sslEnabled" ) ) )
    return;

  mSslEnabled = ( uri.param( QStringLiteral( "sslEnabled" ) ) == QLatin1String( "true" ) );
  mSslCryptoProvider = uri.param( QStringLiteral( "sslCryptoProvider" ) );
  mSslValidateCertificate = uri.param( QStringLiteral( "sslValidateCertificate" ) ) == QLatin1String( "true" );
  if ( mSslValidateCertificate )
    mSslHostNameInCertificate = uri.param( QStringLiteral( "sslHostNameInCertificate" ) );
  mSslKeyStore = uri.param( QStringLiteral( "sslKeyStore" ) );
  mSslTrustStore = uri.param( QStringLiteral( "sslTrustStore" ) );
}

QString QgsHanaConnectionStringBuilder::toString() const
{
  // For more details on how to escape special characters in passwords,
  // see https://stackoverflow.com/questions/55150362/maybe-illegal-character-in-odbc-sql-server-connection-string-pwd
  const QString pwd = QString( mPassword ).replace( '}', QLatin1String( "}}" ) );
  QString ret = QStringLiteral( "DRIVER={%1};SERVERNODE=%2:%3;DATABASENAME=%4;UID=%5;PWD={%6};CHAR_AS_UTF8=1" ).arg(
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
  return ret;
}
