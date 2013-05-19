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

#include <qmath.h>

#include "qgslogger.h"
#include "qgsrasterbandstats.h"
#include "qgsrasterhistogram.h"
#include "qgsrasterinterface.h"
#include "qgsrectangle.h"

QgsRasterInterface::QgsRasterInterface( QgsRasterInterface * input )
    : mInput( input )
    , mOn( true )
{
}

QgsRasterInterface::~QgsRasterInterface()
{
}

void QgsRasterInterface::initStatistics( QgsRasterBandStats &theStatistics,
    int theBandNo,
    int theStats,
    const QgsRectangle & theExtent,
    int theSampleSize )
{
  QgsDebugMsg( QString( "theBandNo = %1 theSampleSize = %2" ).arg( theBandNo ).arg( theSampleSize ) );

  theStatistics.bandNumber = theBandNo;
  theStatistics.statsGathered = theStats;

  QgsRectangle myExtent;
  if ( theExtent.isEmpty() )
  {
    myExtent = extent();
  }
  else
  {
    myExtent = extent().intersect( &theExtent );
  }
  theStatistics.extent = myExtent;

  if ( theSampleSize > 0 )
  {
    // Calc resolution from theSampleSize
    double xRes, yRes;
    xRes = yRes = sqrt(( myExtent.width() * myExtent.height() ) / theSampleSize );

    // But limit by physical resolution
    if ( capabilities() & Size )
    {
      double srcXRes = extent().width() / xSize();
      double srcYRes = extent().height() / ySize();
      if ( xRes < srcXRes ) xRes = srcXRes;
      if ( yRes < srcYRes ) yRes = srcYRes;
    }
    QgsDebugMsg( QString( "xRes = %1 yRes = %2" ).arg( xRes ).arg( yRes ) );

    theStatistics.width = static_cast <int>( myExtent.width() / xRes );
    theStatistics.height = static_cast <int>( myExtent.height() / yRes );
  }
  else
  {
    if ( capabilities() & Size )
    {
      theStatistics.width = xSize();
      theStatistics.height = ySize();
    }
    else
    {
      theStatistics.width = 1000;
      theStatistics.height = 1000;
    }
  }
  QgsDebugMsg( QString( "theStatistics.width = %1 theStatistics.height = %2" ).arg( theStatistics.width ).arg( theStatistics.height ) );
}

bool QgsRasterInterface::hasStatistics( int theBandNo,
                                        int theStats,
                                        const QgsRectangle & theExtent,
                                        int theSampleSize )
{
  QgsDebugMsg( QString( "theBandNo = %1 theStats = %2 theSampleSize = %3" ).arg( theBandNo ).arg( theStats ).arg( theSampleSize ) );
  if ( mStatistics.size() == 0 ) return false;

  QgsRasterBandStats myRasterBandStats;
  initStatistics( myRasterBandStats, theBandNo, theStats, theExtent, theSampleSize );

  foreach ( QgsRasterBandStats stats, mStatistics )
  {
    if ( stats.contains( myRasterBandStats ) )
    {
      QgsDebugMsg( "Has cached statistics." );
      return true;
    }
  }
  return false;
}

