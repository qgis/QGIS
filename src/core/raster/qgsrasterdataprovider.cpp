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

void QgsRasterDataProvider::setUseSrcNoDataValue( int bandNo, bool use )
{
  if ( mUseSrcNoDataValue.size() < bandNo )
  {
    for ( int i = mUseSrcNoDataValue.size(); i < bandNo; i++ )
    {
      mUseSrcNoDataValue.append( false );
    }
  }
  mUseSrcNoDataValue[bandNo-1] = use;
}

double QgsRasterDataProvider::noDataValue( int bandNo ) const
{
  if ( mSrcHasNoDataValue.value( bandNo - 1 ) && mUseSrcNoDataValue.value( bandNo - 1 ) )
  {
    return mSrcNoDataValue.value( bandNo -1 );
  }
  return mInternalNoDataValue.value( bandNo -1 );
}

QgsRasterBlock * QgsRasterDataProvider::block( int theBandNo, QgsRectangle  const & theExtent, int theWidth, int theHeight )
{
  QgsDebugMsg( QString( "theBandNo = %1 theWidth = %2 theHeight = %3" ).arg( theBandNo ).arg( theWidth ).arg( theHeight ) );
  QgsDebugMsg( QString( "theExtent = %1" ).arg( theExtent.toString() ) );

  QgsRasterBlock *block = new QgsRasterBlock( dataType( theBandNo ), theWidth, theHeight, noDataValue( theBandNo ) );

  if ( block->isEmpty() )
  {
    QgsDebugMsg( "Couldn't create raster block" );
    return block;
  }

  // Read necessary extent only
  QgsRectangle tmpExtent = extent().intersect( &theExtent );

  if ( tmpExtent.isEmpty() )
  {
    QgsDebugMsg( "Extent outside provider extent" );
    block->setIsNoData();
    return block;
  }

  double xRes = theExtent.width() / theWidth;
  double yRes = theExtent.height() / theHeight;
  double tmpXRes, tmpYRes;
  double providerXRes = 0;
  double providerYRes = 0;
  if ( capabilities() & ExactResolution )
  {
    providerXRes = extent().width() / xSize();
    providerYRes = extent().height() / ySize();
    tmpXRes = qMax( providerXRes, xRes );
    tmpYRes = qMax( providerYRes, yRes );
    if ( doubleNear( tmpXRes, xRes ) ) tmpXRes = xRes;
    if ( doubleNear( tmpYRes, yRes ) ) tmpYRes = yRes;
  }
  else
  {
    tmpXRes = xRes;
    tmpYRes = yRes;
  }

  if ( tmpExtent != theExtent ||
       tmpXRes > xRes || tmpYRes > yRes )
  {
    // Read smaller extent or lower resolution

    // Calculate row/col limits (before tmpExtent is aligned)
    int fromRow = qRound(( theExtent.yMaximum() - tmpExtent.yMaximum() ) / yRes );
    int toRow = qRound(( theExtent.yMaximum() - tmpExtent.yMinimum() ) / yRes ) - 1;
    int fromCol = qRound(( tmpExtent.xMinimum() - theExtent.xMinimum() ) / xRes ) ;
    int toCol = qRound(( tmpExtent.xMaximum() - theExtent.xMinimum() ) / xRes ) - 1;

    QgsDebugMsg( QString( "fromRow = %1 toRow = %2 fromCol = %3 toCol = %4" ).arg( fromRow ).arg( toRow ).arg( fromCol ).arg( toCol ) );

    if ( fromRow < 0 || fromRow >= theHeight || toRow < 0 || toRow >= theHeight ||
         fromCol < 0 || fromCol >= theWidth || toCol < 0 || toCol >= theWidth )
    {
      // Should not happen
      QgsDebugMsg( "Row or column limits out of range" );
      return block;
    }

    // If lower source resolution is used, the extent must beS aligned to original
    // resolution to avoid possible shift due to resampling
    if ( tmpXRes > xRes )
    {
      int col = floor(( tmpExtent.xMinimum() - extent().xMinimum() ) / providerXRes );
      tmpExtent.setXMinimum( extent().xMinimum() + col * providerXRes );
      col = ceil(( tmpExtent.xMaximum() - extent().xMinimum() ) / providerXRes );
      tmpExtent.setXMaximum( extent().xMinimum() + col * providerXRes );
    }
    if ( tmpYRes > yRes )
    {
      int row = floor(( extent().yMaximum() - tmpExtent.yMaximum() ) / providerYRes );
      tmpExtent.setYMaximum( extent().yMaximum() - row * providerYRes );
      row = ceil(( extent().yMaximum() - tmpExtent.yMinimum() ) / providerYRes );
      tmpExtent.setYMinimum( extent().yMaximum() - row * providerYRes );
    }
    int tmpWidth = qRound( tmpExtent.width() / tmpXRes );
    int tmpHeight = qRound( tmpExtent.height() / tmpYRes );
    tmpXRes = tmpExtent.width() / tmpWidth;
    tmpYRes = tmpExtent.height() / tmpHeight;

    QgsDebugMsg( QString( "Reading smaller block tmpWidth = %1 theHeight = %2" ).arg( tmpWidth ).arg( tmpHeight ) );
    QgsDebugMsg( QString( "tmpExtent = %1" ).arg( tmpExtent.toString() ) );

    block->setIsNoData();

    QgsRasterBlock *tmpBlock = new QgsRasterBlock( dataType( theBandNo ), tmpWidth, tmpHeight, noDataValue( theBandNo ) );

    readBlock( theBandNo, tmpExtent, tmpWidth, tmpHeight, tmpBlock->data() );

    int pixelSize = dataTypeSize( theBandNo );

    double xMin = theExtent.xMinimum();
    double yMax = theExtent.yMaximum();
    double tmpXMin = tmpExtent.xMinimum();
    double tmpYMax = tmpExtent.yMaximum();

    for ( int row = fromRow; row <= toRow; row++ )
    {
      double y = yMax - ( row + 0.5 ) * yRes;
      int tmpRow = floor(( tmpYMax - y ) / tmpYRes );

      for ( int col = fromCol; col <= toCol; col++ )
      {
        double x = xMin + ( col + 0.5 ) * xRes;
        int tmpCol = floor(( x - tmpXMin ) / tmpXRes );

        if ( tmpRow < 0 || tmpRow >= tmpHeight || tmpCol < 0 || tmpCol >= tmpWidth )
        {
          QgsDebugMsg( "Source row or column limits out of range" );
          block->setIsNoData(); // so that the problem becomes obvious and fixed
          delete tmpBlock;
          return block;
        }

        size_t tmpIndex = tmpRow * tmpWidth + tmpCol;
        size_t index = row * theWidth + col;

        char *tmpBits = tmpBlock->bits( tmpIndex );
        char *bits = block->bits( index );
        if ( !tmpBits )
        {
          QgsDebugMsg( QString( "Cannot get input block data tmpRow = %1 tmpCol = %2 tmpIndex = %3." ).arg( tmpRow ).arg( tmpCol ).arg( tmpIndex ) );
          continue;
        }
        if ( !bits )
        {
          QgsDebugMsg( "Cannot set output block data." );
          continue;
        }
        memcpy( bits, tmpBits, pixelSize );
      }
    }

    delete tmpBlock;
  }
  else
  {
    readBlock( theBandNo, theExtent, theWidth, theHeight, block->data() );
  }

  // apply user no data values
  // TODO: there are other readBlock methods where no data are not applied
  QList<QgsRasterBlock::Range> myNoDataRangeList = userNoDataValue( theBandNo );
  if ( !myNoDataRangeList.isEmpty() )
  {
    double myNoDataValue = noDataValue( theBandNo );
    size_t size = theWidth * theHeight;
    for ( size_t i = 0; i < size; i++ )
    {
      double value = block->value( i );

      if ( QgsRasterBlock::valueInRange( value, myNoDataRangeList ) )
      {
        block->setValue( i, myNoDataValue );
      }
    }
  }

  return block;
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

#if 0
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
#endif

QMap<int, void *> QgsRasterDataProvider::identify( const QgsPoint & point )
{
  QMap<int, void *> results;

  QgsRectangle myExtent = extent();

  for ( int i = 1; i <= bandCount(); i++ )
  {
    double x = point.x();
    double y = point.y();

    // Calculate the row / column where the point falls
    double xres = ( myExtent.xMaximum() - myExtent.xMinimum() ) / xSize();
    double yres = ( myExtent.yMaximum() - myExtent.yMinimum() ) / ySize();

    int col = ( int ) floor(( x - myExtent.xMinimum() ) / xres );
    int row = ( int ) floor(( myExtent.yMaximum() - y ) / yres );

    double xMin = myExtent.xMinimum() + col * xres;
    double xMax = xMin + xres;
    double yMax = myExtent.yMaximum() - row * yres;
    double yMin = yMax - yres;
    QgsRectangle pixelExtent( xMin, yMin, xMax, yMax );

    void * data = block( i, pixelExtent, 1, 2 );

    if ( data )
    {
      results.insert( i, data );
    }
  }
  return results;
}

QString QgsRasterDataProvider::lastErrorFormat()
{
  return "text/plain";
}

bool QgsRasterDataProvider::hasPyramids()
{
  QList<QgsRasterPyramid> myPyramidList = buildPyramidList();

  if ( myPyramidList.isEmpty() )
    return false;

  QList<QgsRasterPyramid>::iterator myRasterPyramidIterator;
  for ( myRasterPyramidIterator = myPyramidList.begin();
        myRasterPyramidIterator != myPyramidList.end();
        ++myRasterPyramidIterator )
  {
    if ( myRasterPyramidIterator->exists )
    {
      return true;
    }
  }
  return false;
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
    QgsDebugMsg( "Cannot collect statistics (raster size or block size) are unknown" );
    return QgsRasterBandStats(); //invalid raster band stats
  }

  int myNXBlocks = ( xSize() + xBlockSize() - 1 ) / xBlockSize();
  int myNYBlocks = ( ySize() + yBlockSize() - 1 ) / yBlockSize();

  void *myData = CPLMalloc( xBlockSize() * yBlockSize() * ( dataTypeSize( theBandNo ) ) );

  // unfortunately we need to make two passes through the data to calculate stddev
  bool myFirstIterationFlag = true;

  int myBandXSize = xSize();
  int myBandYSize = ySize();
  int maxCount = xSize() * ySize();
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

          if ( mValidNoDataValue &&
               (( std::isnan( myNoDataValue ) && std::isnan( myValue ) ) ||  qAbs( myValue - myNoDataValue ) <= TINY_VALUE ) )
          {
            continue; // NULL
          }

          myRasterBandStats.sum += myValue;
          // sum can easily run out of limits
          myRasterBandStats.mean += myValue / maxCount;
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
  //myRasterBandStats.mean = myRasterBandStats.sum / myRasterBandStats.elementCount;
  myRasterBandStats.mean = maxCount * ( myRasterBandStats.mean / myRasterBandStats.elementCount );

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

          if ( mValidNoDataValue &&
               (( std::isnan( myNoDataValue ) && std::isnan( myValue ) ) ||  qAbs( myValue - myNoDataValue ) <= TINY_VALUE ) )
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
  myRasterBandStats.stdDev = static_cast < double >( sqrt( myRasterBandStats.sumOfSquares / ( myRasterBandStats.elementCount - 1 ) ) );

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

  int myDataType = dataType( theBandNo );

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

  void *myData = CPLMalloc( myXBlockSize * myYBlockSize * ( QgsRasterBlock::typeSize( dataType( theBandNo ) ) ) );

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

      readBlock( theBandNo, myPartExtent, myBlockWidth, myBlockHeight, myData );

      // Collect the histogram counts.
      for ( int myY = 0; myY < myBlockHeight; myY++ )
      {
        for ( int myX = 0; myX < myBlockWidth; myX++ )
        {
          double myValue = readValue( myData, myDataType, myX + ( myY * myBlockWidth ) );
          //QgsDebugMsg ( QString ( "%1 %2 value %3" ).arg (myX).arg(myY).arg( myValue ) );

          // TODO: user nodata
          if ( isNoDataValue( theBandNo, myValue ) )
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
    if ( mySrcDataType == QgsRasterBlock::Byte )
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
    if ( mySrcDataType == QgsRasterBlock::Byte )
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
    if ( mySrcDataType == QgsRasterBlock::Byte )
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

  int myDataType = dataType( theBandNo );

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

  void *myData = CPLMalloc( myXBlockSize * myYBlockSize * ( QgsRasterBlock::typeSize( dataType( theBandNo ) ) ) );

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

      readBlock( theBandNo, myPartExtent, myBlockWidth, myBlockHeight, myData );

      // Collect the histogram counts.
      for ( int myY = 0; myY < myBlockHeight; myY++ )
      {
        for ( int myX = 0; myX < myBlockWidth; myX++ )
        {
          double myValue = readValue( myData, myDataType, myX + ( myY * myBlockWidth ) );
          //QgsDebugMsg ( QString ( "%1 %2 value %3" ).arg (myX).arg(myY).arg( myValue ) );

          // TODO: user defined nodata values
          if ( isNoDataValue( theBandNo, myValue ) )
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
    return std::numeric_limits<double>::quiet_NaN();

  switch ( type )
  {
    case QgsRasterBlock::Byte:
      return ( double )(( GByte * )data )[index];
      break;
    case QgsRasterBlock::UInt16:
      return ( double )(( GUInt16 * )data )[index];
      break;
    case QgsRasterBlock::Int16:
      return ( double )(( GInt16 * )data )[index];
      break;
    case QgsRasterBlock::UInt32:
      return ( double )(( GUInt32 * )data )[index];
      break;
    case QgsRasterBlock::Int32:
      return ( double )(( GInt32 * )data )[index];
      break;
    case QgsRasterBlock::Float32:
      return ( double )(( float * )data )[index];
      break;
    case QgsRasterBlock::Float64:
      return ( double )(( double * )data )[index];
      break;
    default:
      QgsLogger::warning( "GDAL data type is not supported" );
  }

  return std::numeric_limits<double>::quiet_NaN();
}

void QgsRasterDataProvider::setUserNoDataValue( int bandNo, QList<QgsRasterBlock::Range> noData )
{
  //if ( bandNo > bandCount() ) return;
  if ( bandNo >= mUserNoDataValue.size() )
  {
    for ( int i = mUserNoDataValue.size(); i < bandNo; i++ )
    {
      mUserNoDataValue.append( QList<QgsRasterBlock::Range>() );
    }
  }
  QgsDebugMsg( QString( "set %1 band %1 no data ranges" ).arg( noData.size() ) );

  if ( mUserNoDataValue[bandNo-1] != noData )
  {
    // Clear statistics
    int i = 0;
    while ( i < mStatistics.size() )
    {
      if ( mStatistics.value( i ).bandNumber == bandNo )
      {
        mStatistics.removeAt( i );
        mHistograms.removeAt( i );
      }
      else
      {
        i++;
      }
    }
    mUserNoDataValue[bandNo-1] = noData;
  }
}

// ENDS
