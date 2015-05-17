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

QgsRasterProjector::QgsRasterProjector(
  QgsCoordinateReferenceSystem theSrcCRS,
  QgsCoordinateReferenceSystem theDestCRS,
  int theSrcDatumTransform,
  int theDestDatumTransform,
  QgsRectangle theDestExtent,
  int theDestRows, int theDestCols,
  double theMaxSrcXRes, double theMaxSrcYRes,
  QgsRectangle theExtent )
    : QgsRasterInterface( 0 )
    , mSrcCRS( theSrcCRS )
    , mDestCRS( theDestCRS )
    , mSrcDatumTransform( theSrcDatumTransform )
    , mDestDatumTransform( theDestDatumTransform )
    , mDestExtent( theDestExtent )
    , mExtent( theExtent )
    , mDestRows( theDestRows ), mDestCols( theDestCols )
    , pHelperTop( 0 ), pHelperBottom( 0 )
    , mMaxSrcXRes( theMaxSrcXRes ), mMaxSrcYRes( theMaxSrcYRes )
{
  QgsDebugMsg( "Entered" );
  QgsDebugMsg( "theDestExtent = " + theDestExtent.toString() );

  calc();
}

QgsRasterProjector::QgsRasterProjector(
  QgsCoordinateReferenceSystem theSrcCRS,
  QgsCoordinateReferenceSystem theDestCRS,
  QgsRectangle theDestExtent,
  int theDestRows, int theDestCols,
  double theMaxSrcXRes, double theMaxSrcYRes,
  QgsRectangle theExtent )
    : QgsRasterInterface( 0 )
    , mSrcCRS( theSrcCRS )
    , mDestCRS( theDestCRS )
    , mSrcDatumTransform( -1 )
    , mDestDatumTransform( -1 )
    , mDestExtent( theDestExtent )
    , mExtent( theExtent )
    , mDestRows( theDestRows ), mDestCols( theDestCols )
    , pHelperTop( 0 ), pHelperBottom( 0 )
    , mMaxSrcXRes( theMaxSrcXRes ), mMaxSrcYRes( theMaxSrcYRes )
{
  QgsDebugMsg( "Entered" );
  QgsDebugMsg( "theDestExtent = " + theDestExtent.toString() );

  calc();
}

QgsRasterProjector::QgsRasterProjector(
  QgsCoordinateReferenceSystem theSrcCRS,
  QgsCoordinateReferenceSystem theDestCRS,
  double theMaxSrcXRes, double theMaxSrcYRes,
  QgsRectangle theExtent )
    : QgsRasterInterface( 0 )
    , mSrcCRS( theSrcCRS )
    , mDestCRS( theDestCRS )
    , mSrcDatumTransform( -1 )
    , mDestDatumTransform( -1 )
    , mExtent( theExtent )
    , mDestRows( 0 )
    , mDestCols( 0 )
    , mDestXRes( 0.0 )
    , mDestYRes( 0.0 )
    , mSrcRows( 0 )
    , mSrcCols( 0 )
    , mSrcXRes( 0.0 )
    , mSrcYRes( 0.0 )
    , mDestRowsPerMatrixRow( 0.0 )
    , mDestColsPerMatrixCol( 0.0 )
    , pHelperTop( 0 ), pHelperBottom( 0 )
    , mHelperTopRow( 0 )
    , mCPCols( 0 )
    , mCPRows( 0 )
    , mSqrTolerance( 0.0 )
    , mMaxSrcXRes( theMaxSrcXRes )
    , mMaxSrcYRes( theMaxSrcYRes )
    , mApproximate( false )
{
  QgsDebugMsg( "Entered" );
}

QgsRasterProjector::QgsRasterProjector()
    : QgsRasterInterface( 0 )
    , mSrcDatumTransform( -1 )
    , mDestDatumTransform( -1 )
    , mDestRows( 0 )
    , mDestCols( 0 )
    , mDestXRes( 0.0 )
    , mDestYRes( 0.0 )
    , mSrcRows( 0 )
    , mSrcCols( 0 )
    , mSrcXRes( 0.0 )
    , mSrcYRes( 0.0 )
    , mDestRowsPerMatrixRow( 0.0 )
    , mDestColsPerMatrixCol( 0.0 )
    , pHelperTop( 0 )
    , pHelperBottom( 0 )
    , mHelperTopRow( 0 )
    , mCPCols( 0 )
    , mCPRows( 0 )
    , mSqrTolerance( 0.0 )
    , mMaxSrcXRes( 0 )
    , mMaxSrcYRes( 0 )
    , mApproximate( false )
{
  QgsDebugMsg( "Entered" );
}

