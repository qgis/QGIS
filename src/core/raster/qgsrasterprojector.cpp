/***************************************************************************
    qgsrasterprojector.cpp - Raster projector
     --------------------------------------
    Date                 : Jan 16, 2011
    Copyright            : (C) 2005 by Radim Blazek
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
#include <algorithm>

#include "qgsrasterdataprovider.h"
#include "qgscrscache.h"
#include "qgslogger.h"
#include "qgsrasterprojector.h"
#include "qgscoordinatetransform.h"
#include "qgsexception.h"


QgsRasterProjector::QgsRasterProjector()
  : QgsRasterInterface( nullptr )
{
  QgsDebugMsgLevel( "Entered", 4 );
}


QgsRasterProjector *QgsRasterProjector::clone() const
{
  QgsDebugMsgLevel( "Entered", 4 );
  QgsRasterProjector *projector = new QgsRasterProjector;
  projector->mSrcCRS = mSrcCRS;
  projector->mDestCRS = mDestCRS;
  projector->mSrcDatumTransform = mSrcDatumTransform;
  projector->mDestDatumTransform = mDestDatumTransform;
  projector->mPrecision = mPrecision;
  return projector;
}

int QgsRasterProjector::bandCount() const
{
  if ( mInput ) return mInput->bandCount();

  return 0;
}

Qgis::DataType QgsRasterProjector::dataType( int bandNo ) const
{
  if ( mInput ) return mInput->dataType( bandNo );

  return Qgis::UnknownDataType;
}


/// @cond PRIVATE


void QgsRasterProjector::setCrs( const QgsCoordinateReferenceSystem &srcCRS, const QgsCoordinateReferenceSystem &destCRS, int srcDatumTransform, int destDatumTransform )
{
  mSrcCRS = srcCRS;
  mDestCRS = destCRS;
  mSrcDatumTransform = srcDatumTransform;
  mDestDatumTransform = destDatumTransform;
}


ProjectorData::ProjectorData( const QgsRectangle &extent, int width, int height, QgsRasterInterface *input, const QgsCoordinateTransform &inverseCt, QgsRasterProjector::Precision precision )
  : mApproximate( false )
  , mInverseCt( inverseCt )
  , mDestExtent( extent )
  , mDestRows( height )
  , mDestCols( width )
  , mDestXRes( 0.0 )
  , mDestYRes( 0.0 )
  , mSrcRows( 0 )
  , mSrcCols( 0 )
  , mSrcXRes( 0.0 )
  , mSrcYRes( 0.0 )
  , mDestRowsPerMatrixRow( 0.0 )
  , mDestColsPerMatrixCol( 0.0 )
  , mHelperTopRow( 0 )
  , mCPCols( 0 )
  , mCPRows( 0 )
  , mSqrTolerance( 0.0 )
  , mMaxSrcXRes( 0 )
  , mMaxSrcYRes( 0 )
{
  QgsDebugMsgLevel( "Entered", 4 );

  // Get max source resolution and extent if possible
  if ( input )
  {
    QgsRasterDataProvider *provider = dynamic_cast<QgsRasterDataProvider *>( input->sourceInput() );
    if ( provider )
    {
      if ( provider->capabilities() & QgsRasterDataProvider::Size )
      {
        mMaxSrcXRes = provider->extent().width() / provider->xSize();
        mMaxSrcYRes = provider->extent().height() / provider->ySize();
      }
      // Get source extent
      if ( mExtent.isEmpty() )
      {
        mExtent = provider->extent();
      }
    }
  }

  mDestXRes = mDestExtent.width() / ( mDestCols );
  mDestYRes = mDestExtent.height() / ( mDestRows );

  // Calculate tolerance
  // TODO: Think it over better
  // Note: we are checking on matrix each even point, that means that the real error
  // in that moment is approximately half size
  double myDestRes = mDestXRes < mDestYRes ? mDestXRes : mDestYRes;
  mSqrTolerance = myDestRes * myDestRes;

  if ( precision == QgsRasterProjector::Approximate )
  {
    mApproximate = true;
  }
  else
  {
    mApproximate = false;
  }

  // Always try to calculate mCPMatrix, it is used in calcSrcExtent() for both Approximate and Exact
  // Initialize the matrix by corners and middle points
  mCPCols = mCPRows = 3;
  for ( int i = 0; i < mCPRows; i++ )
  {
    QList<QgsPointXY> myRow;
    myRow.append( QgsPointXY() );
    myRow.append( QgsPointXY() );
    myRow.append( QgsPointXY() );
    mCPMatrix.insert( i, myRow );
    // And the legal points
    QList<bool> myLegalRow;
    myLegalRow.append( bool( false ) );
    myLegalRow.append( bool( false ) );
    myLegalRow.append( bool( false ) );
    mCPLegalMatrix.insert( i, myLegalRow );
  }
  for ( int i = 0; i < mCPRows; i++ )
  {
    calcRow( i, inverseCt );
  }

  while ( true )
  {
    bool myColsOK = checkCols( inverseCt );
    if ( !myColsOK )
    {
      insertRows( inverseCt );
    }
    bool myRowsOK = checkRows( inverseCt );
    if ( !myRowsOK )
    {
      insertCols( inverseCt );
    }
    if ( myColsOK && myRowsOK )
    {
      QgsDebugMsgLevel( "CP matrix within tolerance", 4 );
      break;
    }
    // What is the maximum reasonable size of transformatio matrix?
    // TODO: consider better when to break - ratio
    if ( mCPRows * mCPCols > 0.25 * mDestRows * mDestCols )
      //if ( mCPRows * mCPCols > mDestRows * mDestCols )
    {
      QgsDebugMsgLevel( "Too large CP matrix", 4 );
      mApproximate = false;
      break;
    }
  }
  QgsDebugMsgLevel( QString( "CPMatrix size: mCPRows = %1 mCPCols = %2" ).arg( mCPRows ).arg( mCPCols ), 4 );
  mDestRowsPerMatrixRow = static_cast< float >( mDestRows ) / ( mCPRows - 1 );
  mDestColsPerMatrixCol = static_cast< float >( mDestCols ) / ( mCPCols - 1 );

  QgsDebugMsgLevel( "CPMatrix:", 5 );
  QgsDebugMsgLevel( cpToString(), 5 );

  // init helper points
  pHelperTop = new QgsPointXY[mDestCols];
  pHelperBottom = new QgsPointXY[mDestCols];
  calcHelper( 0, pHelperTop );
  calcHelper( 1, pHelperBottom );
  mHelperTopRow = 0;

  // Calculate source dimensions
  calcSrcExtent();
  calcSrcRowsCols();
  mSrcYRes = mSrcExtent.height() / mSrcRows;
  mSrcXRes = mSrcExtent.width() / mSrcCols;
}

ProjectorData::~ProjectorData()
{
  delete[] pHelperTop;
  delete[] pHelperBottom;
}


void ProjectorData::calcSrcExtent()
{
  /* Run around the mCPMatrix and find source extent */
  // Attention, source limits are not necessarily on destination edges, e.g.
  // for destination EPSG:32661 Polar Stereographic and source EPSG:4326,
  // the maximum y may be in the middle of destination extent
  // TODO: How to find extent exactly and quickly?
  // For now, we run through all matrix
  // mCPMatrix is used for both Approximate and Exact because QgsCoordinateTransform::transformBoundingBox()
  // is not precise enough, see #13665
  QgsPointXY myPoint = mCPMatrix[0][0];
  mSrcExtent = QgsRectangle( myPoint.x(), myPoint.y(), myPoint.x(), myPoint.y() );
  for ( int i = 0; i < mCPRows; i++ )
  {
    for ( int j = 0; j < mCPCols ; j++ )
    {
      myPoint = mCPMatrix[i][j];
      if ( mCPLegalMatrix[i][j] )
      {
        mSrcExtent.combineExtentWith( myPoint.x(), myPoint.y() );
      }
    }
  }
  // Expand a bit to avoid possible approx coords falling out because of representation error?

  // Combine with maximum source  extent
  mSrcExtent = mSrcExtent.intersect( &mExtent );

  // If mMaxSrcXRes, mMaxSrcYRes are defined (fixed src resolution)
  // align extent to src resolution to avoid jumping of reprojected pixels
  // when shifting resampled grid.
  // Important especially if we are over mMaxSrcXRes, mMaxSrcYRes limits
  // Note however, that preceding filters (like resampler) may read data
  // on different resolution.

  QgsDebugMsgLevel( "mSrcExtent = " + mSrcExtent.toString(), 4 );
  QgsDebugMsgLevel( "mExtent = " + mExtent.toString(), 4 );
  if ( !mExtent.isEmpty() )
  {
    if ( mMaxSrcXRes > 0 )
    {
      // with floor/ceil it should work correctly also for mSrcExtent.xMinimum() < mExtent.xMinimum()
      double col = std::floor( ( mSrcExtent.xMinimum() - mExtent.xMinimum() ) / mMaxSrcXRes );
      double x = mExtent.xMinimum() + col * mMaxSrcXRes;
      mSrcExtent.setXMinimum( x );

      col = std::ceil( ( mSrcExtent.xMaximum() - mExtent.xMinimum() ) / mMaxSrcXRes );
      x = mExtent.xMinimum() + col * mMaxSrcXRes;
      mSrcExtent.setXMaximum( x );
    }
    if ( mMaxSrcYRes > 0 )
    {
      double row = std::floor( ( mExtent.yMaximum() - mSrcExtent.yMaximum() ) / mMaxSrcYRes );
      double y = mExtent.yMaximum() - row * mMaxSrcYRes;
      mSrcExtent.setYMaximum( y );

      row = std::ceil( ( mExtent.yMaximum() - mSrcExtent.yMinimum() ) / mMaxSrcYRes );
      y = mExtent.yMaximum() - row * mMaxSrcYRes;
      mSrcExtent.setYMinimum( y );
    }
  }
  QgsDebugMsgLevel( "mSrcExtent = " + mSrcExtent.toString(), 4 );
}

