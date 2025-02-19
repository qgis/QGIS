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
  u.layerType = QgsMapLayerFactory::typeToString( mType );
  switch ( mType )
  {
    case Qgis::LayerType::Vector:
      u.wkbType = mWkbType;
      break;
    case Qgis::LayerType::Raster:
    case Qgis::LayerType::Mesh:
    case Qgis::LayerType::VectorTile:
    case Qgis::LayerType::PointCloud:
    case Qgis::LayerType::Plugin:
    case Qgis::LayerType::Group:
    case Qgis::LayerType::Annotation:
    case Qgis::LayerType::TiledScene:
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
