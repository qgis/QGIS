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

#include "qgsexpressioncontext.h"
#include "qgsexpressioncontextutils.h"
#include "qgsrasterlayer.h"
#include "qgsrasterlayerelevationproperties.h"
#include "qgsrasterlayertemporalproperties.h"
#include "qgsrasterminmaxorigin.h"
#include "qgsrectangle.h"
#include "qgsthreadingutils.h"

#include <QString>

using namespace Qt::StringLiterals;

int QgsRasterLayerUtils::renderedBandForElevationAndTemporalRange(
  QgsRasterLayer *layer,
  const QgsDateTimeRange &temporalRange,
  const QgsDoubleRange &elevationRange,
  bool &matched )
{
  if ( !layer )
  {
    matched = false;
    return -1;
  }

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
  QList< int > temporalBands;
  switch ( temporalProperties->mode() )
  {
    case Qgis::RasterTemporalMode::RedrawLayerOnly:
    case Qgis::RasterTemporalMode::TemporalRangeFromDataProvider:
    case Qgis::RasterTemporalMode::FixedDateTime:
    case Qgis::RasterTemporalMode::FixedTemporalRange:
    case Qgis::RasterTemporalMode::FixedRangePerBand:
    {
      temporalBands << temporalProperties->filteredBandsForTemporalRange( layer, temporalRange );
      break;
    }

    case Qgis::RasterTemporalMode::RepresentsTemporalValues:
    {
      temporalBands << temporalProperties->bandNumber();
      break;
    }
  }

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
        bandScope->setVariable( u"band"_s, band );
        bandScope->setVariable( u"band_name"_s, layer->dataProvider()->displayBandName( band ) );
        bandScope->setVariable( u"band_description"_s, layer->dataProvider()->bandDescription( band ) );

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
  }
  BUILTIN_UNREACHABLE;
}

void QgsRasterLayerUtils::computeMinMax( QgsRasterDataProvider *provider,
    int band,
    const QgsRasterMinMaxOrigin &mmo,
    Qgis::RasterRangeLimit limits,
    const QgsRectangle &extent,
    int sampleSize,
    double &min SIP_OUT,
    double &max SIP_OUT )
{
  min = std::numeric_limits<double>::quiet_NaN();
  max = std::numeric_limits<double>::quiet_NaN();

  if ( !provider )
    return;

  QGIS_CHECK_OTHER_QOBJECT_THREAD_ACCESS( provider );

  if ( limits == Qgis::RasterRangeLimit::MinimumMaximum )
  {
    QgsRasterBandStats myRasterBandStats = provider->bandStatistics( band, Qgis::RasterBandStatistic::Min | Qgis::RasterBandStatistic::Max, extent, sampleSize );
    // Check if statistics were actually gathered, None means a failure
    if ( myRasterBandStats.statsGathered == static_cast< int >( Qgis::RasterBandStatistic::NoStatistic ) )
    {
      // Best guess we can do
      switch ( provider->dataType( band ) )
      {
        case Qgis::DataType::Byte:
        {
          myRasterBandStats.minimumValue = 0;
          myRasterBandStats.maximumValue = 255;
          break;
        }
        case Qgis::DataType::Int8:
        {
          myRasterBandStats.minimumValue = std::numeric_limits<int8_t>::lowest();
          myRasterBandStats.maximumValue = std::numeric_limits<int8_t>::max();
          break;
        }
        case Qgis::DataType::UInt16:
        {
          myRasterBandStats.minimumValue = 0;
          myRasterBandStats.maximumValue = std::numeric_limits<uint16_t>::max();
          break;
        }
        case Qgis::DataType::UInt32:
        {
          myRasterBandStats.minimumValue = 0;
          myRasterBandStats.maximumValue = std::numeric_limits<uint32_t>::max();
          break;
        }
        case Qgis::DataType::Int16:
        case Qgis::DataType::CInt16:
        {
          myRasterBandStats.minimumValue = std::numeric_limits<int16_t>::lowest();
          myRasterBandStats.maximumValue = std::numeric_limits<int16_t>::max();
          break;
        }
        case Qgis::DataType::Int32:
        case Qgis::DataType::CInt32:
        {
          myRasterBandStats.minimumValue = std::numeric_limits<int32_t>::lowest();
          myRasterBandStats.maximumValue = std::numeric_limits<int32_t>::max();
          break;
        }
        case Qgis::DataType::Float32:
        case Qgis::DataType::CFloat32:
        {
          myRasterBandStats.minimumValue = std::numeric_limits<float_t>::lowest();
          myRasterBandStats.maximumValue = std::numeric_limits<float_t>::max();
          break;
        }
        case Qgis::DataType::Float64:
        case Qgis::DataType::CFloat64:
        {
          myRasterBandStats.minimumValue = std::numeric_limits<double_t>::lowest();
          myRasterBandStats.maximumValue = std::numeric_limits<double_t>::max();
          break;
        }
        case Qgis::DataType::ARGB32:
        case Qgis::DataType::ARGB32_Premultiplied:
        case Qgis::DataType::UnknownDataType:
        {
          // Nothing to guess
          break;
        }
      }
    }
    min = myRasterBandStats.minimumValue;
    max = myRasterBandStats.maximumValue;
  }
  else if ( limits == Qgis::RasterRangeLimit::StdDev )
  {
    const QgsRasterBandStats myRasterBandStats = provider->bandStatistics( band, Qgis::RasterBandStatistic::Mean | Qgis::RasterBandStatistic::StdDev, extent, sampleSize );
    min = myRasterBandStats.mean - ( mmo.stdDevFactor() * myRasterBandStats.stdDev );
    max = myRasterBandStats.mean + ( mmo.stdDevFactor() * myRasterBandStats.stdDev );
  }
  else if ( limits == Qgis::RasterRangeLimit::CumulativeCut )
  {
    const double myLower = mmo.cumulativeCutLower();
    const double myUpper = mmo.cumulativeCutUpper();
    QgsDebugMsgLevel( u"myLower = %1 myUpper = %2"_s.arg( myLower ).arg( myUpper ), 4 );
    provider->cumulativeCut( band, myLower, myUpper, min, max, extent, sampleSize );
  }
  QgsDebugMsgLevel( u"band = %1 min = %2 max = %3"_s.arg( band ).arg( min ).arg( max ), 4 );
}

QgsRectangle QgsRasterLayerUtils::alignRasterExtent( const QgsRectangle &extent, const QgsPointXY &origin, double pixelSizeX, double pixelSizeY )
{
  // This may be negative to indicate inverted NS axis: make sure to use absolute value for calculations
  const double absPixelSizeY { std::abs( pixelSizeY ) };
  const double minX { origin.x() + std::floor( ( extent.xMinimum() - origin.x() ) / pixelSizeX ) *pixelSizeX };
  const double minY { origin.y() + std::floor( ( extent.yMinimum() - origin.y() ) / absPixelSizeY ) *absPixelSizeY };
  const double maxX { origin.x() + std::ceil( ( extent.xMaximum() - origin.x() ) / pixelSizeX ) *pixelSizeX };
  const double maxY { origin.y() + std::ceil( ( extent.yMaximum() - origin.y() ) / absPixelSizeY ) *absPixelSizeY };
  return QgsRectangle( minX, minY, maxX, maxY );
}