QString ProjectorData::cpToString()
{
  QString myString;
  for ( int i = 0; i < mCPRows; i++ )
  {
    if ( i > 0 )
      myString += '\n';
    for ( int j = 0; j < mCPCols; j++ )
    {
      if ( j > 0 )
        myString += QLatin1String( "  " );
      QgsPointXY myPoint = mCPMatrix[i][j];
      if ( mCPLegalMatrix[i][j] )
      {
        myString += myPoint.toString();
      }
      else
      {
        myString += QLatin1String( "(-,-)" );
      }
    }
  }
  return myString;
}

void ProjectorData::calcSrcRowsCols()
{
  // Wee need to calculate minimum cell size in the source
  // TODO: Think it over better, what is the right source resolution?
  //       Taking distances between cell centers projected to source along source
  //       axis would result in very high resolution
  // TODO: different resolution for rows and cols ?

  double myMinSize = std::numeric_limits<double>::max();

  if ( mApproximate )
  {
    // For now, we take cell sizes projected to source but not to source axes
    double myDestColsPerMatrixCell = static_cast< double >( mDestCols ) / mCPCols;
    double myDestRowsPerMatrixCell = static_cast< double >( mDestRows ) / mCPRows;
    QgsDebugMsgLevel( QString( "myDestColsPerMatrixCell = %1 myDestRowsPerMatrixCell = %2" ).arg( myDestColsPerMatrixCell ).arg( myDestRowsPerMatrixCell ), 4 );
    for ( int i = 0; i < mCPRows - 1; i++ )
    {
      for ( int j = 0; j < mCPCols - 1; j++ )
      {
        QgsPointXY myPointA = mCPMatrix[i][j];
        QgsPointXY myPointB = mCPMatrix[i][j + 1];
        QgsPointXY myPointC = mCPMatrix[i + 1][j];
        if ( mCPLegalMatrix[i][j] && mCPLegalMatrix[i][j + 1] && mCPLegalMatrix[i + 1][j] )
        {
          double mySize = std::sqrt( myPointA.sqrDist( myPointB ) ) / myDestColsPerMatrixCell;
          if ( mySize < myMinSize )
            myMinSize = mySize;

          mySize = std::sqrt( myPointA.sqrDist( myPointC ) ) / myDestRowsPerMatrixCell;
          if ( mySize < myMinSize )
            myMinSize = mySize;
        }
      }
    }
  }
  else
  {
    // take highest from corners, points in in the middle of corners and center (3 x 3 )
    //double
    QgsRectangle srcExtent;
    int srcXSize, srcYSize;
    if ( QgsRasterProjector::extentSize( mInverseCt, mDestExtent, mDestCols, mDestRows, srcExtent, srcXSize, srcYSize ) )
    {
      double srcXRes = srcExtent.width() / srcXSize;
      double srcYRes = srcExtent.height() / srcYSize;
      myMinSize = std::min( srcXRes, srcYRes );
    }
    else
    {
      QgsDebugMsg( "Cannot get src extent/size" );
    }
  }

  // Make it a bit higher resolution
  // TODO: find the best coefficient, attention, increasing resolution for WMS
  // is changing WMS content
  myMinSize *= 0.75;

  QgsDebugMsgLevel( QString( "mMaxSrcXRes = %1 mMaxSrcYRes = %2" ).arg( mMaxSrcXRes ).arg( mMaxSrcYRes ), 4 );
  // mMaxSrcXRes, mMaxSrcYRes may be 0 - no limit (WMS)
  double myMinXSize = mMaxSrcXRes > myMinSize ? mMaxSrcXRes : myMinSize;
  double myMinYSize = mMaxSrcYRes > myMinSize ? mMaxSrcYRes : myMinSize;
  QgsDebugMsgLevel( QString( "myMinXSize = %1 myMinYSize = %2" ).arg( myMinXSize ).arg( myMinYSize ), 4 );
  QgsDebugMsgLevel( QString( "mSrcExtent.width = %1 mSrcExtent.height = %2" ).arg( mSrcExtent.width() ).arg( mSrcExtent.height() ), 4 );

  // we have to round to keep alignment set in calcSrcExtent
  mSrcRows = static_cast< int >( std::round( mSrcExtent.height() / myMinYSize ) );
  mSrcCols = static_cast< int >( std::round( mSrcExtent.width() / myMinXSize ) );

  QgsDebugMsgLevel( QString( "mSrcRows = %1 mSrcCols = %2" ).arg( mSrcRows ).arg( mSrcCols ), 4 );
}


