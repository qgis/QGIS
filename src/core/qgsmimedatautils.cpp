/***************************************************************************
    qgsmimedatautils.cpp
    ---------------------
    begin                : November 2011
    copyright            : (C) 2011 by Martin Dobias
    email                : wonder.sk at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsmimedatautils.h"

#include "qgsdataitem.h"
#include "qgslogger.h"

static const char* QGIS_URILIST_MIMETYPE = "application/x-vnd.qgis.qgis.uri";

QgsMimeDataUtils::Uri::Uri( QgsLayerItem* layerItem )
    : providerKey( layerItem->providerKey() ), name( layerItem->layerName() ), uri( layerItem->uri() )
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
  QRegExp rx( "^([^:]+):([^:]+):([^:]+):(.+)" );
  if ( rx.indexIn( encData ) != -1 )
  {
    layerType = rx.cap( 1 );
    providerKey = rx.cap( 2 );
    name = rx.cap( 3 );
    uri = rx.cap( 4 );
    QgsDebugMsg( "type: " + layerType + " key: " + providerKey + " name: " + name + " uri: " + uri );
  }
}

QString QgsMimeDataUtils::Uri::data() const
{
  return layerType + ":" + providerKey + ":" + name + ":" + uri;
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
  foreach( const QgsMimeDataUtils::Uri& u, layers )
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
