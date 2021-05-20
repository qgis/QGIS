/***************************************************************************
                             qgslayeritem.h.cpp
                             -------------------
    begin                : 2011-04-01
    copyright            : (C) 2011 Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayeritem.h"
#include "qgsapplication.h"
#include "qgsmaplayer.h"
#include "qgsvectorlayer.h"

QIcon QgsLayerItem::iconForWkbType( QgsWkbTypes::Type type )
{
  QgsWkbTypes::GeometryType geomType = QgsWkbTypes::geometryType( QgsWkbTypes::Type( type ) );
  switch ( geomType )
  {
    case QgsWkbTypes::NullGeometry:
      return iconTable();
    case QgsWkbTypes::PointGeometry:
      return iconPoint();
    case QgsWkbTypes::LineGeometry:
      return iconLine();
    case QgsWkbTypes::PolygonGeometry:
      return iconPolygon();
    default:
      break;
  }
  return iconDefault();
}

QIcon QgsLayerItem::iconPoint()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconPointLayer.svg" ) );
}

QIcon QgsLayerItem::iconLine()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconLineLayer.svg" ) );
}

QIcon QgsLayerItem::iconPolygon()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconPolygonLayer.svg" ) );
}

QIcon QgsLayerItem::iconTable()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconTableLayer.svg" ) );
}

QIcon QgsLayerItem::iconRaster()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconRaster.svg" ) );
}

QIcon QgsLayerItem::iconMesh()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconMeshLayer.svg" ) );
}

QIcon QgsLayerItem::iconVectorTile()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconVectorTileLayer.svg" ) );
}

QIcon QgsLayerItem::iconPointCloud()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconPointCloudLayer.svg" ) );
}

QIcon QgsLayerItem::iconDefault()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconLayer.png" ) );
}

QgsLayerItem::QgsLayerItem( QgsDataItem *parent, const QString &name, const QString &path,
                            const QString &uri, LayerType layerType, const QString &providerKey )
  : QgsDataItem( Layer, parent, name, path, providerKey )
  , mUri( uri )
  , mLayerType( layerType )
{
  mIconName = iconName( layerType );
}

QgsMapLayerType QgsLayerItem::mapLayerType() const
{
  switch ( mLayerType )
  {
    case QgsLayerItem::Raster:
      return QgsMapLayerType::RasterLayer;

    case QgsLayerItem::Mesh:
      return QgsMapLayerType::MeshLayer;

    case QgsLayerItem::VectorTile:
      return QgsMapLayerType::VectorTileLayer;

    case QgsLayerItem::Plugin:
      return QgsMapLayerType::PluginLayer;

    case QgsLayerItem::PointCloud:
      return QgsMapLayerType::PointCloudLayer;

    case QgsLayerItem::NoType:
    case QgsLayerItem::Vector:
    case QgsLayerItem::Point:
    case QgsLayerItem::Polygon:
    case QgsLayerItem::Line:
    case QgsLayerItem::TableLayer:
    case QgsLayerItem::Table:
    case QgsLayerItem::Database:
      return QgsMapLayerType::VectorLayer;
  }

  return QgsMapLayerType::VectorLayer; // no warnings
}

QgsLayerItem::LayerType QgsLayerItem::typeFromMapLayer( QgsMapLayer *layer )
{
  switch ( layer->type() )
  {
    case QgsMapLayerType::VectorLayer:
    {
      switch ( qobject_cast< QgsVectorLayer * >( layer )->geometryType() )
      {
        case QgsWkbTypes::PointGeometry:
          return Point;

        case QgsWkbTypes::LineGeometry:
          return Line;

        case QgsWkbTypes::PolygonGeometry:
          return Polygon;

        case QgsWkbTypes::NullGeometry:
          return TableLayer;

        case QgsWkbTypes::UnknownGeometry:
          return Vector;
      }

      return Vector; // no warnings
    }

    case QgsMapLayerType::RasterLayer:
      return Raster;
    case QgsMapLayerType::PluginLayer:
      return Plugin;
    case QgsMapLayerType::MeshLayer:
      return Mesh;
    case QgsMapLayerType::PointCloudLayer:
      return PointCloud;
    case QgsMapLayerType::VectorTileLayer:
      return VectorTile;
    case QgsMapLayerType::AnnotationLayer:
      return Vector; // will never happen!
  }
  return Vector; // no warnings
}

QString QgsLayerItem::layerTypeAsString( QgsLayerItem::LayerType layerType )
{
  static int enumIdx = staticMetaObject.indexOfEnumerator( "LayerType" );
  return staticMetaObject.enumerator( enumIdx ).valueToKey( layerType );
}

QString QgsLayerItem::iconName( QgsLayerItem::LayerType layerType )
{
  switch ( layerType )
  {
    case Point:
      return QStringLiteral( "/mIconPointLayer.svg" );
    case Line:
      return QStringLiteral( "/mIconLineLayer.svg" );
    case Polygon:
      return QStringLiteral( "/mIconPolygonLayer.svg" );
    // TODO add a new icon for generic Vector layers
    case Vector :
      return QStringLiteral( "/mIconVector.svg" );
    case TableLayer:
    case Table:
      return QStringLiteral( "/mIconTableLayer.svg" );
    case Raster:
      return QStringLiteral( "/mIconRaster.svg" );
    case Mesh:
      return QStringLiteral( "/mIconMeshLayer.svg" );
    case PointCloud:
      return QStringLiteral( "/mIconPointCloudLayer.svg" );
    default:
      return QStringLiteral( "/mIconLayer.png" );
  }
}

bool QgsLayerItem::deleteLayer()
{
  return false;
}

bool QgsLayerItem::equal( const QgsDataItem *other )
{
  //QgsDebugMsg ( mPath + " x " + other->mPath );
  if ( type() != other->type() )
  {
    return false;
  }
  //const QgsLayerItem *o = qobject_cast<const QgsLayerItem *> ( other );
  const QgsLayerItem *o = qobject_cast<const QgsLayerItem *>( other );
  if ( !o )
    return false;

  return ( mPath == o->mPath && mName == o->mName && mUri == o->mUri && mProviderKey == o->mProviderKey );
}

QgsMimeDataUtils::UriList QgsLayerItem::mimeUris() const
{
  QgsMimeDataUtils::Uri u;

  switch ( mapLayerType() )
  {
    case QgsMapLayerType::VectorLayer:
      u.layerType = QStringLiteral( "vector" );
      switch ( mLayerType )
      {
        case Point:
          u.wkbType = QgsWkbTypes::Point;
          break;
        case Line:
          u.wkbType = QgsWkbTypes::LineString;
          break;
        case Polygon:
          u.wkbType = QgsWkbTypes::Polygon;
          break;
        case TableLayer:
          u.wkbType = QgsWkbTypes::NoGeometry;
          break;

        case Database:
        case Table:
        case NoType:
        case Vector:
        case Raster:
        case Plugin:
        case Mesh:
        case PointCloud:
        case VectorTile:
          break;
      }
      break;
    case QgsMapLayerType::RasterLayer:
      u.layerType = QStringLiteral( "raster" );
      break;
    case QgsMapLayerType::MeshLayer:
      u.layerType = QStringLiteral( "mesh" );
      break;
    case QgsMapLayerType::VectorTileLayer:
      u.layerType = QStringLiteral( "vector-tile" );
      break;
    case QgsMapLayerType::PointCloudLayer:
      u.layerType = QStringLiteral( "pointcloud" );
      break;
    case QgsMapLayerType::PluginLayer:
      u.layerType = QStringLiteral( "plugin" );
      break;
    case QgsMapLayerType::AnnotationLayer:
      u.layerType = QStringLiteral( "annotation" );
      break;
  }

  u.providerKey = providerKey();
  u.name = layerName();
  u.uri = uri();
  u.supportedCrs = supportedCrs();
  u.supportedFormats = supportedFormats();
  return { u };
}