inline void ProjectorData::destPointOnCPMatrix( int row, int col, double *theX, double *theY )
{
  *theX = mDestExtent.xMinimum() + col * mDestExtent.width() / ( mCPCols - 1 );
  *theY = mDestExtent.yMaximum() - row * mDestExtent.height() / ( mCPRows - 1 );
}

inline int ProjectorData::matrixRow( int destRow )
{
  return static_cast< int >( std::floor( ( destRow + 0.5 ) / mDestRowsPerMatrixRow ) );
}
inline int ProjectorData::matrixCol( int destCol )
{
  return static_cast< int >( std::floor( ( destCol + 0.5 ) / mDestColsPerMatrixCol ) );
}

void ProjectorData::calcHelper( int matrixRow, QgsPointXY *points )
{
  // TODO?: should we also precalc dest cell center coordinates for x and y?
  for ( int myDestCol = 0; myDestCol < mDestCols; myDestCol++ )
  {
    double myDestX = mDestExtent.xMinimum() + ( myDestCol + 0.5 ) * mDestXRes;

    int myMatrixCol = matrixCol( myDestCol );

    double myDestXMin, myDestYMin, myDestXMax, myDestYMax;

    destPointOnCPMatrix( matrixRow, myMatrixCol, &myDestXMin, &myDestYMin );
    destPointOnCPMatrix( matrixRow, myMatrixCol + 1, &myDestXMax, &myDestYMax );

    double xfrac = ( myDestX - myDestXMin ) / ( myDestXMax - myDestXMin );

    QgsPointXY &mySrcPoint0 = mCPMatrix[matrixRow][myMatrixCol];
    QgsPointXY &mySrcPoint1 = mCPMatrix[matrixRow][myMatrixCol + 1];
    double s = mySrcPoint0.x() + ( mySrcPoint1.x() - mySrcPoint0.x() ) * xfrac;
    double t = mySrcPoint0.y() + ( mySrcPoint1.y() - mySrcPoint0.y() ) * xfrac;

    points[myDestCol].setX( s );
    points[myDestCol].setY( t );
  }
}

