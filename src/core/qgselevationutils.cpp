/***************************************************************************
  qgselevationutils.cpp
  ------------------
  Date                 : November 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgselevationutils.h"
#include "qgsproject.h"
#include "qgsmaplayerelevationproperties.h"
#include "qgsrasterlayerelevationproperties.h"

QgsDoubleRange QgsElevationUtils::calculateZRangeForProject( QgsProject *project )
{
  const QMap<QString, QgsMapLayer *> &mapLayers = project->mapLayers();
  QgsMapLayer *currentLayer = nullptr;

  double min = std::numeric_limits<double>::quiet_NaN();
  double max = std::numeric_limits<double>::quiet_NaN();

  for ( QMap<QString, QgsMapLayer *>::const_iterator it = mapLayers.constBegin(); it != mapLayers.constEnd(); ++it )
  {
    currentLayer = it.value();

    if ( !currentLayer->elevationProperties() || !currentLayer->elevationProperties()->hasElevation() )
      continue;

    const QgsDoubleRange layerRange = currentLayer->elevationProperties()->calculateZRange( currentLayer );
    if ( layerRange.isInfinite() )
      continue;

    if ( layerRange.lower() > std::numeric_limits< double >::lowest() )
    {
      if ( std::isnan( min ) || layerRange.lower() < min )
        min = layerRange.lower();
    }

    if ( layerRange.upper() < std::numeric_limits< double >::max() )
    {
      if ( std::isnan( max ) || layerRange.upper() > max )
        max = layerRange.upper();
    }
  }

  return QgsDoubleRange( std::isnan( min ) ? std::numeric_limits< double >::lowest() : min,
                         std::isnan( max ) ? std::numeric_limits< double >::max() : max );
}

bool QgsElevationUtils::canEnableElevationForLayer( QgsMapLayer *layer )
{
  return static_cast< bool >( layer->elevationProperties() );
}

bool QgsElevationUtils::enableElevationForLayer( QgsMapLayer *layer )
{
  switch ( layer->type() )
  {
    case Qgis::LayerType::Raster:
    {
      if ( QgsRasterLayerElevationProperties *properties = qobject_cast<QgsRasterLayerElevationProperties * >( layer->elevationProperties() ) )
      {
        properties->setEnabled( true );
        properties->setMode( Qgis::RasterElevationMode::RepresentsElevationSurface );
        // This could potentially be made smarter, eg by checking the data type of bands. But that's likely overkill..!
        properties->setBandNumber( 1 );
        return true;
      }
      break;
    }

    // can't automatically enable elevation for these layer types
    case Qgis::LayerType::Vector:
    case Qgis::LayerType::Plugin:
    case Qgis::LayerType::Mesh:
    case Qgis::LayerType::VectorTile:
    case Qgis::LayerType::Annotation:
    case Qgis::LayerType::PointCloud:
    case Qgis::LayerType::Group:
    case Qgis::LayerType::TiledScene:
      break;
  }
  return false;
}

