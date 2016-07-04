/***************************************************************************
      qgsafsdataitems.cpp
      -------------------
    begin                : Jun 03, 2015
    copyright            : (C) 2015 by Sandro Mani
    email                : smani@sourcepole.ch
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
#include "qgsafsdataitems.h"
#include "qgsafsprovider.h"
#include "qgsarcgisrestutils.h"
#include "qgsafssourceselect.h"

#include <QCoreApplication>
#include <QSettings>
#include <QUrl>


QgsAfsRootItem::QgsAfsRootItem( QgsDataItem* parent, const QString &name, const QString &path )
    : QgsDataCollectionItem( parent, name, path )
{
  mCapabilities |= Fast;
  mIconName = "mIconAfs.svg";
  populate();
}

QVector<QgsDataItem*> QgsAfsRootItem::createChildren()
{
  QVector<QgsDataItem*> connections;

  foreach ( QString connName, QgsOWSConnection::connectionList( "ArcGisFeatureServer" ) )
  {
    QgsOWSConnection connection( "ArcGisFeatureServer", connName );
    QString path = "afs:/" + connName;
    connections.append( new QgsAfsConnectionItem( this, connName, path, connection.uri().param( "url" ) ) );
  }
  return connections;
}

QList<QAction*> QgsAfsRootItem::actions()
{
  QAction* actionNew = new QAction( tr( "New Connection..." ), this );
  connect( actionNew, SIGNAL( triggered() ), this, SLOT( newConnection() ) );
  return QList<QAction*>() << actionNew;
}

QWidget * QgsAfsRootItem::paramWidget()
{
  QgsAfsSourceSelect *select = new QgsAfsSourceSelect( 0, 0, true );
  connect( select, SIGNAL( connectionsChanged() ), this, SLOT( connectionsChanged() ) );
  return select;
}

void QgsAfsRootItem::connectionsChanged()
{
  refresh();
}

void QgsAfsRootItem::newConnection()
{
  QgsNewHttpConnection nc( 0, "/Qgis/connections-afs/" );
  nc.setWindowTitle( tr( "Create a new AFS connection" ) );

  if ( nc.exec() )
  {
    refresh();
  }
}

///////////////////////////////////////////////////////////////////////////////

QgsAfsConnectionItem::QgsAfsConnectionItem( QgsDataItem* parent, const QString &name, const QString &path, const QString &url )
    : QgsDataCollectionItem( parent, name, path )
    , mUrl( url )
{
  mIconName = "mIconAfs.svg";
}

QVector<QgsDataItem*> QgsAfsConnectionItem::createChildren()
{
  QVector<QgsDataItem*> layers;
  QString errorTitle, errorMessage;
  QVariantMap serviceData = QgsArcGisRestUtils::getServiceInfo( mUrl, errorTitle, errorMessage );
  if ( serviceData.isEmpty() )
  {
    return layers;
  }
  QString authid = QgsArcGisRestUtils::parseSpatialReference( serviceData["spatialReference"].toMap() ).authid();

  foreach ( const QVariant& layerInfo, serviceData["layers"].toList() )
  {
    QVariantMap layerInfoMap = layerInfo.toMap();
    QString id = layerInfoMap["id"].toString();
    QgsAfsLayerItem* layer = new QgsAfsLayerItem( this, mName, mUrl + "/" + id, layerInfoMap["name"].toString(), authid );
    layers.append( layer );
  }

  return layers;
}

bool QgsAfsConnectionItem::equal( const QgsDataItem *other )
{
  const QgsAfsConnectionItem *o = dynamic_cast<const QgsAfsConnectionItem *>( other );
  return ( type() == other->type() && o != 0 && mPath == o->mPath && mName == o->mName );
}

QList<QAction*> QgsAfsConnectionItem::actions()
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

void QgsAfsConnectionItem::editConnection()
{
  QgsNewHttpConnection nc( 0, "/Qgis/connections-afs/", mName );
  nc.setWindowTitle( tr( "Modify AFS connection" ) );

  if ( nc.exec() )
  {
    mParent->refresh();
  }
}

void QgsAfsConnectionItem::deleteConnection()
{
  QgsOWSConnection::deleteConnection( "ArcGisFeatureServer", mName );
  mParent->refresh();
}

///////////////////////////////////////////////////////////////////////////////

QgsAfsLayerItem::QgsAfsLayerItem( QgsDataItem* parent, const QString &name, const QString &url, const QString &title, const QString& authid )
    : QgsLayerItem( parent, title, parent->path() + "/" + name, QString(), QgsLayerItem::Vector, "arcgisfeatureserver" )
{
  mUri = QString( "crs='%1' url='%2'" ).arg( authid ).arg( url );
  setState( Populated );
  mIconName = "mIconConnect.png";
}