void ProjectorData::nextHelper()
{
  // We just switch pHelperTop and pHelperBottom, memory is not lost
  QgsPointXY *tmp = nullptr;
  tmp = pHelperTop;
  pHelperTop = pHelperBottom;
  pHelperBottom = tmp;
  calcHelper( mHelperTopRow + 2, pHelperBottom );
  mHelperTopRow++;
}

bool ProjectorData::srcRowCol( int destRow, int destCol, int *srcRow, int *srcCol )
{
  if ( mApproximate )
  {
    return approximateSrcRowCol( destRow, destCol, srcRow, srcCol );
  }
  else
  {
    return preciseSrcRowCol( destRow, destCol, srcRow, srcCol );
  }
}

bool ProjectorData::preciseSrcRowCol( int destRow, int destCol, int *srcRow, int *srcCol )
{
#ifdef QGISDEBUG
  QgsDebugMsgLevel( QString( "theDestRow = %1" ).arg( destRow ), 5 );
  QgsDebugMsgLevel( QString( "theDestRow = %1 mDestExtent.yMaximum() = %2 mDestYRes = %3" ).arg( destRow ).arg( mDestExtent.yMaximum() ).arg( mDestYRes ), 5 );
#endif

  // Get coordinate of center of destination cell
  double x = mDestExtent.xMinimum() + ( destCol + 0.5 ) * mDestXRes;
  double y = mDestExtent.yMaximum() - ( destRow + 0.5 ) * mDestYRes;
  double z = 0;

#ifdef QGISDEBUG
  QgsDebugMsgLevel( QString( "x = %1 y = %2" ).arg( x ).arg( y ), 5 );
#endif

  if ( mInverseCt.isValid() )
  {
    mInverseCt.transformInPlace( x, y, z );
  }

#ifdef QGISDEBUG
  QgsDebugMsgLevel( QString( "x = %1 y = %2" ).arg( x ).arg( y ), 5 );
#endif

  if ( !mExtent.contains( QgsPointXY( x, y ) ) )
  {
    return false;
  }
  // Get source row col
  *srcRow = static_cast< int >( std::floor( ( mSrcExtent.yMaximum() - y ) / mSrcYRes ) );
  *srcCol = static_cast< int >( std::floor( ( x - mSrcExtent.xMinimum() ) / mSrcXRes ) );
#ifdef QGISDEBUG
  QgsDebugMsgLevel( QString( "mSrcExtent.yMinimum() = %1 mSrcExtent.yMaximum() = %2 mSrcYRes = %3" ).arg( mSrcExtent.yMinimum() ).arg( mSrcExtent.yMaximum() ).arg( mSrcYRes ), 5 );
  QgsDebugMsgLevel( QString( "theSrcRow = %1 srcCol = %2" ).arg( *srcRow ).arg( *srcCol ), 5 );
#endif

  // With epsg 32661 (Polar Stereographic) it was happening that *srcCol == mSrcCols
  // For now silently correct limits to avoid crashes
  // TODO: review
  // should not happen
  if ( *srcRow >= mSrcRows ) return false;
  if ( *srcRow < 0 ) return false;
  if ( *srcCol >= mSrcCols ) return false;
  if ( *srcCol < 0 ) return false;

  return true;
}

