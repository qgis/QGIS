/***************************************************************************
    qgsgeonodedataitems.cpp
    ---------------------
    begin                : Feb 2017
    copyright            : (C) 2017 by Muhammad Yarjuna Rohmat, Ismail Sunni
    email                : rohmat at kartoza dot com, ismail at kartoza dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslogger.h"
#include "qgsgeonodedataitems.h"
#include "qgsproviderregistry.h"
#include "qgsgeonoderequest.h"

typedef QList<QgsDataItemProvider *> *dataItemProviders_t();

QgsGeoNodeConnectionItem::QgsGeoNodeConnectionItem( QgsDataItem *parent, QString name, QString path, std::unique_ptr<QgsGeoNodeConnection> conn )
  : QgsDataCollectionItem( parent, name, path, QStringLiteral( "GeoNode" ) )
  , mGeoNodeName( parent->name() )
  , mUri( conn->uri().uri() )
{
  mConnection = std::move( conn );
  mIconName = QStringLiteral( "mIconConnect.svg" );
}

QVector<QgsDataItem *> QgsGeoNodeConnectionItem::createChildren()
{
  QVector<QgsDataItem *> services;

  QString url = mConnection->uri().param( QStringLiteral( "url" ) );
  QgsGeoNodeRequest geonodeRequest( url, true );

  QStringList wmsUrl = geonodeRequest.fetchServiceUrlsBlocking( QStringLiteral( "WMS" ) );
  QStringList wfsUrl = geonodeRequest.fetchServiceUrlsBlocking( QStringLiteral( "WFS" ) );
  QStringList wcsUrl = geonodeRequest.fetchServiceUrlsBlocking( QStringLiteral( "WCS" ) );
  QStringList xyzUrl = geonodeRequest.fetchServiceUrlsBlocking( QStringLiteral( "XYZ" ) );

  if ( !wmsUrl.isEmpty() )
  {
    QString path = mPath + QStringLiteral( "/wms" );
    QgsDataItem *service = new QgsGeoNodeServiceItem( this, mConnection.get(), QStringLiteral( "WMS" ), path );
    services.append( service );
  }

  if ( !wfsUrl.isEmpty() )
  {
    QString path = mPath + QStringLiteral( "/wfs" );
    QgsDataItem *service = new QgsGeoNodeServiceItem( this, mConnection.get(), QStringLiteral( "WFS" ), path );
    services.append( service );
  }

  if ( !wcsUrl.isEmpty() )
  {
    QString path = mPath + QStringLiteral( "/wcs" );
    QgsDataItem *service = new QgsGeoNodeServiceItem( this, mConnection.get(), QStringLiteral( "WCS" ), path );
    services.append( service );
  }

  if ( !xyzUrl.isEmpty() )
  {
    QString path = mPath + QStringLiteral( "/xyz" );
    QgsDataItem *service = new QgsGeoNodeServiceItem( this, mConnection.get(), QStringLiteral( "XYZ" ), path );
    services.append( service );
  }

  return services;
}


QgsGeoNodeServiceItem::QgsGeoNodeServiceItem( QgsDataItem *parent, QgsGeoNodeConnection *conn, QString serviceName, QString path )
  : QgsDataCollectionItem( parent, serviceName, path, QStringLiteral( "GeoNode" ) )
  , mName( conn->connectionName() )
  , mServiceName( serviceName )
  , mConnection( conn )
{
  if ( serviceName == QLatin1String( "WMS" ) || serviceName == QLatin1String( "XYZ" ) )
  {
    mIconName = QStringLiteral( "mIconWms.svg" );
  }
  else if ( serviceName == QLatin1String( "WCS" ) )
  {
    mIconName = QStringLiteral( "mIconWcs.svg" );
  }
  else
  {
    mIconName = QStringLiteral( "mIconWfs.svg" );
  }
}

QVector<QgsDataItem *> QgsGeoNodeServiceItem::createChildren()
{
  QVector<QgsDataItem *> children;
  QHash<QgsDataItem *, QString> serviceItems; // service/provider key

  // Try to open with service provider
  bool skipProvider = false;

  QgsGeoNodeConnectionItem *parentItem = dynamic_cast<QgsGeoNodeConnectionItem *>( mParent );
  QString pathPrefix = parentItem->mGeoNodeName.toLower() + QStringLiteral( ":/" );

  while ( !skipProvider )
  {
    const QString &key = mServiceName != QLatin1String( "WFS" ) ? mServiceName == QLatin1String( "WCS" ) ? QStringLiteral( "wcs" ) : QStringLiteral( "wms" ) : mServiceName;

    const QList<QgsDataItemProvider *> providerList = QgsProviderRegistry::instance()->dataItemProviders( key );
    if ( providerList.isEmpty() )
    {
      skipProvider = true;
      continue;
    }

    QString path = pathPrefix + mName;

    QVector<QgsDataItem *> items;
    for ( QgsDataItemProvider *pr : providerList )
    {
      if ( !pr->name().startsWith( mServiceName ) )
        continue;

      items = pr->createDataItems( path, this );
      if ( !items.isEmpty() )
      {
        break;
      }
    }

    if ( items.isEmpty() )
    {
      skipProvider = true;
      continue;
    }

    if ( mServiceName == QLatin1String( "XYZ" ) )
    {
      return items;
    }

    for ( QgsDataItem *item : std::as_const( items ) )
    {
      item->populate( true ); // populate in foreground - this is already run in a thread

      if ( item->rowCount() > 0 )
      {
        serviceItems.insert( item, key );
      }
      else
      {
        //delete item;
      }
    }

    skipProvider = true;
  }

  auto serviceItemIt = serviceItems.constBegin();
  for ( ; serviceItemIt != serviceItems.constEnd(); ++serviceItemIt )
  {
    QgsDataItem *item = serviceItemIt.key();
    QString providerKey = serviceItemIt.value();

    // Add layers directly to service item
    const QVector< QgsDataItem * > serviceChildItems = item->children();
    for ( QgsDataItem *subItem : serviceChildItems )
    {
      if ( subItem->path().endsWith( QLatin1String( "error" ) ) )
      {
        continue;
      }
      item->removeChildItem( subItem );
      subItem->setParent( this );
      replacePath( subItem, providerKey.toLower() + QStringLiteral( ":/" ), pathPrefix );
      children.append( subItem );
    }

    delete item;
  }

  return children;
}

// reset path recursively
void QgsGeoNodeServiceItem::replacePath( QgsDataItem *item, const QString &before, const QString &after )
{
  item->setPath( item->path().replace( before, after ) );
  const QVector< QgsDataItem * > children = item->children();
  for ( QgsDataItem *subItem : children )
  {
    replacePath( subItem, before, after );
  }
}

QgsGeoNodeRootItem::QgsGeoNodeRootItem( QgsDataItem *parent, QString name, QString path )
  : QgsConnectionsRootItem( parent, name, path, QStringLiteral( "GeoNode" ) )
{
  mCapabilities |= Qgis::BrowserItemCapability::Fast;
  {
    mIconName = QStringLiteral( "mIconGeonode.svg" );
  }
  populate();
}

QVector<QgsDataItem *> QgsGeoNodeRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;

  const QStringList names = QgsGeoNodeConnectionUtils::connectionList();
  for ( const QString &connName : names )
  {
    std::unique_ptr< QgsGeoNodeConnection > connection( new QgsGeoNodeConnection( connName ) );
    QString path = mPath + '/' + connName;
    QgsDataItem *conn = new QgsGeoNodeConnectionItem( this, connName, path, std::move( connection ) );
    connections.append( conn );
  }
  return connections;
}


QString QgsGeoNodeDataItemProvider::name()
{
  return QStringLiteral( "GeoNode" );
}

int QgsGeoNodeDataItemProvider::capabilities() const
{
  return QgsDataProvider::Net;
}

QgsDataItem *QgsGeoNodeDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  QgsDebugMsgLevel( "thePath = " + path, 4 );
  if ( path.isEmpty() )
  {
    return new QgsGeoNodeRootItem( parentItem, QStringLiteral( "GeoNode" ), QStringLiteral( "geonode:" ) );
  }

  // path schema: geonode:/connection name (used by OWS)
  if ( path.startsWith( QLatin1String( "geonode:/" ) ) )
  {
    QString connectionName = path.split( '/' ).last();
    if ( QgsGeoNodeConnectionUtils::connectionList().contains( connectionName ) )
    {
      std::unique_ptr< QgsGeoNodeConnection > connection( new QgsGeoNodeConnection( connectionName ) );
      return new QgsGeoNodeConnectionItem( parentItem, QStringLiteral( "GeoNode" ), path, std::move( connection ) );
    }
  }

  return nullptr;
}


bool QgsGeoNodeServiceItem::layerCollection() const
{
  return true;
}
