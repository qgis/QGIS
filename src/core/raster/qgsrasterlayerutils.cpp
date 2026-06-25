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

int QgsRasterLayerUtils::renderedBandForElevationAndTemporalRange( QgsRasterLayer *layer, const QgsDateTimeRange &temporalRange, const QgsDoubleRange &elevationRange, bool &matched )
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
  if ( ( !temporalProperties->isActive() || temporalRange.isInfinite() ) && ( !elevationProperties->hasElevation() || elevationRange.isInfinite() ) )
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

void QgsRasterLayerUtils::computeMinMax(
  QgsRasterDataProvider *provider, int band, const QgsRasterMinMaxOrigin &mmo, Qgis::RasterRangeLimit limits, const QgsRectangle &extent, int sampleSize, double &min SIP_OUT, double &max SIP_OUT
)
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
  // Return original extent if pixel sizes are zero (to avoid division by zero) or if the extent is empty
  if ( qgsDoubleNear( pixelSizeX, 0.0 ) || qgsDoubleNear( pixelSizeY, 0.0 ) || extent.isEmpty() )
  {
    return extent;
  }
  // Y pixel size may be negative to indicate inverted NS axis: use absolute value for calculations
  const double absPixelSizeY { std::abs( pixelSizeY ) };
  const double minX { origin.x() + std::floor( ( extent.xMinimum() - origin.x() ) / pixelSizeX ) * pixelSizeX };
  const double minY { origin.y() + std::floor( ( extent.yMinimum() - origin.y() ) / absPixelSizeY ) * absPixelSizeY };
  const double maxX { origin.x() + std::ceil( ( extent.xMaximum() - origin.x() ) / pixelSizeX ) * pixelSizeX };
  const double maxY { origin.y() + std::ceil( ( extent.yMaximum() - origin.y() ) / absPixelSizeY ) * absPixelSizeY };
  return QgsRectangle( minX, minY, maxX, maxY );
}

QList<QgsRasterReliefColor> QgsRasterLayerUtils::calculateOptimizedReliefClasses( QgsRasterDataProvider *provider, int band )
{
  QList<QgsRasterReliefColor> resultList;

  if ( !provider || !provider->isValid() || band < 1 || band > provider->bandCount() )
  {
    return resultList;
  }

  const QgsRasterBandStats stats = provider->bandStatistics( band, Qgis::RasterBandStatistic::Min | Qgis::RasterBandStatistic::Max );

  // store elevation frequency in 252 elevation classes
  double frequency[252] = { 0 };
  const double frequencyClassRange = ( stats.maximumValue - stats.minimumValue ) / 252.0;

  // go through raster cells and get frequency of classes
  QgsRasterIterator iter( provider );
  iter.startRasterRead( band, provider->xSize(), provider->ySize(), provider->extent() );

  int iterCols = 0;
  int iterRows = 0;
  int iterLeft = 0;
  int iterTop = 0;
  std::unique_ptr<QgsRasterBlock> block;

  bool isNoData = false;
  while ( iter.readNextRasterPart( band, iterCols, iterRows, block, iterLeft, iterTop ) )
  {
    for ( int row = 0; row < iterRows; ++row )
    {
      for ( int col = 0; col < iterCols; ++col )
      {
        const double elevation = block->valueAndNoData( row, col, isNoData );
        if ( isNoData )
          continue;

        int elevationClass = frequencyClassForElevation( elevation, stats.minimumValue, frequencyClassRange );
        elevationClass = std::max( std::min( elevationClass, 251 ), 0 );
        frequency[elevationClass] += 1.0;
      }
    }
  }

  //log10 transformation for all frequency values
  for ( int i = 0; i < 252; ++i )
  {
    frequency[i] = std::log10( frequency[i] );
  }

  //start with 9 uniformly distributed classes
  QList<int> classBreaks;
  classBreaks.append( 0 );
  classBreaks.append( 28 );
  classBreaks.append( 56 );
  classBreaks.append( 84 );
  classBreaks.append( 112 );
  classBreaks.append( 140 );
  classBreaks.append( 168 );
  classBreaks.append( 196 );
  classBreaks.append( 224 );
  classBreaks.append( 252 );

  for ( int i = 0; i < 10; ++i )
  {
    optimiseClassBreaks( classBreaks, frequency );
  }

#ifdef QGISDEBUG
  //debug, print out all the classbreaks
  for ( int breakValue : std::as_const( classBreaks ) )
  {
    QgsDebugMsgLevel( QString::number( breakValue ), 2 );
  }
#endif

  //set colors according to optimised class breaks
  QVector<QColor> colorList;
  colorList.reserve( 9 );
  colorList.push_back( QColor( 7, 165, 144 ) );
  colorList.push_back( QColor( 12, 221, 162 ) );
  colorList.push_back( QColor( 33, 252, 183 ) );
  colorList.push_back( QColor( 247, 252, 152 ) );
  colorList.push_back( QColor( 252, 196, 8 ) );
  colorList.push_back( QColor( 252, 166, 15 ) );
  colorList.push_back( QColor( 175, 101, 15 ) );
  colorList.push_back( QColor( 255, 133, 92 ) );
  colorList.push_back( QColor( 204, 204, 204 ) );

  resultList.reserve( classBreaks.size() );
  for ( int i = 1; i < classBreaks.size(); ++i )
  {
    const double minElevation = stats.minimumValue + classBreaks[i - 1] * frequencyClassRange;
    const double maxElevation = stats.minimumValue + classBreaks[i] * frequencyClassRange;
    resultList.push_back( QgsRasterReliefColor( colorList.at( i - 1 ), minElevation, maxElevation ) );
  }

  return resultList;
}

