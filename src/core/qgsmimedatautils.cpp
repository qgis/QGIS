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
  QStringList parts;
  QChar split = ':';
  QChar escape = '\\';
  QString part;
  bool inEscape = false;
  for ( int i = 0; i < encData.length(); ++i )
  {
    if ( encData.at( i ) == escape && !inEscape )
    {
      inEscape = true;
    }
    else if ( encData.at( i ) == split && !inEscape )
    {
      parts << part;
      part = "";
    }
    else
    {
      part += encData.at( i );
      inEscape = false;
    }
  }
  if ( !part.isEmpty() )
  {
    parts << part;
  }

  if ( parts.size() == 4 )
  {
    layerType = parts[0];
    providerKey = parts[1];
    name = parts[2];
    uri = parts[3];
    QgsDebugMsg( "type: " + layerType + " key: " + providerKey + " name: " + name + " uri: " + uri );
  }
}

QString QgsMimeDataUtils::Uri::data() const
{
  QString escapedName = name;
  QString escapeUri = uri;
  escapedName.replace( ":", "\\:" );
  escapeUri.replace( ":", "\\:" );
  return layerType + ":" + providerKey + ":" + escapedName + ":" + escapeUri;
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
