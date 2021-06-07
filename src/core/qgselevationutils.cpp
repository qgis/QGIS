/***************************************************************************
  qgselevationutils.cpp
  ------------------
  Date                 : November 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgselevationutils.h"
#include "qgsproject.h"
#include "qgsmaplayerelevationproperties.h"


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

