/***************************************************************************
    qgsowsdataitems.cpp
    ---------------------
    begin                : May 2012
    copyright            : (C) 2012 by Radim Blazek
    email                : radim dot blazek at gmail dot com
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
#include "qgsowsprovider.h"
#include "qgslogger.h"
#include "qgsdatasourceuri.h"
#include "qgsowsconnection.h"

#ifdef HAVE_GUI
#include "qgsnewhttpconnection.h"
#include "qgsowssourceselect.h"
#endif
#include "qgsgeonodeconnection.h"
#include "qgsgeonodenewconnection.h"
>>>>>>> add geonode data item to the browser panel as an extention of ows provider

#include "qgsapplication.h"

#include <QFileInfo>

// ---------------------------------------------------------------------------
QgsOWSConnectionItem::QgsOWSConnectionItem( QgsDataItem *parent, QString name, QString path )
  : QgsDataCollectionItem( parent, name, path )
{
  mIconName = QStringLiteral( "mIconConnect.png" );
  mCapabilities |= Collapse;
}

QgsOWSConnectionItem::~QgsOWSConnectionItem()
{
}

QVector<QgsDataItem *> QgsOWSConnectionItem::createChildren()
{
  QVector<QgsDataItem *> children;
  QHash<QgsDataItem *, QString> serviceItems; // service/provider key

  int layerCount = 0;
  // Try to open with WMS,WFS,WCS
  Q_FOREACH ( const QString &key, QStringList() << "wms" << "WFS" << "wcs" )
  {
    QgsDebugMsg( "Add connection for provider " + key );
    std::unique_ptr< QLibrary > library( QgsProviderRegistry::instance()->createProviderLibrary( key ) );
    if ( !library )
    {
      QgsDebugMsg( "Cannot get provider " + key );
      continue;
    }

    dataItem_t *dItem = ( dataItem_t * ) cast_to_fptr( library->resolve( "dataItem" ) );
    if ( !dItem )
    {
      QgsDebugMsg( library->fileName() + " does not have dataItem" );
      continue;
    }

    QString path = key.toLower() + ":/" + name();
    QgsDebugMsg( "path = " + path );
    QgsDataItem *item = dItem( path, this );  // empty path -> top level
    if ( !item )
    {
      QgsDebugMsg( "Connection not found by provider" );
      continue;
    }

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

  Q_FOREACH ( QgsDataItem *item, serviceItems.keys() )
  {
    QgsDebugMsg( QString( "serviceItems.size = %1 layerCount = %2 rowCount = %3" ).arg( serviceItems.size() ).arg( layerCount ).arg( item->rowCount() ) );
    QString providerKey = serviceItems.value( item );
    if ( serviceItems.size() == 1 || layerCount <= 30 || item->rowCount() <= 10 )
    {
      // Add layers directly to OWS connection
      Q_FOREACH ( QgsDataItem *subItem, item->children() )
      {
        item->removeChildItem( subItem );
        subItem->setParent( this );
        replacePath( subItem, providerKey.toLower() + ":/", QStringLiteral( "ows:/" ) );
        children.append( subItem );
      }
      delete item;
    }
    else // Add service
    {
      replacePath( item, item->path(), path() + '/' + providerKey.toLower() );
      children.append( item );
    }
  }

  return children;
}

// reset path recursively
void QgsOWSConnectionItem::replacePath( QgsDataItem *item, QString before, QString after )
{
  item->setPath( item->path().replace( before, after ) );
  Q_FOREACH ( QgsDataItem *subItem, item->children() )
  {
    replacePath( subItem, before, after );
  }
}

bool QgsOWSConnectionItem::equal( const QgsDataItem *other )
{
  if ( type() != other->type() )
  {
    return false;
  }
  const QgsOWSConnectionItem *o = dynamic_cast<const QgsOWSConnectionItem *>( other );
  return ( o && mPath == o->mPath && mName == o->mName );
}

#ifdef HAVE_GUI
QList<QAction *> QgsOWSConnectionItem::actions()
{
  QList<QAction *> lst;

  QAction *actionEdit = new QAction( tr( "Edit..." ), this );
  connect( actionEdit, &QAction::triggered, this, &QgsOWSConnectionItem::editConnection );
  lst.append( actionEdit );

  QAction *actionDelete = new QAction( tr( "Delete" ), this );
  connect( actionDelete, &QAction::triggered, this, &QgsOWSConnectionItem::deleteConnection );
  lst.append( actionDelete );

  return lst;
}

void QgsOWSConnectionItem::editConnection()
{
#if 0
  QgsNewHttpConnection nc( 0, "qgis/connections-ows/", mName );

  if ( nc.exec() )
  {
    // the parent should be updated
    mParent->refreshConnections();
  }
#endif
}

void QgsOWSConnectionItem::deleteConnection()
{
#if 0
  QgsOWSConnection::deleteConnection( "OWS", mName );
  // the parent should be updated
  mParent->refreshConnections();
#endif
}
#endif


// ---------------------------------------------------------------------------


QgsOWSRootItem::QgsOWSRootItem( QgsDataItem *parent, QString name, QString path )
  : QgsDataCollectionItem( parent, name, path )
{
  mCapabilities |= Fast;
  mIconName = QStringLiteral( "mIconOws.svg" );
  populate();
}

QgsOWSRootItem::~QgsOWSRootItem()
{
}

QVector<QgsDataItem *> QgsOWSRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;
  // Combine all WMS,WFS,WCS connections
  QStringList connNames;
  Q_FOREACH ( const QString &service, QStringList() << "WMS" << "WFS" << "WCS" )
  {
    Q_FOREACH ( const QString &connName, QgsOwsConnection::connectionList( service ) )
    {
      if ( !connNames.contains( connName ) )
      {
        connNames << connName;
      }
    }
  }
  Q_FOREACH ( const QString &connName, connNames )
  {
    QgsDataItem *conn = new QgsOWSConnectionItem( this, connName, "ows:/" + connName );
    connections.append( conn );
  }
  return connections;
}

#ifdef HAVE_GUI
QList<QAction *> QgsOWSRootItem::actions()
{
  QList<QAction *> lst;

#if 0
  QAction *actionNew = new QAction( tr( "New Connection..." ), this );
  connect( actionNew, SIGNAL( triggered() ), this, SLOT( newConnection() ) );
  lst.append( actionNew );
#endif

  return lst;
}


QWidget *QgsOWSRootItem::paramWidget()
{
#if 0
  QgsOWSSourceSelect *select = new QgsOWSSourceSelect( 0, 0, true, true );
  connect( select, SIGNAL( connectionsChanged() ), this, SLOT( connectionsChanged() ) );
  return select;
#endif
  return nullptr;
}
void QgsOWSRootItem::connectionsChanged()
{
  refresh();
}

void QgsOWSRootItem::newConnection()
{
#if 0
  QgsNewHttpConnection nc( 0, "qgis/connections-ows/" );

  if ( nc.exec() )
  {
    refreshConnections();
  }
#endif
}
#endif


// ---------------------------------------------------------------------------

static QStringList extensions = QStringList();
static QStringList wildcards = QStringList();

QGISEXTERN QList<QgsDataItemProvider *> dataItemProviders()
{
  return QList<QgsDataItemProvider *>()
         << new QgsOwsDataItemProvider
         << new QgsGeoNodeDataItemProvider;
}

/*QGISEXTERN int dataCapabilities()
{
  return QgsDataProvider::Net;
}

QGISEXTERN QgsDataItem *dataItem( QString path, QgsDataItem *parentItem )
{
  if ( path.isEmpty() )
  {
    return new QgsOWSRootItem( parentItem, QStringLiteral( "OWS" ), QStringLiteral( "ows:" ) );
  }
  return nullptr;
}

//QGISEXTERN QgsOWSSourceSelect * selectWidget( QWidget * parent, Qt::WindowFlags fl )
QGISEXTERN QDialog *selectWidget( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode )
{
  Q_UNUSED( parent );
  Q_UNUSED( fl );
  Q_UNUSED( widgetMode );
  //return new QgsOWSSourceSelect( parent, fl, widgetMode );
  return nullptr;
}*/

