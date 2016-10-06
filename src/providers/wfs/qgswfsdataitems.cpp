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
#include "qgsdataprovider.h"
#include "qgslogger.h"
#include "qgsnewhttpconnection.h"
#include "qgswfsconstants.h"
#include "qgswfsconnection.h"
#include "qgswfscapabilities.h"
#include "qgswfsdataitems.h"
#include "qgswfssourceselect.h"
#include "qgswfsdatasourceuri.h"

#include <QSettings>
#include <QCoreApplication>
#include <QEventLoop>


QgsWfsLayerItem::QgsWfsLayerItem( QgsDataItem* parent, QString name, const QgsDataSourceUri& uri, QString featureType, QString title, QString crsString )
    : QgsLayerItem( parent, title, parent->path() + '/' + name, QString(), QgsLayerItem::Vector, "WFS" )
{
  QSettings settings;
  bool useCurrentViewExtent = settings.value( "/Windows/WFSSourceSelect/FeatureCurrentViewExtent", true ).toBool();
  mUri = QgsWFSDataSourceURI::build( uri.uri(), featureType, crsString, QString(), useCurrentViewExtent );
  setState( Populated );
  mIconName = "mIconConnect.png";
}

QgsWfsLayerItem::~QgsWfsLayerItem()
{
}

////

QgsWfsConnectionItem::QgsWfsConnectionItem( QgsDataItem* parent, QString name, QString path, QString uri )
    : QgsDataCollectionItem( parent, name, path )
    , mUri( uri )
    , mCapabilities( nullptr )
{
  mIconName = "mIconWfs.svg";
}

QgsWfsConnectionItem::~QgsWfsConnectionItem()
{
}

QVector<QgsDataItem*> QgsWfsConnectionItem::createChildren()
{
  QgsDataSourceUri uri( mUri );
  QgsDebugMsg( "mUri = " + mUri );

  QgsWfsCapabilities capabilities( mUri );

  const bool synchronous = true;
  const bool forceRefresh = false;
  capabilities.requestCapabilities( synchronous, forceRefresh );

  QVector<QgsDataItem*> layers;
  if ( capabilities.errorCode() == QgsWfsCapabilities::NoError )
  {
    QgsWfsCapabilities::Capabilities caps = capabilities.capabilities();
    Q_FOREACH ( const QgsWfsCapabilities::FeatureType& featureType, caps.featureTypes )
    {
      //QgsWFSLayerItem* layer = new QgsWFSLayerItem( this, mName, featureType.name, featureType.title );
      QgsWfsLayerItem* layer = new QgsWfsLayerItem( this, mName, uri, featureType.name, featureType.title, featureType.crslist.first() );
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

QList<QAction*> QgsWfsConnectionItem::actions()
{
  QList<QAction*> lst;

  QAction* actionEdit = new QAction( tr( "Edit..." ), this );
  connect( actionEdit, SIGNAL( triggered() ), this, SLOT( editConnection() ) );
  lst.append( actionEdit );

  QAction* actionDelete = new QAction( tr( "Delete" ), this );
  connect( actionDelete, SIGNAL( triggered() ), this, SLOT( deleteConnection() ) );
  lst.append( actionDelete );

  return lst;
}

void QgsWfsConnectionItem::editConnection()
{
  QgsNewHttpConnection nc( nullptr, QgsWFSConstants::CONNECTIONS_WFS, mName );
  nc.setWindowTitle( tr( "Modify WFS connection" ) );

  if ( nc.exec() )
  {
    // the parent should be updated
    mParent->refresh();
  }
}

void QgsWfsConnectionItem::deleteConnection()
{
  QgsWfsConnection::deleteConnection( mName );
  // the parent should be updated
  mParent->refresh();
}



//////


QgsWfsRootItem::QgsWfsRootItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
  mCapabilities |= Fast;
  mIconName = "mIconWfs.svg";
  populate();
}

QgsWfsRootItem::~QgsWfsRootItem()
{
}

QVector<QgsDataItem*> QgsWfsRootItem::createChildren()
{
  QVector<QgsDataItem*> connections;

  Q_FOREACH ( const QString& connName, QgsWfsConnection::connectionList() )
  {
    QgsWfsConnection connection( connName );
    QString path = "wfs:/" + connName;
    QgsDataItem * conn = new QgsWfsConnectionItem( this, connName, path, connection.uri().uri() );
    connections.append( conn );
  }
  return connections;
}

QList<QAction*> QgsWfsRootItem::actions()
{
  QList<QAction*> lst;

  QAction* actionNew = new QAction( tr( "New Connection..." ), this );
  connect( actionNew, SIGNAL( triggered() ), this, SLOT( newConnection() ) );
  lst.append( actionNew );

  return lst;
}

QWidget * QgsWfsRootItem::paramWidget()
{
  QgsWFSSourceSelect *select = new QgsWFSSourceSelect( nullptr, 0, true );
  connect( select, SIGNAL( connectionsChanged() ), this, SLOT( connectionsChanged() ) );
  return select;
}

void QgsWfsRootItem::connectionsChanged()
{
  refresh();
}

void QgsWfsRootItem::newConnection()
{
  QgsNewHttpConnection nc( nullptr, QgsWFSConstants::CONNECTIONS_WFS );
  nc.setWindowTitle( tr( "Create a new WFS connection" ) );

  if ( nc.exec() )
  {
    refresh();
  }
}

// ---------------------------------------------------------------------------

QGISEXTERN QgsWFSSourceSelect * selectWidget( QWidget * parent, Qt::WindowFlags fl )
{
  return new QgsWFSSourceSelect( parent, fl );
}

QGISEXTERN int dataCapabilities()
{
  return  QgsDataProvider::Net;
}

QGISEXTERN QgsDataItem * dataItem( QString thePath, QgsDataItem* parentItem )
{
  QgsDebugMsg( "thePath = " + thePath );
  if ( thePath.isEmpty() )
  {
    return new QgsWfsRootItem( parentItem, "WFS", "wfs:" );
  }

  // path schema: wfs:/connection name (used by OWS)
  if ( thePath.startsWith( "wfs:/" ) )
  {
    QString connectionName = thePath.split( '/' ).last();
    if ( QgsWfsConnection::connectionList().contains( connectionName ) )
    {
      QgsWfsConnection connection( connectionName );
      return new QgsWfsConnectionItem( parentItem, "WFS", thePath, connection.uri().uri() );
    }
  }

  return nullptr;
}