QgsRasterBandStats QgsRasterInterface::bandStatistics( int theBandNo,
    int theStats,
    const QgsRectangle & theExtent,
    int theSampleSize )
{
  QgsDebugMsg( QString( "theBandNo = %1 theStats = %2 theSampleSize = %3" ).arg( theBandNo ).arg( theStats ).arg( theSampleSize ) );

  // TODO: null values set on raster layer!!!

  QgsRasterBandStats myRasterBandStats;
  initStatistics( myRasterBandStats, theBandNo, theStats, theExtent, theSampleSize );

  foreach ( QgsRasterBandStats stats, mStatistics )
  {
    if ( stats.contains( myRasterBandStats ) )
    {
      QgsDebugMsg( "Using cached statistics." );
      return stats;
    }
  }

  QgsRectangle myExtent = myRasterBandStats.extent;
  int myWidth = myRasterBandStats.width;
  int myHeight = myRasterBandStats.height;

  //int myDataType = dataType( theBandNo );

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

  int myNXBlocks = ( myWidth + myXBlockSize - 1 ) / myXBlockSize;
  int myNYBlocks = ( myHeight + myYBlockSize - 1 ) / myYBlockSize;

  double myXRes = myExtent.width() / myWidth;
  double myYRes = myExtent.height() / myHeight;
  // TODO: progress signals

  // used by single pass stdev
  double myMean = 0;
  double mySumOfSquares = 0;

  bool myFirstIterationFlag = true;
  for ( int myYBlock = 0; myYBlock < myNYBlocks; myYBlock++ )
  {
    for ( int myXBlock = 0; myXBlock < myNXBlocks; myXBlock++ )
    {
      QgsDebugMsg( QString( "myYBlock = %1 myXBlock = %2" ).arg( myYBlock ).arg( myXBlock ) );
      int myBlockWidth = qMin( myXBlockSize, myWidth - myXBlock * myXBlockSize );
      int myBlockHeight = qMin( myYBlockSize, myHeight - myYBlock * myYBlockSize );

      double xmin = myExtent.xMinimum() + myXBlock * myXBlockSize * myXRes;
      double xmax = xmin + myBlockWidth * myXRes;
      double ymin = myExtent.yMaximum() - myYBlock * myYBlockSize * myYRes;
      double ymax = ymin - myBlockHeight * myYRes;

      QgsRectangle myPartExtent( xmin, ymin, xmax, ymax );

      QgsRasterBlock* blk = block( theBandNo, myPartExtent, myBlockWidth, myBlockHeight );

      // Collect the histogram counts.
      for ( size_t i = 0; i < (( size_t ) myBlockHeight ) * myBlockWidth; i++ )
      {
        if ( blk->isNoData( i ) ) continue; // NULL

        double myValue = blk->value( i );

        myRasterBandStats.sum += myValue;
        myRasterBandStats.elementCount++;

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
        double myDelta = myValue - myMean;
        myMean += myDelta / myRasterBandStats.elementCount;
        mySumOfSquares += myDelta * ( myValue - myMean );
      }
      delete blk;
    }
  }

  myRasterBandStats.range = myRasterBandStats.maximumValue - myRasterBandStats.minimumValue;
  myRasterBandStats.mean = myRasterBandStats.sum / myRasterBandStats.elementCount;

  myRasterBandStats.sumOfSquares = mySumOfSquares; // OK with single pass?

  // stdDev may differ  from GDAL stats, because GDAL is using naive single pass
  // algorithm which is more error prone (because of rounding errors)
  // Divide result by sample size - 1 and get square root to get stdev
  myRasterBandStats.stdDev = sqrt( mySumOfSquares / ( myRasterBandStats.elementCount - 1 ) );

  QgsDebugMsg( "************ STATS **************" );
  QgsDebugMsg( QString( "MIN %1" ).arg( myRasterBandStats.minimumValue ) );
  QgsDebugMsg( QString( "MAX %1" ).arg( myRasterBandStats.maximumValue ) );
  QgsDebugMsg( QString( "RANGE %1" ).arg( myRasterBandStats.range ) );
  QgsDebugMsg( QString( "MEAN %1" ).arg( myRasterBandStats.mean ) );
  QgsDebugMsg( QString( "STDDEV %1" ).arg( myRasterBandStats.stdDev ) );

  myRasterBandStats.statsGathered = QgsRasterBandStats::All;
  mStatistics.append( myRasterBandStats );

  return myRasterBandStats;
}

