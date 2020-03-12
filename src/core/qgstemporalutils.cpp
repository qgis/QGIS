/***************************************************************************
  qgstemporalutils.cpp
  -----------------------
  Date                 : March 2020
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

#include "qgstemporalutils.h"
#include "qgsproject.h"
#include "qgsmaplayertemporalproperties.h"
#include "qgsrasterlayer.h"

QgsDateTimeRange QgsTemporalUtils::calculateTemporalRangeForProject( QgsProject *project )
{
  const QMap<QString, QgsMapLayer *> &mapLayers = project->mapLayers();
  QgsMapLayer *currentLayer = nullptr;

  QDateTime minDate;
  QDateTime maxDate;

  for ( QMap<QString, QgsMapLayer *>::const_iterator it = mapLayers.constBegin(); it != mapLayers.constEnd(); ++it )
  {
    currentLayer = it.value();

    if ( !currentLayer->temporalProperties() || !currentLayer->temporalProperties()->isActive() )
      continue;

    if ( currentLayer->type() == QgsMapLayerType::RasterLayer )
    {
      QgsRasterLayer *rasterLayer  = qobject_cast<QgsRasterLayer *>( currentLayer );

      QgsDateTimeRange layerRange;
      switch ( rasterLayer->temporalProperties()->mode() )
      {
        case QgsRasterLayerTemporalProperties::ModeFixedTemporalRange:
          layerRange = rasterLayer->temporalProperties()->fixedTemporalRange();
          break;

        case QgsRasterLayerTemporalProperties::ModeTemporalRangeFromDataProvider:
          layerRange = rasterLayer->dataProvider()->temporalCapabilities()->availableTemporalRange();
          break;
      }

      if ( !minDate.isValid() ||  layerRange.begin() < minDate )
        minDate = layerRange.begin();
      if ( !maxDate.isValid() ||  layerRange.end() > maxDate )
        maxDate = layerRange.end();
    }
  }

  return QgsDateTimeRange( minDate, maxDate );
}
