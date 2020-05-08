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
#include "qgsmeshlayer.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayertemporalproperties.h"

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
    QgsDateTimeRange layerRange;

    switch ( currentLayer->type() )
    {
      case QgsMapLayerType::RasterLayer:
      {
        QgsRasterLayer *rasterLayer  = qobject_cast<QgsRasterLayer *>( currentLayer );

        switch ( rasterLayer->temporalProperties()->mode() )
        {
          case QgsRasterLayerTemporalProperties::ModeFixedTemporalRange:
            layerRange = rasterLayer->temporalProperties()->fixedTemporalRange();
            break;

          case QgsRasterLayerTemporalProperties::ModeTemporalRangeFromDataProvider:
            layerRange = rasterLayer->dataProvider()->temporalCapabilities()->availableTemporalRange();
            break;
        }
        break;
      }

      case QgsMapLayerType::VectorLayer:
      {
        QgsVectorLayer *vectorLayer = qobject_cast<QgsVectorLayer *>( currentLayer );

        QgsVectorLayerTemporalProperties *properties = static_cast< QgsVectorLayerTemporalProperties * >( vectorLayer->temporalProperties() );
        switch ( properties->mode() )
        {
          case QgsVectorLayerTemporalProperties::ModeFixedTemporalRange:
            layerRange = properties->fixedTemporalRange();
            break;

          case QgsVectorLayerTemporalProperties::ModeFeatureDateTimeInstantFromField:
          {
            const int fieldIndex = vectorLayer->fields().lookupField( properties->startField() );
            if ( fieldIndex >= 0 )
            {
              layerRange = QgsDateTimeRange( vectorLayer->minimumValue( fieldIndex ).toDateTime(),
                                             vectorLayer->maximumValue( fieldIndex ).toDateTime() );
            }
            break;
          }

          case QgsVectorLayerTemporalProperties::ModeFeatureDateTimeStartAndEndFromFields:
          {
            const int startFieldIndex = vectorLayer->fields().lookupField( properties->startField() );
            const int endFieldIndex = vectorLayer->fields().lookupField( properties->endField() );
            if ( startFieldIndex >= 0 && endFieldIndex >= 0 )
            {
              layerRange = QgsDateTimeRange( std::min( vectorLayer->minimumValue( startFieldIndex ).toDateTime(),
                                             vectorLayer->minimumValue( endFieldIndex ).toDateTime() ),
                                             std::max( vectorLayer->maximumValue( startFieldIndex ).toDateTime(),
                                                 vectorLayer->maximumValue( endFieldIndex ).toDateTime() ) );
            }
            else if ( startFieldIndex >= 0 )
            {
              layerRange = QgsDateTimeRange( vectorLayer->minimumValue( startFieldIndex ).toDateTime(),
                                             vectorLayer->maximumValue( startFieldIndex ).toDateTime() );
            }
            else if ( endFieldIndex >= 0 )
            {
              layerRange = QgsDateTimeRange( vectorLayer->minimumValue( endFieldIndex ).toDateTime(),
                                             vectorLayer->maximumValue( endFieldIndex ).toDateTime() );
            }
            break;
          }

          case QgsVectorLayerTemporalProperties::ModeRedrawLayerOnly:
            break;
        }
        break;
      }

      case QgsMapLayerType::MeshLayer:
      {
        QgsMeshLayer *meshLayer = qobject_cast<QgsMeshLayer *>( currentLayer );
        layerRange = meshLayer->temporalProperties()->timeExtent();
        break;
      }

      case QgsMapLayerType::PluginLayer:
      case QgsMapLayerType::VectorTileLayer:
        break;
    }


    if ( !minDate.isValid() ||  layerRange.begin() < minDate )
      minDate = layerRange.begin();
    if ( !maxDate.isValid() ||  layerRange.end() > maxDate )
      maxDate = layerRange.end();


  }

  return QgsDateTimeRange( minDate, maxDate );
}