QgsGeoNodeConnectionItem::QgsGeoNodeConnectionItem( QgsDataItem *parent, QString name, QString path, QString uri )
  : QgsDataCollectionItem( parent, name, path )
  , mUri( uri )
  , mConnection( name )
{
  mIconName = QStringLiteral( "mIconConnect.png" );
}

QVector<QgsDataItem *> QgsGeoNodeConnectionItem::createChildren()
{
  QVector<QgsDataItem *> services;

  QString wmsUrl = mConnection.serviceUrl( QStringLiteral( "WMS" ) );
  QString wfsUrl = mConnection.serviceUrl( QStringLiteral( "WFS" ) );

  if ( !wmsUrl.isEmpty() )
  {
    QString path = mPath + "/wms";
    QgsDataItem *service = new QgsGeoNodeServiceItem( this, mConnection.mConnName, QString( "WMS" ), path, wmsUrl );
    services.append( service );
  }

  if ( !wfsUrl.isEmpty() )
  {
    QString path = mPath + "/wfs";
    QgsDataItem *service = new QgsGeoNodeServiceItem( this, mConnection.mConnName, QString( "WFS" ), path, wmsUrl );
    services.append( service );
  }

  return services;
}

QgsGeoNodeServiceItem::QgsGeoNodeServiceItem( QgsDataItem *parent, QString connName, QString serviceName, QString path, QString uri )
  : QgsDataCollectionItem( parent, serviceName, path )
  , mServiceName( serviceName )
  , mUri( uri )
  , mConnection( connName )
{
  if ( serviceName == "WMS" )
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
  QVector<QgsDataItem *> layers;
  QVariantList layerList = mConnection.getLayers( mServiceName );

  Q_FOREACH ( QVariant layerMap, layerList )
  {
    QString layerName = layerMap.toMap()["name"].toString();
    QgsDebugMsg( "on createChildren() ASU LAYER NAME = " + layerName );
    QgsGeoNodeLayerItem *layer = new QgsGeoNodeLayerItem( this, mConnection.mConnName, layerName, QString( "" ) );
    layers.append( layer );
  }

  return layers;
}

