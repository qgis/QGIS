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

static const char* QGIS_URILIST_MIMETYPE = "application/x-vnd.qgis.qgis.uri";


QgsMimeDataUtils::Uri::Uri()
{
}

QgsMimeDataUtils::Uri::Uri( QString& encData )
{
  QgsDebugMsg( "encData: " + encData );
  QStringList decoded = decode( encData );
  if ( decoded.size() < 4 )
    return;

  layerType = decoded[0];
  providerKey = decoded[1];
  name = decoded[2];
  uri = decoded[3];

  if ( layerType == "raster" && decoded.size() == 6 )
  {
    supportedCrs = decode( decoded[4] );
    supportedFormats = decode( decoded[5] );
  }
  else
  {
    supportedCrs.clear();
    supportedFormats.clear();
  }

  QgsDebugMsg( QString( "type:%1 key:%2 name:%3 uri:%4 supportedCRS:%5 supportedFormats:%6" )
               .arg( layerType, providerKey, name, uri,
                     supportedCrs.join( ", " ),
                     supportedFormats.join( ", " ) ) );
}

QString QgsMimeDataUtils::Uri::data() const
{
  return encode( QStringList() << layerType << providerKey << name << uri << encode( supportedCrs ) << encode( supportedFormats ) );
}

// -----

bool QgsMimeDataUtils::isUriList( const QMimeData* data )
{
  return data->hasFormat( QGIS_URILIST_MIMETYPE );
}

QMimeData* QgsMimeDataUtils::encodeUriList( const QgsMimeDataUtils::UriList& layers )
{
  QMimeData *mimeData = new QMimeData();

  mimeData->setData( QGIS_URILIST_MIMETYPE, uriListToByteArray( layers ) );
  return mimeData;
}


QgsMimeDataUtils::UriList QgsMimeDataUtils::decodeUriList( const QMimeData* data )
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


static void _addLayerTreeNodeToUriList( QgsLayerTreeNode* node, QgsMimeDataUtils::UriList& uris )
{
  if ( QgsLayerTree::isGroup( node ) )
  {
    Q_FOREACH ( QgsLayerTreeNode* child, QgsLayerTree::toGroup( node )->children() )
      _addLayerTreeNodeToUriList( child, uris );
  }
  else if ( QgsLayerTree::isLayer( node ) )
  {
    QgsLayerTreeLayer* nodeLayer = QgsLayerTree::toLayer( node );
    if ( !nodeLayer->layer() )
      return;

    QgsMimeDataUtils::Uri uri;
    if ( QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer*>( nodeLayer->layer() ) )
    {
      uri.layerType = "vector";
      uri.name = vlayer->name();
      uri.providerKey = vlayer->dataProvider()->name();
      uri.uri = vlayer->dataProvider()->dataSourceUri();
    }
    else if ( QgsRasterLayer* rlayer = qobject_cast<QgsRasterLayer*>( nodeLayer->layer() ) )
    {
      uri.layerType = "raster";
      uri.name = rlayer->name();
      uri.providerKey = rlayer->dataProvider()->name();
      uri.uri = rlayer->dataProvider()->dataSourceUri();
    }
    else
    {
      // plugin layers do not have a standard way of storing their URI...
      return;
    }
    uris << uri;
  }
}

QByteArray QgsMimeDataUtils::layerTreeNodesToUriList( const QList<QgsLayerTreeNode *>& nodes )
{
  UriList uris;
  Q_FOREACH ( QgsLayerTreeNode* node, nodes )
    _addLayerTreeNodeToUriList( node, uris );
  return uriListToByteArray( uris );
}

QString QgsMimeDataUtils::encode( const QStringList& items )
{
  QString encoded;
  Q_FOREACH ( const QString& item, items )
  {
    QString str = item;
    str.replace( '\\', "\\\\" );
    str.replace( ':', "\\:" );
    encoded += str + ':';
  }
  return encoded.left( encoded.length() - 1 );
}

QStringList QgsMimeDataUtils::decode( const QString& encoded )
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
      item = "";
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


QByteArray QgsMimeDataUtils::uriListToByteArray( const QgsMimeDataUtils::UriList& layers )
{
  QByteArray encodedData;

  QDataStream stream( &encodedData, QIODevice::WriteOnly );
  Q_FOREACH ( const Uri& u, layers )
  {
    stream << u.data();
  }
  return encodedData;
}