QgsRasterProjector::QgsRasterProjector( const QgsRasterProjector &projector )
    : QgsRasterInterface( 0 )
    , pHelperTop( NULL )
    , pHelperBottom( NULL )
    , mHelperTopRow( 0 )
    , mCPCols( 0 )
    , mCPRows( 0 )
    , mSqrTolerance( 0 )
    , mApproximate( false )
{
  mSrcCRS = projector.mSrcCRS;
  mDestCRS = projector.mDestCRS;
  mSrcDatumTransform = projector.mSrcDatumTransform;
  mDestDatumTransform = projector.mDestDatumTransform;
  mMaxSrcXRes = projector.mMaxSrcXRes;
  mMaxSrcYRes = projector.mMaxSrcYRes;
  mExtent = projector.mExtent;
  mDestRows = projector.mDestRows;
  mDestCols = projector.mDestCols;
  mDestXRes = projector.mDestXRes;
  mDestYRes = projector.mDestYRes;
  mSrcRows = projector.mSrcRows;
  mSrcCols = projector.mSrcCols;
  mSrcXRes = projector.mSrcXRes;
  mSrcYRes = projector.mSrcYRes;
  mDestRowsPerMatrixRow = projector.mDestRowsPerMatrixRow;
  mDestColsPerMatrixCol = projector.mDestColsPerMatrixCol;

}

QgsRasterProjector & QgsRasterProjector::operator=( const QgsRasterProjector & projector )
{
  if ( &projector != this )
  {
    mSrcCRS = projector.mSrcCRS;
    mDestCRS = projector.mDestCRS;
    mSrcDatumTransform = projector.mSrcDatumTransform;
    mDestDatumTransform = projector.mDestDatumTransform;
    mMaxSrcXRes = projector.mMaxSrcXRes;
    mMaxSrcYRes = projector.mMaxSrcYRes;
    mExtent = projector.mExtent;
  }
  return *this;
}

QgsRasterInterface * QgsRasterProjector::clone() const
{
  QgsDebugMsg( "Entered" );
  QgsRasterProjector * projector = new QgsRasterProjector( mSrcCRS, mDestCRS, mMaxSrcXRes, mMaxSrcYRes, mExtent );
  projector->mSrcDatumTransform = mSrcDatumTransform;
  projector->mDestDatumTransform = mDestDatumTransform;
  return projector;
}

QgsRasterProjector::~QgsRasterProjector()
{
  delete[] pHelperTop;
  delete[] pHelperBottom;
}

int QgsRasterProjector::bandCount() const
{
  if ( mInput ) return mInput->bandCount();

  return 0;
}

QGis::DataType QgsRasterProjector::dataType( int bandNo ) const
{
  if ( mInput ) return mInput->dataType( bandNo );

  return QGis::UnknownDataType;
}

void QgsRasterProjector::setCRS( const QgsCoordinateReferenceSystem & theSrcCRS, const QgsCoordinateReferenceSystem & theDestCRS, int srcDatumTransform, int destDatumTransform )
{
  mSrcCRS = theSrcCRS;
  mDestCRS = theDestCRS;
  mSrcDatumTransform = srcDatumTransform;
  mDestDatumTransform = destDatumTransform;
}

