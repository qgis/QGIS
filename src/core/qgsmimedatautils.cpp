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
               .arg( layerType ).arg( providerKey ).arg( name ).arg( uri )
               .arg( supportedCrs.join( ", " ) )
               .arg( supportedFormats.join( ", " ) ) );
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
  foreach ( const Uri& u, layers )
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
  UriList list;
  while ( !stream.atEnd() )
  {
    stream >> xUri;
    QgsDebugMsg( xUri );
    list.append( Uri( xUri ) );
  }
  return list;
}

QString QgsMimeDataUtils::encode( const QStringList& items )
{
  QString encoded;
  foreach ( const QString& item, items )
  {
    QString str = item;
    str.replace( "\\", "\\\\" );
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

