/***************************************************************************
                             qgsprovidersublayerdetails.cpp
                             ----------------------------
    begin                : May 2021
    copyright            : (C) 2021 by Nyall Dawson
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

#include "qgsprovidersublayerdetails.h"
#include "qgsmaplayerfactory.h"
#include "qgsmimedatautils.h"


QgsMapLayer *QgsProviderSublayerDetails::toLayer( const LayerOptions &options ) const
{
  QgsMapLayerFactory::LayerOptions layerOptions( options.transformContext );
  layerOptions.loadDefaultStyle = options.loadDefaultStyle;
  layerOptions.loadAllStoredStyles = options.loadAllStoredStyle;
  return QgsMapLayerFactory::createLayer( mUri, mName, mType, layerOptions, mProviderKey );
}

QgsMimeDataUtils::Uri QgsProviderSublayerDetails::toMimeUri() const
{
  QgsMimeDataUtils::Uri u;
  switch ( mType )
  {
    case Qgis::LayerType::Vector:
      u.layerType = QStringLiteral( "vector" );
      u.wkbType = mWkbType;
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
    case Qgis::LayerType::Plugin:
      u.layerType = QStringLiteral( "plugin" );
      break;
    case Qgis::LayerType::Group:
      u.layerType = QStringLiteral( "group" );
      break;
    case Qgis::LayerType::Annotation:
      u.layerType = QStringLiteral( "annotation" );
      break;
    case Qgis::LayerType::TiledScene:
      u.layerType = QStringLiteral( "tiled-scene" );
      break;
  }

  u.providerKey = mProviderKey;
  u.name = mName;
  u.uri = mUri;
  return u;
}

bool QgsProviderSublayerDetails::operator==( const QgsProviderSublayerDetails &other ) const
{
  return mProviderKey == other.mProviderKey
         && mType == other.mType
         && mFlags == other.mFlags
         && mUri == other.mUri
         && mLayerNumber == other.mLayerNumber
         && mName == other.mName
         && mDescription == other.mDescription
         && mFeatureCount == other.mFeatureCount
         && mGeometryColumnName == other.mGeometryColumnName
         && mPath == other.mPath
         && mWkbType == other.mWkbType
         && mSkippedContainerScan == other.mSkippedContainerScan
         && mDriverName == other.mDriverName;
}

bool QgsProviderSublayerDetails::operator!=( const QgsProviderSublayerDetails &other ) const
{
  return !( *this == other );
}
