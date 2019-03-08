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

#include "qgsdataitem.h"
#include "qgslayertree.h"
#include "qgslogger.h"
#include "qgspluginlayer.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsmeshlayer.h"

static const char *QGIS_URILIST_MIMETYPE = "application/x-vnd.qgis.qgis.uri";

QgsMimeDataUtils::Uri::Uri( QString &encData )
{
  QgsDebugMsg( "encData: " + encData );
  QStringList decoded = decode( encData );
  if ( decoded.size() < 4 )
    return;

  layerType = decoded[0];
  providerKey = decoded[1];
  name = decoded[2];
  uri = decoded[3];

  if ( layerType == QLatin1String( "raster" ) && decoded.size() == 6 )
  {
    supportedCrs = decode( decoded[4] );
    supportedFormats = decode( decoded[5] );
  }
  else
  {
    supportedCrs.clear();
    supportedFormats.clear();
  }

  QgsDebugMsg( QStringLiteral( "type:%1 key:%2 name:%3 uri:%4 supportedCRS:%5 supportedFormats:%6" )
               .arg( layerType, providerKey, name, uri,
                     supportedCrs.join( ", " ),
                     supportedFormats.join( ", " ) ) );
}

QString QgsMimeDataUtils::Uri::data() const
{
  return encode( QStringList() << layerType << providerKey << name << uri << encode( supportedCrs ) << encode( supportedFormats ) );
}

QgsVectorLayer *QgsMimeDataUtils::Uri::vectorLayer( bool &owner, QString &error ) const
{
  owner = false;
  if ( layerType != QLatin1String( "vector" ) )
  {
    error = QObject::tr( "%1: Not a vector layer." ).arg( name );
    return nullptr;
  }
  if ( providerKey == QLatin1String( "memory" ) )
  {
    QUrl url = QUrl::fromEncoded( uri.toUtf8() );
    if ( !url.hasQueryItem( QStringLiteral( "pid" ) ) || !url.hasQueryItem( QStringLiteral( "layerid" ) ) )
    {
      error = QObject::tr( "Memory layer uri does not contain process or layer id." );
      return nullptr;
    }
    qint64 pid = url.queryItemValue( QStringLiteral( "pid" ) ).toLongLong();
    if ( pid != QCoreApplication::applicationPid() )
    {
      error = QObject::tr( "Memory layer from another QGIS instance." );
      return nullptr;
    }
    QString layerId = url.queryItemValue( QStringLiteral( "layerid" ) );
    QgsVectorLayer *vectorLayer = QgsProject::instance()->mapLayer<QgsVectorLayer *>( layerId );
    if ( !vectorLayer )
    {
      error = QObject::tr( "Cannot get memory layer." );
      return nullptr;
    }
    return vectorLayer;
  }
  owner = true;
  return new QgsVectorLayer( uri, name, providerKey );
}

QgsRasterLayer *QgsMimeDataUtils::Uri::rasterLayer( bool &owner, QString &error ) const
{
  owner = false;
  if ( layerType != QLatin1String( "raster" ) )
  {
    error = QObject::tr( "%1: Not a raster layer." ).arg( name );
    return nullptr;
  }
  owner = true;
  return new QgsRasterLayer( uri, name, providerKey );
}

QgsMeshLayer *QgsMimeDataUtils::Uri::meshLayer( bool &owner, QString &error ) const
{
  owner = false;
  if ( layerType != QLatin1String( "mesh" ) )
  {
    error = QObject::tr( "%1: Not a mesh layer." ).arg( name );
    return nullptr;
  }
  owner = true;
  return new QgsMeshLayer( uri, name, providerKey );
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
    QgsDebugMsg( xUri );
    list.append( Uri( xUri ) );
  }
  return list;
}


static void _addLayerTreeNodeToUriList( QgsLayerTreeNode *node, QgsMimeDataUtils::UriList &uris )
{
  if ( QgsLayerTree::isGroup( node ) )
  {
    Q_FOREACH ( QgsLayerTreeNode *child, QgsLayerTree::toGroup( node )->children() )
      _addLayerTreeNodeToUriList( child, uris );
  }
  else if ( QgsLayerTree::isLayer( node ) )
  {
    QgsLayerTreeLayer *nodeLayer = QgsLayerTree::toLayer( node );
    QgsMapLayer *layer = nodeLayer->layer();
    if ( !layer )
      return;

    QgsMimeDataUtils::Uri uri;
    uri.name = layer->name();
    uri.uri = layer->dataProvider()->dataSourceUri();
    uri.providerKey = layer->dataProvider()->name();

    switch ( layer->type() )
    {
      case QgsMapLayerType::VectorLayer:
      {
        uri.layerType = QStringLiteral( "vector" );
        if ( uri.providerKey == QStringLiteral( "memory" ) )
        {
          QUrl url = QUrl::fromEncoded( uri.uri.toUtf8() );
          url.addQueryItem( QStringLiteral( "pid" ), QString::number( QCoreApplication::applicationPid() ) );
          url.addQueryItem( QStringLiteral( "layerid" ), layer->id() );
          uri.uri = QString( url.toEncoded() );
        }
        break;
      }
      case QgsMapLayerType::RasterLayer:
      {
        uri.layerType = QStringLiteral( "raster" );
        break;
      }

      case QgsMapLayerType::MeshLayer:
      case QgsMapLayerType::PluginLayer:
      {
        // plugin layers do not have a standard way of storing their URI...
        return;
      }
    }
    uris << uri;
  }
}

QByteArray QgsMimeDataUtils::layerTreeNodesToUriList( const QList<QgsLayerTreeNode *> &nodes )
{
  UriList uris;
  Q_FOREACH ( QgsLayerTreeNode *node, nodes )
    _addLayerTreeNodeToUriList( node, uris );
  return uriListToByteArray( uris );
}

QString QgsMimeDataUtils::encode( const QStringList &items )
{
  QString encoded;
  // Do not escape colon twice
  QRegularExpression re( "(?<!\\\\):" );
  Q_FOREACH ( const QString &item, items )
  {
    QString str = item;
    str.replace( '\\', QLatin1String( "\\\\" ) );
    str.replace( re, QLatin1String( "\\:" ) );
    encoded += str + ':';
  }
  return encoded.left( encoded.length() - 1 );
}

QStringList QgsMimeDataUtils::decode( const QString &encoded )
{
  QStringList items;
  QString item;
  bool inEscape = false;
  Q_FOREACH ( QChar c, encoded )
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
  Q_FOREACH ( const Uri &u, layers )
  {
    stream << u.data();
  }
  return encodedData;
}
