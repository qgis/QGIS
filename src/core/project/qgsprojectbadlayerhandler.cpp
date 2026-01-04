/***************************************************************************
  qgsprojectbadlayerhandler.cpp - QgsProjectBadLayerHandler

 ---------------------
 begin                : 22.10.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsprojectbadlayerhandler.h"

#include "qgsapplication.h"
#include "qgsdatasourceuri.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"

#include <QFileInfo>

void QgsProjectBadLayerHandler::handleBadLayers( const QList<QDomNode> &layers )
{
  if ( !layers.empty() )
    QgsMessageLog::logMessage( QObject::tr( "%n unavailable layer(s) found:", nullptr, layers.size() ) );

  for ( const QDomNode &layer : layers )
  {
    QgsMessageLog::logMessage( QObject::tr( " * %1" ).arg( QgsDataSourceUri::removePassword( dataSource( layer ), true ) ) );
  }
}

QgsProjectBadLayerHandler::DataType QgsProjectBadLayerHandler::dataType( const QDomNode &layerNode )
{
  const QString type = layerNode.toElement().attribute( u"type"_s );

  if ( type.isNull() )
  {
    QgsDebugError( u"cannot find ``type'' attribute"_s );

    return IS_BOGUS;
  }

  if ( "raster" == type )
  {
    QgsDebugMsgLevel( u"is a raster"_s, 2 );

    return IS_RASTER;
  }
  else if ( "vector" == type )
  {
    QgsDebugMsgLevel( u"is a vector"_s, 2 );

    return IS_VECTOR;
  }

  QgsDebugMsgLevel( "is unknown type " + type, 2 );

  return IS_BOGUS;
}

QString QgsProjectBadLayerHandler::dataSource( const QDomNode &layerNode )
{
  const QDomNode dataSourceNode = layerNode.namedItem( u"datasource"_s );

  if ( dataSourceNode.isNull() )
  {
    QgsDebugError( u"cannot find datasource node"_s );

    return QString();
  }

  return dataSourceNode.toElement().text();
}

QgsProjectBadLayerHandler::ProviderType QgsProjectBadLayerHandler::providerType( const QDomNode &layerNode )
{
  // XXX but what about rasters that can be URLs?  _Can_ they be URLs?

  switch ( dataType( layerNode ) )
  {
    case IS_VECTOR:
    {
      const QString ds = dataSource( layerNode );

      QgsDebugMsgLevel( "datasource is " + ds, 2 );

      if ( ds.contains( "host="_L1 ) )
      {
        return IS_URL;
      }
      else if ( ds.contains( "dbname="_L1 ) )
      {
        return IS_DATABASE;
      }
      // be default, then, this should be a file based layer data source
      // XXX is this a reasonable assumption?

      return IS_FILE;
    }

    case IS_RASTER:         // rasters are currently only accessed as
      // physical files
      return IS_FILE;

    default:
      QgsDebugError( u"unknown ``type'' attribute"_s );
  }

  return IS_Unknown;
}

void QgsProjectBadLayerHandler::setDataSource( QDomNode &layerNode, const QString &dataSource )
{
  const QDomNode dataSourceNode = layerNode.namedItem( u"datasource"_s );
  const QDomElement dataSourceElement = dataSourceNode.toElement();
  QDomText dataSourceText = dataSourceElement.firstChild().toText();

  QgsDebugMsgLevel( "datasource changed from " + dataSourceText.data(), 2 );

  dataSourceText.setData( dataSource );

  QgsDebugMsgLevel( "to " + dataSourceText.data(), 2 );
}
