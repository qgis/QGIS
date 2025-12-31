/***************************************************************************
   qgshanasettings.cpp
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
#include "qgshanasettings.h"

#include "qgis.h"
#include "qgssettings.h"

bool QgsHanaIdentifierType::isValid( uint i ) noexcept
{
  return ( i >= InstanceNumber ) && ( i <= PortNumber );
}

QgsHanaIdentifierType::Value QgsHanaIdentifierType::fromInt( uint i )
{
  Q_ASSERT( isValid( i ) );
  return static_cast<Value>( i );
}

QgsHanaSettings::QgsHanaSettings( const QString &name, bool autoLoad )
  : mName( name )
{
  if ( autoLoad )
    load();
}

QString QgsHanaSettings::port() const
{
  if ( QgsHanaIdentifierType::fromInt( mIdentifierType ) == QgsHanaIdentifierType::InstanceNumber )
  {
    if ( mMultitenant )
      return QString( "3" + mIdentifier + "13" );
    else
      return QString( "3" + mIdentifier + "15" );
  }
  else
    return mIdentifier;
}

QStringList QgsHanaSettings::keyColumns( const QString &schemaName, const QString &objectName ) const
{
  return mKeyColumns.value( schemaName ).value( objectName );
}

void QgsHanaSettings::setKeyColumns( const QString &schemaName, const QString &objectName, const QStringList &columnNames )
{
  if ( columnNames.empty() )
    mKeyColumns[schemaName].remove( objectName );
  else
    mKeyColumns[schemaName][objectName] = columnNames;
}

void QgsHanaSettings::setFromDataSourceUri( const QgsDataSourceUri &uri )
{
  if ( uri.hasParam( u"connectionType"_s ) )
    mConnectionType = static_cast<QgsHanaConnectionType>( uri.param( u"connectionType"_s ).toUInt() );
  switch ( mConnectionType )
  {
    case QgsHanaConnectionType::Dsn:
      mDsn = uri.param( u"dsn"_s );
      break;
    case QgsHanaConnectionType::HostPort:
      mDriver = uri.driver();
      mHost = uri.host();
      mIdentifierType = QgsHanaIdentifierType::PortNumber;
      mIdentifier = uri.port();
      mDatabase = uri.database();
      break;
  }

  mSchema = uri.schema();
  mUserName = uri.username();
  mPassword = uri.password();

  mSslEnabled = false;
  mSslCryptoProvider = QString();
  mSslValidateCertificate = false;
  mSslHostNameInCertificate = QString();
  mSslKeyStore = QString();
  mSslTrustStore = QString();
  if ( uri.hasParam( u"sslEnabled"_s ) )
    mSslEnabled = QVariant( uri.param( u"sslEnabled"_s ) ).toBool();
  if ( uri.hasParam( u"sslCryptoProvider"_s ) )
    mSslCryptoProvider = uri.param( u"sslCryptoProvider"_s );
  if ( uri.hasParam( u"sslValidateCertificate"_s ) )
    mSslValidateCertificate = QVariant( uri.param( u"sslValidateCertificate"_s ) ).toBool();
  if ( uri.hasParam( u"sslHostNameInCertificate"_s ) )
    mSslHostNameInCertificate = uri.param( u"sslHostNameInCertificate"_s );
  if ( uri.hasParam( u"sslKeyStore"_s ) )
    mSslKeyStore = uri.param( u"sslKeyStore"_s );
  if ( uri.hasParam( u"sslTrustStore"_s ) )
    mSslTrustStore = uri.param( u"sslTrustStore"_s );

  mProxyEnabled = false;
  mProxyHttp = false;
  mProxyHost = QString();
  mProxyPort = 1080;
  if ( uri.hasParam( u"proxyEnabled"_s ) )
    mProxyEnabled = QVariant( uri.param( u"proxyEnabled"_s ) ).toBool();
  if ( uri.hasParam( u"proxyHttp"_s ) )
    mProxyHttp = QVariant( uri.param( u"proxyHttp"_s ) ).toBool();
  if ( uri.hasParam( u"proxyHost"_s ) )
    mProxyHost = uri.param( u"proxyHost"_s );
  if ( uri.hasParam( u"proxyPort"_s ) )
    mProxyPort = QVariant( uri.param( u"proxyPort"_s ) ).toUInt();

  mUserTablesOnly = true;
  mAllowGeometrylessTables = false;
  mUseEstimatedMetadata = false;
  mSaveUserName = false;
  mSavePassword = false;
  mAuthcfg = QString();

  if ( uri.hasParam( u"userTablesOnly"_s ) )
    mUserTablesOnly = QVariant( uri.param( u"userTablesOnly"_s ) ).toBool();
  if ( uri.hasParam( u"allowGeometrylessTables"_s ) )
    mAllowGeometrylessTables = QVariant( uri.param( u"allowGeometrylessTables"_s ) ).toBool();
  if ( uri.hasParam( u"estimatedmetadata"_s ) )
    mUseEstimatedMetadata = QVariant( uri.param( u"estimatedmetadata"_s ) ).toBool();
  if ( uri.hasParam( u"saveUsername"_s ) )
    mSaveUserName = QVariant( uri.param( u"saveUsername"_s ) ).toBool();
  if ( uri.hasParam( u"savePassword"_s ) )
    mSavePassword = QVariant( uri.param( u"savePassword"_s ) ).toBool();
  if ( uri.hasParam( u"authcfg"_s ) )
    mAuthcfg = uri.param( u"authcfg"_s );
}

QgsDataSourceUri QgsHanaSettings::toDataSourceUri() const
{
  QgsDataSourceUri uri;
  uri.setUseEstimatedMetadata( mUseEstimatedMetadata );
  uri.setParam( u"connectionType"_s, QString::number( static_cast<uint>( mConnectionType ) ) );
  switch ( mConnectionType )
  {
    case QgsHanaConnectionType::Dsn:
      uri.setParam( u"dsn"_s, mDsn );
      uri.setUsername( mUserName );
      uri.setPassword( mPassword );
      break;
    case QgsHanaConnectionType::HostPort:
      uri.setConnection( mHost, port(), mDatabase, mUserName, mPassword );
      uri.setDriver( mDriver );
      break;
  }

  uri.setSchema( mSchema );

  if ( mSslEnabled )
  {
    uri.setParam( u"sslEnabled"_s, u"true"_s );
    if ( !mSslCryptoProvider.isEmpty() )
      uri.setParam( u"sslCryptoProvider"_s, mSslCryptoProvider );
    uri.setParam( u"sslValidateCertificate"_s, mSslValidateCertificate ? u"true"_s : u"false"_s );
    if ( !mSslHostNameInCertificate.isEmpty() )
      uri.setParam( u"sslHostNameInCertificate"_s, mSslHostNameInCertificate );
    if ( !mSslKeyStore.isEmpty() )
      uri.setParam( u"sslKeyStore"_s, mSslKeyStore );
    if ( !mSslTrustStore.isEmpty() )
      uri.setParam( u"sslTrustStore"_s, mSslTrustStore );
  }

  if ( mProxyEnabled )
  {
    uri.setParam( u"proxyEnabled"_s, u"true"_s );
    if ( mProxyHttp )
      uri.setParam( u"proxyHttp"_s, u"true"_s );
    uri.setParam( u"proxyHost"_s, mProxyHost );
    uri.setParam( u"proxyPort"_s, QString::number( mProxyPort ) );
    if ( !mProxyUsername.isEmpty() )
    {
      uri.setParam( u"proxyUsername"_s, mProxyUsername );
      uri.setParam( u"proxyPassword"_s, mProxyPassword );
    }
  }

  return uri;
}

void QgsHanaSettings::load()
{
  QgsSettings settings;
  const QString key = path();
  mConnectionType = QgsHanaConnectionType::HostPort;
  if ( settings.contains( key + u"/connectionType"_s ) )
    mConnectionType = static_cast<QgsHanaConnectionType>( settings.value( key + u"/connectionType"_s ).toUInt() );
  switch ( mConnectionType )
  {
    case QgsHanaConnectionType::Dsn:
      mDsn = settings.value( key + u"/dsn"_s ).toString();
      break;
    case QgsHanaConnectionType::HostPort:
      mDriver = settings.value( key + u"/driver"_s ).toString();
      mHost = settings.value( key + u"/host"_s ).toString();
      mIdentifierType = settings.value( key + u"/identifierType"_s ).toUInt();
      mIdentifier = settings.value( key + u"/identifier"_s ).toString();
      mMultitenant = settings.value( key + u"/multitenant"_s ).toBool();
      mDatabase = settings.value( key + u"/database"_s ).toString();
      break;
  }
  mSchema = settings.value( key + u"/schema"_s ).toString();
  mAuthcfg = settings.value( key + u"/authcfg"_s ).toString();
  mSaveUserName = settings.value( key + u"/saveUsername"_s, false ).toBool();
  if ( mSaveUserName )
    mUserName = settings.value( key + u"/username"_s ).toString();
  mSavePassword = settings.value( key + u"/savePassword"_s, false ).toBool();
  if ( mSavePassword )
    mPassword = settings.value( key + u"/password"_s ).toString();
  mUserTablesOnly = settings.value( key + u"/userTablesOnly"_s, true ).toBool();
  mAllowGeometrylessTables = settings.value( key + u"/allowGeometrylessTables"_s, false ).toBool();
  mUseEstimatedMetadata = settings.value( key + u"/estimatedMetadata"_s, false ).toBool();

  // SSL parameters
  mSslEnabled = settings.value( key + u"/sslEnabled"_s, false ).toBool();
  mSslCryptoProvider = settings.value( key + u"/sslCryptoProvider"_s ).toString();
  mSslKeyStore = settings.value( key + u"/sslKeyStore"_s ).toString();
  mSslTrustStore = settings.value( key + u"/sslTrustStore"_s ).toString();
  mSslValidateCertificate = settings.value( key + u"/sslValidateCertificate"_s, true ).toBool();
  mSslHostNameInCertificate = settings.value( key + u"/sslHostNameInCertificate"_s ).toString();

  // Proxy parameters
  mProxyEnabled = settings.value( key + u"/proxyEnabled"_s, false ).toBool();
  mProxyHttp = settings.value( key + u"/proxyHttp"_s, false ).toBool();
  mProxyHost = settings.value( key + u"/proxyHost"_s ).toString();
  mProxyPort = settings.value( key + u"/proxyPort"_s ).toUInt();
  mProxyUsername = settings.value( key + u"/proxyUsername"_s ).toString();
  mProxyPassword = settings.value( key + u"/proxyPassword"_s ).toString();

  const QString keysPath = key + u"/keys"_s;
  settings.beginGroup( keysPath );
  const QStringList schemaNames = settings.childGroups();
  if ( !schemaNames.empty() )
  {
    for ( const QString &schemaName : schemaNames )
    {
      const QString schemaKey = keysPath + "/" + schemaName;
      QgsSettings subSettings;
      subSettings.beginGroup( schemaKey );
      const QStringList objectNames = subSettings.childKeys();
      if ( objectNames.empty() )
        continue;
      for ( const QString &objectName : objectNames )
      {
        const QVariant value = subSettings.value( objectName );
        if ( !value.isNull() )
          mKeyColumns[schemaName][objectName] = value.toStringList();
      }
    }
  }
}

void QgsHanaSettings::save()
{
  const QString key( path() );
  QgsSettings settings;

  settings.setValue( key + u"/connectionType"_s, static_cast<uint>( mConnectionType ) );
  switch ( mConnectionType )
  {
    case QgsHanaConnectionType::Dsn:
      settings.setValue( key + u"/dsn"_s, mDsn );
      break;
    case QgsHanaConnectionType::HostPort:
      settings.setValue( key + u"/driver"_s, mDriver );
      settings.setValue( key + u"/host"_s, mHost );
      settings.setValue( key + u"/identifierType"_s, mIdentifierType );
      settings.setValue( key + u"/identifier"_s, mIdentifier );
      settings.setValue( key + u"/multitenant"_s, mMultitenant );
      settings.setValue( key + u"/database"_s, mDatabase );
      break;
  }

  settings.setValue( key + u"/schema"_s, mSchema );
  settings.setValue( key + u"/authcfg"_s, mAuthcfg );
  settings.setValue( key + u"/saveUsername"_s, mSaveUserName );
  settings.setValue( key + u"/username"_s, mSaveUserName ? mUserName : QString() );
  settings.setValue( key + u"/savePassword"_s, mSavePassword );
  settings.setValue( key + u"/password"_s, mSavePassword ? mPassword : QString() );
  settings.setValue( key + u"/userTablesOnly"_s, mUserTablesOnly );
  settings.setValue( key + u"/allowGeometrylessTables"_s, mAllowGeometrylessTables );
  settings.setValue( key + u"/estimatedMetadata"_s, mUseEstimatedMetadata );
  settings.setValue( key + u"/sslEnabled"_s, mSslEnabled );
  settings.setValue( key + u"/sslCryptoProvider"_s, mSslCryptoProvider );
  settings.setValue( key + u"/sslKeyStore"_s, mSslKeyStore );
  settings.setValue( key + u"/sslTrustStore"_s, mSslTrustStore );
  settings.setValue( key + u"/sslValidateCertificate"_s, mSslValidateCertificate );
  settings.setValue( key + u"/sslHostNameInCertificate"_s, mSslHostNameInCertificate );
  settings.setValue( key + u"/proxyEnabled"_s, mProxyEnabled );
  settings.setValue( key + u"/proxyHttp"_s, mProxyHttp );
  settings.setValue( key + u"/proxyHost"_s, mProxyHost );
  settings.setValue( key + u"/proxyPort"_s, mProxyPort );
  settings.setValue( key + u"/proxyUsername"_s, mProxyUsername );
  settings.setValue( key + u"/proxyPassword"_s, mProxyPassword );

  if ( !mKeyColumns.empty() )
  {
    const QString keysPath = key + u"/keys/"_s;
    settings.beginGroup( keysPath );
    const QStringList schemaNames = mKeyColumns.keys();
    for ( const QString &schemaName : schemaNames )
    {
      const auto &schemaKeys = mKeyColumns[schemaName];
      if ( schemaKeys.empty() )
        continue;
      settings.beginGroup( schemaName );
      for ( auto it = schemaKeys.constBegin(); it != schemaKeys.constEnd(); it++ )
        settings.setValue( it.key(), it.value() );
      settings.endGroup();
    }
    settings.endGroup();
  }

  settings.sync();
}

void QgsHanaSettings::removeConnection( const QString &name )
{
  const QString key( getBaseKey() + name );
  QgsSettings settings;
  settings.remove( key + u"/connectionType"_s );
  settings.remove( key + u"/dsn"_s );
  settings.remove( key + u"/driver"_s );
  settings.remove( key + u"/host"_s );
  settings.remove( key + u"/identifierType"_s );
  settings.remove( key + u"/identifier"_s );
  settings.remove( key + u"/multitenant"_s );
  settings.remove( key + u"/database"_s );
  settings.remove( key + u"/schema"_s );
  settings.remove( key + u"/userTablesOnly"_s );
  settings.remove( key + u"/allowGeometrylessTables"_s );
  settings.remove( key + u"/estimatedMetadata"_s );
  settings.remove( key + u"/username"_s );
  settings.remove( key + u"/password"_s );
  settings.remove( key + u"/saveUsername"_s );
  settings.remove( key + u"/savePassword"_s );
  settings.remove( key + u"/authcfg"_s );
  settings.remove( key + u"/sslEnabled"_s );
  settings.remove( key + u"/sslCryptoProvider"_s );
  settings.remove( key + u"/sslKeyStore"_s );
  settings.remove( key + u"/sslTrustStore"_s );
  settings.remove( key + u"/sslValidateCertificate"_s );
  settings.remove( key + u"/sslHostNameInCertificate"_s );
  settings.remove( key + u"/proxyEnabled"_s );
  settings.remove( key + u"/proxyHttp"_s );
  settings.remove( key + u"/proxyHost"_s );
  settings.remove( key + u"/proxyPort"_s );
  settings.remove( key + u"/proxyUsername"_s );
  settings.remove( key + u"/proxyPassword"_s );
  settings.remove( key + u"/keys"_s );
  settings.remove( key );
  settings.sync();
}

void QgsHanaSettings::duplicateConnection( const QString &src, const QString &dst )
{
  const QString key( getBaseKey() + src );
  const QString newKey( getBaseKey() + dst );

  QgsSettings settings;
  settings.setValue( newKey + u"/connectionType"_s, settings.value( key + u"/connectionType"_s ).toUInt() );
  settings.setValue( newKey + u"/dsn"_s, settings.value( key + u"/dsn"_s ).toString() );
  settings.setValue( newKey + u"/driver"_s, settings.value( key + u"/driver"_s ).toString() );
  settings.setValue( newKey + u"/host"_s, settings.value( key + u"/host"_s ).toString() );
  settings.setValue( newKey + u"/identifierType"_s, settings.value( key + u"/identifierType"_s ).toUInt() );
  settings.setValue( newKey + u"/identifier"_s, settings.value( key + u"/identifier"_s ).toString() );
  settings.setValue( newKey + u"/multitenant"_s, settings.value( key + u"/multitenant"_s ).toBool() );
  settings.setValue( newKey + u"/database"_s, settings.value( key + u"/database"_s ).toString() );
  settings.setValue( newKey + u"/schema"_s, settings.value( key + u"/schema"_s ).toString() );
  settings.setValue( newKey + u"/userTablesOnly"_s, settings.value( key + u"/userTablesOnly"_s ).toBool() );
  settings.setValue( newKey + u"/allowGeometrylessTables"_s, settings.value( key + u"/allowGeometrylessTables"_s ).toBool() );
  settings.setValue( newKey + u"/estimatedMetadata"_s, settings.value( key + u"/estimatedMetadata"_s ).toBool() );
  settings.setValue( newKey + u"/username"_s, settings.value( key + u"/username"_s ).toString() );
  settings.setValue( newKey + u"/password"_s, settings.value( key + u"/password"_s ).toString() );
  settings.setValue( newKey + u"/saveUsername"_s, settings.value( key + u"/saveUsername"_s ).toBool() );
  settings.setValue( newKey + u"/savePassword"_s, settings.value( key + u"/savePassword"_s ).toBool() );
  settings.setValue( newKey + u"/authcfg"_s, settings.value( key + u"/authcfg"_s ).toString() );
  settings.setValue( newKey + u"/sslEnabled"_s, settings.value( key + u"/sslEnabled"_s ).toBool() );
  settings.setValue( newKey + u"/sslCryptoProvider"_s, settings.value( key + u"/sslCryptoProvider"_s ).toString() );
  settings.setValue( newKey + u"/sslKeyStore"_s, settings.value( key + u"/sslKeyStore"_s ).toString() );
  settings.setValue( newKey + u"/sslTrustStore"_s, settings.value( key + u"/sslTrustStore"_s ).toString() );
  settings.setValue( newKey + u"/sslValidateCertificate"_s, settings.value( key + u"/sslValidateCertificate"_s ).toBool() );
  settings.setValue( newKey + u"/sslHostNameInCertificate"_s, settings.value( key + u"/sslHostNameInCertificate"_s ).toString() );
  settings.setValue( newKey + u"/proxyEnabled"_s, settings.value( key + u"/proxyEnabled"_s ).toBool() );
  settings.setValue( newKey + u"/proxyHttp"_s, settings.value( key + u"/proxyHttp"_s ).toBool() );
  settings.setValue( newKey + u"/proxyHost"_s, settings.value( key + u"/proxyHost"_s ).toUInt() );
  settings.setValue( newKey + u"/proxyPort"_s, settings.value( key + u"/proxyPort"_s ).toString() );
  settings.setValue( newKey + u"/proxyUsername"_s, settings.value( key + u"/proxyUsername"_s ).toString() );
  settings.setValue( newKey + u"/proxyPassword"_s, settings.value( key + u"/proxyPassword"_s ).toString() );
  settings.setValue( newKey + u"/keys"_s, settings.value( key + u"/keys"_s ) );
  settings.sync();
}

QStringList QgsHanaSettings::getConnectionNames()
{
  QgsSettings settings;
  settings.beginGroup( getBaseKey() );
  return settings.childGroups();
}

QString QgsHanaSettings::getSelectedConnection()
{
  const QgsSettings settings;
  return settings.value( getBaseKey() + u"selected"_s ).toString();
}

void QgsHanaSettings::setSelectedConnection( const QString &name )
{
  QgsSettings settings;
  settings.setValue( getBaseKey() + u"selected"_s, name );
}
