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


QgsRasterDataProvider::QgsRasterDataProvider(): mDpi( -1 )
{
}

QgsRasterDataProvider::QgsRasterDataProvider( QString const & uri )
    : QgsDataProvider( uri )
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

  QgsDebugMsg( "Capability: " + abilitiesList.join( ", " ) );

  return abilitiesList.join( ", " );
}

bool QgsRasterDataProvider::identify( const QgsPoint& thePoint, QMap<QString, QString>& theResults )
{
  Q_UNUSED( thePoint );
  theResults.clear();
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
  QgsDebugMsg( QString( "theBandNo = %1" ).arg( theBandNo ) );
  double myNoDataValue = noDataValue();
  QgsRasterBandStats myRasterBandStats;
  myRasterBandStats.elementCount = 0; // because we'll be counting only VALID pixels later
  myRasterBandStats.bandName = generateBandName( theBandNo );
  myRasterBandStats.bandNumber = theBandNo;

  int myDataType = dataType( theBandNo );

  int  myNXBlocks, myNYBlocks, myXBlockSize, myYBlockSize;
  myXBlockSize = xBlockSize();
  myYBlockSize = yBlockSize();

  if ( !( capabilities() & QgsRasterDataProvider::Size ) || xSize() == 0 || ySize() == 0 || myXBlockSize == 0 || myYBlockSize == 0 )
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
          QgsDebugMsgLevel( QString( "%1 %2 value %3" ).arg( iX ).arg( iY ).arg( myValue ), 10 );

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
  QgsDebugMsg( "************ STATS **************" );
  QgsDebugMsg( QString( "VALID NODATA %1" ).arg( mValidNoDataValue ) );
  QgsDebugMsg( QString( "MIN %1" ).arg( myRasterBandStats.minimumValue ) );
  QgsDebugMsg( QString( "MAX %1" ).arg( myRasterBandStats.maximumValue ) );
  QgsDebugMsg( QString( "RANGE %1" ).arg( myRasterBandStats.range ) );
  QgsDebugMsg( QString( "MEAN %1" ).arg( myRasterBandStats.mean ) );
  QgsDebugMsg( QString( "STDDEV %1" ).arg( myRasterBandStats.stdDev ) );
#endif

  CPLFree( myData );
  myRasterBandStats.statsGathered = true;
  return myRasterBandStats;
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
