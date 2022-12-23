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
#include <QRegularExpression>

QgsHanaConnectionStringBuilder::QgsHanaConnectionStringBuilder( const QgsDataSourceUri &uri )
  : mDriver( uri.driver() )
  , mHost( uri.host() )
  , mPort( uri.port() )
  , mDatabase( uri.database() )
  , mSchema( uri.schema() )
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
  QStringList props;

  // See notes for constructing connection string for HANA
  // https://help.sap.com/docs/SAP_HANA_CLIENT/f1b440ded6144a54ada97ff95dac7adf/7cab593774474f2f8db335710b2f5c50.html
  QRegularExpression rxSpecialChars( "\\[|\\]|\\{|\\}|\\(|\\)|\\,|\\;|\\?|\\*|\\=|\\!|\\@" );
  auto addProperty = [&props, &rxSpecialChars]( const QString & name, const QString & value )
  {
    if ( value.isEmpty() )
      return;

    QRegularExpressionMatch match = rxSpecialChars.match( value );
    if ( match.hasMatch() )
    {
      QString newValue = QString( value ).replace( '}', QLatin1String( "}}" ) );
      props.append( name + "={" + newValue + "}" );
    }
    else
    {
      props.append( name + "=" + value );
    }
  };

  if ( !mDsn.isEmpty() )
  {
    addProperty( QStringLiteral( "DSN" ), mDsn );
  }
  else
  {
    addProperty( QStringLiteral( "DRIVER" ), mDriver );
    addProperty( QStringLiteral( "SERVERNODE" ), QStringLiteral( "%1:%2" ).arg( mHost, mPort ) );
    addProperty( QStringLiteral( "DATABASENAME" ), mDatabase );
  }

  addProperty( QStringLiteral( "UID" ), mUserName );
  addProperty( QStringLiteral( "PWD" ), mPassword );
  addProperty( QStringLiteral( "CURRENTSCHEMA" ), mSchema );

  if ( mSslEnabled )
  {
    addProperty( QStringLiteral( "encrypt" ), QStringLiteral( "true" ) );
    addProperty( QStringLiteral( "sslCryptoProvider" ), mSslCryptoProvider );
    addProperty( QStringLiteral( "sslValidateCertificate" ), mSslValidateCertificate ? QStringLiteral( "true" ) : QStringLiteral( "false" ) );
    addProperty( QStringLiteral( "sslHostNameInCertificate" ), mSslHostNameInCertificate );
    addProperty( QStringLiteral( "sslKeyStore" ), mSslKeyStore );
    addProperty( QStringLiteral( "sslTrustStore" ), mSslTrustStore );
  }

  if ( mProxyEnabled )
  {
    if ( mProxyHttp )
      addProperty( QStringLiteral( "proxyHttp" ), QStringLiteral( "TRUE" ) );
    addProperty( QStringLiteral( "proxyHostname" ), mProxyHost );
    addProperty( QStringLiteral( "proxyPort" ), QString::number( mProxyPort ) );
    if ( !mProxyUsername.isEmpty() )
    {
      addProperty( QStringLiteral( "proxyUserName" ), mProxyUsername );
      addProperty( QStringLiteral( "proxyPassword" ), mProxyPassword );
    }
  }

  addProperty( QStringLiteral( "CHAR_AS_UTF8" ), QStringLiteral( "1" ) );

  return props.join( QLatin1Char( ';' ) );
}
