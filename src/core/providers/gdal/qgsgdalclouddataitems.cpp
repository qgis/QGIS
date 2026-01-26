/***************************************************************************
    qgsgdalclouddataitems.cpp
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
#include "qgsgdalclouddataitems.h"

#include "qgsfilebaseddataitemprovider.h"
#include "qgsgdalcloudconnection.h"
#include "qgsgdalutils.h"
#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"

#include "moc_qgsgdalclouddataitems.cpp"

///@cond PRIVATE

//
// QgsGdalCloudRootItem
//

QgsGdalCloudRootItem::QgsGdalCloudRootItem( QgsDataItem *parent, QString name, QString path )
  : QgsConnectionsRootItem( parent, name, path, u"cloud"_s )
{
  mCapabilities |= Qgis::BrowserItemCapability::Fast;
  mIconName = u"mIconCloud.svg"_s;
  populate();
}

QVector<QgsDataItem *> QgsGdalCloudRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;

  QList< QgsGdalUtils::VsiNetworkFileSystemDetails > vsiDetails = QgsGdalUtils::vsiNetworkFileSystems();
  std::sort( vsiDetails.begin(), vsiDetails.end(), []( const QgsGdalUtils::VsiNetworkFileSystemDetails & a, const QgsGdalUtils::VsiNetworkFileSystemDetails & b )
  {
    return QString::localeAwareCompare( a.name, b.name ) < 0;
  } );

  const QStringList connectionList = QgsGdalCloudProviderConnection::connectionList();

  for ( const QgsGdalUtils::VsiNetworkFileSystemDetails &handler : vsiDetails )
  {
    // only expose items for drivers with stored connections
    bool foundConnection = false;
    for ( const QString &connName : std::as_const( connectionList ) )
    {
      const QgsGdalCloudProviderConnection::Data connectionData = QgsGdalCloudProviderConnection::connection( connName );
      if ( connectionData.vsiHandler == handler.identifier )
      {
        foundConnection = true;
        break;
      }
    }
    if ( !foundConnection )
      continue;

    QgsDataItem *conn = new QgsGdalCloudProviderItem( this, handler );
    connections.append( conn );
  }
  return connections;
}

//
// QgsGdalCloudProviderItem
//

QgsGdalCloudProviderItem::QgsGdalCloudProviderItem( QgsDataItem *parent, const QgsGdalUtils::VsiNetworkFileSystemDetails &handler )
  : QgsDataCollectionItem( parent, handler.name, handler.identifier, handler.identifier )
  , mVsiHandler( handler )
{
  mIconName = u"mIconCloud.svg"_s;
}

QVector<QgsDataItem *> QgsGdalCloudProviderItem::createChildren()
{
  QVector<QgsDataItem *> connections;
  const auto connectionList = QgsGdalCloudProviderConnection::connectionList();
  for ( const QString &connName : connectionList )
  {
    const QgsGdalCloudProviderConnection::Data connectionData = QgsGdalCloudProviderConnection::connection( connName );
    if ( connectionData.vsiHandler != mVsiHandler.identifier )
      continue;

    QgsDataItem *conn = new QgsGdalCloudConnectionItem( this, connName, mPath + '/' + connName );
    connections.append( conn );
  }
  return connections;
}

//
// QgsGdalCloudConnectionItem
//

QgsGdalCloudConnectionItem::QgsGdalCloudConnectionItem( QgsDataItem *parent, const QString &name, const QString &path )
  : QgsDataCollectionItem( parent, name, path )
  , mConnName( name )
{
  mIconName = u"mIconConnect.svg"_s;
  mCapabilities |= Qgis::BrowserItemCapability::Fertile;
}

bool QgsGdalCloudConnectionItem::equal( const QgsDataItem *other )
{
  const QgsGdalCloudConnectionItem *o = qobject_cast<const QgsGdalCloudConnectionItem *>( other );
  return ( type() == other->type() && o && mPath == o->mPath && mName == o->mName && mConnName == o->mConnName );
}

QVector<QgsDataItem *> QgsGdalCloudConnectionItem::createChildren()
{
  QVector<QgsDataItem *> children;

  const QgsGdalCloudProviderConnection::Data connectionData = QgsGdalCloudProviderConnection::connection( mConnName );
  QgsGdalCloudProviderConnection conn = QgsGdalCloudProviderConnection( mConnName );
  const QList< QgsGdalCloudProviderConnection::DirectoryObject > objects = conn.contents( connectionData.rootPath );

  QVariantMap extraUriParts;
  if ( !connectionData.credentialOptions.isEmpty() )
  {
    extraUriParts.insert( u"credentialOptions"_s, connectionData.credentialOptions );
  }

  for ( const QgsGdalCloudProviderConnection::DirectoryObject &object : objects )
  {
    const QString subPath = connectionData.rootPath.isEmpty() ? object.name : ( connectionData.rootPath + '/' + object.name );
    if ( object.isDir )
    {
      QgsGdalCloudDirectoryItem *child = new QgsGdalCloudDirectoryItem( this,
          object.name, mPath + '/' + object.name, mConnName, subPath );
      children.append( child );
    }
    else if ( object.isFile )
    {
      const QString filePath = u"/%1/%2/%3"_s.arg( connectionData.vsiHandler, connectionData.container, subPath );
      // QgsFileBasedDataItemProvider uses paths for item uris by default, so we need to specify that the credentialOptions should be appended to the layer URIs
      if ( QgsDataItem *item = QgsFileBasedDataItemProvider::createLayerItemForPath( filePath, this, { u"gdal"_s, u"ogr"_s}, extraUriParts, Qgis::SublayerQueryFlag::FastScan ) )
      {
        item->setCapabilities( item->capabilities2() | Qgis::BrowserItemCapability::ReadOnly );
        children.append( item );
      }
    }
  }

  return children;
}

//
// QgsGdalCloudDirectoryItem
//

QgsGdalCloudDirectoryItem::QgsGdalCloudDirectoryItem( QgsDataItem *parent, QString name, QString path, const QString &connectionName, const QString &directory )
  : QgsDataCollectionItem( parent, name, path, connectionName )
  , mConnName( connectionName )
  , mDirectory( directory )
{
  mIconName = u"mIconFolder.svg"_s;
  mCapabilities |= Qgis::BrowserItemCapability::Fertile;
}

QVector<QgsDataItem *> QgsGdalCloudDirectoryItem::createChildren()
{
  QVector<QgsDataItem *> children;

  const QgsGdalCloudProviderConnection::Data connectionData = QgsGdalCloudProviderConnection::connection( mConnName );
  QgsGdalCloudProviderConnection conn = QgsGdalCloudProviderConnection( mConnName );

  const QList< QgsGdalCloudProviderConnection::DirectoryObject > objects = conn.contents( mDirectory );

  QVariantMap extraUriParts;
  if ( !connectionData.credentialOptions.isEmpty() )
  {
    extraUriParts.insert( u"credentialOptions"_s, connectionData.credentialOptions );
  }

  for ( const QgsGdalCloudProviderConnection::DirectoryObject &object : std::as_const( objects ) )
  {
    const QString subPath = mDirectory + '/' + object.name;
    if ( object.isDir )
    {
      QgsGdalCloudDirectoryItem *child = new QgsGdalCloudDirectoryItem( this,
          object.name, mPath + '/' + object.name, mConnName, subPath );
      children.append( child );
    }
    else if ( object.isFile )
    {
      const QString filePath = u"/%1/%2/%3"_s.arg( connectionData.vsiHandler, connectionData.container, subPath );
      // QgsFileBasedDataItemProvider uses paths for item uris by default, so we need to specify that the credentialOptions should be appended to the layer URIs
      if ( QgsDataItem *item = QgsFileBasedDataItemProvider::createLayerItemForPath( filePath, this, { u"gdal"_s, u"ogr"_s}, extraUriParts, Qgis::SublayerQueryFlag::FastScan ) )
      {
        item->setCapabilities( item->capabilities2() | Qgis::BrowserItemCapability::ReadOnly );
        children.append( item );
      }
    }
  }

  return children;
}

//
// QgsGdalCloudDataItemProvider
//

QString QgsGdalCloudDataItemProvider::name()
{
  return u"GDAL Cloud"_s;
}

QString QgsGdalCloudDataItemProvider::dataProviderKey() const
{
  return u"cloud"_s;
}

Qgis::DataItemProviderCapabilities QgsGdalCloudDataItemProvider::capabilities() const
{
  return Qgis::DataItemProviderCapability::NetworkSources;
}

QgsDataItem *QgsGdalCloudDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  if ( path.isEmpty() )
    return new QgsGdalCloudRootItem( parentItem, QObject::tr( "Cloud" ), u"cloud:"_s );

  return nullptr;
}

///@endcond
