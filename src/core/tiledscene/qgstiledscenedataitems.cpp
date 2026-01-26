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

#include "qgsdataprovider.h"
#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"
#include "qgstiledsceneconnection.h"

#include "moc_qgstiledscenedataitems.cpp"

///@cond PRIVATE

QgsTiledSceneRootItem::QgsTiledSceneRootItem( QgsDataItem *parent, QString name, QString path )
  : QgsConnectionsRootItem( parent, name, path, u"tiled-scene"_s )
{
  mCapabilities |= Qgis::BrowserItemCapability::Fast;
  mIconName = u"mIconTiledScene.svg"_s;
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

  if ( QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( provider ) )
  {
    mIcon = metadata->icon();
  }
  else
  {
    mIconName = u"mIconTiledSceneLayer.svg"_s;
  }
}


// ---------------------------------------------------------------------------

QString QgsTiledSceneDataItemProvider::name()
{
  return u"Scenes"_s;
}

QString QgsTiledSceneDataItemProvider::dataProviderKey() const
{
  return u"tiled-scene"_s;
}

Qgis::DataItemProviderCapabilities QgsTiledSceneDataItemProvider::capabilities() const
{
  return Qgis::DataItemProviderCapability::NetworkSources;
}

QgsDataItem *QgsTiledSceneDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  if ( path.isEmpty() )
    return new QgsTiledSceneRootItem( parentItem, QObject::tr( "Scenes" ), u"tiled-scene:"_s );

  return nullptr;
}

///@endcond