void QgsRasterInterface::initHistogram( QgsRasterHistogram &theHistogram,
                                        int theBandNo,
                                        int theBinCount,
                                        double theMinimum, double theMaximum,
                                        const QgsRectangle & theExtent,
                                        int theSampleSize,
                                        bool theIncludeOutOfRange )
{
  theHistogram.bandNumber = theBandNo;
  theHistogram.minimum = theMinimum;
  theHistogram.maximum = theMaximum;
  theHistogram.includeOutOfRange = theIncludeOutOfRange;

  int mySrcDataType = srcDataType( theBandNo );

  if ( qIsNaN( theHistogram.minimum ) )
  {
    // TODO: this was OK when stats/histogram were calced in provider,
    // but what TODO in other interfaces? Check for mInput for now.
    if ( !mInput && mySrcDataType == QGis::Byte )
    {
      theHistogram.minimum = 0; // see histogram() for shift for rounding
    }
    else
    {
      // We need statistics -> avoid histogramDefaults in hasHistogram if possible
      // TODO: use approximated statistics if aproximated histogram is requested
      // (theSampleSize > 0)
      QgsRasterBandStats stats =  bandStatistics( theBandNo, QgsRasterBandStats::Min, theExtent, theSampleSize );
      theHistogram.minimum = stats.minimumValue;
    }
  }
  if ( qIsNaN( theHistogram.maximum ) )
  {
    if ( !mInput && mySrcDataType == QGis::Byte )
    {
      theHistogram.maximum = 255;
    }
    else
    {
      QgsRasterBandStats stats =  bandStatistics( theBandNo, QgsRasterBandStats::Max, theExtent, theSampleSize );
      theHistogram.maximum = stats.maximumValue;
    }
  }

  QgsRectangle myExtent;
  if ( theExtent.isEmpty() )
  {
    myExtent = extent();
  }
  else
  {
    myExtent = extent().intersect( &theExtent );
  }
  theHistogram.extent = myExtent;

  if ( theSampleSize > 0 )
  {
    // Calc resolution from theSampleSize
    double xRes, yRes;
    xRes = yRes = sqrt(( myExtent.width() * myExtent.height() ) / theSampleSize );

    // But limit by physical resolution
    if ( capabilities() & Size )
    {
      double srcXRes = extent().width() / xSize();
      double srcYRes = extent().height() / ySize();
      if ( xRes < srcXRes ) xRes = srcXRes;
      if ( yRes < srcYRes ) yRes = srcYRes;
    }
    QgsDebugMsg( QString( "xRes = %1 yRes = %2" ).arg( xRes ).arg( yRes ) );

    theHistogram.width = static_cast <int>( myExtent.width() / xRes );
    theHistogram.height = static_cast <int>( myExtent.height() / yRes );
  }
  else
  {
    if ( capabilities() & Size )
    {
      theHistogram.width = xSize();
      theHistogram.height = ySize();
    }
    else
    {
      theHistogram.width = 1000;
      theHistogram.height = 1000;
    }
  }
  QgsDebugMsg( QString( "theHistogram.width = %1 theHistogram.height = %2" ).arg( theHistogram.width ).arg( theHistogram.height ) );

  int myBinCount = theBinCount;
  if ( myBinCount == 0 )
  {
    // TODO: this was OK when stats/histogram were calced in provider,
    // but what TODO in other interfaces? Check for mInput for now.
    if ( !mInput && mySrcDataType == QGis::Byte )
    {
      myBinCount = 256; // Cannot store more values in byte
    }
    else
    {
      // There is no best default value, to display something reasonable in histogram chart, binCount should be small, OTOH, to get precise data for cumulative cut, the number should be big. Because it is easier to define fixed lower value for the chart, we calc optimum binCount for higher resolution (to avoid calculating that where histogram() is used. In any any case, it does not make sense to use more than width*height;
      myBinCount = theHistogram.width * theHistogram.height;
      if ( myBinCount > 1000 )  myBinCount = 1000;
    }
  }
  theHistogram.binCount = myBinCount;
  QgsDebugMsg( QString( "theHistogram.binCount = %1" ).arg( theHistogram.binCount ) );
}


