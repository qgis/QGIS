/***************************************************************************
  qgsmaplayerfactory.cpp
  --------------------------------------
  Date                 : March 2021
  Copyright            : (C) 2021 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaplayerfactory.h"

QgsMapLayerType QgsMapLayerFactory::typeFromString( const QString &string, bool &ok )
{
  ok = true;
  if ( string.compare( QLatin1String( "vector" ), Qt::CaseInsensitive ) == 0 )
    return QgsMapLayerType::VectorLayer;
  else if ( string.compare( QLatin1String( "raster" ), Qt::CaseInsensitive ) == 0 )
    return QgsMapLayerType::RasterLayer;
  else if ( string.compare( QLatin1String( "mesh" ), Qt::CaseInsensitive ) == 0 )
    return QgsMapLayerType::MeshLayer;
  else if ( string.compare( QLatin1String( "vector-tile" ), Qt::CaseInsensitive ) == 0 )
    return QgsMapLayerType::VectorTileLayer;
  else if ( string.compare( QLatin1String( "point-cloud" ), Qt::CaseInsensitive ) == 0 )
    return QgsMapLayerType::PointCloudLayer;
  else if ( string.compare( QLatin1String( "plugin" ), Qt::CaseInsensitive ) == 0 )
    return QgsMapLayerType::PluginLayer;
  else if ( string.compare( QLatin1String( "annotation" ), Qt::CaseInsensitive ) == 0 )
    return QgsMapLayerType::AnnotationLayer;

  ok = false;
  return QgsMapLayerType::VectorLayer;
}

QString QgsMapLayerFactory::typeToString( QgsMapLayerType type )
{
  switch ( type )
  {
    case QgsMapLayerType::VectorLayer:
      return QStringLiteral( "vector" );
    case QgsMapLayerType::RasterLayer:
      return QStringLiteral( "raster" );
    case QgsMapLayerType::PluginLayer:
      return QStringLiteral( "plugin" );
    case QgsMapLayerType::MeshLayer:
      return QStringLiteral( "mesh" );
    case QgsMapLayerType::VectorTileLayer:
      return QStringLiteral( "vector-tile" );
    case QgsMapLayerType::AnnotationLayer:
      return QStringLiteral( "annotation" );
    case QgsMapLayerType::PointCloudLayer:
      return QStringLiteral( "point-cloud" );
  }
  return QString();
}
