/***************************************************************************
                             qgsiconutils.cpp
                             -------------------
    begin                : May 2021
    copyright            : (C) 2021 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsiconutils.h"
#include "qgsapplication.h"
#include "qgsmaplayer.h"
#include "qgsvectorlayer.h"
#include "qgspluginlayer.h"

#include <QIcon>

QIcon QgsIconUtils::iconForWkbType( Qgis::WkbType type )
{
  const Qgis::GeometryType geomType = QgsWkbTypes::geometryType( type );
  return iconForGeometryType( geomType );
}

QIcon QgsIconUtils::iconForGeometryType( Qgis::GeometryType typeGroup )
{
  switch ( typeGroup )
  {
    case Qgis::GeometryType::Null:
      return iconTable();
    case Qgis::GeometryType::Point:
      return iconPoint();
    case Qgis::GeometryType::Line:
      return iconLine();
    case Qgis::GeometryType::Polygon:
      return iconPolygon();
    case Qgis::GeometryType::Unknown:
      return iconGeometryCollection();
  }
  return iconDefaultLayer();
}

QIcon QgsIconUtils::iconPoint()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconPointLayer.svg" ) );
}

QIcon QgsIconUtils::iconLine()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconLineLayer.svg" ) );
}

QIcon QgsIconUtils::iconPolygon()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconPolygonLayer.svg" ) );
}

QIcon QgsIconUtils::iconGeometryCollection()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconGeometryCollectionLayer.svg" ) );
}

QIcon QgsIconUtils::iconTable()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconTableLayer.svg" ) );
}

QIcon QgsIconUtils::iconRaster()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconRaster.svg" ) );
}

QIcon QgsIconUtils::iconMesh()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconMeshLayer.svg" ) );
}

QIcon QgsIconUtils::iconVectorTile()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconVectorTileLayer.svg" ) );
}

QIcon QgsIconUtils::iconPointCloud()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconPointCloudLayer.svg" ) );
}

QIcon QgsIconUtils::iconTiledScene()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconTiledSceneLayer.svg" ) );
}

QIcon QgsIconUtils::iconDefaultLayer()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconLayer.png" ) );
}

QIcon QgsIconUtils::iconForLayer( const QgsMapLayer *layer )
{
  if ( layer )
  {
    switch ( layer->type() )
    {
      case Qgis::LayerType::Raster:
      case Qgis::LayerType::Mesh:
      case Qgis::LayerType::VectorTile:
      case Qgis::LayerType::PointCloud:
      case Qgis::LayerType::Annotation:
      case Qgis::LayerType::Group:
      case Qgis::LayerType::TiledScene:
      {
        return QgsIconUtils::iconForLayerType( layer->type() );
      }
      case Qgis::LayerType::Plugin:
      {
        if ( const QgsPluginLayer *pl = qobject_cast<const QgsPluginLayer *>( layer ) )
        {
          const QIcon icon = pl->icon();
          if ( !icon.isNull() )
            return icon;
        }
        // fallback to default icon if layer did not provide a specific icon
        return QgsIconUtils::iconForLayerType( layer->type() );
      }
      case Qgis::LayerType::Vector:
      {
        const QgsVectorLayer *vl = qobject_cast<const QgsVectorLayer *>( layer );
        if ( !vl )
        {
          return QIcon();
        }
        const Qgis::GeometryType geomType = vl->geometryType();
        switch ( geomType )
        {
          case Qgis::GeometryType::Point:
          {
            return QgsIconUtils::iconPoint();
          }
          case Qgis::GeometryType::Polygon:
          {
            return QgsIconUtils::iconPolygon();
          }
          case Qgis::GeometryType::Line:
          {
            return QgsIconUtils::iconLine();
          }
          case Qgis::GeometryType::Null:
          {
            return QgsIconUtils::iconTable();
          }
          case Qgis::GeometryType::Unknown:
          {
            return QgsIconUtils::iconGeometryCollection();
          }
        }
      }
    }
  }
  return QIcon();
}

QIcon QgsIconUtils::iconForLayerType( Qgis::LayerType type )
{
  switch ( type )
  {
    case Qgis::LayerType::Raster:
      return QgsIconUtils::iconRaster();

    case Qgis::LayerType::Mesh:
      return QgsIconUtils::iconMesh();

    case Qgis::LayerType::VectorTile:
      return QgsIconUtils::iconVectorTile();

    case Qgis::LayerType::PointCloud:
      return QgsIconUtils::iconPointCloud();

    case Qgis::LayerType::TiledScene:
      return QgsIconUtils::iconTiledScene();

    case Qgis::LayerType::Vector:
      return QgsIconUtils::iconGeometryCollection();

    case Qgis::LayerType::Annotation:
      return QgsApplication::getThemeIcon( QStringLiteral( "/mIconAnnotationLayer.svg" ) );

    case Qgis::LayerType::Plugin:
    case Qgis::LayerType::Group:
      break;
  }
  return QIcon();
}
