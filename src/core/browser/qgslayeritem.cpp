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
#include "moc_qgslayeritem.cpp"
#include "qgsmaplayer.h"
#include "qgsvectorlayer.h"
#include "qgsiconutils.h"

QIcon QgsLayerItem::iconForWkbType( Qgis::WkbType type )
{
  return QgsIconUtils::iconForWkbType( type );
}

QIcon QgsLayerItem::iconPoint()
{
  return QgsIconUtils::iconPoint();
}

QIcon QgsLayerItem::iconLine()
{
  return QgsIconUtils::iconLine();
}

QIcon QgsLayerItem::iconPolygon()
{
  return QgsIconUtils::iconPolygon();
}

QIcon QgsLayerItem::iconTable()
{
  return QgsIconUtils::iconTable();
}

QIcon QgsLayerItem::iconRaster()
{
  return QgsIconUtils::iconRaster();
}

QIcon QgsLayerItem::iconMesh()
{
  return QgsIconUtils::iconMesh();
}

QIcon QgsLayerItem::iconVectorTile()
{
  return QgsIconUtils::iconVectorTile();
}

QIcon QgsLayerItem::iconPointCloud()
{
  return QgsIconUtils::iconPointCloud();
}

QgsAbstractDatabaseProviderConnection *QgsLayerItem::databaseConnection() const
{
  if ( parent() )
  {
    return parent()->databaseConnection();
  }
  return nullptr;
}

const QgsLayerMetadata &QgsLayerItem::layerMetadata() const
{
  return mLayerMetadata;
}

void QgsLayerItem::setLayerMetadata( const QgsLayerMetadata &metadata )
{
  mLayerMetadata = metadata;
}

QIcon QgsLayerItem::iconDefault()
{
  return QgsIconUtils::iconDefaultLayer();
}

QgsLayerItem::QgsLayerItem( QgsDataItem *parent, const QString &name, const QString &path,
                            const QString &uri, Qgis::BrowserLayerType layerType, const QString &providerKey )
  : QgsDataItem( Qgis::BrowserItemType::Layer, parent, name, path, providerKey )
  , mUri( uri )
  , mLayerType( layerType )
{
  mIconName = iconName( layerType );
}

Qgis::LayerType QgsLayerItem::mapLayerType() const
{
  switch ( mLayerType )
  {
    case Qgis::BrowserLayerType::Raster:
      return Qgis::LayerType::Raster;

    case Qgis::BrowserLayerType::Mesh:
      return Qgis::LayerType::Mesh;

    case Qgis::BrowserLayerType::VectorTile:
      return Qgis::LayerType::VectorTile;

    case Qgis::BrowserLayerType::Plugin:
      return Qgis::LayerType::Plugin;

    case Qgis::BrowserLayerType::PointCloud:
      return Qgis::LayerType::PointCloud;

    case Qgis::BrowserLayerType::TiledScene:
      return Qgis::LayerType::TiledScene;

    case Qgis::BrowserLayerType::NoType:
    case Qgis::BrowserLayerType::Vector:
    case Qgis::BrowserLayerType::Point:
    case Qgis::BrowserLayerType::Polygon:
    case Qgis::BrowserLayerType::Line:
    case Qgis::BrowserLayerType::TableLayer:
    case Qgis::BrowserLayerType::Table:
    case Qgis::BrowserLayerType::Database:
      return Qgis::LayerType::Vector;
  }

  return Qgis::LayerType::Vector; // no warnings
}

Qgis::BrowserLayerType QgsLayerItem::typeFromMapLayer( QgsMapLayer *layer )
{
  switch ( layer->type() )
  {
    case Qgis::LayerType::Vector:
    {
      switch ( qobject_cast< QgsVectorLayer * >( layer )->geometryType() )
      {
        case Qgis::GeometryType::Point:
          return Qgis::BrowserLayerType::Point;

        case Qgis::GeometryType::Line:
          return Qgis::BrowserLayerType::Line;

        case Qgis::GeometryType::Polygon:
          return Qgis::BrowserLayerType::Polygon;

        case Qgis::GeometryType::Null:
          return Qgis::BrowserLayerType::TableLayer;

        case Qgis::GeometryType::Unknown:
          return Qgis::BrowserLayerType::Vector;
      }

      return Qgis::BrowserLayerType::Vector; // no warnings
    }

    case Qgis::LayerType::Raster:
      return Qgis::BrowserLayerType::Raster;
    case Qgis::LayerType::Plugin:
      return Qgis::BrowserLayerType::Plugin;
    case Qgis::LayerType::Mesh:
      return Qgis::BrowserLayerType::Mesh;
    case Qgis::LayerType::PointCloud:
      return Qgis::BrowserLayerType::PointCloud;
    case Qgis::LayerType::VectorTile:
      return Qgis::BrowserLayerType::VectorTile;
    case Qgis::LayerType::TiledScene:
      return Qgis::BrowserLayerType::TiledScene;
    case Qgis::LayerType::Annotation:
    case Qgis::LayerType::Group:
      return Qgis::BrowserLayerType::Vector; // will never happen!
  }
  return Qgis::BrowserLayerType::Vector; // no warnings
}

