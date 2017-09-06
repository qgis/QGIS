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
#include "qgsnewhttpconnection.h"
#include "qgsgeonodenewconnection.h"
#include "qgsgeonoderequest.h"

typedef QList<QgsDataItemProvider *> *dataItemProviders_t();

QgsGeoNodeConnectionItem::QgsGeoNodeConnectionItem( QgsDataItem *parent, QString name, QString path, QgsGeoNodeConnection *conn )
  : QgsDataCollectionItem( parent, name, path )
  , mGeoNodeName( parent->name() )
  , mUri( conn->uri().uri() )
  , mConnection( conn )
{
  mIconName = QStringLiteral( "mIconConnect.png" );
}

QVector<QgsDataItem *> QgsGeoNodeConnectionItem::createChildren()
{
  QVector<QgsDataItem *> services;

  QString url = mConnection->uri().param( "url" );
  QgsGeoNodeRequest geonodeRequest( url, true );

  QStringList wmsUrl = geonodeRequest.serviceUrls( QStringLiteral( "WMS" ) );
  QStringList wfsUrl = geonodeRequest.serviceUrls( QStringLiteral( "WFS" ) );
  QStringList xyzUrl = geonodeRequest.serviceUrls( QStringLiteral( "XYZ" ) );

  if ( !wmsUrl.isEmpty() )
  {
    QString path = mPath + "/wms";
    QgsDataItem *service = new QgsGeoNodeServiceItem( this, mConnection, QStringLiteral( "WMS" ), path );
    services.append( service );
  }

  if ( !wfsUrl.isEmpty() )
  {
    QString path = mPath + "/wfs";
    QgsDataItem *service = new QgsGeoNodeServiceItem( this, mConnection, QStringLiteral( "WFS" ), path );
    services.append( service );
  }

  if ( !xyzUrl.isEmpty() )
  {
    QString path = mPath + "/xyz";
    QgsDataItem *service = new QgsGeoNodeServiceItem( this, mConnection, QStringLiteral( "XYZ" ), path );
    services.append( service );
  }

  return services;
}

QList<QAction *> QgsGeoNodeConnectionItem::actions()
{
  QAction *actionEdit = new QAction( tr( "Edit..." ), this );
  QAction *actionDelete = new QAction( tr( "Delete" ), this );
  connect( actionEdit, &QAction::triggered, this, &QgsGeoNodeConnectionItem::editConnection );
  connect( actionDelete, &QAction::triggered, this, &QgsGeoNodeConnectionItem::deleteConnection );
  return QList<QAction *>() << actionEdit << actionDelete;
}

void QgsGeoNodeConnectionItem::editConnection()
{
  QgsGeoNodeNewConnection *nc = new QgsGeoNodeNewConnection( nullptr, mConnection->connName() );
  nc->setWindowTitle( tr( "Modify GeoNode connection" ) );

  if ( nc->exec() )
  {
    // the parent should be updated
    mParent->refresh();
  }
}