void QgsRasterProjector::calc()
{
  QgsDebugMsg( "Entered" );
  mCPMatrix.clear();
  mCPLegalMatrix.clear();
  delete[] pHelperTop;
  pHelperTop = 0;
  delete[] pHelperBottom;
  pHelperBottom = 0;

  // Get max source resolution and extent if possible
  mMaxSrcXRes = 0;
  mMaxSrcYRes = 0;
  if ( mInput )
  {
    QgsRasterDataProvider *provider = dynamic_cast<QgsRasterDataProvider*>( mInput->srcInput() );
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

  const QgsCoordinateTransform* inverseCt = QgsCoordinateTransformCache::instance()->transform( mDestCRS.authid(), mSrcCRS.authid(), mDestDatumTransform, mSrcDatumTransform );

  // Initialize the matrix by corners and middle points
  mCPCols = mCPRows = 3;
  for ( int i = 0; i < mCPRows; i++ )
  {
    QList<QgsPoint> myRow;
    myRow.append( QgsPoint() );
    myRow.append( QgsPoint() );
    myRow.append( QgsPoint() );
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
      QgsDebugMsg( "CP matrix within tolerance" );
      mApproximate = true;
      break;
    }
    // What is the maximum reasonable size of transformatio matrix?
    // TODO: consider better when to break - ratio
    if ( mCPRows * mCPCols > 0.25 * mDestRows * mDestCols )
    {
      QgsDebugMsg( "Too large CP matrix" );
      mApproximate = false;
      break;
    }
  }
  QgsDebugMsg( QString( "CPMatrix size: mCPRows = %1 mCPCols = %2" ).arg( mCPRows ).arg( mCPCols ) );
  mDestRowsPerMatrixRow = ( float )mDestRows / ( mCPRows - 1 );
  mDestColsPerMatrixCol = ( float )mDestCols / ( mCPCols - 1 );

  QgsDebugMsgLevel( "CPMatrix:", 5 );
  QgsDebugMsgLevel( cpToString(), 5 );

  // Calculate source dimensions
  calcSrcExtent();
  calcSrcRowsCols();
  mSrcYRes = mSrcExtent.height() / mSrcRows;
  mSrcXRes = mSrcExtent.width() / mSrcCols;

  // init helper points
  pHelperTop = new QgsPoint[mDestCols];
  pHelperBottom = new QgsPoint[mDestCols];
  calcHelper( 0, pHelperTop );
  calcHelper( 1, pHelperBottom );
  mHelperTopRow = 0;
}

void QgsRasterProjector::calcSrcExtent()
{
  /* Run around the mCPMatrix and find source extent */
  // Attention, source limits are not necessarily on destination edges, e.g.
  // for destination EPSG:32661 Polar Stereographic and source EPSG:4326,
  // the maximum y may be in the middle of destination extent
  // TODO: How to find extent exactly and quickly?
  // For now, we runt through all matrix
  QgsPoint myPoint = mCPMatrix[0][0];
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

  QgsDebugMsg( "mSrcExtent = " + mSrcExtent.toString() );
  QgsDebugMsg( "mExtent = " + mExtent.toString() );
  if ( !mExtent.isEmpty() )
  {
    if ( mMaxSrcXRes > 0 )
    {
      // with floor/ceil it should work correctly also for mSrcExtent.xMinimum() < mExtent.xMinimum()
      double col = floor(( mSrcExtent.xMinimum() - mExtent.xMinimum() ) / mMaxSrcXRes );
      double x = mExtent.xMinimum() + col * mMaxSrcXRes;
      mSrcExtent.setXMinimum( x );

      col = ceil(( mSrcExtent.xMaximum() - mExtent.xMinimum() ) / mMaxSrcXRes );
      x = mExtent.xMinimum() + col * mMaxSrcXRes;
      mSrcExtent.setXMaximum( x );
    }
    if ( mMaxSrcYRes > 0 )
    {
      double row = floor(( mExtent.yMaximum() - mSrcExtent.yMaximum() ) / mMaxSrcYRes );
      double y = mExtent.yMaximum() - row * mMaxSrcYRes;
      mSrcExtent.setYMaximum( y );

      row = ceil(( mExtent.yMaximum() - mSrcExtent.yMinimum() ) / mMaxSrcYRes );
      y = mExtent.yMaximum() - row * mMaxSrcYRes;
      mSrcExtent.setYMinimum( y );
    }
  }
  QgsDebugMsg( "mSrcExtent = " + mSrcExtent.toString() );
}

QString QgsRasterProjector::cpToString()
{
  QString myString;
  for ( int i = 0; i < mCPRows; i++ )
  {
    if ( i > 0 )
      myString += "\n";
    for ( int j = 0; j < mCPCols; j++ )
    {
      if ( j > 0 )
        myString += "  ";
      QgsPoint myPoint = mCPMatrix[i][j];
      if ( mCPLegalMatrix[i][j] )
      {
        myString += myPoint.toString();
      }
      else
      {
        myString += "(-,-)";
      }
    }
  }
  return myString;
}

