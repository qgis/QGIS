/***************************************************************************
    qgswfsdataitems.cpp
    ---------------------
    begin                : October 2011
    copyright            : (C) 2011 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsdataitemprovider.h"
#include "qgsdataprovider.h"
#include "qgslogger.h"
#include "qgswfsconstants.h"
#include "qgswfsconnection.h"
#include "qgswfscapabilities.h"
#include "qgswfsdataitems.h"
#include "qgswfsdatasourceuri.h"
#include "qgssettings.h"
#include "qgsgeonodeconnection.h"
#include "qgsgeonoderequest.h"

#ifdef HAVE_GUI
#include "qgsnewhttpconnection.h"
#include "qgswfssourceselect.h"
#endif

#include <QCoreApplication>
#include <QEventLoop>


QgsWfsLayerItem::QgsWfsLayerItem( QgsDataItem *parent, QString name, const QgsDataSourceUri &uri, QString featureType, QString title, QString crsString )
  : QgsLayerItem( parent, title, parent->path() + '/' + name, QString(), QgsLayerItem::Vector, QStringLiteral( "WFS" ) )
{
  QgsSettings settings;
  bool useCurrentViewExtent = settings.value( QStringLiteral( "Windows/WFSSourceSelect/FeatureCurrentViewExtent" ), true ).toBool();
  mUri = QgsWFSDataSourceURI::build( uri.uri(), featureType, crsString, QString(), useCurrentViewExtent );
  setState( Populated );
  mIconName = QStringLiteral( "mIconConnect.png" );
}

QgsWfsLayerItem::~QgsWfsLayerItem()
{
}

////

QgsWfsConnectionItem::QgsWfsConnectionItem( QgsDataItem *parent, QString name, QString path, QString uri )
  : QgsDataCollectionItem( parent, name, path )
  , mUri( uri )
  , mWfsCapabilities( nullptr )
{
  mIconName = QStringLiteral( "mIconWfs.svg" );
  mCapabilities |= Collapse;
}

QgsWfsConnectionItem::~QgsWfsConnectionItem()
{
}

QVector<QgsDataItem *> QgsWfsConnectionItem::createChildren()
{
  QgsDataSourceUri uri( mUri );
  QgsDebugMsg( "mUri = " + mUri );

  QgsWfsCapabilities capabilities( mUri );

  const bool synchronous = true;
  const bool forceRefresh = false;
  capabilities.requestCapabilities( synchronous, forceRefresh );

  QVector<QgsDataItem *> layers;
  if ( capabilities.errorCode() == QgsWfsCapabilities::NoError )
  {
    QgsWfsCapabilities::Capabilities caps = capabilities.capabilities();
    Q_FOREACH ( const QgsWfsCapabilities::FeatureType &featureType, caps.featureTypes )
    {
      //QgsWFSLayerItem* layer = new QgsWFSLayerItem( this, mName, featureType.name, featureType.title );
      QgsWfsLayerItem *layer = new QgsWfsLayerItem( this, mName, uri, featureType.name, featureType.title, featureType.crslist.first() );
      layers.append( layer );
    }
  }
  else
  {
    //layers.append( new QgsErrorItem( this, tr( "Failed to retrieve layers" ), mPath + "/error" ) );
    // TODO: show the error without adding child
  }

  return layers;
}

#ifdef HAVE_GUI
QList<QAction *> QgsWfsConnectionItem::actions()
{
  QList<QAction *> lst;

  QAction *actionEdit = new QAction( tr( "Edit..." ), this );
  connect( actionEdit, &QAction::triggered, this, &QgsWfsConnectionItem::editConnection );
  lst.append( actionEdit );

  QAction *actionDelete = new QAction( tr( "Delete" ), this );
  connect( actionDelete, &QAction::triggered, this, &QgsWfsConnectionItem::deleteConnection );
  lst.append( actionDelete );

  return lst;
}

void QgsWfsConnectionItem::editConnection()
{
  QgsNewHttpConnection nc( nullptr, QgsWFSConstants::CONNECTIONS_WFS, mName );
  nc.setWindowTitle( tr( "Modify WFS Connection" ) );

  if ( nc.exec() )
  {
    // the parent should be updated
    mParent->refreshConnections();
  }
}

void QgsWfsConnectionItem::deleteConnection()
{
  QgsWfsConnection::deleteConnection( mName );
  // the parent should be updated
  mParent->refreshConnections();
}
#endif


//////


QgsWfsRootItem::QgsWfsRootItem( QgsDataItem *parent, QString name, QString path )
  : QgsDataCollectionItem( parent, name, path )
{
  mCapabilities |= Fast;
  mIconName = QStringLiteral( "mIconWfs.svg" );
  populate();
}

QgsWfsRootItem::~QgsWfsRootItem()
{
}

QVector<QgsDataItem *> QgsWfsRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;

  Q_FOREACH ( const QString &connName, QgsWfsConnection::connectionList() )
  {
    QgsWfsConnection connection( connName );
    QString path = "wfs:/" + connName;
    QgsDataItem *conn = new QgsWfsConnectionItem( this, connName, path, connection.uri().uri() );
    connections.append( conn );
  }
  return connections;
}

#ifdef HAVE_GUI
QList<QAction *> QgsWfsRootItem::actions()
{
  QList<QAction *> lst;

  QAction *actionNew = new QAction( tr( "New Connection..." ), this );
  connect( actionNew, &QAction::triggered, this, &QgsWfsRootItem::newConnection );
  lst.append( actionNew );

  return lst;
}

QWidget *QgsWfsRootItem::paramWidget()
{
  QgsWFSSourceSelect *select = new QgsWFSSourceSelect( nullptr, 0, QgsProviderRegistry::WidgetMode::Manager );
  connect( select, &QgsWFSSourceSelect::connectionsChanged, this, &QgsWfsRootItem::connectionsChanged );
  return select;
}

void QgsWfsRootItem::connectionsChanged()
{
  refresh();
}

void QgsWfsRootItem::newConnection()
{
  QgsNewHttpConnection nc( nullptr, QgsWFSConstants::CONNECTIONS_WFS );
  nc.setWindowTitle( tr( "Create a New WFS Connection" ) );

  if ( nc.exec() )
  {
    refreshConnections();
  }
}
#endif


//////


QgsDataItem *QgsWfsDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  QgsDebugMsg( "thePath = " + path );
  if ( path.isEmpty() )
  {
    return new QgsWfsRootItem( parentItem, QStringLiteral( "WFS" ), QStringLiteral( "wfs:" ) );
  }

  // path schema: wfs:/connection name (used by OWS)
  if ( path.startsWith( QLatin1String( "wfs:/" ) ) )
  {
    QString connectionName = path.split( '/' ).last();
    if ( QgsWfsConnection::connectionList().contains( connectionName ) )
    {
      QgsWfsConnection connection( connectionName );
      return new QgsWfsConnectionItem( parentItem, QStringLiteral( "WFS" ), path, connection.uri().uri() );
    }
  }
  else if ( path.startsWith( QLatin1String( "geonode:/" ) ) )
  {
    QString connectionName = path.split( '/' ).last();
    if ( QgsGeoNodeConnectionUtils::connectionList().contains( connectionName ) )
    {
      QgsGeoNodeConnection connection( connectionName );

      QString url = connection.uri().param( "url" );
      QgsGeoNodeRequest geonodeRequest( url, true );

      QgsWFSDataSourceURI sourceUri( geonodeRequest.fetchServiceUrlsBlocking( QStringLiteral( "WFS" ) )[0] );

      QgsDebugMsg( QString( "WFS full uri: '%1'." ).arg( QString( sourceUri.uri() ) ) );

      return new QgsWfsConnectionItem( parentItem, QStringLiteral( "WFS" ), path, sourceUri.uri() );
    }
  }

  return nullptr;
}

QVector<QgsDataItem *> QgsWfsDataItemProvider::createDataItems( const QString &path, QgsDataItem *parentItem )
{
  QVector<QgsDataItem *> items;
  if ( path.startsWith( QLatin1String( "geonode:/" ) ) )
  {
    QString connectionName = path.split( '/' ).last();
    if ( QgsGeoNodeConnectionUtils::connectionList().contains( connectionName ) )
    {
      QgsGeoNodeConnection connection( connectionName );

      QString url = connection.uri().param( "url" );
      QgsGeoNodeRequest geonodeRequest( url, true );

      QStringList encodedUris( geonodeRequest.fetchServiceUrlsBlocking( QStringLiteral( "WFS" ) ) );

      if ( !encodedUris.isEmpty() )
      {
        Q_FOREACH ( QString encodedUri, encodedUris )
        {
          QgsWFSDataSourceURI uri( encodedUri );
          QgsDebugMsg( QString( "WFS full uri: '%1'." ).arg( QString( uri.uri() ) ) );

          QgsDataItem *item = new QgsWfsConnectionItem( parentItem, QStringLiteral( "WFS" ), path, uri.uri() );
          if ( item )
          {
            items.append( item );
          }
        }
      }
    }
  }

  return items;
}

// ---------------------------------------------------------------------------

#ifdef HAVE_GUI
QGISEXTERN QgsWFSSourceSelect *selectWidget( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode )
{
  return new QgsWFSSourceSelect( parent, fl, widgetMode );
}
#endif

QGISEXTERN int dataCapabilities()
{
  return  QgsDataProvider::Net;
}

QGISEXTERN QgsDataItem *dataItem( QString path, QgsDataItem *parentItem )
{
  QgsDebugMsg( "path = " + path );
  if ( path.isEmpty() )
  {
    return new QgsWfsRootItem( parentItem, QStringLiteral( "WFS" ), QStringLiteral( "wfs:" ) );
  }

  // path schema: wfs:/connection name (used by OWS)
  if ( path.startsWith( QLatin1String( "wfs:/" ) ) )
  {
    QString connectionName = path.split( '/' ).last();
    if ( QgsWfsConnection::connectionList().contains( connectionName ) )
    {
      QgsWfsConnection connection( connectionName );
      return new QgsWfsConnectionItem( parentItem, QStringLiteral( "WFS" ), path, connection.uri().uri() );
    }
  }

  return nullptr;
}

QGISEXTERN QList<QgsDataItemProvider *> *dataItemProviders()
{
  QList<QgsDataItemProvider *> *providers = new QList<QgsDataItemProvider *>();

  *providers << new QgsWfsDataItemProvider;

  return providers;
}