QgsGeoNodeServiceItem::QgsGeoNodeServiceItem( QgsDataItem *parent, QgsGeoNodeConnection *conn, QString serviceName, QString path )
  : QgsDataCollectionItem( parent, serviceName, path )
  , mName( conn->connName() )
  , mServiceName( serviceName )
  , mConnection( conn )
{
  if ( serviceName == QStringLiteral( "WMS" ) || serviceName == QStringLiteral( "XYZ" ) )
  {
    mIconName = QStringLiteral( "mIconWms.svg" );
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

  int layerCount = 0;
  // Try to open with service provider
  bool skipProvider = false;

  QgsGeoNodeConnectionItem *parentItem = dynamic_cast<QgsGeoNodeConnectionItem *>( mParent );
  QString pathPrefix = parentItem->mGeoNodeName.toLower() + ":/";

  while ( !skipProvider )
  {
    const QString &key = mServiceName != QString( "WFS" ) ? QString( "WMS" ).toLower() : mServiceName;
    std::unique_ptr< QLibrary > library( QgsProviderRegistry::instance()->createProviderLibrary( key ) );
    if ( !library )
    {
      skipProvider = true;
      continue;
    }

    dataItemProviders_t *dataItemProvidersFn = reinterpret_cast< dataItemProviders_t * >( cast_to_fptr( library->resolve( "dataItemProviders" ) ) );
    dataItem_t *dItem = ( dataItem_t * ) cast_to_fptr( library->resolve( "dataItem" ) );
    if ( !dItem && !dataItemProvidersFn )
    {
      skipProvider = true;
      continue;
    }

    QString path =  pathPrefix + mName;

    QVector<QgsDataItem *> items;
    QList<QgsDataItemProvider *> *providerList = dataItemProvidersFn();
    Q_FOREACH ( QgsDataItemProvider *pr, *providerList )
    {
      items = pr->name().startsWith( mServiceName ) ? pr->createDataItems( path, this ) : items;
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

    if ( mServiceName == QStringLiteral( "XYZ" ) )
    {
      return items;
    }

    Q_FOREACH ( QgsDataItem *item, items )
    {
      item->populate( true ); // populate in foreground - this is already run in a thread

      layerCount += item->rowCount();
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

  // add new action menus to the layer item
  QAction *actionCopyStyle = new QAction( tr( "Copy Style" ), this );
  QAction *actionPasteStyle = new QAction( tr( "Paste Style" ), this );
  QList<QAction *> actions;
  actions << actionCopyStyle << actionPasteStyle;

  Q_FOREACH ( QgsDataItem *item, serviceItems.keys() )
  {
    QString providerKey = serviceItems.value( item );

    // Add layers directly to service item
    Q_FOREACH ( QgsDataItem *subItem, item->children() )
    {
      if ( subItem->path().endsWith( QString( "error" ) ) )
      {
        continue;
      }
      item->removeChildItem( subItem );
      subItem->setParent( this );
      replacePath( subItem, providerKey.toLower() + ":/", pathPrefix );
      subItem->setActions( actions );
      children.append( subItem );
    }

    delete item;
  }

  return children;
}

// reset path recursively
void QgsGeoNodeServiceItem::replacePath( QgsDataItem *item, QString before, QString after )
{
  item->setPath( item->path().replace( before, after ) );
  Q_FOREACH ( QgsDataItem *subItem, item->children() )
  {
    replacePath( subItem, before, after );
  }
}

QgsGeoNodeRootItem::QgsGeoNodeRootItem( QgsDataItem *parent, QString name, QString path ) : QgsDataCollectionItem( parent, name, path )
{
  mCapabilities |= Fast;
  {
    mIconName = QStringLiteral( "mIconGeonode.svg" );
  }
  populate();
}

QVector<QgsDataItem *> QgsGeoNodeRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;

  Q_FOREACH ( const QString &connName, QgsGeoNodeConnectionUtils::connectionList() )
  {
    QgsGeoNodeConnection *connection = nullptr;
    connection = new QgsGeoNodeConnection( connName );
    QString path = mPath + "/" + connName;
    QgsDataItem *conn = new QgsGeoNodeConnectionItem( this, connName, path, connection );
    connections.append( conn );
  }
  return connections;
}

QList<QAction *> QgsGeoNodeRootItem::actions()
{
  QAction *actionNew = new QAction( tr( "New Connection..." ), this );
  connect( actionNew, &QAction::triggered, this, &QgsGeoNodeRootItem::newConnection );
  return QList<QAction *>() << actionNew;
}

void QgsGeoNodeRootItem::newConnection()
{
  QgsGeoNodeNewConnection *nc = new QgsGeoNodeNewConnection( nullptr );

  if ( nc->exec() )
  {
    refresh();
  }
}


QgsDataItem *QgsGeoNodeDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  QgsDebugMsg( "thePath = " + path );
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
      QgsGeoNodeConnection *connection = new QgsGeoNodeConnection( connectionName );
      return new QgsGeoNodeConnectionItem( parentItem, QStringLiteral( "GeoNode" ), path, connection );
    }
  }

  return nullptr;
}
