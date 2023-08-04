/***************************************************************************
    qgstiledscenedataitems.cpp
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
#include "qgstiledscenedataitems.h"
#include "qgstiledsceneconnection.h"
#include "qgsdataprovider.h"

///@cond PRIVATE

QgsTiledSceneRootItem::QgsTiledSceneRootItem( QgsDataItem *parent, QString name, QString path )
  : QgsConnectionsRootItem( parent, name, path, QStringLiteral( "tiled-scene" ) )
{
  mCapabilities |= Qgis::BrowserItemCapability::Fast;
  mIconName = QStringLiteral( "mIconTiledSceneLayer.svg" );
  populate();
}

QVector<QgsDataItem *> QgsTiledSceneRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;
  const auto connectionList = QgsTiledSceneProviderConnection::connectionList();
  for ( const QString &connName : connectionList )
  {
    const QgsTiledSceneProviderConnection::Data connectionData = QgsTiledSceneProviderConnection::connection( connName );
    const QString uri = QgsTiledSceneProviderConnection::encodedLayerUri( connectionData );
    QgsDataItem *conn = new QgsTiledSceneLayerItem( this, connName, mPath + '/' + connName, uri, connectionData.provider );
    connections.append( conn );
  }
  return connections;
}


// ---------------------------------------------------------------------------


QgsTiledSceneLayerItem::QgsTiledSceneLayerItem( QgsDataItem *parent, QString name, QString path, const QString &encodedUri, const QString &provider )
  : QgsLayerItem( parent, name, path, encodedUri, Qgis::BrowserLayerType::TiledScene, provider )
{
  setState( Qgis::BrowserItemState::Populated );

  // TODO icon should be taken from associated provider metadata
  mIconName = QStringLiteral( "mIconTiledSceneLayer.svg" );
}


// ---------------------------------------------------------------------------

QString QgsTiledSceneDataItemProvider::name()
{
  return QStringLiteral( "Tiled Scene" );
}

QString QgsTiledSceneDataItemProvider::dataProviderKey() const
{
  return QStringLiteral( "tiled-scene" );
}

int QgsTiledSceneDataItemProvider::capabilities() const
{
  return QgsDataProvider::Net;
}

QgsDataItem *QgsTiledSceneDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  if ( path.isEmpty() )
    return new QgsTiledSceneRootItem( parentItem, QStringLiteral( "Tiled Scene" ), QStringLiteral( "tiled-scene:" ) );

  return nullptr;
}

///@endcond