void QgsRasterProjector::calcSrcRowsCols()
{
  // Wee need to calculate minimum cell size in the source
  // TODO: Think it over better, what is the right source resolution?
  //       Taking distances between cell centers projected to source along source
  //       axis would result in very high resolution
  // TODO: different resolution for rows and cols ?

  // For now, we take cell sizes projected to source but not to source axes
  double myDestColsPerMatrixCell = ( double )mDestCols / mCPCols;
  double myDestRowsPerMatrixCell = ( double )mDestRows / mCPRows;
  QgsDebugMsg( QString( "myDestColsPerMatrixCell = %1 myDestRowsPerMatrixCell = %2" ).arg( myDestColsPerMatrixCell ).arg( myDestRowsPerMatrixCell ) );

  double myMinSize = DBL_MAX;

  for ( int i = 0; i < mCPRows - 1; i++ )
  {
    for ( int j = 0; j < mCPCols - 1; j++ )
    {
      QgsPoint myPointA = mCPMatrix[i][j];
      QgsPoint myPointB = mCPMatrix[i][j+1];
      QgsPoint myPointC = mCPMatrix[i+1][j];
      if ( mCPLegalMatrix[i][j] && mCPLegalMatrix[i][j+1] && mCPLegalMatrix[i+1][j] )
      {
        double mySize = sqrt( myPointA.sqrDist( myPointB ) ) / myDestColsPerMatrixCell;
        if ( mySize < myMinSize )
          myMinSize = mySize;

        mySize = sqrt( myPointA.sqrDist( myPointC ) ) / myDestRowsPerMatrixCell;
        if ( mySize < myMinSize )
          myMinSize = mySize;
      }
    }
  }

  // Make it a bit higher resolution
  // TODO: find the best coefficient, attention, increasing resolution for WMS
  // is changing WMS content
  myMinSize *= 0.75;

  QgsDebugMsg( QString( "mMaxSrcXRes = %1 mMaxSrcYRes = %2" ).arg( mMaxSrcXRes ).arg( mMaxSrcYRes ) );
  // mMaxSrcXRes, mMaxSrcYRes may be 0 - no limit (WMS)
  double myMinXSize = mMaxSrcXRes > myMinSize ? mMaxSrcXRes : myMinSize;
  double myMinYSize = mMaxSrcYRes > myMinSize ? mMaxSrcYRes : myMinSize;
  QgsDebugMsg( QString( "myMinXSize = %1 myMinYSize = %2" ).arg( myMinXSize ).arg( myMinYSize ) );
  QgsDebugMsg( QString( "mSrcExtent.width = %1 mSrcExtent.height = %2" ).arg( mSrcExtent.width() ).arg( mSrcExtent.height() ) );

  // we have to round to keep alignment set in calcSrcExtent
  mSrcRows = ( int ) qRound( mSrcExtent.height() / myMinYSize );
  mSrcCols = ( int ) qRound( mSrcExtent.width() / myMinXSize );

  QgsDebugMsg( QString( "mSrcRows = %1 mSrcCols = %2" ).arg( mSrcRows ).arg( mSrcCols ) );
}


inline void QgsRasterProjector::destPointOnCPMatrix( int theRow, int theCol, double *theX, double *theY )
{
  *theX = mDestExtent.xMinimum() + theCol * mDestExtent.width() / ( mCPCols - 1 );
  *theY = mDestExtent.yMaximum() - theRow * mDestExtent.height() / ( mCPRows - 1 );
}

inline int QgsRasterProjector::matrixRow( int theDestRow )
{
  return ( int )( floor(( theDestRow + 0.5 ) / mDestRowsPerMatrixRow ) );
}
inline int QgsRasterProjector::matrixCol( int theDestCol )
{
  return ( int )( floor(( theDestCol + 0.5 ) / mDestColsPerMatrixCol ) );
}

QgsPoint QgsRasterProjector::srcPoint( int theDestRow, int theCol )
{
  Q_UNUSED( theDestRow );
  Q_UNUSED( theCol );
  return QgsPoint();
}

