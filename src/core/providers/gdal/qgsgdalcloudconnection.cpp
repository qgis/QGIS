/***************************************************************************
    qgsgdalcloudconnection.cpp
    ---------------------
    begin                : June 2024
    copyright            : (C) 2024 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgdalcloudconnection.h"

#include "qgsdatasourceuri.h"
#include "qgssettingsentryimpl.h"
#include "qgsgdalutils.h"
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <gdal.h>

///@cond PRIVATE


const QgsSettingsEntryString *QgsGdalCloudProviderConnection::settingsVsiHandler = new QgsSettingsEntryString( QStringLiteral( "handler" ), sTreeConnectionCloud );
const QgsSettingsEntryString *QgsGdalCloudProviderConnection::settingsContainer = new QgsSettingsEntryString( QStringLiteral( "container" ), sTreeConnectionCloud );
const QgsSettingsEntryString *QgsGdalCloudProviderConnection::settingsPath = new QgsSettingsEntryString( QStringLiteral( "path" ), sTreeConnectionCloud );
const QgsSettingsEntryVariantMap *QgsGdalCloudProviderConnection::settingsCredentialOptions = new QgsSettingsEntryVariantMap( QStringLiteral( "credential-options" ), sTreeConnectionCloud );

///@endcond

QString QgsGdalCloudProviderConnection::encodedUri( const QgsGdalCloudProviderConnection::Data &data )
{
  QgsDataSourceUri uri;

  if ( !data.vsiHandler.isEmpty() )
    uri.setParam( QStringLiteral( "handler" ), data.vsiHandler );
  if ( !data.container.isEmpty() )
    uri.setParam( QStringLiteral( "container" ), data.container );
  if ( !data.rootPath.isEmpty() )
    uri.setParam( QStringLiteral( "rootPath" ), data.rootPath );

  QStringList credentialOptions;
  for ( auto it = data.credentialOptions.constBegin(); it != data.credentialOptions.constEnd(); ++it )
  {
    if ( !it.value().toString().isEmpty() )
    {
      credentialOptions.append( QStringLiteral( "%1=%2" ).arg( it.key(), it.value().toString() ) );
    }
  }
  if ( !credentialOptions.empty() )
    uri.setParam( "credentialOptions", credentialOptions.join( '|' ) );

  return uri.encodedUri();
}

QgsGdalCloudProviderConnection::Data QgsGdalCloudProviderConnection::decodedUri( const QString &uri )
{
  QgsDataSourceUri dsUri;
  dsUri.setEncodedUri( uri );

  QgsGdalCloudProviderConnection::Data conn;
  conn.vsiHandler = dsUri.param( QStringLiteral( "handler" ) );
  conn.container = dsUri.param( QStringLiteral( "container" ) );
  conn.rootPath = dsUri.param( QStringLiteral( "rootPath" ) );

  const QStringList credentialOptions = dsUri.param( QStringLiteral( "credentialOptions" ) ).split( '|' );
  for ( const QString &option : credentialOptions )
  {
    const thread_local QRegularExpression credentialOptionKeyValueRegex( QStringLiteral( "(.*?)=(.*)" ) );
    const QRegularExpressionMatch keyValueMatch = credentialOptionKeyValueRegex.match( option );
    if ( keyValueMatch.hasMatch() )
    {
      conn.credentialOptions.insert( keyValueMatch.captured( 1 ), keyValueMatch.captured( 2 ) );
    }
  }

  return conn;
}

QStringList QgsGdalCloudProviderConnection::connectionList()
{
  return QgsGdalCloudProviderConnection::sTreeConnectionCloud->items();
}

QgsGdalCloudProviderConnection::Data QgsGdalCloudProviderConnection::connection( const QString &name )
{
  if ( !settingsContainer->exists( name ) )
    return QgsGdalCloudProviderConnection::Data();

  QgsGdalCloudProviderConnection::Data conn;
  conn.vsiHandler = settingsVsiHandler->value( name );
  conn.container = settingsContainer->value( name );
  conn.rootPath = settingsPath->value( name );
  conn.credentialOptions = settingsCredentialOptions->value( name );

  return conn;
}

void QgsGdalCloudProviderConnection::addConnection( const QString &name, const Data &conn )
{
  settingsVsiHandler->setValue( conn.vsiHandler, name );
  settingsContainer->setValue( conn.container, name );
  settingsPath->setValue( conn.rootPath, name );
  settingsCredentialOptions->setValue( conn.credentialOptions, name );
}

QString QgsGdalCloudProviderConnection::selectedConnection()
{
  return sTreeConnectionCloud->selectedItem();
}

void QgsGdalCloudProviderConnection::setSelectedConnection( const QString &name )
{
  sTreeConnectionCloud->setSelectedItem( name );
}

QgsGdalCloudProviderConnection::QgsGdalCloudProviderConnection( const QString &name )
  : QgsAbstractProviderConnection( name )
{
  const QgsGdalCloudProviderConnection::Data connectionData = connection( name );
  setUri( encodedUri( connectionData ) );
}

QgsGdalCloudProviderConnection::QgsGdalCloudProviderConnection( const QString &uri, const QVariantMap &configuration )
  : QgsAbstractProviderConnection( uri, configuration )
{
}

void QgsGdalCloudProviderConnection::store( const QString &name ) const
{
  QgsGdalCloudProviderConnection::Data connectionData = decodedUri( uri() );
  addConnection( name, connectionData );
}

void QgsGdalCloudProviderConnection::remove( const QString &name ) const
{
  sTreeConnectionCloud->deleteItem( name );
}

QList<QgsGdalCloudProviderConnection::DirectoryObject> QgsGdalCloudProviderConnection::contents( const QString &path ) const
{
  const QgsGdalCloudProviderConnection::Data connectionDetails = decodedUri( uri() );

  if ( !connectionDetails.credentialOptions.isEmpty() )
  {
    QgsGdalUtils::applyVsiCredentialOptions( connectionDetails.vsiHandler,
        connectionDetails.container, connectionDetails.credentialOptions );
  }

  char **papszOptions = nullptr;
  papszOptions = CSLAddString( papszOptions, "NAME_AND_TYPE_ONLY=YES" );

  const QString vsiPath = QStringLiteral( "/%1/%2/%3" ).arg( connectionDetails.vsiHandler,
                          connectionDetails.container,
                          path );

  VSIDIR *dir = VSIOpenDir( vsiPath.toUtf8().constData(), 0, papszOptions );
  if ( !dir )
  {
    CSLDestroy( papszOptions );
    return {};
  }

  QList< QgsGdalCloudProviderConnection::DirectoryObject > objects;
  while ( const VSIDIREntry *entry = VSIGetNextDirEntry( dir ) )
  {
    QgsGdalCloudProviderConnection::DirectoryObject object;
    object.name = QString( entry->pszName );
    object.isFile = VSI_ISREG( entry->nMode );
    object.isDir = VSI_ISDIR( entry->nMode );
    objects << object;
  }

  VSICloseDir( dir );
  CSLDestroy( papszOptions );

  return objects;
}
