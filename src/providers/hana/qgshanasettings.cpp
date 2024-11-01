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
#include "qgis.h"
#include "qgshanasettings.h"
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
  if ( uri.hasParam( QStringLiteral( "connectionType" ) ) )
    mConnectionType = static_cast<QgsHanaConnectionType>( uri.param( QStringLiteral( "connectionType" ) ).toUInt() );
  switch ( mConnectionType )
  {
    case QgsHanaConnectionType::Dsn:
      mDsn = uri.param( QStringLiteral( "dsn" ) );
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
  if ( uri.hasParam( QStringLiteral( "sslEnabled" ) ) )
    mSslEnabled = QVariant( uri.param( QStringLiteral( "sslEnabled" ) ) ).toBool();
  if ( uri.hasParam( QStringLiteral( "sslCryptoProvider" ) ) )
    mSslCryptoProvider = uri.param( QStringLiteral( "sslCryptoProvider" ) );
  if ( uri.hasParam( QStringLiteral( "sslValidateCertificate" ) ) )
    mSslValidateCertificate = QVariant( uri.param( QStringLiteral( "sslValidateCertificate" ) ) ).toBool();
  if ( uri.hasParam( QStringLiteral( "sslHostNameInCertificate" ) ) )
    mSslHostNameInCertificate = uri.param( QStringLiteral( "sslHostNameInCertificate" ) );
  if ( uri.hasParam( QStringLiteral( "sslKeyStore" ) ) )
    mSslKeyStore = uri.param( QStringLiteral( "sslKeyStore" ) );
  if ( uri.hasParam( QStringLiteral( "sslTrustStore" ) ) )
    mSslTrustStore = uri.param( QStringLiteral( "sslTrustStore" ) );

  mProxyEnabled = false;
  mProxyHttp = false;
  mProxyHost = QString();
  mProxyPort = 1080;
  if ( uri.hasParam( QStringLiteral( "proxyEnabled" ) ) )
    mProxyEnabled = QVariant( uri.param( QStringLiteral( "proxyEnabled" ) ) ).toBool();
  if ( uri.hasParam( QStringLiteral( "proxyHttp" ) ) )
    mProxyHttp = QVariant( uri.param( QStringLiteral( "proxyHttp" ) ) ).toBool();
  if ( uri.hasParam( QStringLiteral( "proxyHost" ) ) )
    mProxyHost = uri.param( QStringLiteral( "proxyHost" ) );
  if ( uri.hasParam( QStringLiteral( "proxyPort" ) ) )
    mProxyPort = QVariant( uri.param( QStringLiteral( "proxyPort" ) ) ).toUInt();

  mUserTablesOnly = true;
  mAllowGeometrylessTables = false;
  mUseEstimatedMetadata = false;
  mSaveUserName = false;
  mSavePassword = false;
  mAuthcfg = QString();

  if ( uri.hasParam( QStringLiteral( "userTablesOnly" ) ) )
    mUserTablesOnly = QVariant( uri.param( QStringLiteral( "userTablesOnly" ) ) ).toBool();
  if ( uri.hasParam( QStringLiteral( "allowGeometrylessTables" ) ) )
    mAllowGeometrylessTables = QVariant( uri.param( QStringLiteral( "allowGeometrylessTables" ) ) ).toBool();
  if ( uri.hasParam( QStringLiteral( "estimatedmetadata" ) ) )
    mUseEstimatedMetadata = QVariant( uri.param( QStringLiteral( "estimatedmetadata" ) ) ).toBool();
  if ( uri.hasParam( QStringLiteral( "saveUsername" ) ) )
    mSaveUserName = QVariant( uri.param( QStringLiteral( "saveUsername" ) ) ).toBool();
  if ( uri.hasParam( QStringLiteral( "savePassword" ) ) )
    mSavePassword = QVariant( uri.param( QStringLiteral( "savePassword" ) ) ).toBool();
  if ( uri.hasParam( QStringLiteral( "authcfg" ) ) )
    mAuthcfg = uri.param( QStringLiteral( "authcfg" ) );
}

