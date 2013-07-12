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
#include "qgslogger.h"
#include "qgsnewhttpconnection.h"
#include "qgsowsconnection.h"
#include "qgswfscapabilities.h"
#include "qgswfsdataitems.h"
#include "qgswfsprovider.h"
#include "qgswfssourceselect.h"

#include <QSettings>
#include <QCoreApplication>


QgsWFSLayerItem::QgsWFSLayerItem( QgsDataItem* parent, QString name, QgsDataSourceURI uri, QString featureType, QString title )
    : QgsLayerItem( parent, title, parent->path() + "/" + name, QString(), QgsLayerItem::Vector, "WFS" )
{
  mUri = QgsWFSCapabilities( uri.encodedUri() ).uriGetFeature( featureType );
  mPopulated = true;
  mIcon = QgsApplication::getThemeIcon( "mIconWfs.svg" );
}

QgsWFSLayerItem::~QgsWFSLayerItem()
{
}

////

QgsWFSConnectionItem::QgsWFSConnectionItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path ), mName( name ), mCapabilities( NULL )
{
  mIcon = QgsApplication::getThemeIcon( "mIconWfs.svg" );
}

QgsWFSConnectionItem::~QgsWFSConnectionItem()
{
}

QVector<QgsDataItem*> QgsWFSConnectionItem::createChildren()
{
  mGotCapabilities = false;

  QString encodedUri = mPath;
  QgsDataSourceURI uri;
  uri.setEncodedUri( encodedUri );
  QgsDebugMsg( "encodedUri = " + encodedUri );

  mCapabilities = new QgsWFSCapabilities( encodedUri );
  connect( mCapabilities, SIGNAL( gotCapabilities() ), this, SLOT( gotCapabilities() ) );

  mCapabilities->requestCapabilities();

  while ( !mGotCapabilities )
  {
    QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents );
  }

  QVector<QgsDataItem*> layers;
  if ( mCapabilities->errorCode() == QgsWFSCapabilities::NoError )
  {
    QgsWFSCapabilities::GetCapabilities caps = mCapabilities->capabilities();
    foreach ( const QgsWFSCapabilities::FeatureType& featureType, caps.featureTypes )
    {
      //QgsWFSLayerItem* layer = new QgsWFSLayerItem( this, mName, featureType.name, featureType.title );
      QgsWFSLayerItem* layer = new QgsWFSLayerItem( this, mName, uri, featureType.name, featureType.title );
      layers.append( layer );
    }
  }
  else
  {
    //layers.append( new QgsErrorItem( this, tr( "Failed to retrieve layers" ), mPath + "/error" ) );
    // TODO: show the error without adding child
  }

  mCapabilities->deleteLater();
  mCapabilities = NULL;

  return layers;
}

void QgsWFSConnectionItem::gotCapabilities()
{
  mGotCapabilities = true;
}

QList<QAction*> QgsWFSConnectionItem::actions()
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

void QgsWFSConnectionItem::editConnection()
{
  QgsNewHttpConnection nc( 0, "/Qgis/connections-wfs/", mName );
  nc.setWindowTitle( tr( "Modify WFS connection" ) );

  if ( nc.exec() )
  {
    // the parent should be updated
    mParent->refresh();
  }
}

void QgsWFSConnectionItem::deleteConnection()
{
  QgsOWSConnection::deleteConnection( "WFS", mName );
  // the parent should be updated
  mParent->refresh();
}



//////


QgsWFSRootItem::QgsWFSRootItem( QgsDataItem* parent, QString name, QString path )
    : QgsDataCollectionItem( parent, name, path )
{
  mIcon = QgsApplication::getThemeIcon( "mIconWfs.svg" );

  populate();
}

QgsWFSRootItem::~QgsWFSRootItem()
{
}

QVector<QgsDataItem*> QgsWFSRootItem::createChildren()
{
  QVector<QgsDataItem*> connections;

  foreach ( QString connName, QgsOWSConnection::connectionList( "WFS" ) )
  {
    QgsOWSConnection connection( "WFS", connName );
    QgsDataItem * conn = new QgsWFSConnectionItem( this, connName, connection.uri().encodedUri() );
    conn->setIcon( QgsApplication::getThemeIcon( "mIconConnect.png" ) );
    connections.append( conn );
  }
  return connections;
}

QList<QAction*> QgsWFSRootItem::actions()
{
  QList<QAction*> lst;

  QAction* actionNew = new QAction( tr( "New Connection..." ), this );
  connect( actionNew, SIGNAL( triggered() ), this, SLOT( newConnection() ) );
  lst.append( actionNew );

  return lst;
}

QWidget * QgsWFSRootItem::paramWidget()
{
  QgsWFSSourceSelect *select = new QgsWFSSourceSelect( 0, 0, true );
  connect( select, SIGNAL( connectionsChanged() ), this, SLOT( connectionsChanged() ) );
  return select;
}

void QgsWFSRootItem::connectionsChanged()
{
  refresh();
}

void QgsWFSRootItem::newConnection()
{
  QgsNewHttpConnection nc( 0, "/Qgis/connections-wfs/" );
  nc.setWindowTitle( tr( "Create a new WFS connection" ) );

  if ( nc.exec() )
  {
    refresh();
  }
}

// ---------------------------------------------------------------------------

QGISEXTERN QgsWFSSourceSelect * selectWidget( QWidget * parent, Qt::WFlags fl )
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
    return new QgsWFSRootItem( parentItem, "WFS", "wfs:" );
  }

  return new QgsWFSConnectionItem( parentItem, "WFS", thePath );
}
