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
  return ( i >= INSTANCE_NUMBER ) && ( i <= PORT_NUMBER );
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
  if ( QgsHanaIdentifierType::fromInt( mIdentifierType ) == QgsHanaIdentifierType::INSTANCE_NUMBER )
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
  mDriver = uri.driver();
  mHost = uri.host();
  mIdentifierType = QgsHanaIdentifierType::PORT_NUMBER;
  mIdentifier = uri.port();
  mSchema = uri.schema();
  mDatabase = uri.database();
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

  mUserTablesOnly = true;
  mAllowGeometrylessTables = false;
  mSaveUserName = false;
  mSavePassword = false;
  mAuthcfg = QString();

  if ( uri.hasParam( QStringLiteral( "userTablesOnly" ) ) )
    mUserTablesOnly = QVariant( uri.param( QStringLiteral( "userTablesOnly" ) ) ).toBool();
  if ( uri.hasParam( QStringLiteral( "allowGeometrylessTables" ) ) )
    mAllowGeometrylessTables = QVariant( uri.param( QStringLiteral( "allowGeometrylessTables" ) ) ).toBool();
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
  uri.setConnection( mHost, port(), mDatabase, mUserName, mPassword );
  uri.setDriver( mDriver );
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

  return uri;
}

void QgsHanaSettings::load()
{
  QgsSettings settings;
  const QString key = path();
  mDriver = settings.value( key + "/driver" ).toString();
  mHost = settings.value( key + "/host" ).toString();
  mIdentifierType = settings.value( key + "/identifierType" ).toUInt();
  mIdentifier = settings.value( key + "/identifier" ).toString();
  mMultitenant = settings.value( key + "/multitenant" ).toBool();
  mDatabase = settings.value( key + "/database" ).toString();
  mSchema = settings.value( key + "/schema" ).toString();
  mAuthcfg = settings.value( key + "/authcfg" ).toString();
  mSaveUserName = settings.value( key + "/saveUsername", false ).toBool();
  if ( mSaveUserName )
    mUserName = settings.value( key + "/username" ).toString();
  mSavePassword = settings.value( key + "/savePassword", false ).toBool();
  if ( mSavePassword )
    mPassword = settings.value( key + "/password" ).toString();
  mUserTablesOnly = settings.value( key + "/userTablesOnly", true ).toBool();
  mAllowGeometrylessTables = settings.value( key + "/allowGeometrylessTables", false ).toBool();
  mSslEnabled = settings.value( key + "/sslEnabled", false ).toBool();
  mSslCryptoProvider = settings.value( key + "/sslCryptoProvider" ).toString();
  mSslKeyStore = settings.value( key + "/sslKeyStore" ).toString();
  mSslTrustStore = settings.value( key + "/sslTrustStore" ).toString();
  mSslValidateCertificate = settings.value( key + "/sslValidateCertificate", true ).toBool();
  mSslHostNameInCertificate = settings.value( key + "/sslHostNameInCertificate" ).toString();

  const QString keysPath = key + "/keys";
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
  settings.setValue( key + "/driver", mDriver );
  settings.setValue( key + "/host", mHost );
  settings.setValue( key + "/identifierType", mIdentifierType );
  settings.setValue( key + "/identifier", mIdentifier );
  settings.setValue( key + "/multitenant", mMultitenant );
  settings.setValue( key + "/database", mDatabase );
  settings.setValue( key + "/schema", mSchema );
  settings.setValue( key + "/authcfg", mAuthcfg );
  settings.setValue( key + "/saveUsername", mSaveUserName );
  settings.setValue( key + "/username", mSaveUserName ? mUserName : QString( ) );
  settings.setValue( key + "/savePassword", mSavePassword );
  settings.setValue( key + "/password", mSavePassword ? mPassword : QString( ) );
  settings.setValue( key + "/userTablesOnly", mUserTablesOnly );
  settings.setValue( key + "/allowGeometrylessTables", mAllowGeometrylessTables );
  settings.setValue( key + "/sslEnabled", mSslEnabled );
  settings.setValue( key + "/sslCryptoProvider", mSslCryptoProvider );
  settings.setValue( key + "/sslKeyStore", mSslKeyStore );
  settings.setValue( key + "/sslTrustStore", mSslTrustStore );
  settings.setValue( key + "/sslValidateCertificate", mSslValidateCertificate );
  settings.setValue( key + "/sslHostNameInCertificate", mSslHostNameInCertificate );

  if ( !mKeyColumns.empty() )
  {
    const QString keysPath = key + "/keys/";
    settings.beginGroup( keysPath );
    const QStringList schemaNames = mKeyColumns.keys();
    for ( const QString &schemaName : schemaNames )
    {
      const auto &schemaKeys = mKeyColumns[schemaName];
      if ( schemaKeys.empty() )
        continue;
      const QStringList objectNames = schemaKeys.keys();
      settings.beginGroup( schemaName );
      for ( const QString &objectName : objectNames )
        settings.setValue( objectName, schemaKeys[objectName] );
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
  settings.remove( key + "/driver" );
  settings.remove( key + "/host" );
  settings.remove( key + "/identifierType" );
  settings.remove( key + "/identifier" );
  settings.remove( key + "/multitenant" );
  settings.remove( key + "/database" );
  settings.remove( key + "/schema" );
  settings.remove( key + "/userTablesOnly" );
  settings.remove( key + "/allowGeometrylessTables" );
  settings.remove( key + "/username" );
  settings.remove( key + "/password" );
  settings.remove( key + "/saveUsername" );
  settings.remove( key + "/savePassword" );
  settings.remove( key + "/authcfg" );
  settings.remove( key + "/sslEnabled" );
  settings.remove( key + "/sslCryptoProvider" );
  settings.remove( key + "/sslKeyStore" );
  settings.remove( key + "/sslTrustStore" );
  settings.remove( key + "/sslValidateCertificate" );
  settings.remove( key + "/sslHostNameInCertificate" );
  settings.remove( key + "/keys" );
  settings.remove( key );
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
  return settings.value( getBaseKey() + "selected" ).toString();
}

void QgsHanaSettings::setSelectedConnection( const QString &name )
{
  QgsSettings settings;
  settings.setValue( getBaseKey() + "selected", name );
}
