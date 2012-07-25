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

#include <cmath>

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

QgsRasterBandStats QgsRasterDataProvider::bandStatistics( int theBandNo )
{
  double myNoDataValue = noDataValue();
  QgsRasterBandStats myRasterBandStats;
  myRasterBandStats.elementCount = 0; // because we'll be counting only VALID pixels later
  myRasterBandStats.bandName = generateBandName( theBandNo );
  myRasterBandStats.bandNumber = theBandNo;

  int myDataType = dataType( theBandNo );

  int  myNXBlocks, myNYBlocks, myXBlockSize, myYBlockSize;
  myXBlockSize = xBlockSize();
  myYBlockSize = yBlockSize();

  if ( myXBlockSize == 0 || myYBlockSize == 0 )
  {
    return QgsRasterBandStats(); //invalid raster band stats
  }

  myNXBlocks = ( xSize() + myXBlockSize - 1 ) / myXBlockSize;
  myNYBlocks = ( ySize() + myYBlockSize - 1 ) / myYBlockSize;

  void *myData = CPLMalloc( myXBlockSize * myYBlockSize * ( dataTypeSize( theBandNo ) / 8 ) );

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
      if (( iXBlock + 1 ) * myXBlockSize > myBandXSize )
        nXValid = myBandXSize - iXBlock * myXBlockSize;
      else
        nXValid = myXBlockSize;

      if (( iYBlock + 1 ) * myYBlockSize > myBandYSize )
        nYValid = myBandYSize - iYBlock * myYBlockSize;
      else
        nYValid = myYBlockSize;

      // Collect the histogram counts.
      for ( int iY = 0; iY < nYValid; iY++ )
      {
        for ( int iX = 0; iX < nXValid; iX++ )
        {
          double myValue = readValue( myData, myDataType, iX + ( iY * myXBlockSize ) );
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
      if (( iXBlock + 1 ) * myXBlockSize > myBandXSize )
        nXValid = myBandXSize - iXBlock * myXBlockSize;
      else
        nXValid = myXBlockSize;

      if (( iYBlock + 1 ) * myYBlockSize > myBandYSize )
        nYValid = myBandYSize - iYBlock * myYBlockSize;
      else
        nYValid = myYBlockSize;

      // Collect the histogram counts.
      for ( int iY = 0; iY < nYValid; iY++ )
      {
        for ( int iX = 0; iX < nXValid; iX++ )
        {
          double myValue = readValue( myData, myDataType, iX + ( iY * myXBlockSize ) );
          //QgsDebugMsg ( "myValue = " + QString::number(myValue) );

          if ( mValidNoDataValue && ( qAbs( myValue - myNoDataValue ) <= TINY_VALUE ) )
          {
            continue; // NULL
          }

          myRasterBandStats.sumOfSquares += static_cast < double >
                                            ( pow( myValue - myRasterBandStats.mean, 2 ) );
        }
      }
    } //end of column wise loop
  } //end of row wise loop

  //divide result by sample size - 1 and get square root to get stdev
  myRasterBandStats.stdDev = static_cast < double >( sqrt( myRasterBandStats.sumOfSquares /
                             ( myRasterBandStats.elementCount - 1 ) ) );

#ifdef QGISDEBUG
  QgsLogger::debug( "************ STATS **************", 1, __FILE__, __FUNCTION__, __LINE__ );
  QgsLogger::debug( "VALID NODATA", mValidNoDataValue, 1, __FILE__, __FUNCTION__, __LINE__ );
  QgsLogger::debug( "NULL", noDataValue() , 1, __FILE__, __FUNCTION__, __LINE__ );
  QgsLogger::debug( "MIN", myRasterBandStats.minimumValue, 1, __FILE__, __FUNCTION__, __LINE__ );
  QgsLogger::debug( "MAX", myRasterBandStats.maximumValue, 1, __FILE__, __FUNCTION__, __LINE__ );
  QgsLogger::debug( "RANGE", myRasterBandStats.range, 1, __FILE__, __FUNCTION__, __LINE__ );
  QgsLogger::debug( "MEAN", myRasterBandStats.mean, 1, __FILE__, __FUNCTION__, __LINE__ );
  QgsLogger::debug( "STDDEV", myRasterBandStats.stdDev, 1, __FILE__, __FUNCTION__, __LINE__ );
#endif

  CPLFree( myData );
  myRasterBandStats.statsGathered = true;
  return myRasterBandStats;
}