bool ProjectorData::approximateSrcRowCol( int destRow, int destCol, int *srcRow, int *srcCol )
{
  int myMatrixRow = matrixRow( destRow );
  int myMatrixCol = matrixCol( destCol );

  if ( myMatrixRow > mHelperTopRow )
  {
    // TODO: make it more robust (for random, not sequential reading)
    nextHelper();
  }

  double myDestY = mDestExtent.yMaximum() - ( destRow + 0.5 ) * mDestYRes;

  // See the schema in javax.media.jai.WarpGrid doc (but up side down)
  // TODO: use some kind of cache of values which can be reused
  double myDestXMin, myDestYMin, myDestXMax, myDestYMax;

  destPointOnCPMatrix( myMatrixRow + 1, myMatrixCol, &myDestXMin, &myDestYMin );
  destPointOnCPMatrix( myMatrixRow, myMatrixCol + 1, &myDestXMax, &myDestYMax );

  double yfrac = ( myDestY - myDestYMin ) / ( myDestYMax - myDestYMin );

  QgsPointXY &myTop = pHelperTop[destCol];
  QgsPointXY &myBot = pHelperBottom[destCol];

  // Warning: this is very SLOW compared to the following code!:
  //double mySrcX = myBot.x() + (myTop.x() - myBot.x()) * yfrac;
  //double mySrcY = myBot.y() + (myTop.y() - myBot.y()) * yfrac;

  double tx = myTop.x();
  double ty = myTop.y();
  double bx = myBot.x();
  double by = myBot.y();
  double mySrcX = bx + ( tx - bx ) * yfrac;
  double mySrcY = by + ( ty - by ) * yfrac;

  if ( !mExtent.contains( QgsPointXY( mySrcX, mySrcY ) ) )
  {
    return false;
  }

  // TODO: check again cell selection (coor is in the middle)

  *srcRow = static_cast< int >( std::floor( ( mSrcExtent.yMaximum() - mySrcY ) / mSrcYRes ) );
  *srcCol = static_cast< int >( std::floor( ( mySrcX - mSrcExtent.xMinimum() ) / mSrcXRes ) );

  // For now silently correct limits to avoid crashes
  // TODO: review
  // should not happen
  if ( *srcRow >= mSrcRows ) return false;
  if ( *srcRow < 0 ) return false;
  if ( *srcCol >= mSrcCols ) return false;
  if ( *srcCol < 0 ) return false;

  return true;
}

