//
// Created by myarjunar on 25/04/17.
//

#include "qgsproviderregistry.h"
#include "qgsowsdataitems.h"
#include "qgsowsprovider.h"
#include "qgslogger.h"
#include "qgsdatasourceuri.h"
#include "qgsowsconnection.h"
#include "qgsnewhttpconnection.h"
#include "qgsgeonodeconnection.h"
#include "qgsgeonodenewconnection.h"
#include "qgsgeonodedataitems.h"

#include "qgsapplication.h"

#include <QFileInfo>


typedef QList<QgsDataItemProvider *> dataItemProviders_t();

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

  QString wmsUrl = mConnection.serviceUrl( QStringLiteral( "WMS" ) )[0];
  QString wfsUrl = mConnection.serviceUrl( QStringLiteral( "WFS" ) )[0];

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
  QgsGeoNodeNewConnection *nc = new QgsGeoNodeNewConnection( nullptr, mConnection.mConnName );
  nc->setWindowTitle( tr( "Modify GeoNode connection" ) );

  if ( nc->exec() )
  {
    // the parent should be updated
    mParent->refresh();
  }
}

QgsGeoNodeServiceItem::QgsGeoNodeServiceItem( QgsDataItem *parent, QString connName, QString serviceName, QString path, QString uri )
  : QgsDataCollectionItem( parent, serviceName, path )
  , mName( connName )
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
  QVector<QgsDataItem *> children;
  QHash<QgsDataItem *, QString> serviceItems; // service/provider key

  int layerCount = 0;
  // Try to open with service provider
  bool skipProvider = false;
  while ( !skipProvider )
  {
    const QString &key = mServiceName != QString( "WFS" ) ? mServiceName.toLower() : mServiceName;
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

    QString path = "geonode:/" + mName;
    QgsDebugMsg( "path = " + path );

    QgsDataItem *item;
    if ( mServiceName == QString( "WMS" ) )
    {
      Q_FOREACH ( QgsDataItemProvider *pr, dataItemProvidersFn() )
      {
        item = pr->name() == mServiceName ? pr->createDataItem( path, this ) : nullptr;
        if ( item )
        {
          break;
        }
      }
    }
    else
    {
      item = dItem( path, this );  // empty path -> top level
    }

    if ( !item )
    {
      QgsDebugMsg( "Connection not found by provider" );
      skipProvider = true;
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

    skipProvider = true;
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
        replacePath( subItem, providerKey.toLower() + ":/", QStringLiteral( "geonode:/" ) );
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
void QgsGeoNodeServiceItem::replacePath( QgsDataItem *item, QString before, QString after )
{
  item->setPath( item->path().replace( before, after ) );
  Q_FOREACH ( QgsDataItem *subItem, item->children() )
  {
    replacePath( subItem, before, after );
  }
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
    if ( QgsGeoNodeConnection::connectionList().contains( connectionName ) )
    {
      QgsGeoNodeConnection connection( connectionName );
      return new QgsGeoNodeConnectionItem( parentItem, QStringLiteral( "GeoNode" ), path, connection.uri().encodedUri() );
    }
  }

  return nullptr;
}
