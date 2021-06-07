/***************************************************************************
    qgsvectortiledataitems.cpp
    ---------------------
    begin                : March 2020
    copyright            : (C) 2020 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/
#include "qgsvectortiledataitems.h"

#include "qgssettings.h"
#include "qgsvectortileconnection.h"

///@cond PRIVATE

QgsVectorTileRootItem::QgsVectorTileRootItem( QgsDataItem *parent, QString name, QString path )
  : QgsConnectionsRootItem( parent, name, path, QStringLiteral( "vectortile" ) )
{
  mCapabilities |= Fast;
  mIconName = QStringLiteral( "mIconVectorTileLayer.svg" );
  populate();
}

QVector<QgsDataItem *> QgsVectorTileRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;
  const auto connectionList = QgsVectorTileProviderConnection::connectionList();
  for ( const QString &connName : connectionList )
  {
    QString uri = QgsVectorTileProviderConnection::encodedLayerUri( QgsVectorTileProviderConnection::connection( connName ) );
    QgsDataItem *conn = new QgsVectorTileLayerItem( this, connName, mPath + '/' + connName, uri );
    connections.append( conn );
  }
  return connections;
}


// ---------------------------------------------------------------------------


QgsVectorTileLayerItem::QgsVectorTileLayerItem( QgsDataItem *parent, QString name, QString path, const QString &encodedUri )
  : QgsLayerItem( parent, name, path, encodedUri, QgsLayerItem::VectorTile, QString() )
{
  setState( Populated );
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