QgsDataSourceUri QgsHanaSettings::toDataSourceUri() const
{
  QgsDataSourceUri uri;
  uri.setUseEstimatedMetadata( mUseEstimatedMetadata );
  uri.setParam( QStringLiteral( "connectionType" ), QString::number( static_cast<uint>( mConnectionType ) ) );
  switch ( mConnectionType )
  {
    case QgsHanaConnectionType::Dsn:
      uri.setParam( QStringLiteral( "dsn" ), mDsn );
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
    uri.setParam( QStringLiteral( "sslEnabled" ), QStringLiteral( "true" ) );
    if ( !mSslCryptoProvider.isEmpty() )
      uri.setParam( QStringLiteral( "sslCryptoProvider" ), mSslCryptoProvider );
    uri.setParam( QStringLiteral( "sslValidateCertificate" ), mSslValidateCertificate ? QStringLiteral( "true" ) : QStringLiteral( "false" ) );
    if ( !mSslHostNameInCertificate.isEmpty() )
      uri.setParam( QStringLiteral( "sslHostNameInCertificate" ), mSslHostNameInCertificate );
    if ( !mSslKeyStore.isEmpty() )
      uri.setParam( QStringLiteral( "sslKeyStore" ), mSslKeyStore );
    if ( !mSslTrustStore.isEmpty() )
      uri.setParam( QStringLiteral( "sslTrustStore" ), mSslTrustStore );
  }

  if ( mProxyEnabled )
  {
    uri.setParam( QStringLiteral( "proxyEnabled" ), QStringLiteral( "true" ) );
    if ( mProxyHttp )
      uri.setParam( QStringLiteral( "proxyHttp" ), QStringLiteral( "true" ) );
    uri.setParam( QStringLiteral( "proxyHost" ), mProxyHost );
    uri.setParam( QStringLiteral( "proxyPort" ), QString::number( mProxyPort ) );
    if ( !mProxyUsername.isEmpty() )
    {
      uri.setParam( QStringLiteral( "proxyUsername" ), mProxyUsername );
      uri.setParam( QStringLiteral( "proxyPassword" ), mProxyPassword );
    }
  }

  return uri;
}

