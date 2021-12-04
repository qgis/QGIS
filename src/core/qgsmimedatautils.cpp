/***************************************************************************
    qgsmimedatautils.cpp
    ---------------------
    begin                : November 2011
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
#include <QStringList>

#include "qgsmimedatautils.h"

#include "qgslayertree.h"
#include "qgslogger.h"
#include "qgspluginlayer.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsmeshlayer.h"

#include <QRegularExpression>

static const char *QGIS_URILIST_MIMETYPE = "application/x-vnd.qgis.qgis.uri";

QgsMimeDataUtils::Uri::Uri( const QString &encData )
{
  QgsDebugMsgLevel( "encData: " + encData, 4 );
  const QStringList decoded = decode( encData );
  if ( decoded.size() < 4 )
    return;

  layerType = decoded[0];
  providerKey = decoded[1];
  name = decoded[2];
  uri = decoded[3];

  if ( layerType == QLatin1String( "raster" ) && decoded.size() >= 6 )
  {
    supportedCrs = decode( decoded[4] );
    supportedFormats = decode( decoded[5] );
  }
  else
  {
    supportedCrs.clear();
    supportedFormats.clear();
  }

  if ( decoded.size() > 6 )
    layerId = decoded.at( 6 );
  if ( decoded.size() > 7 )
    pId = decoded.at( 7 );
  if ( decoded.size() > 8 )
    wkbType = QgsWkbTypes::parseType( decoded.at( 8 ) );
  if ( decoded.size() > 9 )
    filePath = decoded.at( 9 );

  QgsDebugMsgLevel( QStringLiteral( "type:%1 key:%2 name:%3 uri:%4 supportedCRS:%5 supportedFormats:%6" )
                    .arg( layerType, providerKey, name, uri,
                          supportedCrs.join( ',' ),
                          supportedFormats.join( ',' ) ), 2 );
}

QgsMimeDataUtils::Uri::Uri( QgsMapLayer *layer )
  : providerKey( layer->providerType() )
  , name( layer->name() )
  , uri( layer->dataProvider() ? layer->dataProvider()->dataSourceUri() : layer->source() )
  , layerId( layer->id() )
  , pId( QString::number( QCoreApplication::applicationPid() ) )
{
  switch ( layer->type() )
  {
    case QgsMapLayerType::VectorLayer:
    {
      layerType = QStringLiteral( "vector" );
      wkbType = qobject_cast< QgsVectorLayer *>( layer )->wkbType();
      break;
    }
    case QgsMapLayerType::RasterLayer:
    {
      layerType = QStringLiteral( "raster" );
      break;
    }

    case QgsMapLayerType::MeshLayer:
    {
      layerType = QStringLiteral( "mesh" );
      break;
    }
    case QgsMapLayerType::PointCloudLayer:
    {
      layerType = QStringLiteral( "pointcloud" );
      break;
    }
    case QgsMapLayerType::VectorTileLayer:
    {
      layerType = QStringLiteral( "vector-tile" );
      break;
    }

    case QgsMapLayerType::PluginLayer:
    case QgsMapLayerType::GroupLayer:
    case QgsMapLayerType::AnnotationLayer:
    {
      // plugin layers do not have a standard way of storing their URI...
      return;
    }
  }
}

QString QgsMimeDataUtils::Uri::data() const
{
  return encode( { layerType,
                   providerKey,
                   name,
                   uri,
                   encode( supportedCrs ),
                   encode( supportedFormats ),
                   layerId,
                   pId,
                   QgsWkbTypes::displayString( wkbType ),
                   filePath
                 } );
}

QgsVectorLayer *QgsMimeDataUtils::Uri::vectorLayer( bool &owner, QString &error ) const
{
  owner = false;
  error.clear();
  if ( layerType != QLatin1String( "vector" ) )
  {
    error = QObject::tr( "%1: Not a vector layer." ).arg( name );
    return nullptr;
  }

  if ( !layerId.isEmpty() && QgsMimeDataUtils::hasOriginatedFromCurrentAppInstance( *this ) )
  {
    if ( QgsVectorLayer *vectorLayer = QgsProject::instance()->mapLayer<QgsVectorLayer *>( layerId ) )
    {
      return vectorLayer;
    }
  }
  if ( providerKey == QLatin1String( "memory" ) )
  {
    error = QObject::tr( "Cannot get memory layer." );
    return nullptr;
  }

  owner = true;
  const QgsVectorLayer::LayerOptions options { QgsProject::instance()->transformContext() };
  return new QgsVectorLayer( uri, name, providerKey, options );
}

QgsRasterLayer *QgsMimeDataUtils::Uri::rasterLayer( bool &owner, QString &error ) const
{
  owner = false;
  error.clear();
  if ( layerType != QLatin1String( "raster" ) )
  {
    error = QObject::tr( "%1: Not a raster layer." ).arg( name );
    return nullptr;
  }

  if ( !layerId.isEmpty() && QgsMimeDataUtils::hasOriginatedFromCurrentAppInstance( *this ) )
  {
    if ( QgsRasterLayer *rasterLayer = QgsProject::instance()->mapLayer<QgsRasterLayer *>( layerId ) )
    {
      return rasterLayer;
    }
  }

  owner = true;
  return new QgsRasterLayer( uri, name, providerKey );
}

QgsMeshLayer *QgsMimeDataUtils::Uri::meshLayer( bool &owner, QString &error ) const
{
  owner = false;
  error.clear();
  if ( layerType != QLatin1String( "mesh" ) )
  {
    error = QObject::tr( "%1: Not a mesh layer." ).arg( name );
    return nullptr;
  }

  if ( !layerId.isEmpty() && QgsMimeDataUtils::hasOriginatedFromCurrentAppInstance( *this ) )
  {
    if ( QgsMeshLayer *meshLayer = QgsProject::instance()->mapLayer<QgsMeshLayer *>( layerId ) )
    {
      return meshLayer;
    }
  }

  owner = true;
  return new QgsMeshLayer( uri, name, providerKey );
}

QgsMapLayer *QgsMimeDataUtils::Uri::mapLayer() const
{
  if ( !layerId.isEmpty() && QgsMimeDataUtils::hasOriginatedFromCurrentAppInstance( *this ) )
  {
    return QgsProject::instance()->mapLayer( layerId );
  }
  return nullptr;
}

// -----

bool QgsMimeDataUtils::isUriList( const QMimeData *data )
{
  return data->hasFormat( QGIS_URILIST_MIMETYPE );
}

QMimeData *QgsMimeDataUtils::encodeUriList( const QgsMimeDataUtils::UriList &layers )
{
  QMimeData *mimeData = new QMimeData();

  mimeData->setData( QGIS_URILIST_MIMETYPE, uriListToByteArray( layers ) );
  return mimeData;
}


QgsMimeDataUtils::UriList QgsMimeDataUtils::decodeUriList( const QMimeData *data )
{
  QByteArray encodedData = data->data( QGIS_URILIST_MIMETYPE );
  QDataStream stream( &encodedData, QIODevice::ReadOnly );
  QString xUri; // extended uri: layer_type:provider_key:uri
  UriList list;
  while ( !stream.atEnd() )
  {
    stream >> xUri;
    QgsDebugMsgLevel( xUri, 4 );
    list.append( Uri( xUri ) );
  }
  return list;
}


static void _addLayerTreeNodeToUriList( QgsLayerTreeNode *node, QgsMimeDataUtils::UriList &uris )
{
  if ( QgsLayerTree::isGroup( node ) )
  {
    const auto constChildren = QgsLayerTree::toGroup( node )->children();
    for ( QgsLayerTreeNode *child : constChildren )
      _addLayerTreeNodeToUriList( child, uris );
  }
  else if ( QgsLayerTree::isLayer( node ) )
  {
    QgsLayerTreeLayer *nodeLayer = QgsLayerTree::toLayer( node );
    QgsMapLayer *layer = nodeLayer->layer();
    if ( !layer )
      return;

    if ( layer->type() == QgsMapLayerType::PluginLayer )
      return; // plugin layers do not have a standard way of storing their URI...

    uris << QgsMimeDataUtils::Uri( layer );
  }
}

QByteArray QgsMimeDataUtils::layerTreeNodesToUriList( const QList<QgsLayerTreeNode *> &nodes )
{
  UriList uris;
  const auto constNodes = nodes;
  for ( QgsLayerTreeNode *node : constNodes )
    _addLayerTreeNodeToUriList( node, uris );
  return uriListToByteArray( uris );
}

bool QgsMimeDataUtils::hasOriginatedFromCurrentAppInstance( const QgsMimeDataUtils::Uri &uri )
{
  if ( uri.pId.isEmpty() )
    return false;

  const qint64 pid = uri.pId.toLongLong();
  return pid == QCoreApplication::applicationPid();
}

QString QgsMimeDataUtils::encode( const QStringList &items )
{
  QString encoded;
  // Do not escape colon twice
  const QRegularExpression re( QStringLiteral( "(?<!\\\\):" ) );
  const auto constItems = items;
  for ( const QString &item : constItems )
  {
    QString str = item;
    str.replace( '\\', QLatin1String( "\\\\" ) );
    str.replace( re, QStringLiteral( "\\:" ) );
    encoded += str + ':';
  }
  return encoded.left( encoded.length() - 1 );
}

QStringList QgsMimeDataUtils::decode( const QString &encoded )
{
  QStringList items;
  QString item;
  bool inEscape = false;
  const auto constEncoded = encoded;
  for ( const QChar c : constEncoded )
  {
    if ( c == '\\' && inEscape )
    {
      item += c;
    }
    else if ( c == '\\' )
    {
      inEscape = true;
    }
    else if ( c == ':' && !inEscape )
    {
      items.append( item );
      item.clear();
    }
    else
    {
      item += c;
      inEscape = false;
    }
  }
  items.append( item );
  return items;
}


QByteArray QgsMimeDataUtils::uriListToByteArray( const QgsMimeDataUtils::UriList &layers )
{
  QByteArray encodedData;

  QDataStream stream( &encodedData, QIODevice::WriteOnly );
  const auto constLayers = layers;
  for ( const Uri &u : constLayers )
  {
    stream << u.data();
  }
  return encodedData;
}