bool QgsRasterInterface::hasHistogram( int theBandNo,
                                       int theBinCount,
                                       double theMinimum, double theMaximum,
                                       const QgsRectangle & theExtent,
                                       int theSampleSize,
                                       bool theIncludeOutOfRange )
{
  QgsDebugMsg( QString( "theBandNo = %1 theBinCount = %2 theMinimum = %3 theMaximum = %4 theSampleSize = %5" ).arg( theBandNo ).arg( theBinCount ).arg( theMinimum ).arg( theMaximum ).arg( theSampleSize ) );
  // histogramDefaults() needs statistics if theMinimum or theMaximum is NaN ->
  // do other checks which don't need statistics before histogramDefaults()
  if ( mHistograms.size() == 0 ) return false;

  QgsRasterHistogram myHistogram;
  initHistogram( myHistogram, theBandNo, theBinCount, theMinimum, theMaximum, theExtent, theSampleSize, theIncludeOutOfRange );

  foreach ( QgsRasterHistogram histogram, mHistograms )
  {
    if ( histogram == myHistogram )
    {
      QgsDebugMsg( "Has cached histogram." );
      return true;
    }
  }
  return false;
}

QgsRasterHistogram QgsRasterInterface::histogram( int theBandNo,
    int theBinCount,
    double theMinimum, double theMaximum,
    const QgsRectangle & theExtent,
    int theSampleSize,
    bool theIncludeOutOfRange )
{
  QgsDebugMsg( QString( "theBandNo = %1 theBinCount = %2 theMinimum = %3 theMaximum = %4 theSampleSize = %5" ).arg( theBandNo ).arg( theBinCount ).arg( theMinimum ).arg( theMaximum ).arg( theSampleSize ) );

  QgsRasterHistogram myHistogram;
  initHistogram( myHistogram, theBandNo, theBinCount, theMinimum, theMaximum, theExtent, theSampleSize, theIncludeOutOfRange );

  // Find cached
  foreach ( QgsRasterHistogram histogram, mHistograms )
  {
    if ( histogram == myHistogram )
    {
      QgsDebugMsg( "Using cached histogram." );
      return histogram;
    }
  }

  int myBinCount = myHistogram.binCount;
  int myWidth = myHistogram.width;
  int myHeight = myHistogram.height;
  QgsRectangle myExtent = myHistogram.extent;
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

  int myNXBlocks = ( myWidth + myXBlockSize - 1 ) / myXBlockSize;
  int myNYBlocks = ( myHeight + myYBlockSize - 1 ) / myYBlockSize;

  double myXRes = myExtent.width() / myWidth;
  double myYRes = myExtent.height() / myHeight;

  double myMinimum = myHistogram.minimum;
  double myMaximum = myHistogram.maximum;

  // To avoid rounding errors
  // TODO: check this
  double myerval = ( myMaximum - myMinimum ) / myHistogram.binCount;
  myMinimum -= 0.1 * myerval;
  myMaximum += 0.1 * myerval;

  QgsDebugMsg( QString( "binCount = %1 myMinimum = %2 myMaximum = %3" ).arg( myHistogram.binCount ).arg( myMinimum ).arg( myMaximum ) );

  double myBinSize = ( myMaximum - myMinimum ) / myBinCount;

  // TODO: progress signals
  for ( int myYBlock = 0; myYBlock < myNYBlocks; myYBlock++ )
  {
    for ( int myXBlock = 0; myXBlock < myNXBlocks; myXBlock++ )
    {
      int myBlockWidth = qMin( myXBlockSize, myWidth - myXBlock * myXBlockSize );
      int myBlockHeight = qMin( myYBlockSize, myHeight - myYBlock * myYBlockSize );

      double xmin = myExtent.xMinimum() + myXBlock * myXBlockSize * myXRes;
      double xmax = xmin + myBlockWidth * myXRes;
      double ymin = myExtent.yMaximum() - myYBlock * myYBlockSize * myYRes;
      double ymax = ymin - myBlockHeight * myYRes;

      QgsRectangle myPartExtent( xmin, ymin, xmax, ymax );

      QgsRasterBlock* blk = block( theBandNo, myPartExtent, myBlockWidth, myBlockHeight );

      // Collect the histogram counts.
      for ( size_t i = 0; i < (( size_t ) myBlockHeight ) * myBlockWidth; i++ )
      {
        if ( blk->isNoData( i ) )
        {
          continue; // NULL
        }
        double myValue = blk->value( i );

        int myBinIndex = static_cast <int>( qFloor(( myValue - myMinimum ) /  myBinSize ) ) ;

        if (( myBinIndex < 0 || myBinIndex > ( myBinCount - 1 ) ) && !theIncludeOutOfRange )
        {
          continue;
        }
        if ( myBinIndex < 0 ) myBinIndex = 0;
        if ( myBinIndex > ( myBinCount - 1 ) ) myBinIndex = myBinCount - 1;

        myHistogram.histogramVector[myBinIndex] += 1;
        myHistogram.nonNullCount++;
      }
      delete blk;
    }
  }

  myHistogram.valid = true;
  mHistograms.append( myHistogram );

#ifdef QGISDEBUG
  QString hist;
  for ( int i = 0; i < qMin( myHistogram.histogramVector.size(), 500 ); i++ )
  {
    hist += QString::number( myHistogram.histogramVector.value( i ) ) + " ";
  }
  QgsDebugMsg( "Histogram (max first 500 bins): " + hist );
#endif

  return myHistogram;
}