int QgsRasterLayerUtils::frequencyClassForElevation( double elevation, double minElevation, double elevationClassRange )
{
  return ( elevation - minElevation ) / elevationClassRange;
}

void QgsRasterLayerUtils::optimiseClassBreaks( QList<int> &breaks, double *frequencies )
{
  const int nClasses = breaks.size() - 1;
  std::vector< double > a( nClasses ); //slopes
  std::vector< double > b( nClasses ); //y-offsets

  for ( int i = 0; i < nClasses; ++i )
  {
    //get all the values between the class breaks into input
    QList<QPair<int, double>> regressionInput;
    regressionInput.reserve( breaks.at( i + 1 ) - breaks.at( i ) );
    for ( int j = breaks.at( i ); j < breaks.at( i + 1 ); ++j )
    {
      regressionInput.push_back( qMakePair( j, frequencies[j] ) );
    }

    double aParam, bParam;
    if ( !regressionInput.isEmpty() && calculateRegression( regressionInput, aParam, bParam ) )
    {
      a[i] = aParam;
      b[i] = bParam;
    }
    else
    {
      a[i] = 0;
      b[i] = 0; //better default value
    }
  }

  const QList<int> classesToRemove;

  //shift class boundaries or eliminate classes which fall together
  for ( int i = 1; i < nClasses; ++i )
  {
    if ( breaks[i] == breaks[i - 1] )
    {
      continue;
    }

    if ( qgsDoubleNear( a[i - 1], a[i] ) )
    {
      continue;
    }
    else
    {
      int newX = ( b[i - 1] - b[i] ) / ( a[i] - a[i - 1] );

      if ( newX <= breaks[i - 1] )
      {
        newX = breaks[i - 1];
        //  classesToRemove.push_back( i );//remove this class later as it falls together with the preceding one
      }
      else if ( i < nClasses - 1 && newX >= breaks[i + 1] )
      {
        newX = breaks[i + 1];
        //  classesToRemove.push_back( i );//remove this class later as it falls together with the next one
      }

      breaks[i] = newX;
    }
  }

  for ( int i = classesToRemove.size() - 1; i >= 0; --i )
  {
    breaks.removeAt( classesToRemove.at( i ) ); // cppcheck-suppress containerOutOfBounds
  }
}

bool QgsRasterLayerUtils::calculateRegression( const QList<QPair<int, double> > &input, double &a, double &b )
{
  double xMean, yMean;
  double xSum = 0;
  double ySum = 0;
  QList<QPair<int, double>>::const_iterator inputIt = input.constBegin();
  for ( ; inputIt != input.constEnd(); ++inputIt )
  {
    xSum += inputIt->first;
    ySum += inputIt->second;
  }
  xMean = xSum / input.size();
  yMean = ySum / input.size();

  double sumCounter = 0;
  double sumDenominator = 0;
  inputIt = input.constBegin();
  for ( ; inputIt != input.constEnd(); ++inputIt )
  {
    sumCounter += ( ( inputIt->first - xMean ) * ( inputIt->second - yMean ) );
    sumDenominator += ( ( inputIt->first - xMean ) * ( inputIt->first - xMean ) );
  }

  a = sumCounter / sumDenominator;
  b = yMean - a * xMean;

  return true;
}
