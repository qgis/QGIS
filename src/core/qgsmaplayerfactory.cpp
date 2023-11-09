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
#include "qgstiledscenelayer.h"

Qgis::LayerType QgsMapLayerFactory::typeFromString( const QString &string, bool &ok )
{
  ok = true;
  if ( string.compare( QLatin1String( "vector" ), Qt::CaseInsensitive ) == 0 )
    return Qgis::LayerType::Vector;
  else if ( string.compare( QLatin1String( "raster" ), Qt::CaseInsensitive ) == 0 )
    return Qgis::LayerType::Raster;
  else if ( string.compare( QLatin1String( "mesh" ), Qt::CaseInsensitive ) == 0 )
    return Qgis::LayerType::Mesh;
  else if ( string.compare( QLatin1String( "vector-tile" ), Qt::CaseInsensitive ) == 0 )
    return Qgis::LayerType::VectorTile;
  else if ( string.compare( QLatin1String( "point-cloud" ), Qt::CaseInsensitive ) == 0 )
    return Qgis::LayerType::PointCloud;
  else if ( string.compare( QLatin1String( "plugin" ), Qt::CaseInsensitive ) == 0 )
    return Qgis::LayerType::Plugin;
  else if ( string.compare( QLatin1String( "annotation" ), Qt::CaseInsensitive ) == 0 )
    return Qgis::LayerType::Annotation;
  else if ( string.compare( QLatin1String( "group" ), Qt::CaseInsensitive ) == 0 )
    return Qgis::LayerType::Group;
  else if ( string.compare( QLatin1String( "tiled-scene" ), Qt::CaseInsensitive ) == 0 )
    return Qgis::LayerType::TiledScene;

  ok = false;
  return Qgis::LayerType::Vector;
}

QString QgsMapLayerFactory::typeToString( Qgis::LayerType type )
{
  switch ( type )
  {
    case Qgis::LayerType::Vector:
      return QStringLiteral( "vector" );
    case Qgis::LayerType::Raster:
      return QStringLiteral( "raster" );
    case Qgis::LayerType::Plugin:
      return QStringLiteral( "plugin" );
    case Qgis::LayerType::Mesh:
      return QStringLiteral( "mesh" );
    case Qgis::LayerType::VectorTile:
      return QStringLiteral( "vector-tile" );
    case Qgis::LayerType::Annotation:
      return QStringLiteral( "annotation" );
    case Qgis::LayerType::PointCloud:
      return QStringLiteral( "point-cloud" );
    case Qgis::LayerType::Group:
      return QStringLiteral( "group" );
    case Qgis::LayerType::TiledScene:
      return QStringLiteral( "tiled-scene" );
  }
  return QString();
}

QgsMapLayer *QgsMapLayerFactory::createLayer( const QString &uri, const QString &name, Qgis::LayerType type, const LayerOptions &options, const QString &provider )
{
  switch ( type )
  {
    case Qgis::LayerType::Vector:
    {
      QgsVectorLayer::LayerOptions vectorOptions;
      vectorOptions.transformContext = options.transformContext;
      vectorOptions.loadDefaultStyle = options.loadDefaultStyle;
      vectorOptions.loadAllStoredStyles = options.loadAllStoredStyles;
      return new QgsVectorLayer( uri, name, provider, vectorOptions );
    }

    case Qgis::LayerType::Raster:
    {
      QgsRasterLayer::LayerOptions rasterOptions;
      rasterOptions.transformContext = options.transformContext;
      rasterOptions.loadDefaultStyle = options.loadDefaultStyle;
      return new QgsRasterLayer( uri, name, provider, rasterOptions );
    }

    case Qgis::LayerType::Mesh:
    {
      QgsMeshLayer::LayerOptions meshOptions;
      meshOptions.transformContext = options.transformContext;
      meshOptions.loadDefaultStyle = options.loadDefaultStyle;
      return new QgsMeshLayer( uri, name, provider, meshOptions );
    }

    case Qgis::LayerType::VectorTile:
    {
      const QgsVectorTileLayer::LayerOptions vectorTileOptions( options.transformContext );
      return new QgsVectorTileLayer( uri, name, vectorTileOptions );
    }

    case Qgis::LayerType::Annotation:
    {
      const QgsAnnotationLayer::LayerOptions annotationOptions( options.transformContext );
      return new QgsAnnotationLayer( name, annotationOptions );
    }

    case Qgis::LayerType::Group:
    {
      const QgsGroupLayer::LayerOptions groupOptions( options.transformContext );
      return new QgsGroupLayer( name, groupOptions );
    }

    case Qgis::LayerType::PointCloud:
    {
      QgsPointCloudLayer::LayerOptions pointCloudOptions;
      pointCloudOptions.loadDefaultStyle = options.loadDefaultStyle;
      pointCloudOptions.transformContext = options.transformContext;
      return new QgsPointCloudLayer( uri, name, provider, pointCloudOptions );
    }

    case Qgis::LayerType::TiledScene:
    {
      QgsTiledSceneLayer::LayerOptions tiledSceneOptions;
      tiledSceneOptions.loadDefaultStyle = options.loadDefaultStyle;
      tiledSceneOptions.transformContext = options.transformContext;
      return new QgsTiledSceneLayer( uri, name, provider, tiledSceneOptions );
    }

    case Qgis::LayerType::Plugin:
      break;
  }
  return nullptr;
}
