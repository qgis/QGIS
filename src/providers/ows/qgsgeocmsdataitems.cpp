/***************************************************************************
    qgsgeocmsdataitems.cpp
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

#include "qgsproviderregistry.h"
#include "qgsowsdataitems.h"
#include "qgslogger.h"
#include "qgsnewhttpconnection.h"
#include "qgsgeonodenewconnection.h"
#include "qgsgeocmsdataitems.h"


typedef QList<QgsDataItemProvider *> dataItemProviders_t();

QgsGeoCMSConnectionItem::QgsGeoCMSConnectionItem( QgsDataItem *parent, QString name, QString path, QgsGeoCMSConnection *conn )
  : QgsDataCollectionItem( parent, name, path )
  , mGeoCMSName( parent->name() )
  , mUri( conn->uri().uri() )
  , mConnection( conn )
{
  mIconName = QStringLiteral( "mIconConnect.png" );
}

QVector<QgsDataItem *> QgsGeoCMSConnectionItem::createChildren()
{
  QVector<QgsDataItem *> services;

  QStringList wmsUrl = mConnection->serviceUrl( QStringLiteral( "WMS" ) );
  QStringList wfsUrl = mConnection->serviceUrl( QStringLiteral( "WFS" ) );
  QStringList xyzUrl = mConnection->serviceUrl( QStringLiteral( "XYZ" ) );

  if ( !wmsUrl.isEmpty() )
  {
    QString path = mPath + "/wms";
    QgsDataItem *service = new QgsGeoCMSServiceItem( this, mConnection, QString( "WMS" ), path );
    services.append( service );
  }

  if ( !wfsUrl.isEmpty() )
  {
    QString path = mPath + "/wfs";
    QgsDataItem *service = new QgsGeoCMSServiceItem( this, mConnection, QString( "WFS" ), path );
    services.append( service );
  }

  if ( !xyzUrl.isEmpty() )
  {
    QString path = mPath + "/xyz";
    QgsDataItem *service = new QgsGeoCMSServiceItem( this, mConnection, QString( "XYZ" ), path );
    services.append( service );
  }

  return services;
}

QList<QAction *> QgsGeoCMSConnectionItem::actions()
{
  QAction *actionEdit = new QAction( tr( "Edit..." ), this );
  QAction *actionDelete = new QAction( tr( "Delete" ), this );
  connect( actionEdit, &QAction::triggered, this, &QgsGeoCMSConnectionItem::editConnection );
  connect( actionDelete, &QAction::triggered, this, &QgsGeoCMSConnectionItem::deleteConnection );
  return QList<QAction *>() << actionEdit << actionDelete;
}

void QgsGeoCMSConnectionItem::editConnection()
{
  QgsGeoNodeNewConnection *nc = new QgsGeoNodeNewConnection( nullptr, mConnection->mConnName );
  nc->setWindowTitle( tr( "Modify GeoCMS connection" ) );

  if ( nc->exec() )
  {
    // the parent should be updated
    mParent->refresh();
  }
}

QgsGeoCMSServiceItem::QgsGeoCMSServiceItem( QgsDataItem *parent, QgsGeoCMSConnection *conn, QString serviceName, QString path )
  : QgsDataCollectionItem( parent, serviceName, path )
  , mName( conn->mConnName )
  , mServiceName( serviceName )
  , mConnection( conn )
{
  if ( serviceName == "WMS" || serviceName == "XYZ" )
  {
    mIconName = QStringLiteral( "mIconWms.svg" );
  }
  else
  {
    mIconName = QStringLiteral( "mIconWfs.svg" );
  }
}

QVector<QgsDataItem *> QgsGeoCMSServiceItem::createChildren()
{
  QVector<QgsDataItem *> children;
  QHash<QgsDataItem *, QString> serviceItems; // service/provider key

  int layerCount = 0;
  // Try to open with service provider
  bool skipProvider = false;

  QgsGeoCMSConnectionItem *parentItem = dynamic_cast<QgsGeoCMSConnectionItem *>( mParent );
  QString pathPrefix = parentItem->mGeoCMSName.toLower() + ":/";

  while ( !skipProvider )
  {
    const QString &key = mServiceName != QString( "WFS" ) ? QString( "WMS" ).toLower() : mServiceName;
    QgsDebugMsg( "Add connection for provider " + key );
    std::unique_ptr< QLibrary > library( QgsProviderRegistry::instance()->createProviderLibrary( key ) );
    if ( !library )
    {
      QgsDebugMsg( "Cannot get provider " + key );
      skipProvider = true;
      continue;
    }

    dataItemProviders_t *dataItemProvidersFn = reinterpret_cast< dataItemProviders_t * >( cast_to_fptr( library->resolve( "dataItemProviders" ) ) );
    dataItem_t *dItem = ( dataItem_t * ) cast_to_fptr( library->resolve( "dataItem" ) );
    if ( !dItem && !dataItemProvidersFn )
    {
      QgsDebugMsg( library->fileName() + " does not have dataItem" );
      skipProvider = true;
      continue;
    }

    QString path =  pathPrefix + mName;
    QgsDebugMsg( "path = " + path );

    QVector<QgsDataItem *> items;
    Q_FOREACH ( QgsDataItemProvider *pr, dataItemProvidersFn() )
    {
      items = pr->name().startsWith( mServiceName ) ? pr->createDataItems( path, this ) : items;
      if ( !items.isEmpty() )
      {
        break;
      }
    }

    if ( items.isEmpty() )
    {
      QgsDebugMsg( "Connection not found by provider" );
      skipProvider = true;
      continue;
    }

    if ( mServiceName == QString( "XYZ" ) )
    {
      QgsDebugMsg( "Add new item : " + mServiceName );
      return items;
    }

    Q_FOREACH ( QgsDataItem *item, items )
    {
      item->populate( true ); // populate in foreground - this is already run in a thread

      layerCount += item->rowCount();
      if ( item->rowCount() > 0 )
      {
        QgsDebugMsg( "Add new item : " + item->name() );
        serviceItems.insert( item, key );
      }
      else
      {
        //delete item;
      }
    }

    skipProvider = true;
  }

  Q_FOREACH ( QgsDataItem *item, serviceItems.keys() )
  {
    QgsDebugMsg( QString( "serviceItems.size = %1 layerCount = %2 rowCount = %3" ).arg( serviceItems.size() ).arg( layerCount ).arg( item->rowCount() ) );
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
      children.append( subItem );
    }

    delete item;
  }

  return children;
}

// reset path recursively
void QgsGeoCMSServiceItem::replacePath( QgsDataItem *item, QString before, QString after )
{
  item->setPath( item->path().replace( before, after ) );
  Q_FOREACH ( QgsDataItem *subItem, item->children() )
  {
    replacePath( subItem, before, after );
  }
}

QgsGeoCMSRootItem::QgsGeoCMSRootItem( QgsDataItem *parent, QString name, QString path ) : QgsDataCollectionItem( parent, name, path )
{
  mCapabilities |= Fast;
  if ( name == QString( "GeoNode" ) )
  {
    mIconName = QStringLiteral( "mIconGeonode.svg" );
  }
  else
  {
    mIconName = QStringLiteral( "mIconConnect.svg" );
  }
  populate();
}

QVector<QgsDataItem *> QgsGeoCMSRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;

  Q_FOREACH ( const QString &connName, QgsGeoCMSConnection::connectionList( mName ) )
  {
    QgsGeoNodeConnection *connection = nullptr;
    if ( mName == QString( "GeoNode" ) )
    {
      connection = new QgsGeoNodeConnection( connName );
    }
    else
    {
      connection = new QgsGeoNodeConnection( connName );
    }
    QString path = mPath + "/" + connName;
    QgsDataItem *conn = new QgsGeoCMSConnectionItem( this, connName, path, connection );
    connections.append( conn );
  }
  return connections;
}

QList<QAction *> QgsGeoCMSRootItem::actions()
{
  QAction *actionNew = new QAction( tr( "New Connection..." ), this );
  connect( actionNew, &QAction::triggered, this, &QgsGeoCMSRootItem::newConnection );
  return QList<QAction *>() << actionNew;
}

void QgsGeoCMSRootItem::newConnection()
{
  QgsGeoNodeNewConnection *nc = new QgsGeoNodeNewConnection( nullptr );

  if ( nc->exec() )
  {
    refresh();
  }
}