void QgsRasterProjector::calcHelper( int theMatrixRow, QgsPoint *thePoints )
{
  // TODO?: should we also precalc dest cell center coordinates for x and y?
  for ( int myDestCol = 0; myDestCol < mDestCols; myDestCol++ )
  {
    double myDestX = mDestExtent.xMinimum() + ( myDestCol + 0.5 ) * mDestXRes;

    int myMatrixCol = matrixCol( myDestCol );

    double myDestXMin, myDestYMin, myDestXMax, myDestYMax;

    destPointOnCPMatrix( theMatrixRow, myMatrixCol, &myDestXMin, &myDestYMin );
    destPointOnCPMatrix( theMatrixRow, myMatrixCol + 1, &myDestXMax, &myDestYMax );

    double xfrac = ( myDestX - myDestXMin ) / ( myDestXMax - myDestXMin );

    QgsPoint &mySrcPoint0 = mCPMatrix[theMatrixRow][myMatrixCol];
    QgsPoint &mySrcPoint1 = mCPMatrix[theMatrixRow][myMatrixCol+1];
    double s = mySrcPoint0.x() + ( mySrcPoint1.x() - mySrcPoint0.x() ) * xfrac;
    double t = mySrcPoint0.y() + ( mySrcPoint1.y() - mySrcPoint0.y() ) * xfrac;

    thePoints[myDestCol].setX( s );
    thePoints[myDestCol].setY( t );
  }
}
void QgsRasterProjector::nextHelper()
{
  // We just switch pHelperTop and pHelperBottom, memory is not lost
  QgsPoint *tmp;
  tmp = pHelperTop;
  pHelperTop = pHelperBottom;
  pHelperBottom = tmp;
  calcHelper( mHelperTopRow + 2, pHelperBottom );
  mHelperTopRow++;
}

bool QgsRasterProjector::srcRowCol( int theDestRow, int theDestCol, int *theSrcRow, int *theSrcCol, const QgsCoordinateTransform* ct )
{
  if ( mApproximate )
  {
    return approximateSrcRowCol( theDestRow, theDestCol, theSrcRow, theSrcCol );
  }
  else
  {
    return preciseSrcRowCol( theDestRow, theDestCol, theSrcRow, theSrcCol, ct );
  }
}

bool QgsRasterProjector::preciseSrcRowCol( int theDestRow, int theDestCol, int *theSrcRow, int *theSrcCol, const QgsCoordinateTransform* ct )
{
#ifdef QGISDEBUG
  QgsDebugMsgLevel( QString( "theDestRow = %1" ).arg( theDestRow ), 5 );
  QgsDebugMsgLevel( QString( "theDestRow = %1 mDestExtent.yMaximum() = %2 mDestYRes = %3" ).arg( theDestRow ).arg( mDestExtent.yMaximum() ).arg( mDestYRes ), 5 );
#endif

  // Get coordinate of center of destination cell
  double x = mDestExtent.xMinimum() + ( theDestCol + 0.5 ) * mDestXRes;
  double y = mDestExtent.yMaximum() - ( theDestRow + 0.5 ) * mDestYRes;
  double z = 0;

#ifdef QGISDEBUG
  QgsDebugMsgLevel( QString( "x = %1 y = %2" ).arg( x ).arg( y ), 5 );
#endif

  if ( ct )
  {
    ct->transformInPlace( x, y, z );
  }

#ifdef QGISDEBUG
  QgsDebugMsgLevel( QString( "x = %1 y = %2" ).arg( x ).arg( y ), 5 );
#endif

  if ( !mExtent.contains( QgsPoint( x, y ) ) )
  {
    return false;
  }
  // Get source row col
  *theSrcRow = ( int ) floor(( mSrcExtent.yMaximum() - y ) / mSrcYRes );
  *theSrcCol = ( int ) floor(( x - mSrcExtent.xMinimum() ) / mSrcXRes );
#ifdef QGISDEBUG
  QgsDebugMsgLevel( QString( "mSrcExtent.yMinimum() = %1 mSrcExtent.yMaximum() = %2 mSrcYRes = %3" ).arg( mSrcExtent.yMinimum() ).arg( mSrcExtent.yMaximum() ).arg( mSrcYRes ), 5 );
  QgsDebugMsgLevel( QString( "theSrcRow = %1 theSrcCol = %2" ).arg( *theSrcRow ).arg( *theSrcCol ), 5 );
#endif

  // With epsg 32661 (Polar Stereographic) it was happening that *theSrcCol == mSrcCols
  // For now silently correct limits to avoid crashes
  // TODO: review
  // should not happen
  if ( *theSrcRow >= mSrcRows ) return false;
  if ( *theSrcRow < 0 ) return false;
  if ( *theSrcCol >= mSrcCols ) return false;
  if ( *theSrcCol < 0 ) return false;

  return true;
}

