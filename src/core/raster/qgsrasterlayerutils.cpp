/***************************************************************************
                          qgsrasterlayerutils.cpp
                          -------------------------
    begin                : March 2024
    copyright            : (C) 2024 by Nyall Dawson
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

#include "qgsrasterlayerutils.h"
#include "qgsrasterlayer.h"
#include "qgsrasterlayerelevationproperties.h"
#include "qgsrasterlayertemporalproperties.h"
#include "qgsexpressioncontext.h"
#include "qgsexpressioncontextutils.h"

int QgsRasterLayerUtils::renderedBandForElevationAndTemporalRange(
  QgsRasterLayer *layer,
  const QgsDateTimeRange &temporalRange,
  const QgsDoubleRange &elevationRange,
  bool &matched )
{
  matched = true;
  const QgsRasterLayerElevationProperties *elevationProperties = qobject_cast< QgsRasterLayerElevationProperties * >( layer->elevationProperties() );
  const QgsRasterLayerTemporalProperties *temporalProperties = qobject_cast< QgsRasterLayerTemporalProperties *>( layer->temporalProperties() );

  // neither active
  if ( ( !temporalProperties->isActive() || temporalRange.isInfinite() )
       && ( !elevationProperties->hasElevation() || elevationRange.isInfinite() ) )
  {
    return -1;
  }

  // only elevation properties enabled
  if ( !temporalProperties->isActive() || temporalRange.isInfinite() )
  {
    const int band = elevationProperties->bandForElevationRange( layer, elevationRange );
    matched = band > 0;
    return band;
  }

  // only temporal properties enabled
  if ( !elevationProperties->hasElevation() || elevationRange.isInfinite() )
  {
    const int band = temporalProperties->bandForTemporalRange( layer, temporalRange );
    matched = band > 0;
    return band;
  }

  // both elevation and temporal properties enabled

  // first find bands matching the temporal range
  const QList< int > temporalBands = temporalProperties->filteredBandsForTemporalRange(
                                       layer, temporalRange );
  if ( temporalBands.empty() )
  {
    matched = false;
    return -1;
  }

  switch ( elevationProperties->mode() )
  {
    case Qgis::RasterElevationMode::FixedElevationRange:
    case Qgis::RasterElevationMode::RepresentsElevationSurface:
      return temporalBands.at( 0 );

    case Qgis::RasterElevationMode::FixedRangePerBand:
    {
      // find the top-most band which matches the map range
      int currentMatchingBand = -1;
      matched = false;
      QgsDoubleRange currentMatchingRange;
      const QMap<int, QgsDoubleRange> rangePerBand = elevationProperties->fixedRangePerBand();
      for ( int band : temporalBands )
      {
        const QgsDoubleRange rangeForBand = rangePerBand.value( band );
        if ( rangeForBand.overlaps( elevationRange ) )
        {
          if ( currentMatchingRange.isInfinite()
               || ( rangeForBand.includeUpper() && rangeForBand.upper() >= currentMatchingRange.upper() )
               || ( !currentMatchingRange.includeUpper() && rangeForBand.upper() >= currentMatchingRange.upper() ) )
          {
            matched = true;
            currentMatchingBand = band;
            currentMatchingRange = rangeForBand;
          }
        }
      }
      return currentMatchingBand;
    }

    case Qgis::RasterElevationMode::DynamicRangePerBand:
    {
      if ( layer )
      {
        QgsExpressionContext context;
        context.appendScopes( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) );
        QgsExpressionContextScope *bandScope = new QgsExpressionContextScope();
        context.appendScope( bandScope );

        QgsProperty lowerProperty = elevationProperties->dataDefinedProperties().property( QgsMapLayerElevationProperties::Property::RasterPerBandLowerElevation );
        QgsProperty upperProperty = elevationProperties->dataDefinedProperties().property( QgsMapLayerElevationProperties::Property::RasterPerBandUpperElevation );
        lowerProperty.prepare( context );
        upperProperty.prepare( context );

        int currentMatchingBand = -1;
        matched = false;
        QgsDoubleRange currentMatchingRange;

        for ( int band : temporalBands )
        {
          bandScope->setVariable( QStringLiteral( "band" ), band );
          bandScope->setVariable( QStringLiteral( "band_name" ), layer->dataProvider()->displayBandName( band ) );
          bandScope->setVariable( QStringLiteral( "band_description" ), layer->dataProvider()->bandDescription( band ) );

          bool ok = false;
          const double lower = lowerProperty.valueAsDouble( context, 0, &ok );
          if ( !ok )
            continue;
          const double upper = upperProperty.valueAsDouble( context, 0, &ok );
          if ( !ok )
            continue;

          const QgsDoubleRange bandRange = QgsDoubleRange( lower, upper );
          if ( bandRange.overlaps( elevationRange ) )
          {
            if ( currentMatchingRange.isInfinite()
                 || ( bandRange.includeUpper() && bandRange.upper() >= currentMatchingRange.upper() )
                 || ( !currentMatchingRange.includeUpper() && bandRange.upper() >= currentMatchingRange.upper() ) )
            {
              currentMatchingBand = band;
              currentMatchingRange = bandRange;
              matched = true;
            }
          }
        }
        return currentMatchingBand;
      }
      return -1;
    }
  }
  BUILTIN_UNREACHABLE;
}
