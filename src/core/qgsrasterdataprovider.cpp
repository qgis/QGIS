/***************************************************************************
    qgsrasterdataprovider.cpp - DataProvider Interface for raster layers
     --------------------------------------
    Date                 : Mar 11, 2005
    Copyright            : (C) 2005 by Brendan Morley
    email                : morb at ozemail dot com dot au
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterdataprovider.h"
#include "qgsrasterprojector.h"
#include "qgslogger.h"

#include <QTime>
#include <QMap>
#include <QByteArray>

#include <qmath.h>

void QgsRasterDataProvider::readBlock( int bandNo, QgsRectangle
                                       const & viewExtent, int width,
                                       int height,
                                       QgsCoordinateReferenceSystem theSrcCRS,
                                       QgsCoordinateReferenceSystem theDestCRS,
                                       void *data )
{
  QgsDebugMsg( "Entered" );
  QgsDebugMsg( "viewExtent = " + viewExtent.toString() );

  if ( ! theSrcCRS.isValid() || ! theDestCRS.isValid() || theSrcCRS == theDestCRS )
  {
    readBlock( bandNo, viewExtent, width, height, data );
    return;
  }

  QTime time;
  time.start();

  double mMaxSrcXRes = 0;
  double mMaxSrcYRes = 0;

  if ( capabilities() & QgsRasterDataProvider::ExactResolution )
  {
    mMaxSrcXRes = extent().width() / xSize();
    mMaxSrcYRes = extent().height() / ySize();
  }

  QgsRasterProjector myProjector( theSrcCRS, theDestCRS, viewExtent, height, width, mMaxSrcXRes, mMaxSrcYRes, extent() );

  QgsDebugMsg( QString( "create projector time  (ms): %1" ).arg( time.elapsed() ) );

  // TODO: init data by nulls

  // If we zoom out too much, projector srcRows / srcCols maybe 0, which can cause problems in providers
  if ( myProjector.srcRows() <= 0 || myProjector.srcCols() <= 0 )
    return;

  // Allocate memory for not projected source data
  int mySize = dataTypeSize( bandNo ) / 8;
  void *mySrcData = malloc( mySize * myProjector.srcRows() * myProjector.srcCols() );

  time.restart();

  readBlock( bandNo, myProjector.srcExtent(), myProjector.srcCols(), myProjector.srcRows(), mySrcData );

  QgsDebugMsg( QString( "read not projected block time  (ms): %1" ).arg( time.elapsed() ) );
  time.restart();

  // Project data from source
  int mySrcRow;
  int mySrcCol;
  int mySrcOffset;
  int myDestOffset;
  for ( int r = 0; r < height; r++ )
  {
    for ( int c = 0; c < width; c++ )
    {
      myProjector.srcRowCol( r, c, &mySrcRow, &mySrcCol );
      mySrcOffset = mySize * ( mySrcRow * myProjector.srcCols() + mySrcCol );
      myDestOffset = mySize * ( r * width + c );
      // retype to char is just to avoid g++ warning
      memcpy(( char* ) data + myDestOffset, ( char* )mySrcData + mySrcOffset, mySize );
    }
  }
  QgsDebugMsg( QString( "reproject block time  (ms): %1" ).arg( time.elapsed() ) );

  free( mySrcData );
}

void * QgsRasterDataProvider::readBlock( int bandNo, QgsRectangle  const & extent, int width, int height )
{
  QgsDebugMsg( QString( "bandNo = %1 width = %2 height = %3" ).arg( bandNo ).arg( width ).arg( height ) );

  // TODO: replace VSIMalloc, it is GDAL function
  void * data = VSIMalloc( dataTypeSize( bandNo ) * width * height );
  readBlock( bandNo, extent, width, height, data );

  return data;
}

QgsRasterDataProvider::QgsRasterDataProvider()
    : QgsRasterInterface( 0 )
    , mDpi( -1 )
{
}

QgsRasterDataProvider::QgsRasterDataProvider( QString const & uri )
    : QgsDataProvider( uri )
    , QgsRasterInterface( 0 )
    , mDpi( -1 )
{
}

//
//Random Static convenience function
//
/////////////////////////////////////////////////////////
//TODO: Change these to private function or make seprate class
// convenience function for building metadata() HTML table cells
// convenience function for creating a string list from a C style string list
QStringList QgsRasterDataProvider::cStringList2Q_( char ** stringList )
{
  QStringList strings;

  // presume null terminated string list
  for ( size_t i = 0; stringList[i]; ++i )
  {
    strings.append( stringList[i] );
  }

  return strings;

} // cStringList2Q_


QString QgsRasterDataProvider::makeTableCell( QString const & value )
{
  return "<p>\n" + value + "</p>\n";
} // makeTableCell_



// convenience function for building metadata() HTML table cells
QString QgsRasterDataProvider::makeTableCells( QStringList const & values )
{
  QString s( "<tr>" );

  for ( QStringList::const_iterator i = values.begin();
        i != values.end();
        ++i )
  {
    s += QgsRasterDataProvider::makeTableCell( *i );
  }

  s += "</tr>";

  return s;
} // makeTableCell_

QString QgsRasterDataProvider::metadata()
{
  QString s;
  return s;
}

QString QgsRasterDataProvider::capabilitiesString() const
{
  QStringList abilitiesList;

  int abilities = capabilities();

  if ( abilities & QgsRasterDataProvider::Identify )
  {
    abilitiesList += tr( "Identify" );
  }

  if ( abilities & QgsRasterDataProvider::BuildPyramids )
  {
    abilitiesList += tr( "Build Pyramids" );
  }

  if ( abilities & QgsRasterDataProvider::Create )
  {
    abilitiesList += tr( "Create Datasources" );
  }

  if ( abilities & QgsRasterDataProvider::Remove )
  {
    abilitiesList += tr( "Remove Datasources" );
  }

  QgsDebugMsg( "Capability: " + abilitiesList.join( ", " ) );

  return abilitiesList.join( ", " );
}

bool QgsRasterDataProvider::identify( const QgsPoint& thePoint, QMap<QString, QString>& theResults )
{
  Q_UNUSED( thePoint );
  theResults.clear();
  return false;
}

bool QgsRasterDataProvider::identify( const QgsPoint & point, QMap<int, QString>& results )
{
  Q_UNUSED( point );
  results.clear();
  return false;
}

QString QgsRasterDataProvider::lastErrorFormat()
{
  return "text/plain";
}

QByteArray QgsRasterDataProvider::noValueBytes( int theBandNo )
{
  int type = dataType( theBandNo );
  int size = dataTypeSize( theBandNo ) / 8;
  QByteArray ba;
  ba.resize( size );
  char * data = ba.data();
  double noval = mNoDataValue[theBandNo-1];
  unsigned char uc;
  unsigned short us;
  short s;
  unsigned int ui;
  int i;
  float f;
  double d;
  switch ( type )
  {
    case Byte:
      uc = ( unsigned char )noval;
      memcpy( data, &uc, size );
      break;
    case UInt16:
      us = ( unsigned short )noval;
      memcpy( data, &us, size );
      break;
    case Int16:
      s = ( short )noval;
      memcpy( data, &s, size );
      break;
    case UInt32:
      ui = ( unsigned int )noval;
      memcpy( data, &ui, size );
      break;
    case Int32:
      i = ( int )noval;
      memcpy( data, &i, size );
      break;
    case Float32:
      f = ( float )noval;
      memcpy( data, &f, size );
      break;
    case Float64:
      d = ( double )noval;
      memcpy( data, &d, size );
      break;
    default:
      QgsLogger::warning( "GDAL data type is not supported" );
  }
  return ba;
}

#if 0
QgsRasterBandStats QgsRasterDataProvider::bandStatistics( int theBandNo )
{
  // TODO: cache stats here in provider
  double myNoDataValue = noDataValue();
  QgsRasterBandStats myRasterBandStats;
  myRasterBandStats.elementCount = 0; // because we'll be counting only VALID pixels later
  myRasterBandStats.bandName = generateBandName( theBandNo );
  myRasterBandStats.bandNumber = theBandNo;

  int myDataType = dataType( theBandNo );

  if ( xBlockSize() == 0 || yBlockSize() == 0 )
  {
    return QgsRasterBandStats(); //invalid raster band stats
  }

  int myNXBlocks = ( xSize() + xBlockSize() - 1 ) / xBlockSize();
  int myNYBlocks = ( ySize() + yBlockSize() - 1 ) / yBlockSize();

  void *myData = CPLMalloc( xBlockSize() * yBlockSize() * ( dataTypeSize( theBandNo ) / 8 ) );

  // unfortunately we need to make two passes through the data to calculate stddev
  bool myFirstIterationFlag = true;

  int myBandXSize = xSize();
  int myBandYSize = ySize();
  for ( int iYBlock = 0; iYBlock < myNYBlocks; iYBlock++ )
  {
    for ( int iXBlock = 0; iXBlock < myNXBlocks; iXBlock++ )
    {
      int  nXValid, nYValid;
      readBlock( theBandNo, iXBlock, iYBlock, myData );

      // Compute the portion of the block that is valid
      // for partial edge blocks.
      if (( iXBlock + 1 ) * xBlockSize() > myBandXSize )
        nXValid = myBandXSize - iXBlock * xBlockSize();
      else
        nXValid = xBlockSize();

      if (( iYBlock + 1 ) * yBlockSize() > myBandYSize )
        nYValid = myBandYSize - iYBlock * yBlockSize();
      else
        nYValid = yBlockSize();

      // Collect the histogram counts.
      for ( int iY = 0; iY < nYValid; iY++ )
      {
        for ( int iX = 0; iX < nXValid; iX++ )
        {
          double myValue = readValue( myData, myDataType, iX + ( iY * xBlockSize() ) );
          //QgsDebugMsg ( QString ( "%1 %2 value %3" ).arg (iX).arg(iY).arg( myValue ) );

          if ( mValidNoDataValue && ( qAbs( myValue - myNoDataValue ) <= TINY_VALUE ) )
          {
            continue; // NULL
          }

          myRasterBandStats.sum += myValue;
          ++myRasterBandStats.elementCount;
          //only use this element if we have a non null element
          if ( myFirstIterationFlag )
          {
            //this is the first iteration so initialise vars
            myFirstIterationFlag = false;
            myRasterBandStats.minimumValue = myValue;
            myRasterBandStats.maximumValue = myValue;
          }               //end of true part for first iteration check
          else
          {
            //this is done for all subsequent iterations
            if ( myValue < myRasterBandStats.minimumValue )
            {
              myRasterBandStats.minimumValue = myValue;
            }
            if ( myValue > myRasterBandStats.maximumValue )
            {
              myRasterBandStats.maximumValue = myValue;
            }
          } //end of false part for first iteration check
        }
      }
    } //end of column wise loop
  } //end of row wise loop


  //end of first pass through data now calculate the range
  myRasterBandStats.range = myRasterBandStats.maximumValue - myRasterBandStats.minimumValue;
  //calculate the mean
  myRasterBandStats.mean = myRasterBandStats.sum / myRasterBandStats.elementCount;

  //for the second pass we will get the sum of the squares / mean
  for ( int iYBlock = 0; iYBlock < myNYBlocks; iYBlock++ )
  {
    for ( int iXBlock = 0; iXBlock < myNXBlocks; iXBlock++ )
    {
      int  nXValid, nYValid;

      readBlock( theBandNo, iXBlock, iYBlock, myData );

      // Compute the portion of the block that is valid
      // for partial edge blocks.
      if (( iXBlock + 1 ) * xBlockSize() > myBandXSize )
        nXValid = myBandXSize - iXBlock * xBlockSize();
      else
        nXValid = xBlockSize();

      if (( iYBlock + 1 ) * yBlockSize() > myBandYSize )
        nYValid = myBandYSize - iYBlock * yBlockSize();
      else
        nYValid = yBlockSize();

      // Collect the histogram counts.
      for ( int iY = 0; iY < nYValid; iY++ )
      {
        for ( int iX = 0; iX < nXValid; iX++ )
        {
          double myValue = readValue( myData, myDataType, iX + ( iY * xBlockSize() ) );
          //QgsDebugMsg ( "myValue = " + QString::number(myValue) );

          if ( mValidNoDataValue && ( qAbs( myValue - myNoDataValue ) <= TINY_VALUE ) )
          {
            continue; // NULL
          }

          myRasterBandStats.sumOfSquares += static_cast < double >
                                            ( qPow( myValue - myRasterBandStats.mean, 2 ) );
        }
      }
    } //end of column wise loop
  } //end of row wise loop

  //divide result by sample size - 1 and get square root to get stdev
  myRasterBandStats.stdDev = static_cast < double >( sqrt( myRasterBandStats.sumOfSquares /
                             ( myRasterBandStats.elementCount - 1 ) ) );

  QgsDebugMsg( "************ STATS **************" );
  QgsDebugMsg( QString( "VALID NODATA %1" ).arg( mValidNoDataValue ) );
  QgsDebugMsg( QString( "MIN %1" ).arg( myRasterBandStats.minimumValue ) );
  QgsDebugMsg( QString( "MAX %1" ).arg( myRasterBandStats.maximumValue ) );
  QgsDebugMsg( QString( "RANGE %1" ).arg( myRasterBandStats.range ) );
  QgsDebugMsg( QString( "MEAN %1" ).arg( myRasterBandStats.mean ) );
  QgsDebugMsg( QString( "STDDEV %1" ).arg( myRasterBandStats.stdDev ) );

  CPLFree( myData );
  myRasterBandStats.statsGathered = true;
  return myRasterBandStats;
}
#endif

void QgsRasterDataProvider::initStatistics( QgsRasterBandStats &theStatistics,
    int theBandNo,
    int theStats,
    const QgsRectangle & theExtent,
    int theSampleSize )
{
  QgsDebugMsg( QString( "theBandNo = %1 theSampleSize = %2" ).arg( theBandNo ).arg( theSampleSize ) );

  theStatistics.bandName = generateBandName( theBandNo );
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

bool QgsRasterDataProvider::hasStatistics( int theBandNo,
    int theStats,
    const QgsRectangle & theExtent,
    int theSampleSize )
{
  QgsDebugMsg( QString( "theBandNo = %1 theStats = %2 theSampleSize = %3" ).arg( theBandNo ).arg( theStats ).arg( theSampleSize ) );
  if ( mStatistics.size() == 0 ) return false;

  QgsRasterBandStats myRasterBandStats;
  initStatistics( myRasterBandStats, theBandNo, theStats, theExtent, theSampleSize );

  foreach( QgsRasterBandStats stats, mStatistics )
  {
    if ( stats.contains( myRasterBandStats ) )
    {
      QgsDebugMsg( "Has cached statistics." );
      return true;
    }
  }
  return false;
}

// Find cached
QgsRasterBandStats QgsRasterDataProvider::bandStatistics( int theBandNo,
    int theStats,
    const QgsRectangle & theExtent,
    int theSampleSize )
{
  QgsDebugMsg( QString( "theBandNo = %1 theStats = %2 theSampleSize = %3" ).arg( theBandNo ).arg( theStats ).arg( theSampleSize ) );

  // TODO: null values set on raster layer!!!

  QgsRasterBandStats myRasterBandStats;
  initStatistics( myRasterBandStats, theBandNo, theStats, theExtent, theSampleSize );

  foreach( QgsRasterBandStats stats, mStatistics )
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

  double myNoDataValue = noDataValue();
  int myDataType = dataType( theBandNo );

  if ( xBlockSize() == 0 || yBlockSize() == 0 ) // should not happen
  {
    return myRasterBandStats;
  }

  int myNXBlocks = ( myWidth + xBlockSize() - 1 ) / xBlockSize();
  int myNYBlocks = ( myHeight + yBlockSize() - 1 ) / yBlockSize();

  void *myData = CPLMalloc( xBlockSize() * yBlockSize() * ( dataTypeSize( theBandNo ) / 8 ) );

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
      int myBlockWidth = qMin( xBlockSize(), myWidth - myXBlock * xBlockSize() );
      int myBlockHeight = qMin( yBlockSize(), myHeight - myYBlock * yBlockSize() );

      double xmin = myExtent.xMinimum() + myXBlock * xBlockSize() * myXRes;
      double xmax = xmin + myBlockWidth * myXRes;
      double ymin = myExtent.yMaximum() - myYBlock * yBlockSize() * myYRes;
      double ymax = ymin - myBlockHeight * myYRes;

      QgsRectangle myPartExtent( xmin, ymin, xmax, ymax );

      readBlock( theBandNo, myPartExtent, myBlockWidth, myBlockHeight, myData );

      // Collect the histogram counts.
      for ( int myY = 0; myY < myBlockHeight; myY++ )
      {
        for ( int myX = 0; myX < myBlockWidth; myX++ )
        {
          double myValue = readValue( myData, myDataType, myX + ( myY * myBlockWidth ) );
          //QgsDebugMsg ( QString ( "%1 %2 value %3" ).arg (myX).arg(myY).arg( myValue ) );

          if ( mValidNoDataValue && ( qAbs( myValue - myNoDataValue ) <= TINY_VALUE ) )
          {
            continue; // NULL
          }

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

          //myRasterBandStats.sumOfSquares += static_cast < double >
          //                                  ( qPow( myValue - myRasterBandStats.mean, 2 ) );

          // Single pass stdev
          double myDelta = myValue - myMean;
          myMean += myDelta / myRasterBandStats.elementCount;
          mySumOfSquares += myDelta * ( myValue - myMean );
        }
      }
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
  QgsDebugMsg( QString( "VALID NODATA %1" ).arg( mValidNoDataValue ) );
  QgsDebugMsg( QString( "MIN %1" ).arg( myRasterBandStats.minimumValue ) );
  QgsDebugMsg( QString( "MAX %1" ).arg( myRasterBandStats.maximumValue ) );
  QgsDebugMsg( QString( "RANGE %1" ).arg( myRasterBandStats.range ) );
  QgsDebugMsg( QString( "MEAN %1" ).arg( myRasterBandStats.mean ) );
  QgsDebugMsg( QString( "STDDEV %1" ).arg( myRasterBandStats.stdDev ) );

  CPLFree( myData );

  myRasterBandStats.statsGathered = QgsRasterBandStats::All;
  mStatistics.append( myRasterBandStats );

  return myRasterBandStats;
}

void QgsRasterDataProvider::initHistogram( QgsRasterHistogram &theHistogram,
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
    if ( mySrcDataType == QgsRasterDataProvider::Byte )
    {
      theHistogram.minimum = 0; // see histogram() for shift for rounding
    }
    else
    {
      // We need statistcs -> avoid histogramDefaults in hasHistogram if possible
      // TODO: use approximated statistics if aproximated histogram is requested
      // (theSampleSize > 0)
      QgsRasterBandStats stats =  bandStatistics( theBandNo, QgsRasterBandStats::Min, theExtent, theSampleSize );
      theHistogram.minimum = stats.minimumValue;
    }
  }
  if ( qIsNaN( theHistogram.maximum ) )
  {
    if ( mySrcDataType == QgsRasterDataProvider::Byte )
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
    if ( mySrcDataType == QgsRasterDataProvider::Byte )
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

bool QgsRasterDataProvider::hasHistogram( int theBandNo,
    int theBinCount,
    double theMinimum, double theMaximum,
    const QgsRectangle & theExtent,
    int theSampleSize,
    bool theIncludeOutOfRange )
{
  QgsDebugMsg( QString( "theBandNo = %1 theBinCount = %2 theMinimum = %3 theMaximum = %4 theSampleSize = %5" ).arg( theBandNo ).arg( theBinCount ).arg( theMinimum ).arg( theMaximum ).arg( theSampleSize ) );
  // histogramDefaults() needs statistics if theMinimum or theMaximum is NaN ->
  // do other checks which dont need statistics before histogramDefaults()
  if ( mHistograms.size() == 0 ) return false;

  QgsRasterHistogram myHistogram;
  initHistogram( myHistogram, theBandNo, theBinCount, theMinimum, theMaximum, theExtent, theSampleSize, theIncludeOutOfRange );

  foreach( QgsRasterHistogram histogram, mHistograms )
  {
    if ( histogram == myHistogram )
    {
      QgsDebugMsg( "Has cached histogram." );
      return true;
    }
  }
  return false;
}

QgsRasterHistogram QgsRasterDataProvider::histogram( int theBandNo,
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
  foreach( QgsRasterHistogram histogram, mHistograms )
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

  double myNoDataValue = noDataValue();
  int myDataType = dataType( theBandNo );

  if ( xBlockSize() == 0 || yBlockSize() == 0 ) // should not happen
  {
    return myHistogram;
  }

  int myNXBlocks = ( myWidth + xBlockSize() - 1 ) / xBlockSize();
  int myNYBlocks = ( myHeight + yBlockSize() - 1 ) / yBlockSize();

  void *myData = CPLMalloc( xBlockSize() * yBlockSize() * ( dataTypeSize( theBandNo ) / 8 ) );

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
      int myBlockWidth = qMin( xBlockSize(), myWidth - myXBlock * xBlockSize() );
      int myBlockHeight = qMin( yBlockSize(), myHeight - myYBlock * yBlockSize() );

      double xmin = myExtent.xMinimum() + myXBlock * xBlockSize() * myXRes;
      double xmax = xmin + myBlockWidth * myXRes;
      double ymin = myExtent.yMaximum() - myYBlock * yBlockSize() * myYRes;
      double ymax = ymin - myBlockHeight * myYRes;

      QgsRectangle myPartExtent( xmin, ymin, xmax, ymax );

      readBlock( theBandNo, myPartExtent, myBlockWidth, myBlockHeight, myData );

      // Collect the histogram counts.
      for ( int myY = 0; myY < myBlockHeight; myY++ )
      {
        for ( int myX = 0; myX < myBlockWidth; myX++ )
        {
          double myValue = readValue( myData, myDataType, myX + ( myY * myBlockWidth ) );
          //QgsDebugMsg ( QString ( "%1 %2 value %3" ).arg (myX).arg(myY).arg( myValue ) );

          if ( mValidNoDataValue && ( qAbs( myValue - myNoDataValue ) <= TINY_VALUE ) )
          {
            continue; // NULL
          }

          int myBinIndex = static_cast <int>( qFloor(( myValue - myMinimum ) /  myBinSize ) ) ;
          //QgsDebugMsg( QString( "myValue = %1 myBinIndex = %2" ).arg( myValue ).arg( myBinIndex ) );

          if (( myBinIndex < 0 || myBinIndex > ( myBinCount - 1 ) ) && !theIncludeOutOfRange )
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
  }

  CPLFree( myData );

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

void QgsRasterDataProvider::cumulativeCut( int theBandNo,
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

double QgsRasterDataProvider::readValue( void *data, int type, int index )
{
  if ( !data )
    return mValidNoDataValue ? noDataValue() : 0.0;

  switch ( type )
  {
    case Byte:
      return ( double )(( GByte * )data )[index];
      break;
    case UInt16:
      return ( double )(( GUInt16 * )data )[index];
      break;
    case Int16:
      return ( double )(( GInt16 * )data )[index];
      break;
    case UInt32:
      return ( double )(( GUInt32 * )data )[index];
      break;
    case Int32:
      return ( double )(( GInt32 * )data )[index];
      break;
    case Float32:
      return ( double )(( float * )data )[index];
      break;
    case Float64:
      return ( double )(( double * )data )[index];
      break;
    default:
      QgsLogger::warning( "GDAL data type is not supported" );
  }

  return mValidNoDataValue ? noDataValue() : 0.0;
}

// ENDS