QString QgsLayerItem::layerTypeAsString( Qgis::BrowserLayerType layerType )
{
  return qgsEnumValueToKey( layerType );
}

QString QgsLayerItem::iconName( Qgis::BrowserLayerType layerType )
{
  switch ( layerType )
  {
    case Qgis::BrowserLayerType::Point:
      return QStringLiteral( "/mIconPointLayer.svg" );
    case Qgis::BrowserLayerType::Line:
      return QStringLiteral( "/mIconLineLayer.svg" );
    case Qgis::BrowserLayerType::Polygon:
      return QStringLiteral( "/mIconPolygonLayer.svg" );
    case Qgis::BrowserLayerType::Vector :
      return QStringLiteral( "/mIconGeometryCollectionLayer.svg" );
    case Qgis::BrowserLayerType::TableLayer:
    case Qgis::BrowserLayerType::Table:
      return QStringLiteral( "/mIconTableLayer.svg" );
    case Qgis::BrowserLayerType::Raster:
      return QStringLiteral( "/mIconRaster.svg" );
    case Qgis::BrowserLayerType::Mesh:
      return QStringLiteral( "/mIconMeshLayer.svg" );
    case Qgis::BrowserLayerType::PointCloud:
      return QStringLiteral( "/mIconPointCloudLayer.svg" );
    case Qgis::BrowserLayerType::VectorTile:
      return QStringLiteral( "/mIconVectorTileLayer.svg" );
    case Qgis::BrowserLayerType::TiledScene:
      return QStringLiteral( "/mIconTiledSceneLayer.svg" );

    case Qgis::BrowserLayerType::NoType:
    case Qgis::BrowserLayerType::Database:
    case Qgis::BrowserLayerType::Plugin:
      return QStringLiteral( "/mIconLayer.png" );
  }
  BUILTIN_UNREACHABLE
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
    case Qgis::LayerType::Vector:
      u.layerType = QStringLiteral( "vector" );
      switch ( mLayerType )
      {
        case Qgis::BrowserLayerType::Point:
          u.wkbType = Qgis::WkbType::Point;
          break;
        case Qgis::BrowserLayerType::Line:
          u.wkbType = Qgis::WkbType::LineString;
          break;
        case Qgis::BrowserLayerType::Polygon:
          u.wkbType = Qgis::WkbType::Polygon;
          break;
        case Qgis::BrowserLayerType::TableLayer:
          u.wkbType = Qgis::WkbType::NoGeometry;
          break;

        case Qgis::BrowserLayerType::Database:
        case Qgis::BrowserLayerType::Table:
        case Qgis::BrowserLayerType::NoType:
        case Qgis::BrowserLayerType::Vector:
        case Qgis::BrowserLayerType::Raster:
        case Qgis::BrowserLayerType::Plugin:
        case Qgis::BrowserLayerType::Mesh:
        case Qgis::BrowserLayerType::PointCloud:
        case Qgis::BrowserLayerType::VectorTile:
        case Qgis::BrowserLayerType::TiledScene:
          break;
      }
      break;
    case Qgis::LayerType::Raster:
      u.layerType = QStringLiteral( "raster" );
      break;
    case Qgis::LayerType::Mesh:
      u.layerType = QStringLiteral( "mesh" );
      break;
    case Qgis::LayerType::VectorTile:
      u.layerType = QStringLiteral( "vector-tile" );
      break;
    case Qgis::LayerType::PointCloud:
      u.layerType = QStringLiteral( "pointcloud" );
      break;
    case Qgis::LayerType::TiledScene:
      u.layerType = QStringLiteral( "tiled-scene" );
      break;
    case Qgis::LayerType::Plugin:
      u.layerType = QStringLiteral( "plugin" );
      break;
    case Qgis::LayerType::Group:
      u.layerType = QStringLiteral( "group" );
      break;
    case Qgis::LayerType::Annotation:
      u.layerType = QStringLiteral( "annotation" );
      break;
  }

  u.providerKey = providerKey();
  u.name = layerName();
  u.uri = uri();
  u.supportedCrs = supportedCrs();
  u.supportedFormats = supportedFormats();

  if ( capabilities2() & Qgis::BrowserItemCapability::ItemRepresentsFile )
  {
    u.filePath = path();
  }

  return { u };
}
