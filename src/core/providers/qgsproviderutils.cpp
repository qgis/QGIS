/***************************************************************************
                             qgsproviderutils.cpp
                             ----------------------------
    begin                : June 2021
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

#include "qgsproviderutils.h"
#include "qgsprovidersublayerdetails.h"

#include <QFileInfo>

bool QgsProviderUtils::sublayerDetailsAreIncomplete( const QList<QgsProviderSublayerDetails> &details, SublayerCompletenessFlags flags )
{
  const bool ignoreUnknownGeometryTypes = flags & SublayerCompletenessFlag::IgnoreUnknownGeometryType;
  const bool ignoreUnknownFeatureCount = flags & SublayerCompletenessFlag::IgnoreUnknownFeatureCount;

  for ( const QgsProviderSublayerDetails &sublayer : details )
  {
    switch ( sublayer.type() )
    {
      case QgsMapLayerType::VectorLayer:
        if ( sublayer.skippedContainerScan()
             || ( !ignoreUnknownGeometryTypes && sublayer.wkbType() == QgsWkbTypes::Unknown )
             || ( !ignoreUnknownFeatureCount &&
                  ( sublayer.featureCount() == static_cast< long long >( Qgis::FeatureCountState::Uncounted )
                    || sublayer.featureCount() == static_cast< long long >( Qgis::FeatureCountState::UnknownCount ) ) ) )
          return true;
        break;

      case QgsMapLayerType::RasterLayer:
      case QgsMapLayerType::PluginLayer:
      case QgsMapLayerType::MeshLayer:
      case QgsMapLayerType::VectorTileLayer:
      case QgsMapLayerType::AnnotationLayer:
      case QgsMapLayerType::PointCloudLayer:
      case QgsMapLayerType::GroupLayer:
        break;
    }
  }

  return false;
}

QString QgsProviderUtils::suggestLayerNameFromFilePath( const QString &path )
{
  const QFileInfo info( path );
  // default to base name of file
  QString name = info.completeBaseName();

  // special handling for .adf files -- use directory as base name, not the unhelpful .adf file name
  if ( info.suffix().compare( QLatin1String( "adf" ), Qt::CaseInsensitive ) == 0 )
  {
    const QString dirName = info.path();
    name = QFileInfo( dirName ).completeBaseName();
  }
  // special handling for ept.json files -- use directory as base name
  else if ( info.fileName().compare( QLatin1String( "ept.json" ), Qt::CaseInsensitive ) == 0 )
  {
    const QString dirName = info.path();
    name = QFileInfo( dirName ).completeBaseName();
  }

  return name;
}