void QgsHanaSettings::load()
{
  QgsSettings settings;
  const QString key = path();
  mConnectionType = QgsHanaConnectionType::HostPort;
  if ( settings.contains( key + QStringLiteral( "/connectionType" ) ) )
    mConnectionType = static_cast<QgsHanaConnectionType>( settings.value( key + QStringLiteral( "/connectionType" ) ).toUInt() );
  switch ( mConnectionType )
  {
    case QgsHanaConnectionType::Dsn:
      mDsn = settings.value( key + QStringLiteral( "/dsn" ) ).toString();
      break;
    case QgsHanaConnectionType::HostPort:
      mDriver = settings.value( key + QStringLiteral( "/driver" ) ).toString();
      mHost = settings.value( key + QStringLiteral( "/host" ) ).toString();
      mIdentifierType = settings.value( key + QStringLiteral( "/identifierType" ) ).toUInt();
      mIdentifier = settings.value( key + QStringLiteral( "/identifier" ) ).toString();
      mMultitenant = settings.value( key + QStringLiteral( "/multitenant" ) ).toBool();
      mDatabase = settings.value( key + QStringLiteral( "/database" ) ).toString();
      break;
  }
  mSchema = settings.value( key + QStringLiteral( "/schema" ) ).toString();
  mAuthcfg = settings.value( key + QStringLiteral( "/authcfg" ) ).toString();
  mSaveUserName = settings.value( key + QStringLiteral( "/saveUsername" ), false ).toBool();
  if ( mSaveUserName )
    mUserName = settings.value( key + QStringLiteral( "/username" ) ).toString();
  mSavePassword = settings.value( key + QStringLiteral( "/savePassword" ), false ).toBool();
  if ( mSavePassword )
    mPassword = settings.value( key + QStringLiteral( "/password" ) ).toString();
  mUserTablesOnly = settings.value( key + QStringLiteral( "/userTablesOnly" ), true ).toBool();
  mAllowGeometrylessTables = settings.value( key + QStringLiteral( "/allowGeometrylessTables" ), false ).toBool();
  mUseEstimatedMetadata = settings.value( key + QStringLiteral( "/estimatedMetadata" ), false ).toBool();

  // SSL parameters
  mSslEnabled = settings.value( key + QStringLiteral( "/sslEnabled" ), false ).toBool();
  mSslCryptoProvider = settings.value( key + QStringLiteral( "/sslCryptoProvider" ) ).toString();
  mSslKeyStore = settings.value( key + QStringLiteral( "/sslKeyStore" ) ).toString();
  mSslTrustStore = settings.value( key + QStringLiteral( "/sslTrustStore" ) ).toString();
  mSslValidateCertificate = settings.value( key + QStringLiteral( "/sslValidateCertificate" ), true ).toBool();
  mSslHostNameInCertificate = settings.value( key + QStringLiteral( "/sslHostNameInCertificate" ) ).toString();

  // Proxy parameters
  mProxyEnabled = settings.value( key + QStringLiteral( "/proxyEnabled" ), false ).toBool();
  mProxyHttp = settings.value( key + QStringLiteral( "/proxyHttp" ), false ).toBool();
  mProxyHost = settings.value( key + QStringLiteral( "/proxyHost" ) ).toString();
  mProxyPort = settings.value( key + QStringLiteral( "/proxyPort" ) ).toUInt();
  mProxyUsername = settings.value( key + QStringLiteral( "/proxyUsername" ) ).toString();
  mProxyPassword = settings.value( key + QStringLiteral( "/proxyPassword" ) ).toString();

  const QString keysPath = key + QStringLiteral( "/keys" );
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

  settings.setValue( key + QStringLiteral( "/connectionType" ), static_cast<uint>( mConnectionType ) );
  switch ( mConnectionType )
  {
    case QgsHanaConnectionType::Dsn:
      settings.setValue( key + QStringLiteral( "/dsn" ), mDsn );
      break;
    case QgsHanaConnectionType::HostPort:
      settings.setValue( key + QStringLiteral( "/driver" ), mDriver );
      settings.setValue( key + QStringLiteral( "/host" ), mHost );
      settings.setValue( key + QStringLiteral( "/identifierType" ), mIdentifierType );
      settings.setValue( key + QStringLiteral( "/identifier" ), mIdentifier );
      settings.setValue( key + QStringLiteral( "/multitenant" ), mMultitenant );
      settings.setValue( key + QStringLiteral( "/database" ), mDatabase );
      break;
  }

  settings.setValue( key + QStringLiteral( "/schema" ), mSchema );
  settings.setValue( key + QStringLiteral( "/authcfg" ), mAuthcfg );
  settings.setValue( key + QStringLiteral( "/saveUsername" ), mSaveUserName );
  settings.setValue( key + QStringLiteral( "/username" ), mSaveUserName ? mUserName : QString() );
  settings.setValue( key + QStringLiteral( "/savePassword" ), mSavePassword );
  settings.setValue( key + QStringLiteral( "/password" ), mSavePassword ? mPassword : QString() );
  settings.setValue( key + QStringLiteral( "/userTablesOnly" ), mUserTablesOnly );
  settings.setValue( key + QStringLiteral( "/allowGeometrylessTables" ), mAllowGeometrylessTables );
  settings.setValue( key + QStringLiteral( "/estimatedMetadata" ), mUseEstimatedMetadata );
  settings.setValue( key + QStringLiteral( "/sslEnabled" ), mSslEnabled );
  settings.setValue( key + QStringLiteral( "/sslCryptoProvider" ), mSslCryptoProvider );
  settings.setValue( key + QStringLiteral( "/sslKeyStore" ), mSslKeyStore );
  settings.setValue( key + QStringLiteral( "/sslTrustStore" ), mSslTrustStore );
  settings.setValue( key + QStringLiteral( "/sslValidateCertificate" ), mSslValidateCertificate );
  settings.setValue( key + QStringLiteral( "/sslHostNameInCertificate" ), mSslHostNameInCertificate );
  settings.setValue( key + QStringLiteral( "/proxyEnabled" ), mProxyEnabled );
  settings.setValue( key + QStringLiteral( "/proxyHttp" ), mProxyHttp );
  settings.setValue( key + QStringLiteral( "/proxyHost" ), mProxyHost );
  settings.setValue( key + QStringLiteral( "/proxyPort" ), mProxyPort );
  settings.setValue( key + QStringLiteral( "/proxyUsername" ), mProxyUsername );
  settings.setValue( key + QStringLiteral( "/proxyPassword" ), mProxyPassword );

  if ( !mKeyColumns.empty() )
  {
    const QString keysPath = key + QStringLiteral( "/keys/" );
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
  settings.remove( key + QStringLiteral( "/connectionType" ) );
  settings.remove( key + QStringLiteral( "/dsn" ) );
  settings.remove( key + QStringLiteral( "/driver" ) );
  settings.remove( key + QStringLiteral( "/host" ) );
  settings.remove( key + QStringLiteral( "/identifierType" ) );
  settings.remove( key + QStringLiteral( "/identifier" ) );
  settings.remove( key + QStringLiteral( "/multitenant" ) );
  settings.remove( key + QStringLiteral( "/database" ) );
  settings.remove( key + QStringLiteral( "/schema" ) );
  settings.remove( key + QStringLiteral( "/userTablesOnly" ) );
  settings.remove( key + QStringLiteral( "/allowGeometrylessTables" ) );
  settings.remove( key + QStringLiteral( "/estimatedMetadata" ) );
  settings.remove( key + QStringLiteral( "/username" ) );
  settings.remove( key + QStringLiteral( "/password" ) );
  settings.remove( key + QStringLiteral( "/saveUsername" ) );
  settings.remove( key + QStringLiteral( "/savePassword" ) );
  settings.remove( key + QStringLiteral( "/authcfg" ) );
  settings.remove( key + QStringLiteral( "/sslEnabled" ) );
  settings.remove( key + QStringLiteral( "/sslCryptoProvider" ) );
  settings.remove( key + QStringLiteral( "/sslKeyStore" ) );
  settings.remove( key + QStringLiteral( "/sslTrustStore" ) );
  settings.remove( key + QStringLiteral( "/sslValidateCertificate" ) );
  settings.remove( key + QStringLiteral( "/sslHostNameInCertificate" ) );
  settings.remove( key + QStringLiteral( "/proxyEnabled" ) );
  settings.remove( key + QStringLiteral( "/proxyHttp" ) );
  settings.remove( key + QStringLiteral( "/proxyHost" ) );
  settings.remove( key + QStringLiteral( "/proxyPort" ) );
  settings.remove( key + QStringLiteral( "/proxyUsername" ) );
  settings.remove( key + QStringLiteral( "/proxyPassword" ) );
  settings.remove( key + QStringLiteral( "/keys" ) );
  settings.remove( key );
  settings.sync();
}

void QgsHanaSettings::duplicateConnection( const QString &src, const QString &dst )
{
  const QString key( getBaseKey() + src );
  const QString newKey( getBaseKey() + dst );

  QgsSettings settings;
  settings.setValue( newKey + QStringLiteral( "/connectionType" ), settings.value( key + QStringLiteral( "/connectionType" ) ).toUInt() );
  settings.setValue( newKey + QStringLiteral( "/dsn" ), settings.value( key + QStringLiteral( "/dsn" ) ).toString() );
  settings.setValue( newKey + QStringLiteral( "/driver" ), settings.value( key + QStringLiteral( "/driver" ) ).toString() );
  settings.setValue( newKey + QStringLiteral( "/host" ), settings.value( key + QStringLiteral( "/host" ) ).toString() );
  settings.setValue( newKey + QStringLiteral( "/identifierType" ), settings.value( key + QStringLiteral( "/identifierType" ) ).toUInt() );
  settings.setValue( newKey + QStringLiteral( "/identifier" ), settings.value( key + QStringLiteral( "/identifier" ) ).toString() );
  settings.setValue( newKey + QStringLiteral( "/multitenant" ), settings.value( key + QStringLiteral( "/multitenant" ) ).toBool() );
  settings.setValue( newKey + QStringLiteral( "/database" ), settings.value( key + QStringLiteral( "/database" ) ).toString() );
  settings.setValue( newKey + QStringLiteral( "/schema" ), settings.value( key + QStringLiteral( "/schema" ) ).toString() );
  settings.setValue( newKey + QStringLiteral( "/userTablesOnly" ), settings.value( key + QStringLiteral( "/userTablesOnly" ) ).toBool() );
  settings.setValue( newKey + QStringLiteral( "/allowGeometrylessTables" ), settings.value( key + QStringLiteral( "/allowGeometrylessTables" ) ).toBool() );
  settings.setValue( newKey + QStringLiteral( "/estimatedMetadata" ), settings.value( key + QStringLiteral( "/estimatedMetadata" ) ).toBool() );
  settings.setValue( newKey + QStringLiteral( "/username" ), settings.value( key + QStringLiteral( "/username" ) ).toString() );
  settings.setValue( newKey + QStringLiteral( "/password" ), settings.value( key + QStringLiteral( "/password" ) ).toString() );
  settings.setValue( newKey + QStringLiteral( "/saveUsername" ), settings.value( key + QStringLiteral( "/saveUsername" ) ).toBool() );
  settings.setValue( newKey + QStringLiteral( "/savePassword" ), settings.value( key + QStringLiteral( "/savePassword" ) ).toBool() );
  settings.setValue( newKey + QStringLiteral( "/authcfg" ), settings.value( key + QStringLiteral( "/authcfg" ) ).toString() );
  settings.setValue( newKey + QStringLiteral( "/sslEnabled" ), settings.value( key + QStringLiteral( "/sslEnabled" ) ).toBool() );
  settings.setValue( newKey + QStringLiteral( "/sslCryptoProvider" ), settings.value( key + QStringLiteral( "/sslCryptoProvider" ) ).toString() );
  settings.setValue( newKey + QStringLiteral( "/sslKeyStore" ), settings.value( key + QStringLiteral( "/sslKeyStore" ) ).toString() );
  settings.setValue( newKey + QStringLiteral( "/sslTrustStore" ), settings.value( key + QStringLiteral( "/sslTrustStore" ) ).toString() );
  settings.setValue( newKey + QStringLiteral( "/sslValidateCertificate" ), settings.value( key + QStringLiteral( "/sslValidateCertificate" ) ).toBool() );
  settings.setValue( newKey + QStringLiteral( "/sslHostNameInCertificate" ), settings.value( key + QStringLiteral( "/sslHostNameInCertificate" ) ).toString() );
  settings.setValue( newKey + QStringLiteral( "/proxyEnabled" ), settings.value( key + QStringLiteral( "/proxyEnabled" ) ).toBool() );
  settings.setValue( newKey + QStringLiteral( "/proxyHttp" ), settings.value( key + QStringLiteral( "/proxyHttp" ) ).toBool() );
  settings.setValue( newKey + QStringLiteral( "/proxyHost" ), settings.value( key + QStringLiteral( "/proxyHost" ) ).toUInt() );
  settings.setValue( newKey + QStringLiteral( "/proxyPort" ), settings.value( key + QStringLiteral( "/proxyPort" ) ).toString() );
  settings.setValue( newKey + QStringLiteral( "/proxyUsername" ), settings.value( key + QStringLiteral( "/proxyUsername" ) ).toString() );
  settings.setValue( newKey + QStringLiteral( "/proxyPassword" ), settings.value( key + QStringLiteral( "/proxyPassword" ) ).toString() );
  settings.setValue( newKey + QStringLiteral( "/keys" ), settings.value( key + QStringLiteral( "/keys" ) ) );
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
  return settings.value( getBaseKey() + QStringLiteral( "selected" ) ).toString();
}

void QgsHanaSettings::setSelectedConnection( const QString &name )
{
  QgsSettings settings;
  settings.setValue( getBaseKey() + QStringLiteral( "selected" ), name );
}