void ProjectorData::insertRows( const QgsCoordinateTransform &ct )
{
  for ( int r = 0; r < mCPRows - 1; r++ )
  {
    QList<QgsPointXY> myRow;
    QList<bool> myLegalRow;
    myRow.reserve( mCPCols );
    myLegalRow.reserve( mCPCols );
    for ( int c = 0; c < mCPCols; ++c )
    {
      myRow.append( QgsPointXY() );
      myLegalRow.append( false );
    }
    QgsDebugMsgLevel( QString( "insert new row at %1" ).arg( 1 + r * 2 ), 3 );
    mCPMatrix.insert( 1 + r * 2, myRow );
    mCPLegalMatrix.insert( 1 + r * 2, myLegalRow );
  }
  mCPRows += mCPRows - 1;
  for ( int r = 1; r < mCPRows - 1; r += 2 )
  {
    calcRow( r, ct );
  }
}

void ProjectorData::insertCols( const QgsCoordinateTransform &ct )
{
  for ( int r = 0; r < mCPRows; r++ )
  {
    for ( int c = 0; c < mCPCols - 1; c++ )
    {
      mCPMatrix[r].insert( 1 + c * 2, QgsPointXY() );
      mCPLegalMatrix[r].insert( 1 + c * 2, false );
    }
  }
  mCPCols += mCPCols - 1;
  for ( int c = 1; c < mCPCols - 1; c += 2 )
  {
    calcCol( c, ct );
  }

}

void ProjectorData::calcCP( int row, int col, const QgsCoordinateTransform &ct )
{
  double myDestX, myDestY;
  destPointOnCPMatrix( row, col, &myDestX, &myDestY );
  QgsPointXY myDestPoint( myDestX, myDestY );
  try
  {
    if ( ct.isValid() )
    {
      mCPMatrix[row][col] = ct.transform( myDestPoint );
      mCPLegalMatrix[row][col] = true;
    }
    else
    {
      mCPLegalMatrix[row][col] = false;
    }
  }
  catch ( QgsCsException &e )
  {
    Q_UNUSED( e );
    // Caught an error in transform
    mCPLegalMatrix[row][col] = false;
  }
}

bool ProjectorData::calcRow( int row, const QgsCoordinateTransform &ct )
{
  QgsDebugMsgLevel( QString( "theRow = %1" ).arg( row ), 3 );
  for ( int i = 0; i < mCPCols; i++ )
  {
    calcCP( row, i, ct );
  }

  return true;
}

bool ProjectorData::calcCol( int col, const QgsCoordinateTransform &ct )
{
  QgsDebugMsgLevel( QString( "theCol = %1" ).arg( col ), 3 );
  for ( int i = 0; i < mCPRows; i++ )
  {
    calcCP( i, col, ct );
  }

  return true;
}

bool ProjectorData::checkCols( const QgsCoordinateTransform &ct )
{
  if ( !ct.isValid() )
  {
    return false;
  }

  for ( int c = 0; c < mCPCols; c++ )
  {
    for ( int r = 1; r < mCPRows - 1; r += 2 )
    {
      double myDestX, myDestY;
      destPointOnCPMatrix( r, c, &myDestX, &myDestY );
      QgsPointXY myDestPoint( myDestX, myDestY );

      QgsPointXY mySrcPoint1 = mCPMatrix[r - 1][c];
      QgsPointXY mySrcPoint2 = mCPMatrix[r][c];
      QgsPointXY mySrcPoint3 = mCPMatrix[r + 1][c];

      QgsPointXY mySrcApprox( ( mySrcPoint1.x() + mySrcPoint3.x() ) / 2, ( mySrcPoint1.y() + mySrcPoint3.y() ) / 2 );
      if ( !mCPLegalMatrix[r - 1][c] || !mCPLegalMatrix[r][c] || !mCPLegalMatrix[r + 1][c] )
      {
        // There was an error earlier in transform, just abort
        return false;
      }
      try
      {
        QgsPointXY myDestApprox = ct.transform( mySrcApprox, QgsCoordinateTransform::ReverseTransform );
        double mySqrDist = myDestApprox.sqrDist( myDestPoint );
        if ( mySqrDist > mSqrTolerance )
        {
          return false;
        }
      }
      catch ( QgsCsException &e )
      {
        Q_UNUSED( e );
        // Caught an error in transform
        return false;
      }
    }
  }
  return true;
}

