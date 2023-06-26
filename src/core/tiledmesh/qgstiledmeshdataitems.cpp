/***************************************************************************
    qgstiledmeshdataitems.cpp
    ---------------------
    begin                : June 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstiledmeshdataitems.h"
#include "qgstiledmeshconnection.h"
#include "qgsdataprovider.h"

///@cond PRIVATE

QgsTiledMeshRootItem::QgsTiledMeshRootItem( QgsDataItem *parent, QString name, QString path )
  : QgsConnectionsRootItem( parent, name, path, QStringLiteral( "tiled-mesh" ) )
{
  mCapabilities |= Qgis::BrowserItemCapability::Fast;
  mIconName = QStringLiteral( "mIconTiledMeshLayer.svg" );
  populate();
}

QVector<QgsDataItem *> QgsTiledMeshRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;
  const auto connectionList = QgsTiledMeshProviderConnection::connectionList();
  for ( const QString &connName : connectionList )
  {
    const QgsTiledMeshProviderConnection::Data connectionData = QgsTiledMeshProviderConnection::connection( connName );
    const QString uri = QgsTiledMeshProviderConnection::encodedLayerUri( connectionData );
    QgsDataItem *conn = new QgsTiledMeshLayerItem( this, connName, mPath + '/' + connName, uri, connectionData.provider );
    connections.append( conn );
  }
  return connections;
}


// ---------------------------------------------------------------------------


QgsTiledMeshLayerItem::QgsTiledMeshLayerItem( QgsDataItem *parent, QString name, QString path, const QString &encodedUri, const QString &provider )
  : QgsLayerItem( parent, name, path, encodedUri, Qgis::BrowserLayerType::TiledMesh, provider )
{
  setState( Qgis::BrowserItemState::Populated );

  // TODO icon should be taken from associated provider metadata
  mIconName = QStringLiteral( "mIconTiledMeshLayer.svg" );
}


// ---------------------------------------------------------------------------

QString QgsTiledMeshDataItemProvider::name()
{
  return QStringLiteral( "Tiled Mesh" );
}

QString QgsTiledMeshDataItemProvider::dataProviderKey() const
{
  return QStringLiteral( "tiled-mesh" );
}

int QgsTiledMeshDataItemProvider::capabilities() const
{
  return QgsDataProvider::Net;
}

QgsDataItem *QgsTiledMeshDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  if ( path.isEmpty() )
    return new QgsTiledMeshRootItem( parentItem, QStringLiteral( "Tiled Mesh" ), QStringLiteral( "tiled-mesh:" ) );

  return nullptr;
}

///@endcond
