/***************************************************************************
    qgsrasterface.cpp - Internal raster processing modules interface
     --------------------------------------
    Date                 : Jun 21, 2012
    Copyright            : (C) 2012 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <limits>
#include <typeinfo>

#include <QByteArray>
#include <QTime>
#include <QStringList>

#include "qgslogger.h"
#include "qgsrasterbandstats.h"
#include "qgsrasterhistogram.h"
#include "qgsrasterinterface.h"
#include "qgsrectangle.h"

QgsRasterInterface::QgsRasterInterface( QgsRasterInterface *input )
  : mInput( input )
{
}

void QgsRasterInterface::initStatistics( QgsRasterBandStats &statistics,
    int bandNo,
    int stats,
    const QgsRectangle &boundingBox,
    int sampleSize ) const
{
  QgsDebugMsgLevel( QStringLiteral( "theBandNo = %1 sampleSize = %2" ).arg( bandNo ).arg( sampleSize ), 4 );

  statistics.bandNumber = bandNo;
  statistics.statsGathered = stats;

  QgsRectangle finalExtent;
  if ( boundingBox.isEmpty() )
  {
    finalExtent = extent();
  }
  else
  {
    finalExtent = extent().intersect( boundingBox );
  }
  statistics.extent = finalExtent;

  if ( sampleSize > 0 )
  {
    // Calc resolution from theSampleSize
    double xRes, yRes;
    xRes = yRes = std::sqrt( ( finalExtent.width() * finalExtent.height() ) / sampleSize );

    // But limit by physical resolution
    if ( capabilities() & Size )
    {
      const double srcXRes = extent().width() / xSize();
      const double srcYRes = extent().height() / ySize();
      if ( xRes < srcXRes ) xRes = srcXRes;
      if ( yRes < srcYRes ) yRes = srcYRes;
    }
    QgsDebugMsgLevel( QStringLiteral( "xRes = %1 yRes = %2" ).arg( xRes ).arg( yRes ), 4 );

    statistics.width = static_cast <int>( std::ceil( finalExtent.width() / xRes ) );
    statistics.height = static_cast <int>( std::ceil( finalExtent.height() / yRes ) );
  }
  else
  {
    if ( capabilities() & Size )
    {
      statistics.width = xSize();
      statistics.height = ySize();
    }
    else
    {
      statistics.width = 1000;
      statistics.height = 1000;
    }
  }
  QgsDebugMsgLevel( QStringLiteral( "theStatistics.width = %1 statistics.height = %2" ).arg( statistics.width ).arg( statistics.height ), 4 );
}

bool QgsRasterInterface::hasStatistics( int bandNo,
                                        int stats,
                                        const QgsRectangle &extent,
                                        int sampleSize )
{
  QgsDebugMsgLevel( QStringLiteral( "theBandNo = %1 stats = %2 sampleSize = %3" ).arg( bandNo ).arg( stats ).arg( sampleSize ), 4 );
  if ( mStatistics.isEmpty() ) return false;

  QgsRasterBandStats myRasterBandStats;
  initStatistics( myRasterBandStats, bandNo, stats, extent, sampleSize );

  const auto constMStatistics = mStatistics;
  for ( const QgsRasterBandStats &stats : constMStatistics )
  {
    if ( stats.contains( myRasterBandStats ) )
    {
      QgsDebugMsgLevel( QStringLiteral( "Has cached statistics." ), 4 );
      return true;
    }
  }
  return false;
}

QgsRasterBandStats QgsRasterInterface::bandStatistics( int bandNo,
    int stats,
    const QgsRectangle &extent,
    int sampleSize, QgsRasterBlockFeedback *feedback )
{
  QgsDebugMsgLevel( QStringLiteral( "theBandNo = %1 stats = %2 sampleSize = %3" ).arg( bandNo ).arg( stats ).arg( sampleSize ), 4 );

  // TODO: null values set on raster layer!!!

  QgsRasterBandStats myRasterBandStats;
  initStatistics( myRasterBandStats, bandNo, stats, extent, sampleSize );

  const auto constMStatistics = mStatistics;
  for ( const QgsRasterBandStats &stats : constMStatistics )
  {
    if ( stats.contains( myRasterBandStats ) )
    {
      QgsDebugMsgLevel( QStringLiteral( "Using cached statistics." ), 4 );
      return stats;
    }
  }

  const QgsRectangle myExtent = myRasterBandStats.extent;
  const int myWidth = myRasterBandStats.width;
  const int myHeight = myRasterBandStats.height;

  //int myDataType = dataType( bandNo );

  int myXBlockSize = xBlockSize();
  int myYBlockSize = yBlockSize();
  if ( myXBlockSize == 0 ) // should not happen, but happens
  {
    myXBlockSize = 500;
  }
  if ( myYBlockSize == 0 ) // should not happen, but happens
  {
    myYBlockSize = 500;
  }

  const int myNXBlocks = ( myWidth + myXBlockSize - 1 ) / myXBlockSize;
  const int myNYBlocks = ( myHeight + myYBlockSize - 1 ) / myYBlockSize;

  const double myXRes = myExtent.width() / myWidth;
  const double myYRes = myExtent.height() / myHeight;
  // TODO: progress signals

  // used by single pass stdev
  double myMean = 0;
  double mySumOfSquares = 0;

  bool myFirstIterationFlag = true;
  bool isNoData = false;
  for ( int myYBlock = 0; myYBlock < myNYBlocks; myYBlock++ )
  {
    for ( int myXBlock = 0; myXBlock < myNXBlocks; myXBlock++ )
    {
      if ( feedback && feedback->isCanceled() )
        return myRasterBandStats;

      QgsDebugMsgLevel( QStringLiteral( "myYBlock = %1 myXBlock = %2" ).arg( myYBlock ).arg( myXBlock ), 4 );
      const int myBlockWidth = std::min( myXBlockSize, myWidth - myXBlock * myXBlockSize );
      const int myBlockHeight = std::min( myYBlockSize, myHeight - myYBlock * myYBlockSize );

      const double xmin = myExtent.xMinimum() + myXBlock * myXBlockSize * myXRes;
      const double xmax = xmin + myBlockWidth * myXRes;
      const double ymin = myExtent.yMaximum() - myYBlock * myYBlockSize * myYRes;
      const double ymax = ymin - myBlockHeight * myYRes;

      const QgsRectangle myPartExtent( xmin, ymin, xmax, ymax );

      std::unique_ptr< QgsRasterBlock > blk( block( bandNo, myPartExtent, myBlockWidth, myBlockHeight, feedback ) );

      // Collect the histogram counts.
      for ( qgssize i = 0; i < ( static_cast< qgssize >( myBlockHeight ) ) * myBlockWidth; i++ )
      {
        const double myValue = blk->valueAndNoData( i, isNoData );
        if ( isNoData )
          continue; // NULL

        myRasterBandStats.sum += myValue;
        myRasterBandStats.elementCount++;

        if ( !std::isfinite( myValue ) ) continue; // inf

        if ( myFirstIterationFlag )
        {
          myFirstIterationFlag = false;
          myRasterBandStats.minimumValue = myValue;
          myRasterBandStats.maximumValue = myValue;
        }
        else
        {
          if ( myValue < myRasterBandStats.minimumValue )
          {
            myRasterBandStats.minimumValue = myValue;
          }
          if ( myValue > myRasterBandStats.maximumValue )
          {
            myRasterBandStats.maximumValue = myValue;
          }
        }

        // Single pass stdev
        const double myDelta = myValue - myMean;
        myMean += myDelta / myRasterBandStats.elementCount;
        mySumOfSquares += myDelta * ( myValue - myMean );
      }
    }
  }

  myRasterBandStats.range = myRasterBandStats.maximumValue - myRasterBandStats.minimumValue;
  myRasterBandStats.mean = myRasterBandStats.sum / myRasterBandStats.elementCount;

  myRasterBandStats.sumOfSquares = mySumOfSquares; // OK with single pass?

  // stdDev may differ  from GDAL stats, because GDAL is using naive single pass
  // algorithm which is more error prone (because of rounding errors)
  // Divide result by sample size - 1 and get square root to get stdev
  myRasterBandStats.stdDev = std::sqrt( mySumOfSquares / ( myRasterBandStats.elementCount - 1 ) );

  QgsDebugMsgLevel( QStringLiteral( "************ STATS **************" ), 4 );
  QgsDebugMsgLevel( QStringLiteral( "MIN %1" ).arg( myRasterBandStats.minimumValue ), 4 );
  QgsDebugMsgLevel( QStringLiteral( "MAX %1" ).arg( myRasterBandStats.maximumValue ), 4 );
  QgsDebugMsgLevel( QStringLiteral( "RANGE %1" ).arg( myRasterBandStats.range ), 4 );
  QgsDebugMsgLevel( QStringLiteral( "MEAN %1" ).arg( myRasterBandStats.mean ), 4 );
  QgsDebugMsgLevel( QStringLiteral( "STDDEV %1" ).arg( myRasterBandStats.stdDev ), 4 );

  myRasterBandStats.statsGathered = QgsRasterBandStats::All;
  mStatistics.append( myRasterBandStats );

  return myRasterBandStats;
}

void QgsRasterInterface::initHistogram( QgsRasterHistogram &histogram,
                                        int bandNo,
                                        int binCount,
                                        double minimum, double maximum,
                                        const QgsRectangle &boundingBox,
                                        int sampleSize,
                                        bool includeOutOfRange )
{
  histogram.bandNumber = bandNo;
  histogram.minimum = minimum;
  histogram.maximum = maximum;
  histogram.includeOutOfRange = includeOutOfRange;

  const Qgis::DataType mySrcDataType = sourceDataType( bandNo );

  if ( std::isnan( histogram.minimum ) )
  {
    // TODO: this was OK when stats/histogram were calced in provider,
    // but what TODO in other interfaces? Check for mInput for now.
    if ( !mInput && mySrcDataType == Qgis::DataType::Byte )
    {
      histogram.minimum = 0; // see histogram() for shift for rounding
    }
    else
    {
      // We need statistics -> avoid histogramDefaults in hasHistogram if possible
      // TODO: use approximated statistics if approximated histogram is requested
      // (theSampleSize > 0)
      const QgsRasterBandStats stats = bandStatistics( bandNo, QgsRasterBandStats::Min, boundingBox, sampleSize );
      histogram.minimum = stats.minimumValue;
    }
  }
  if ( std::isnan( histogram.maximum ) )
  {
    if ( !mInput && mySrcDataType == Qgis::DataType::Byte )
    {
      histogram.maximum = 255;
    }
    else
    {
      const QgsRasterBandStats stats = bandStatistics( bandNo, QgsRasterBandStats::Max, boundingBox, sampleSize );
      histogram.maximum = stats.maximumValue;
    }
  }

  QgsRectangle finalExtent;
  if ( boundingBox.isEmpty() )
  {
    finalExtent = extent();
  }
  else
  {
    finalExtent = extent().intersect( boundingBox );
  }
  histogram.extent = finalExtent;

  if ( sampleSize > 0 )
  {
    // Calc resolution from theSampleSize
    double xRes, yRes;
    xRes = yRes = std::sqrt( ( static_cast<double>( finalExtent.width( ) ) * finalExtent.height() ) / sampleSize );

    // But limit by physical resolution
    if ( capabilities() & Size )
    {
      const double srcXRes = extent().width() / xSize();
      const double srcYRes = extent().height() / ySize();
      if ( xRes < srcXRes ) xRes = srcXRes;
      if ( yRes < srcYRes ) yRes = srcYRes;
    }
    QgsDebugMsgLevel( QStringLiteral( "xRes = %1 yRes = %2" ).arg( xRes ).arg( yRes ), 4 );

    histogram.width = static_cast <int>( finalExtent.width() / xRes );
    histogram.height = static_cast <int>( finalExtent.height() / yRes );
  }
  else
  {
    if ( capabilities() & Size )
    {
      histogram.width = xSize();
      histogram.height = ySize();
    }
    else
    {
      histogram.width = 1000;
      histogram.height = 1000;
    }
  }
  QgsDebugMsgLevel( QStringLiteral( "theHistogram.width = %1 histogram.height = %2" ).arg( histogram.width ).arg( histogram.height ), 4 );

  qint64 myBinCount = binCount;
  if ( myBinCount == 0 )
  {
    // TODO: this was OK when stats/histogram were calced in provider,
    // but what TODO in other interfaces? Check for mInput for now.
    if ( !mInput && mySrcDataType == Qgis::DataType::Byte )
    {
      myBinCount = 256; // Cannot store more values in byte
    }
    else
    {
      // There is no best default value, to display something reasonable in histogram chart,
      // binCount should be small, OTOH, to get precise data for cumulative cut, the number should be big.
      // Because it is easier to define fixed lower value for the chart, we calc optimum binCount
      // for higher resolution (to avoid calculating that where histogram() is used. In any case,
      // it does not make sense to use more than width*height;

      // for Int16/Int32 make sure bin count <= actual range, because there is no sense in having
      // bins at fractional values
      if ( !mInput && (
             mySrcDataType == Qgis::DataType::Int16 || mySrcDataType == Qgis::DataType::Int32 ||
             mySrcDataType == Qgis::DataType::UInt16 || mySrcDataType == Qgis::DataType::UInt32 ) )
      {
        myBinCount = std::min( static_cast<qint64>( histogram.width ) * histogram.height, static_cast<qint64>( std::ceil( histogram.maximum - histogram.minimum + 1 ) ) );
      }
      else
      {
        // This is for not integer types
        myBinCount = static_cast<qint64>( histogram.width ) * static_cast<qint64>( histogram.height );
      }
    }
  }
  // Hard limit 10'000'000
  histogram.binCount = static_cast<int>( std::min( 10000000LL, myBinCount ) );
  QgsDebugMsgLevel( QStringLiteral( "theHistogram.binCount = %1" ).arg( histogram.binCount ), 4 );
}


bool QgsRasterInterface::hasHistogram( int bandNo,
                                       int binCount,
                                       double minimum, double maximum,
                                       const QgsRectangle &extent,
                                       int sampleSize,
                                       bool includeOutOfRange )
{
  QgsDebugMsgLevel( QStringLiteral( "theBandNo = %1 binCount = %2 minimum = %3 maximum = %4 sampleSize = %5" ).arg( bandNo ).arg( binCount ).arg( minimum ).arg( maximum ).arg( sampleSize ), 4 );
  // histogramDefaults() needs statistics if minimum or maximum is NaN ->
  // do other checks which don't need statistics before histogramDefaults()
  if ( mHistograms.isEmpty() ) return false;

  QgsRasterHistogram myHistogram;
  initHistogram( myHistogram, bandNo, binCount, minimum, maximum, extent, sampleSize, includeOutOfRange );

  const auto constMHistograms = mHistograms;
  for ( const QgsRasterHistogram &histogram : constMHistograms )
  {
    if ( histogram == myHistogram )
    {
      QgsDebugMsgLevel( QStringLiteral( "Has cached histogram." ), 4 );
      return true;
    }
  }
  return false;
}

QgsRasterHistogram QgsRasterInterface::histogram( int bandNo,
    int binCount,
    double minimum, double maximum,
    const QgsRectangle &extent,
    int sampleSize,
    bool includeOutOfRange, QgsRasterBlockFeedback *feedback )
{
  QgsDebugMsgLevel( QStringLiteral( "theBandNo = %1 binCount = %2 minimum = %3 maximum = %4 sampleSize = %5" ).arg( bandNo ).arg( binCount ).arg( minimum ).arg( maximum ).arg( sampleSize ), 4 );

  QgsRasterHistogram myHistogram;
  initHistogram( myHistogram, bandNo, binCount, minimum, maximum, extent, sampleSize, includeOutOfRange );

  // Find cached
  const auto constMHistograms = mHistograms;
  for ( const QgsRasterHistogram &histogram : constMHistograms )
  {
    if ( histogram == myHistogram )
    {
      QgsDebugMsgLevel( QStringLiteral( "Using cached histogram." ), 4 );
      return histogram;
    }
  }

  const int myBinCount = myHistogram.binCount;
  const int myWidth = myHistogram.width;
  const int myHeight = myHistogram.height;
  const QgsRectangle myExtent = myHistogram.extent;
  myHistogram.histogramVector.resize( myBinCount );

  int myXBlockSize = xBlockSize();
  int myYBlockSize = yBlockSize();
  if ( myXBlockSize == 0 ) // should not happen, but happens
  {
    myXBlockSize = 500;
  }
  if ( myYBlockSize == 0 ) // should not happen, but happens
  {
    myYBlockSize = 500;
  }

  const int myNXBlocks = ( myWidth + myXBlockSize - 1 ) / myXBlockSize;
  const int myNYBlocks = ( myHeight + myYBlockSize - 1 ) / myYBlockSize;

  const double myXRes = myExtent.width() / myWidth;
  const double myYRes = myExtent.height() / myHeight;

  double myMinimum = myHistogram.minimum;
  double myMaximum = myHistogram.maximum;

  // To avoid rounding errors
  // TODO: check this
  const double myerval = ( myMaximum - myMinimum ) / myHistogram.binCount;
  myMinimum -= 0.1 * myerval;
  myMaximum += 0.1 * myerval;

  QgsDebugMsgLevel( QStringLiteral( "binCount = %1 myMinimum = %2 myMaximum = %3" ).arg( myHistogram.binCount ).arg( myMinimum ).arg( myMaximum ), 4 );

  const double myBinSize = ( myMaximum - myMinimum ) / myBinCount;

  // TODO: progress signals
  bool isNoData = false;
  for ( int myYBlock = 0; myYBlock < myNYBlocks; myYBlock++ )
  {
    for ( int myXBlock = 0; myXBlock < myNXBlocks; myXBlock++ )
    {
      if ( feedback && feedback->isCanceled() )
        return myHistogram;

      const int myBlockWidth = std::min( myXBlockSize, myWidth - myXBlock * myXBlockSize );
      const int myBlockHeight = std::min( myYBlockSize, myHeight - myYBlock * myYBlockSize );

      const double xmin = myExtent.xMinimum() + myXBlock * myXBlockSize * myXRes;
      const double xmax = xmin + myBlockWidth * myXRes;
      const double ymin = myExtent.yMaximum() - myYBlock * myYBlockSize * myYRes;
      const double ymax = ymin - myBlockHeight * myYRes;

      const QgsRectangle myPartExtent( xmin, ymin, xmax, ymax );

      std::unique_ptr< QgsRasterBlock > blk( block( bandNo, myPartExtent, myBlockWidth, myBlockHeight, feedback ) );

      // Collect the histogram counts.
      for ( qgssize i = 0; i < ( static_cast< qgssize >( myBlockHeight ) ) * myBlockWidth; i++ )
      {
        const double myValue = blk->valueAndNoData( i, isNoData );
        if ( isNoData )
        {
          continue; // NULL
        }

        int myBinIndex = static_cast <int>( std::floor( ( myValue - myMinimum ) /  myBinSize ) );

        if ( ( myBinIndex < 0 || myBinIndex > ( myBinCount - 1 ) ) && !includeOutOfRange )
        {
          continue;
        }
        if ( myBinIndex < 0 ) myBinIndex = 0;
        if ( myBinIndex > ( myBinCount - 1 ) ) myBinIndex = myBinCount - 1;

        myHistogram.histogramVector[myBinIndex] += 1;
        myHistogram.nonNullCount++;
      }
    }
  }

  myHistogram.valid = true;
  mHistograms.append( myHistogram );

#ifdef QGISDEBUG
  QString hist;
  for ( std::size_t i = 0; i < std::min< std::size_t >( myHistogram.histogramVector.size(), 500 ); i++ )
  {
    hist += QString::number( myHistogram.histogramVector.value( i ) ) + ' ';
  }
  QgsDebugMsgLevel( QStringLiteral( "Histogram (max first 500 bins): " ) + hist, 4 );
#endif

  return myHistogram;
}

void QgsRasterInterface::cumulativeCut( int bandNo,
                                        double lowerCount, double upperCount,
                                        double &lowerValue, double &upperValue,
                                        const QgsRectangle &extent,
                                        int sampleSize )
{
  QgsDebugMsgLevel( QStringLiteral( "theBandNo = %1 lowerCount = %2 upperCount = %3 sampleSize = %4" ).arg( bandNo ).arg( lowerCount ).arg( upperCount ).arg( sampleSize ), 4 );

  const Qgis::DataType mySrcDataType = sourceDataType( bandNo );

  // Init to NaN is better than histogram min/max to catch errors
  lowerValue = std::numeric_limits<double>::quiet_NaN();
  upperValue = std::numeric_limits<double>::quiet_NaN();

  //get band stats to specify real histogram min/max (fix #9793 Byte bands)
  const QgsRasterBandStats stats = bandStatistics( bandNo, QgsRasterBandStats::Min, extent, sampleSize );
  if ( stats.maximumValue < stats.minimumValue )
    return;

  // for byte bands make sure bin count == actual range
  const int myBinCount = ( mySrcDataType == Qgis::DataType::Byte ) ? int( std::ceil( stats.maximumValue - stats.minimumValue + 1 ) ) : 0;
  const QgsRasterHistogram myHistogram = histogram( bandNo, myBinCount, stats.minimumValue, stats.maximumValue, extent, sampleSize );
  //QgsRasterHistogram myHistogram = histogram( bandNo, 0, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(), extent, sampleSize );

  const double myBinXStep = ( myHistogram.maximum - myHistogram.minimum ) / myHistogram.binCount;
  int myCount = 0;
  const int myMinCount = static_cast< int >( std::round( lowerCount * myHistogram.nonNullCount ) );
  const int myMaxCount = static_cast< int >( std::round( upperCount * myHistogram.nonNullCount ) );
  bool myLowerFound = false;
  QgsDebugMsgLevel( QStringLiteral( "binCount = %1 minimum = %2 maximum = %3 myBinXStep = %4" ).arg( myHistogram.binCount ).arg( myHistogram.minimum ).arg( myHistogram.maximum ).arg( myBinXStep ), 4 );
  QgsDebugMsgLevel( QStringLiteral( "myMinCount = %1 myMaxCount = %2" ).arg( myMinCount ).arg( myMaxCount ), 4 );

  for ( int myBin = 0; myBin < myHistogram.histogramVector.size(); myBin++ )
  {
    const int myBinValue = myHistogram.histogramVector.value( myBin );
    myCount += myBinValue;
    if ( !myLowerFound && myCount > myMinCount )
    {
      lowerValue = myHistogram.minimum + myBin * myBinXStep;
      myLowerFound = true;
      QgsDebugMsgLevel( QStringLiteral( "found lowerValue %1 at bin %2" ).arg( lowerValue ).arg( myBin ), 4 );
    }
    if ( myCount >= myMaxCount )
    {
      upperValue = myHistogram.minimum + myBin * myBinXStep;
      QgsDebugMsgLevel( QStringLiteral( "found upperValue %1 at bin %2" ).arg( upperValue ).arg( myBin ), 4 );
      break;
    }
  }

  // fix integer data - round down/up
  if ( mySrcDataType == Qgis::DataType::Byte ||
       mySrcDataType == Qgis::DataType::Int16 || mySrcDataType == Qgis::DataType::Int32 ||
       mySrcDataType == Qgis::DataType::UInt16 || mySrcDataType == Qgis::DataType::UInt32 )
  {
    if ( !std::isnan( lowerValue ) )
      lowerValue = std::floor( lowerValue );
    if ( !std::isnan( upperValue ) )
      upperValue = std::ceil( upperValue );
  }
}

QString QgsRasterInterface::capabilitiesString() const
{
  QStringList abilitiesList;

  const int abilities = capabilities();

  // Not all all capabilities are here (Size, IdentifyValue, IdentifyText,
  // IdentifyHtml, IdentifyFeature) because those are quite technical and probably
  // would be confusing for users

  if ( abilities & QgsRasterInterface::Identify )
  {
    abilitiesList += tr( "Identify" );
  }

  if ( abilities & QgsRasterInterface::Create )
  {
    abilitiesList += tr( "Create Datasources" );
  }

  if ( abilities & QgsRasterInterface::Remove )
  {
    abilitiesList += tr( "Remove Datasources" );
  }

  if ( abilities & QgsRasterInterface::BuildPyramids )
  {
    abilitiesList += tr( "Build Pyramids" );
  }

  QgsDebugMsgLevel( "Capability: " + abilitiesList.join( QLatin1String( ", " ) ), 4 );

  return abilitiesList.join( QLatin1String( ", " ) );
}

QString QgsRasterInterface::generateBandName( int bandNumber ) const
{
  if ( mInput )
    return mInput->generateBandName( bandNumber );

  return tr( "Band" ) + QStringLiteral( " %1" ) .arg( bandNumber, 1 + static_cast< int >( std::log10( static_cast< double >( bandCount() ) ) ), 10, QChar( '0' ) );
}

QString QgsRasterInterface::colorInterpretationName( int bandNo ) const
{
  if ( mInput )
    return mInput->colorInterpretationName( bandNo );

  return QString();
}

QString QgsRasterInterface::displayBandName( int bandNumber ) const
{
  QString name = generateBandName( bandNumber );
  const QString colorInterp = colorInterpretationName( bandNumber );
  if ( colorInterp != QLatin1String( "Undefined" ) )
  {
    name.append( QStringLiteral( " (%1)" ).arg( colorInterp ) );
  }
  return name;
}

QgsRenderContext QgsRasterBlockFeedback::renderContext() const
{
  return mRenderContext;
}

void QgsRasterBlockFeedback::setRenderContext( const QgsRenderContext &renderContext )
{
  mRenderContext = renderContext;
}