bool ProjectorData::checkRows( const QgsCoordinateTransform &ct )
{
  if ( !ct.isValid() )
  {
    return false;
  }

  for ( int r = 0; r < mCPRows; r++ )
  {
    for ( int c = 1; c < mCPCols - 1; c += 2 )
    {
      double myDestX, myDestY;
      destPointOnCPMatrix( r, c, &myDestX, &myDestY );

      QgsPointXY myDestPoint( myDestX, myDestY );
      QgsPointXY mySrcPoint1 = mCPMatrix[r][c - 1];
      QgsPointXY mySrcPoint2 = mCPMatrix[r][c];
      QgsPointXY mySrcPoint3 = mCPMatrix[r][c + 1];

      QgsPointXY mySrcApprox( ( mySrcPoint1.x() + mySrcPoint3.x() ) / 2, ( mySrcPoint1.y() + mySrcPoint3.y() ) / 2 );
      if ( !mCPLegalMatrix[r][c - 1] || !mCPLegalMatrix[r][c] || !mCPLegalMatrix[r][c + 1] )
      {
        // There was an error earlier in transform, just abort
        return false;
      }
      try
      {
        QgsPointXY myDestApprox = ct.transform( mySrcApprox, QgsCoordinateTransform::ReverseTransform );
        double mySqrDist = myDestApprox.sqrDist( myDestPoint );
        if ( mySqrDist > mSqrTolerance )
        {
          return false;
        }
      }
      catch ( QgsCsException &e )
      {
        Q_UNUSED( e );
        // Caught an error in transform
        return false;
      }
    }
  }
  return true;
}

/// @endcond


QString QgsRasterProjector::precisionLabel( Precision precision )
{
  switch ( precision )
  {
    case Approximate:
      return tr( "Approximate" );
    case Exact:
      return tr( "Exact" );
  }
  return QStringLiteral( "Unknown" );
}

QgsRasterBlock *QgsRasterProjector::block( int bandNo, QgsRectangle  const &extent, int width, int height, QgsRasterBlockFeedback *feedback )
{
  QgsDebugMsgLevel( QString( "extent:\n%1" ).arg( extent.toString() ), 4 );
  QgsDebugMsgLevel( QString( "width = %1 height = %2" ).arg( width ).arg( height ), 4 );
  if ( !mInput )
  {
    QgsDebugMsgLevel( "Input not set", 4 );
    return new QgsRasterBlock();
  }

  if ( feedback && feedback->isCanceled() )
    return new QgsRasterBlock();

  if ( ! mSrcCRS.isValid() || ! mDestCRS.isValid() || mSrcCRS == mDestCRS )
  {
    QgsDebugMsgLevel( "No projection necessary", 4 );
    return mInput->block( bandNo, extent, width, height, feedback );
  }

  QgsCoordinateTransform inverseCt = QgsCoordinateTransformCache::instance()->transform( mDestCRS.authid(), mSrcCRS.authid(), mDestDatumTransform, mSrcDatumTransform );

  ProjectorData pd( extent, width, height, mInput, inverseCt, mPrecision );

  QgsDebugMsgLevel( QString( "srcExtent:\n%1" ).arg( pd.srcExtent().toString() ), 4 );
  QgsDebugMsgLevel( QString( "srcCols = %1 srcRows = %2" ).arg( pd.srcCols() ).arg( pd.srcRows() ), 4 );

  // If we zoom out too much, projector srcRows / srcCols maybe 0, which can cause problems in providers
  if ( pd.srcRows() <= 0 || pd.srcCols() <= 0 )
  {
    QgsDebugMsgLevel( "Zero srcRows or srcCols", 4 );
    return new QgsRasterBlock();
  }

  std::unique_ptr< QgsRasterBlock > inputBlock( mInput->block( bandNo, pd.srcExtent(), pd.srcCols(), pd.srcRows(), feedback ) );
  if ( !inputBlock || inputBlock->isEmpty() )
  {
    QgsDebugMsg( "No raster data!" );
    return new QgsRasterBlock();
  }

  qgssize pixelSize = QgsRasterBlock::typeSize( mInput->dataType( bandNo ) );

  std::unique_ptr< QgsRasterBlock > outputBlock( new QgsRasterBlock( inputBlock->dataType(), width, height ) );
  if ( inputBlock->hasNoDataValue() )
  {
    outputBlock->setNoDataValue( inputBlock->noDataValue() );
  }
  if ( !outputBlock->isValid() )
  {
    QgsDebugMsg( "Cannot create block" );
    return outputBlock.release();
  }

  // set output to no data, it should be fast
  outputBlock->setIsNoData();

  // No data: because isNoData()/setIsNoData() is slow with respect to simple memcpy,
  // we use if only if necessary:
  // 1) no data value exists (numerical) -> memcpy, not necessary isNoData()/setIsNoData()
  // 2) no data value does not exist but it may contain no data (numerical no data bitmap)
  //    -> must use isNoData()/setIsNoData()
  // 3) no data are not used (no no data value, no no data bitmap) -> simple memcpy
  // 4) image - simple memcpy

  // To copy no data values stored in bitmaps we have to use isNoData()/setIsNoData(),
  // we cannot fill output block with no data because we use memcpy for data, not setValue().
  bool doNoData = !QgsRasterBlock::typeIsNumeric( inputBlock->dataType() ) && inputBlock->hasNoData() && !inputBlock->hasNoDataValue();

  outputBlock->setIsNoData();

  int srcRow, srcCol;
  for ( int i = 0; i < height; ++i )
  {
    if ( feedback && feedback->isCanceled() )
      break;
    for ( int j = 0; j < width; ++j )
    {
      bool inside = pd.srcRowCol( i, j, &srcRow, &srcCol );
      if ( !inside ) continue; // we have everything set to no data

      qgssize srcIndex = static_cast< qgssize >( srcRow ) * pd.srcCols() + srcCol;

      // isNoData() may be slow so we check doNoData first
      if ( doNoData && inputBlock->isNoData( srcRow, srcCol ) )
      {
        outputBlock->setIsNoData( i, j );
        continue;
      }

      qgssize destIndex = static_cast< qgssize >( i ) * width + j;
      char *srcBits = inputBlock->bits( srcIndex );
      char *destBits = outputBlock->bits( destIndex );
      if ( !srcBits )
      {
        // QgsDebugMsg( QString( "Cannot get input block data: row = %1 col = %2" ).arg( i ).arg( j ) );
        continue;
      }
      if ( !destBits )
      {
        // QgsDebugMsg( QString( "Cannot set output block data: srcRow = %1 srcCol = %2" ).arg( srcRow ).arg( srcCol ) );
        continue;
      }
      memcpy( destBits, srcBits, pixelSize );
      outputBlock->setIsData( i, j );
    }
  }

  return outputBlock.release();
}