void QgsRasterInterface::cumulativeCut( int theBandNo,
                                        double theLowerCount, double theUpperCount,
                                        double &theLowerValue, double &theUpperValue,
                                        const QgsRectangle & theExtent,
                                        int theSampleSize )
{
  QgsDebugMsg( QString( "theBandNo = %1 theLowerCount = %2 theUpperCount = %3 theSampleSize = %4" ).arg( theBandNo ).arg( theLowerCount ).arg( theUpperCount ).arg( theSampleSize ) );

  QgsRasterHistogram myHistogram = histogram( theBandNo, 0, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN(), theExtent, theSampleSize );

  // Init to NaN is better than histogram min/max to catch errors
  theLowerValue = std::numeric_limits<double>::quiet_NaN();
  theUpperValue = std::numeric_limits<double>::quiet_NaN();

  double myBinXStep = ( myHistogram.maximum - myHistogram.minimum ) / myHistogram.binCount;
  int myCount = 0;
  int myMinCount = ( int ) qRound( theLowerCount * myHistogram.nonNullCount );
  int myMaxCount = ( int ) qRound( theUpperCount * myHistogram.nonNullCount );
  bool myLowerFound = false;
  QgsDebugMsg( QString( "binCount = %1 minimum = %2 maximum = %3 myBinXStep = %4" ).arg( myHistogram.binCount ).arg( myHistogram.minimum ).arg( myHistogram.maximum ).arg( myBinXStep ) );
  QgsDebugMsg( QString( "myMinCount = %1 myMaxCount = %2" ).arg( myMinCount ).arg( myMaxCount ) );

  for ( int myBin = 0; myBin < myHistogram.histogramVector.size(); myBin++ )
  {
    int myBinValue = myHistogram.histogramVector.value( myBin );
    myCount += myBinValue;
    if ( !myLowerFound && myCount > myMinCount )
    {
      theLowerValue = myHistogram.minimum + myBin * myBinXStep;
      myLowerFound = true;
    }
    if ( myCount >= myMaxCount )
    {
      theUpperValue = myHistogram.minimum + myBin * myBinXStep;
      break;
    }
  }
}

QString QgsRasterInterface::capabilitiesString() const
{
  QStringList abilitiesList;

  int abilities = capabilities();

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

  QgsDebugMsg( "Capability: " + abilitiesList.join( ", " ) );

  return abilitiesList.join( ", " );
}
