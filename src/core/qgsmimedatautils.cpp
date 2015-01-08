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
#include "qgslogger.h"

static const char* QGIS_URILIST_MIMETYPE = "application/x-vnd.qgis.qgis.uri";

QgsMimeDataUtils::Uri::Uri( QgsLayerItem* layerItem )
    : providerKey( layerItem->providerKey() )
    , name( layerItem->layerName() )
    , uri( layerItem->uri() )
    , supportedCrs( layerItem->supportedCRS() )
    , supportedFormats( layerItem->supportedFormats() )
{
  switch ( layerItem->mapLayerType() )
  {
    case QgsMapLayer::VectorLayer:
      layerType = "vector";
      break;
    case QgsMapLayer::RasterLayer:
      layerType = "raster";
      break;
    case QgsMapLayer::PluginLayer:
      layerType = "plugin";
      break;
  }
}

QgsMimeDataUtils::Uri::Uri( QString& encData )
{
  QStringList decoded = decode( encData );
  if ( decoded.size() == 6 )
  {
    layerType = decoded[0];
    providerKey = decoded[1];
    name = decoded[2];
    uri = decoded[3];
    supportedCrs = decode( decoded[4] );
    supportedFormats = decode( decoded[5] );
    QgsDebugMsg( "type: " + layerType + " key: " + providerKey + " name: " + name + " uri: " + uri + " supportedCRS: " + decoded[4] + " supportedFormats: " + decoded[5] );
  }
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

QMimeData* QgsMimeDataUtils::encodeUriList( QgsMimeDataUtils::UriList layers )
{
  QMimeData *mimeData = new QMimeData();
  QByteArray encodedData;

  QDataStream stream( &encodedData, QIODevice::WriteOnly );
  foreach ( const QgsMimeDataUtils::Uri& u, layers )
  {
    stream << u.data();
  }

  mimeData->setData( QGIS_URILIST_MIMETYPE, encodedData );
  return mimeData;
}


QgsMimeDataUtils::UriList QgsMimeDataUtils::decodeUriList( const QMimeData* data )
{
  QByteArray encodedData = data->data( QGIS_URILIST_MIMETYPE );
  QDataStream stream( &encodedData, QIODevice::ReadOnly );
  QString xUri; // extended uri: layer_type:provider_key:uri
  QgsMimeDataUtils::UriList list;
  while ( !stream.atEnd() )
  {
    stream >> xUri;
    QgsDebugMsg( xUri );
    list.append( QgsMimeDataUtils::Uri( xUri ) );
  }
  return list;
}

QString QgsMimeDataUtils::encode( const QStringList& items )
{
  QString encoded;
  foreach ( const QString& item, items )
  {
    QString str = item;
    str.replace( ":", "\\:" );
    encoded += str + ":";
  }
  return encoded.left( encoded.length() - 1 );
}

QStringList QgsMimeDataUtils::decode( const QString& encoded )
{
  QStringList items;
  QString item;
  bool inEscape = false;
  foreach ( const QChar& c, encoded )
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

