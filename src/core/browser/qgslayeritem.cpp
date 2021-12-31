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
#include "qgsiconutils.h"

QIcon QgsLayerItem::iconForWkbType( QgsWkbTypes::Type type )
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

QgsMapLayerType QgsLayerItem::mapLayerType() const
{
  switch ( mLayerType )
  {
    case Qgis::BrowserLayerType::Raster:
      return QgsMapLayerType::RasterLayer;

    case Qgis::BrowserLayerType::Mesh:
      return QgsMapLayerType::MeshLayer;

    case Qgis::BrowserLayerType::VectorTile:
      return QgsMapLayerType::VectorTileLayer;

    case Qgis::BrowserLayerType::Plugin:
      return QgsMapLayerType::PluginLayer;

    case Qgis::BrowserLayerType::PointCloud:
      return QgsMapLayerType::PointCloudLayer;

    case Qgis::BrowserLayerType::NoType:
    case Qgis::BrowserLayerType::Vector:
    case Qgis::BrowserLayerType::Point:
    case Qgis::BrowserLayerType::Polygon:
    case Qgis::BrowserLayerType::Line:
    case Qgis::BrowserLayerType::TableLayer:
    case Qgis::BrowserLayerType::Table:
    case Qgis::BrowserLayerType::Database:
      return QgsMapLayerType::VectorLayer;
  }

  return QgsMapLayerType::VectorLayer; // no warnings
}

Qgis::BrowserLayerType QgsLayerItem::typeFromMapLayer( QgsMapLayer *layer )
{
  switch ( layer->type() )
  {
    case QgsMapLayerType::VectorLayer:
    {
      switch ( qobject_cast< QgsVectorLayer * >( layer )->geometryType() )
      {
        case QgsWkbTypes::PointGeometry:
          return Qgis::BrowserLayerType::Point;

        case QgsWkbTypes::LineGeometry:
          return Qgis::BrowserLayerType::Line;

        case QgsWkbTypes::PolygonGeometry:
          return Qgis::BrowserLayerType::Polygon;

        case QgsWkbTypes::NullGeometry:
          return Qgis::BrowserLayerType::TableLayer;

        case QgsWkbTypes::UnknownGeometry:
          return Qgis::BrowserLayerType::Vector;
      }

      return Qgis::BrowserLayerType::Vector; // no warnings
    }

    case QgsMapLayerType::RasterLayer:
      return Qgis::BrowserLayerType::Raster;
    case QgsMapLayerType::PluginLayer:
      return Qgis::BrowserLayerType::Plugin;
    case QgsMapLayerType::MeshLayer:
      return Qgis::BrowserLayerType::Mesh;
    case QgsMapLayerType::PointCloudLayer:
      return Qgis::BrowserLayerType::PointCloud;
    case QgsMapLayerType::VectorTileLayer:
      return Qgis::BrowserLayerType::VectorTile;
    case QgsMapLayerType::AnnotationLayer:
    case QgsMapLayerType::GroupLayer:
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
        case Qgis::BrowserLayerType::Point:
          u.wkbType = QgsWkbTypes::Point;
          break;
        case Qgis::BrowserLayerType::Line:
          u.wkbType = QgsWkbTypes::LineString;
          break;
        case Qgis::BrowserLayerType::Polygon:
          u.wkbType = QgsWkbTypes::Polygon;
          break;
        case Qgis::BrowserLayerType::TableLayer:
          u.wkbType = QgsWkbTypes::NoGeometry;
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
    case QgsMapLayerType::GroupLayer:
      u.layerType = QStringLiteral( "group" );
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

  if ( capabilities2() & Qgis::BrowserItemCapability::ItemRepresentsFile )
  {
    u.filePath = path();
  }

  return { u };
}