QgsGeoNodeLayerItem::QgsGeoNodeLayerItem( QgsDataItem *parent, QString connName, QString layerName, QString serviceName )
  : QgsDataCollectionItem( parent, layerName, parent->path() + '/' + layerName )
  , mConnection( connName )
{
  setState( Populated );
  if ( serviceName == "WMS" )
  {
    mIconName = QStringLiteral( "mIconWms.svg" );
  }
  else
  {
    mIconName = QStringLiteral( "mIconWfs.svg" );
  }
}

QgsGeoNodeRootItem::QgsGeoNodeRootItem( QgsDataItem *parent, QString name, QString path ) : QgsDataCollectionItem( parent, name, path )
{
  mCapabilities |= Fast;
  mIconName = QStringLiteral( "mIconGeonode.svg" );
  populate();
}

QVector<QgsDataItem *> QgsGeoNodeRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;

  Q_FOREACH ( const QString &connName, QgsGeoNodeConnection::connectionList() )
  {
    QgsGeoNodeConnection connection( connName );
    QString path = mPath + "/" + connName;
    QgsDataItem *conn = new QgsGeoNodeConnectionItem( this, connName, path, connection.uri().uri() );
    connections.append( conn );
  }
  return connections;
}

/*QList<QAction *> QgsGeoNodeRootItem::actions()
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
}*/

QgsDataItem *QgsOwsDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  if ( path.isEmpty() )
  {
    return new QgsOWSRootItem( parentItem, QStringLiteral( "OWS" ), QStringLiteral( "ows:" ) );
  }
  return nullptr;
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
    if ( QgsGeoNodeConnection::connectionList().contains( connectionName ) )
    {
      QgsGeoNodeConnection connection( connectionName );
      return new QgsGeoNodeConnectionItem( parentItem, QStringLiteral( "GeoNode" ), path, connection.uri().encodedUri() );
    }
  }

  return nullptr;
}