bool QgsRasterProjector::approximateSrcRowCol( int theDestRow, int theDestCol, int *theSrcRow, int *theSrcCol )
{
  int myMatrixRow = matrixRow( theDestRow );
  int myMatrixCol = matrixCol( theDestCol );

  if ( myMatrixRow > mHelperTopRow )
  {
    // TODO: make it more robust (for random, not sequential reading)
    nextHelper();
  }

  double myDestY = mDestExtent.yMaximum() - ( theDestRow + 0.5 ) * mDestYRes;

  // See the schema in javax.media.jai.WarpGrid doc (but up side down)
  // TODO: use some kind of cache of values which can be reused
  double myDestXMin, myDestYMin, myDestXMax, myDestYMax;

  destPointOnCPMatrix( myMatrixRow + 1, myMatrixCol, &myDestXMin, &myDestYMin );
  destPointOnCPMatrix( myMatrixRow, myMatrixCol + 1, &myDestXMax, &myDestYMax );

  double yfrac = ( myDestY - myDestYMin ) / ( myDestYMax - myDestYMin );

  QgsPoint &myTop = pHelperTop[theDestCol];
  QgsPoint &myBot = pHelperBottom[theDestCol];

  // Warning: this is very SLOW compared to the following code!:
  //double mySrcX = myBot.x() + (myTop.x() - myBot.x()) * yfrac;
  //double mySrcY = myBot.y() + (myTop.y() - myBot.y()) * yfrac;

  double tx = myTop.x();
  double ty = myTop.y();
  double bx = myBot.x();
  double by = myBot.y();
  double mySrcX = bx + ( tx - bx ) * yfrac;
  double mySrcY = by + ( ty - by ) * yfrac;

  if ( !mExtent.contains( QgsPoint( mySrcX, mySrcY ) ) )
  {
    return false;
  }

  // TODO: check again cell selection (coor is in the middle)

  *theSrcRow = ( int ) floor(( mSrcExtent.yMaximum() - mySrcY ) / mSrcYRes );
  *theSrcCol = ( int ) floor(( mySrcX - mSrcExtent.xMinimum() ) / mSrcXRes );

  // For now silently correct limits to avoid crashes
  // TODO: review
  // should not happen
  if ( *theSrcRow >= mSrcRows ) return false;
  if ( *theSrcRow < 0 ) return false;
  if ( *theSrcCol >= mSrcCols ) return false;
  if ( *theSrcCol < 0 ) return false;

  return true;
}

void QgsRasterProjector::insertRows( const QgsCoordinateTransform* ct )
{
  for ( int r = 0; r < mCPRows - 1; r++ )
  {
    QList<QgsPoint> myRow;
    QList<bool> myLegalRow;
    for ( int c = 0; c < mCPCols; c++ )
    {
      myRow.append( QgsPoint() );
      myLegalRow.append( false );
    }
    QgsDebugMsgLevel( QString( "insert new row at %1" ).arg( 1 + r*2 ), 3 );
    mCPMatrix.insert( 1 + r*2, myRow );
    mCPLegalMatrix.insert( 1 + r*2, myLegalRow );
  }
  mCPRows += mCPRows - 1;
  for ( int r = 1; r < mCPRows - 1; r += 2 )
  {
    calcRow( r, ct );
  }
}

void QgsRasterProjector::insertCols( const QgsCoordinateTransform* ct )
{
  for ( int r = 0; r < mCPRows; r++ )
  {
    QList<QgsPoint> myRow;
    QList<bool> myLegalRow;
    for ( int c = 0; c < mCPCols - 1; c++ )
    {
      mCPMatrix[r].insert( 1 + c*2, QgsPoint() );
      mCPLegalMatrix[r].insert( 1 + c*2, false );
    }
  }
  mCPCols += mCPCols - 1;
  for ( int c = 1; c < mCPCols - 1; c += 2 )
  {
    calcCol( c, ct );
  }

}

void QgsRasterProjector::calcCP( int theRow, int theCol, const QgsCoordinateTransform* ct )
{
  double myDestX, myDestY;
  destPointOnCPMatrix( theRow, theCol, &myDestX, &myDestY );
  QgsPoint myDestPoint( myDestX, myDestY );
  try
  {
    if ( ct )
    {
      mCPMatrix[theRow][theCol] = ct->transform( myDestPoint );
      mCPLegalMatrix[theRow][theCol] = true;
    }
    else
    {
      mCPLegalMatrix[theRow][theCol] = false;
    }
  }
  catch ( QgsCsException &e )
  {
    Q_UNUSED( e );
    // Caught an error in transform
    mCPLegalMatrix[theRow][theCol] = false;
  }
}

