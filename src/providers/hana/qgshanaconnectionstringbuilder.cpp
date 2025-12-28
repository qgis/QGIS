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

#include "qgis.h"

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
  if ( uri.hasParam( u"dsn"_s ) )
    mDsn = uri.param( u"dsn"_s );

  // SSL parameters
  mSslEnabled = ( uri.param( u"sslEnabled"_s ) == "true"_L1 );
  if ( mSslEnabled )
  {
    mSslCryptoProvider = uri.param( u"sslCryptoProvider"_s );
    mSslValidateCertificate = uri.param( u"sslValidateCertificate"_s ) == "true"_L1;
    if ( mSslValidateCertificate )
      mSslHostNameInCertificate = uri.param( u"sslHostNameInCertificate"_s );
    mSslKeyStore = uri.param( u"sslKeyStore"_s );
    mSslTrustStore = uri.param( u"sslTrustStore"_s );
  }
  // Proxy parameters
  mProxyEnabled = ( uri.param( u"proxyEnabled"_s ) == "true"_L1 );
  if ( mProxyEnabled )
  {
    mProxyHttp = ( uri.param( u"proxyHttp"_s ) == "true"_L1 );
    mProxyHost = uri.param( u"proxyHost"_s );
    mProxyPort = QVariant( uri.param( u"proxyPort"_s ) ).toUInt();
    mProxyUsername = uri.param( u"proxyUsername"_s );
    mProxyPassword = uri.param( u"proxyPassword"_s );
  }
}

QString QgsHanaConnectionStringBuilder::toString() const
{
  QStringList props;

  // See notes for constructing connection string for HANA
  // https://help.sap.com/docs/SAP_HANA_CLIENT/f1b440ded6144a54ada97ff95dac7adf/7cab593774474f2f8db335710b2f5c50.html
  QRegularExpression rxSpecialChars( "\\[|\\]|\\{|\\}|\\(|\\)|\\,|\\;|\\?|\\*|\\=|\\!|\\@" );
  auto addProperty = [&props, &rxSpecialChars]( const QString &name, const QString &value ) {
    if ( value.isEmpty() )
      return;

    QRegularExpressionMatch match = rxSpecialChars.match( value );
    if ( match.hasMatch() )
    {
      QString newValue = QString( value ).replace( '}', "}}"_L1 );
      props.append( name + "={" + newValue + "}" );
    }
    else
    {
      props.append( name + "=" + value );
    }
  };

  if ( !mDsn.isEmpty() )
  {
    addProperty( u"DSN"_s, mDsn );
  }
  else
  {
    addProperty( u"DRIVER"_s, mDriver );
    addProperty( u"SERVERNODE"_s, u"%1:%2"_s.arg( mHost, mPort ) );
    addProperty( u"DATABASENAME"_s, mDatabase );
  }

  addProperty( u"UID"_s, mUserName );
  addProperty( u"PWD"_s, mPassword );
  addProperty( u"CURRENTSCHEMA"_s, mSchema );

  if ( mSslEnabled )
  {
    addProperty( u"encrypt"_s, u"true"_s );
    addProperty( u"sslCryptoProvider"_s, mSslCryptoProvider );
    addProperty( u"sslValidateCertificate"_s, mSslValidateCertificate ? u"true"_s : u"false"_s );
    addProperty( u"sslHostNameInCertificate"_s, mSslHostNameInCertificate );
    addProperty( u"sslKeyStore"_s, mSslKeyStore );
    addProperty( u"sslTrustStore"_s, mSslTrustStore );
  }

  if ( mProxyEnabled )
  {
    if ( mProxyHttp )
      addProperty( u"proxyHttp"_s, u"TRUE"_s );
    addProperty( u"proxyHostname"_s, mProxyHost );
    addProperty( u"proxyPort"_s, QString::number( mProxyPort ) );
    if ( !mProxyUsername.isEmpty() )
    {
      addProperty( u"proxyUserName"_s, mProxyUsername );
      addProperty( u"proxyPassword"_s, mProxyPassword );
    }
  }

  addProperty( u"CHAR_AS_UTF8"_s, u"1"_s );
  addProperty( u"sessionVariable:APPLICATION"_s, u"QGIS %1"_s.arg( Qgis::version() ) );

  return props.join( QLatin1Char( ';' ) );
}
