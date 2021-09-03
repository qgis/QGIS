/***************************************************************************
    qgsvectortiledataitems.cpp
    ---------------------
    begin                : March 2020
    copyright            : (C) 2020 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsvectortiledataitems.h"

#include "qgssettings.h"
#include "qgsvectortileconnection.h"
#include "qgsdataprovider.h"

///@cond PRIVATE

QgsVectorTileRootItem::QgsVectorTileRootItem( QgsDataItem *parent, QString name, QString path )
  : QgsConnectionsRootItem( parent, name, path, QStringLiteral( "vectortile" ) )
{
  mCapabilities |= Qgis::BrowserItemCapability::Fast;
  mIconName = QStringLiteral( "mIconVectorTileLayer.svg" );
  populate();
}

QVector<QgsDataItem *> QgsVectorTileRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;
  const auto connectionList = QgsVectorTileProviderConnection::connectionList();
  for ( const QString &connName : connectionList )
  {
    const QString uri = QgsVectorTileProviderConnection::encodedLayerUri( QgsVectorTileProviderConnection::connection( connName ) );
    QgsDataItem *conn = new QgsVectorTileLayerItem( this, connName, mPath + '/' + connName, uri );
    connections.append( conn );
  }
  return connections;
}


// ---------------------------------------------------------------------------


QgsVectorTileLayerItem::QgsVectorTileLayerItem( QgsDataItem *parent, QString name, QString path, const QString &encodedUri )
  : QgsLayerItem( parent, name, path, encodedUri, Qgis::BrowserLayerType::VectorTile, QString() )
{
  setState( Qgis::BrowserItemState::Populated );
  mIconName = QStringLiteral( "mIconVectorTileLayer.svg" );
}


// ---------------------------------------------------------------------------

QString QgsVectorTileDataItemProvider::name()
{
  return QStringLiteral( "Vector Tiles" );
}

QString QgsVectorTileDataItemProvider::dataProviderKey() const
{
  return QStringLiteral( "vectortile" );
}

int QgsVectorTileDataItemProvider::capabilities() const
{
  return QgsDataProvider::Net;
}

QgsDataItem *QgsVectorTileDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  if ( path.isEmpty() )
    return new QgsVectorTileRootItem( parentItem, QStringLiteral( "Vector Tiles" ), QStringLiteral( "vectortile:" ) );
  return nullptr;
}

///@endcond