bool QgsRasterProjector::calcRow( int theRow, const QgsCoordinateTransform* ct )
{
  QgsDebugMsgLevel( QString( "theRow = %1" ).arg( theRow ), 3 );
  for ( int i = 0; i < mCPCols; i++ )
  {
    calcCP( theRow, i, ct );
  }

  return true;
}

bool QgsRasterProjector::calcCol( int theCol, const QgsCoordinateTransform* ct )
{
  QgsDebugMsgLevel( QString( "theCol = %1" ).arg( theCol ), 3 );
  for ( int i = 0; i < mCPRows; i++ )
  {
    calcCP( i, theCol, ct );
  }

  return true;
}

bool QgsRasterProjector::checkCols( const QgsCoordinateTransform* ct )
{
  if ( !ct )
  {
    return false;
  }

  for ( int c = 0; c < mCPCols; c++ )
  {
    for ( int r = 1; r < mCPRows - 1; r += 2 )
    {
      double myDestX, myDestY;
      destPointOnCPMatrix( r, c, &myDestX, &myDestY );
      QgsPoint myDestPoint( myDestX, myDestY );

      QgsPoint mySrcPoint1 = mCPMatrix[r-1][c];
      QgsPoint mySrcPoint2 = mCPMatrix[r][c];
      QgsPoint mySrcPoint3 = mCPMatrix[r+1][c];

      QgsPoint mySrcApprox(( mySrcPoint1.x() + mySrcPoint3.x() ) / 2, ( mySrcPoint1.y() + mySrcPoint3.y() ) / 2 );
      if ( !mCPLegalMatrix[r-1][c] || !mCPLegalMatrix[r][c] || !mCPLegalMatrix[r+1][c] )
      {
        // There was an error earlier in transform, just abort
        return false;
      }
      try
      {
        QgsPoint myDestApprox = ct->transform( mySrcApprox, QgsCoordinateTransform::ReverseTransform );
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

bool QgsRasterProjector::checkRows( const QgsCoordinateTransform* ct )
{
  if ( !ct )
  {
    return false;
  }

  for ( int r = 0; r < mCPRows; r++ )
  {
    for ( int c = 1; c < mCPCols - 1; c += 2 )
    {
      double myDestX, myDestY;
      destPointOnCPMatrix( r, c, &myDestX, &myDestY );

      QgsPoint myDestPoint( myDestX, myDestY );
      QgsPoint mySrcPoint1 = mCPMatrix[r][c-1];
      QgsPoint mySrcPoint2 = mCPMatrix[r][c];
      QgsPoint mySrcPoint3 = mCPMatrix[r][c+1];

      QgsPoint mySrcApprox(( mySrcPoint1.x() + mySrcPoint3.x() ) / 2, ( mySrcPoint1.y() + mySrcPoint3.y() ) / 2 );
      if ( !mCPLegalMatrix[r][c-1] || !mCPLegalMatrix[r][c] || !mCPLegalMatrix[r][c+1] )
      {
        // There was an error earlier in transform, just abort
        return false;
      }
      try
      {
        QgsPoint myDestApprox = ct->transform( mySrcApprox, QgsCoordinateTransform::ReverseTransform );
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

QgsRasterBlock * QgsRasterProjector::block( int bandNo, QgsRectangle  const & extent, int width, int height )
{
  QgsDebugMsg( QString( "extent:\n%1" ).arg( extent.toString() ) );
  QgsDebugMsg( QString( "width = %1 height = %2" ).arg( width ).arg( height ) );
  if ( !mInput )
  {
    QgsDebugMsg( "Input not set" );
    return new QgsRasterBlock();
  }

  if ( ! mSrcCRS.isValid() || ! mDestCRS.isValid() || mSrcCRS == mDestCRS )
  {
    QgsDebugMsg( "No projection necessary" );
    return mInput->block( bandNo, extent, width, height );
  }

  mDestExtent = extent;
  mDestRows = height;
  mDestCols = width;
  calc();

  QgsDebugMsg( QString( "srcExtent:\n%1" ).arg( srcExtent().toString() ) );
  QgsDebugMsg( QString( "srcCols = %1 srcRows = %2" ).arg( srcCols() ).arg( srcRows() ) );

  // If we zoom out too much, projector srcRows / srcCols maybe 0, which can cause problems in providers
  if ( srcRows() <= 0 || srcCols() <= 0 )
  {
    QgsDebugMsg( "Zero srcRows or srcCols" );
    return new QgsRasterBlock();
  }

  QgsRasterBlock *inputBlock = mInput->block( bandNo, srcExtent(), srcCols(), srcRows() );
  if ( !inputBlock || inputBlock->isEmpty() )
  {
    QgsDebugMsg( "No raster data!" );
    delete inputBlock;
    return new QgsRasterBlock();
  }

  qgssize pixelSize = QgsRasterBlock::typeSize( mInput->dataType( bandNo ) );

  QgsRasterBlock *outputBlock;
  if ( inputBlock->hasNoDataValue() )
  {
    outputBlock = new QgsRasterBlock( inputBlock->dataType(), width, height, inputBlock->noDataValue() );
  }
  else
  {
    outputBlock = new QgsRasterBlock( inputBlock->dataType(), width, height );
  }
  if ( !outputBlock->isValid() )
  {
    QgsDebugMsg( "Cannot create block" );
    delete inputBlock;
    return outputBlock;
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

  const QgsCoordinateTransform* inverseCt = 0;
  if ( !mApproximate )
  {
    inverseCt = QgsCoordinateTransformCache::instance()->transform( mDestCRS.authid(), mSrcCRS.authid(), mDestDatumTransform, mSrcDatumTransform );
  }

  outputBlock->setIsNoData();

  int srcRow, srcCol;
  for ( int i = 0; i < height; ++i )
  {
    for ( int j = 0; j < width; ++j )
    {
      bool inside = srcRowCol( i, j, &srcRow, &srcCol, inverseCt );
      if ( !inside ) continue; // we have everything set to no data

      qgssize srcIndex = ( qgssize )srcRow * mSrcCols + srcCol;
      QgsDebugMsgLevel( QString( "row = %1 col = %2 srcRow = %3 srcCol = %4" ).arg( i ).arg( j ).arg( srcRow ).arg( srcCol ), 5 );

      // isNoData() may be slow so we check doNoData first
      if ( doNoData && inputBlock->isNoData( srcRow, srcCol ) )
      {
        outputBlock->setIsNoData( i, j );
        continue;
      }

      qgssize destIndex = ( qgssize )i * width + j;
      char *srcBits = inputBlock->bits( srcIndex );
      char *destBits = outputBlock->bits( destIndex );
      if ( !srcBits )
      {
        QgsDebugMsg( QString( "Cannot get input block data: row = %1 col = %2" ).arg( i ).arg( j ) );
        continue;
      }
      if ( !destBits )
      {
        QgsDebugMsg( QString( "Cannot set output block data: srcRow = %1 srcCol = %2" ).arg( srcRow ).arg( srcCol ) );
        continue;
      }
      memcpy( destBits, srcBits, pixelSize );
    }
  }

  delete inputBlock;

  return outputBlock;
}

bool QgsRasterProjector::destExtentSize( const QgsRectangle& theSrcExtent, int theSrcXSize, int theSrcYSize,
    QgsRectangle& theDestExtent, int& theDestXSize, int& theDesYSize )
{
  if ( theSrcExtent.isEmpty() || theSrcXSize <= 0 || theSrcYSize <= 0 )
  {
    return false;
  }
  QgsCoordinateTransform ct( mSrcCRS, mDestCRS );
  theDestExtent = ct.transformBoundingBox( theSrcExtent );

  // We reproject pixel rectangle from center of source, of course, it gives
  // bigger xRes,yRes than reprojected edges (envelope), it may also be that
  // close to margins are higher resolutions (even very, too high)
  // TODO: consider more precise resolution calculation
  double xRes = theSrcExtent.width() / theSrcXSize;
  double yRes = theSrcExtent.height() / theSrcYSize;
  QgsPoint srcCenter = theSrcExtent.center();
  QgsRectangle srcCenterRectangle( srcCenter.x() - xRes / 2, srcCenter.y() - yRes / 2, srcCenter.x() + xRes / 2, srcCenter.y() + yRes / 2 );
  QgsRectangle destCenterRectangle =  ct.transformBoundingBox( srcCenterRectangle );
  theDestXSize = std::max( 1, ( int )( theDestExtent.width() / destCenterRectangle.width() ) );
  theDesYSize = std::max( 1, ( int )( theDestExtent.height() / destCenterRectangle.height() ) );
  return true;
}