QgsRasterHistogram QgsRasterDataProvider::histogram( int theBandNo,
    double theMinimum, double theMaximum,
    int theBinCount,
    const QgsRectangle & theExtent,
    int theSampleSize,
    bool theIncludeOutOfRange )
{
  QgsRasterHistogram myHistogram;
  myHistogram.bandNumber = theBandNo;
  myHistogram.minimum = theMinimum;
  myHistogram.maximum = theMaximum;
  myHistogram.includeOutOfRange = theIncludeOutOfRange;
  //myHistogram.sampleSize = theSampleSize

  // First calc defaults
  QgsRectangle myExtent = theExtent.isEmpty() ? extent() : theExtent;
  myHistogram.extent = myExtent;

  int myWidth, myHeight;
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

    myWidth = static_cast <int>( myExtent.width() / xRes );
    myHeight = static_cast <int>( myExtent.height() / yRes );
  }
  else
  {
    if ( capabilities() & Size )
    {
      myWidth = xSize();
      myHeight = ySize();
    }
    else
    {
      myWidth = 1000;
      myHeight = 1000;
    }
  }
  myHistogram.width = myWidth;
  myHistogram.height = myHeight;
  QgsDebugMsg( QString( "myWidth = %1 myHeight = %2" ).arg( myWidth ).arg( myHeight ) );

  double myNoDataValue = noDataValue();
  int myDataType = dataType( theBandNo );

  int myBinCount = theBinCount;
  if ( myBinCount == 0 )
  {
    if ( myDataType == QgsRasterDataProvider::Byte )
    {
      myBinCount = 256; // Cannot store more values in byte
    }
    else
    {
      // There is no best default value, to display something reasonable in histogram chart, binCount should be small, OTOH, to get precise data for cumulative cut, the number should be big. Because it is easier to define fixed lower value for the chart, we calc optimum binCount for higher resolution (to avoid calculating that where histogram() is used. In any any case, it does not make sense to use more than width*height;
      myBinCount = myWidth * myHeight;
      if ( myBinCount > 1000 )  myBinCount = 1000;
    }
  }
  myHistogram.binCount = theBinCount;
  QgsDebugMsg( QString( "myBinCount = %1" ).arg( myBinCount ) );

  // Check if we have cached
  foreach( QgsRasterHistogram histogram, mHistograms )
  {
    if ( histogram.bandNumber == theBandNo &&
         histogram.minimum == theMinimum &&
         histogram.maximum == theMaximum &&
         histogram.binCount == myBinCount &&
         histogram.extent == myExtent &&
         histogram.width == myWidth &&
         histogram.height == myHeight &&
         histogram.includeOutOfRange == theIncludeOutOfRange )
    {
      return histogram;
    }
  }

  myHistogram.histogramVector.resize( myBinCount );

  int  myNXBlocks, myNYBlocks, myXBlockSize, myYBlockSize;
  myXBlockSize = xBlockSize();
  myYBlockSize = yBlockSize();

  if ( myXBlockSize == 0 || myYBlockSize == 0 ) // should not happen
  {
    return myHistogram;
  }

  myNXBlocks = ( myWidth + myXBlockSize - 1 ) / myXBlockSize;
  myNYBlocks = ( myHeight + myYBlockSize - 1 ) / myYBlockSize;

  void *myData = CPLMalloc( myXBlockSize * myYBlockSize * ( dataTypeSize( theBandNo ) / 8 ) );

  int myBandXSize = xSize();
  int myBandYSize = ySize();
  double myXRes = myExtent.width() / myWidth;
  double myYRes = myExtent.height() / myHeight;
  double binSize = ( theMaximum - theMinimum ) / myBinCount;
  for ( int iYBlock = 0; iYBlock < myNYBlocks; iYBlock++ )
  {
    for ( int iXBlock = 0; iXBlock < myNXBlocks; iXBlock++ )
    {
      int myPartWidth = qMin( myXBlockSize, myWidth - iXBlock * myXBlockSize );
      int myPartHeight = qMin( myYBlockSize, myHeight - iYBlock * myYBlockSize );

      double xmin = myExtent.xMinimum() + iXBlock * myXBlockSize * myXRes;
      double xmax = xmin + myPartWidth * myXRes;
      double ymin = myExtent.yMaximum() - iYBlock * myYBlockSize * myYRes;
      double ymax = ymin - myPartHeight * myYRes;

      QgsRectangle myPartExtent( xmin, ymin, xmax, ymax );

      readBlock( theBandNo, myPartExtent, myPartWidth, myPartHeight, myData );

      // Collect the histogram counts.
      for ( int iY = 0; iY < myPartHeight; iY++ )
      {
        for ( int iX = 0; iX < myPartWidth; iX++ )
        {
          double myValue = readValue( myData, myDataType, iX + ( iY * myPartWidth ) );
          //QgsDebugMsg ( QString ( "%1 %2 value %3" ).arg (iX).arg(iY).arg( myValue ) );

          if ( mValidNoDataValue && ( qAbs( myValue - myNoDataValue ) <= TINY_VALUE ) )
          {
            continue; // NULL
          }

          int myBinIndex = static_cast <int>( floor(( myValue - theMinimum ) /  binSize ) ) ;
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
    } //end of column wise loop
  } //end of row wise loop

  CPLFree( myData );

  myHistogram.valid = true;
  mHistograms.append( myHistogram );

  return myHistogram;
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
