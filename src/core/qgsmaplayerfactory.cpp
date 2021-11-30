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
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"
#include "qgsmeshlayer.h"
#include "qgspointcloudlayer.h"
#include "qgsvectortilelayer.h"
#include "qgsannotationlayer.h"
#include "qgsgrouplayer.h"

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
  else if ( string.compare( QLatin1String( "group" ), Qt::CaseInsensitive ) == 0 )
    return QgsMapLayerType::GroupLayer;

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
    case QgsMapLayerType::GroupLayer:
      return QStringLiteral( "group" );
  }
  return QString();
}

QgsMapLayer *QgsMapLayerFactory::createLayer( const QString &uri, const QString &name, QgsMapLayerType type, const LayerOptions &options, const QString &provider )
{
  switch ( type )
  {
    case QgsMapLayerType::VectorLayer:
    {
      QgsVectorLayer::LayerOptions vectorOptions;
      vectorOptions.transformContext = options.transformContext;
      vectorOptions.loadDefaultStyle = options.loadDefaultStyle;
      return new QgsVectorLayer( uri, name, provider, vectorOptions );
    }

    case QgsMapLayerType::RasterLayer:
    {
      QgsRasterLayer::LayerOptions rasterOptions;
      rasterOptions.transformContext = options.transformContext;
      rasterOptions.loadDefaultStyle = options.loadDefaultStyle;
      return new QgsRasterLayer( uri, name, provider, rasterOptions );
    }

    case QgsMapLayerType::MeshLayer:
    {
      QgsMeshLayer::LayerOptions meshOptions;
      meshOptions.transformContext = options.transformContext;
      meshOptions.loadDefaultStyle = options.loadDefaultStyle;
      return new QgsMeshLayer( uri, name, provider, meshOptions );
    }

    case QgsMapLayerType::VectorTileLayer:
    {
      const QgsVectorTileLayer::LayerOptions vectorTileOptions( options.transformContext );
      return new QgsVectorTileLayer( uri, name, vectorTileOptions );
    }

    case QgsMapLayerType::AnnotationLayer:
    {
      const QgsAnnotationLayer::LayerOptions annotationOptions( options.transformContext );
      return new QgsAnnotationLayer( name, annotationOptions );
    }

    case QgsMapLayerType::GroupLayer:
    {
      const QgsGroupLayer::LayerOptions groupOptions( options.transformContext );
      return new QgsGroupLayer( name, groupOptions );
    }

    case QgsMapLayerType::PointCloudLayer:
    {
      QgsPointCloudLayer::LayerOptions pointCloudOptions;
      pointCloudOptions.loadDefaultStyle = options.loadDefaultStyle;
      pointCloudOptions.transformContext = options.transformContext;
      return new QgsPointCloudLayer( uri, name, provider, pointCloudOptions );
    }

    case QgsMapLayerType::PluginLayer:
      break;
  }
  return nullptr;
}