bool QgsRasterProjector::destExtentSize( const QgsRectangle &srcExtent, int srcXSize, int srcYSize,
    QgsRectangle &destExtent, int &destXSize, int &destYSize )
{
  if ( srcExtent.isEmpty() || srcXSize <= 0 || srcYSize <= 0 )
  {
    return false;
  }
  QgsCoordinateTransform ct = QgsCoordinateTransformCache::instance()->transform( mSrcCRS.authid(), mDestCRS.authid(), mSrcDatumTransform, mDestDatumTransform );

  return extentSize( ct, srcExtent, srcXSize, srcYSize, destExtent, destXSize, destYSize );
}

bool QgsRasterProjector::extentSize( const QgsCoordinateTransform &ct,
                                     const QgsRectangle &srcExtent, int srcXSize, int srcYSize,
                                     QgsRectangle &destExtent, int &destXSize, int &destYSize )
{
  if ( srcExtent.isEmpty() || srcXSize <= 0 || srcYSize <= 0 )
  {
    return false;
  }

  destExtent = ct.transformBoundingBox( srcExtent );

  // We reproject pixel rectangle from 9 points matrix of source extent, of course, it gives
  // bigger xRes,yRes than reprojected edges (envelope)
  double srcXStep = srcExtent.width() / 3;
  double srcYStep = srcExtent.height() / 3;
  double srcXRes = srcExtent.width() / srcXSize;
  double srcYRes = srcExtent.height() / srcYSize;
  double destXRes = std::numeric_limits<double>::max();
  double destYRes = std::numeric_limits<double>::max();

  for ( int i = 0; i < 3; i++ )
  {
    double x = srcExtent.xMinimum() + i * srcXStep;
    for ( int j = 0; j < 3; j++ )
    {
      double y = srcExtent.yMinimum() + j * srcYStep;
      QgsRectangle srcRectangle( x - srcXRes / 2, y - srcYRes / 2, x + srcXRes / 2, y + srcYRes / 2 );
      QgsRectangle destRectangle = ct.transformBoundingBox( srcRectangle );
      if ( destRectangle.width() > 0 )
      {
        destXRes = std::min( destXRes, destRectangle.width() );
      }
      if ( destRectangle.height() > 0 )
      {
        destYRes = std::min( destYRes, destRectangle.height() );
      }
    }
  }
  destXSize = std::max( 1, static_cast< int >( destExtent.width() / destYRes ) );
  destYSize = std::max( 1, static_cast< int >( destExtent.height() / destYRes ) );

  return true;
}

