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
/* $Id: qgsrasterprojector.cpp 15005 2011-01-08 16:35:21Z rblazek $ */

#include <cassert>

#include "qgslogger.h"
#include "qgsrasterprojector.h"
#include "qgscoordinatetransform.h"

QgsRasterProjector::QgsRasterProjector(
  QgsCoordinateReferenceSystem theSrcCRS,
  QgsCoordinateReferenceSystem theDestCRS,
  QgsRectangle theDestExtent,
  int theDestRows, int theDestCols,
  double theMaxSrcXRes, double theMaxSrcYRes,
  QgsRectangle theExtent )
    : mSrcCRS( theSrcCRS )
    , mDestCRS( theDestCRS )
    , mDestExtent( theDestExtent )
    , mExtent( theExtent )
    , mDestRows( theDestRows ), mDestCols( theDestCols )
    , mMaxSrcXRes( theMaxSrcXRes ), mMaxSrcYRes( theMaxSrcYRes )
{
  QgsDebugMsg( "Entered" );
  QgsDebugMsg( "theDestExtent = " + theDestExtent.toString() );

  // reverse transformation
  mCoordinateTransform = new QgsCoordinateTransform( theDestCRS, theSrcCRS );

  mDestXRes = mDestExtent.width() / ( mDestCols );
  mDestYRes = mDestExtent.height() / ( mDestRows );

  // Calculate tolerance
  // TODO: Think it over better
  // Note: we are checking on matrix each even point, that means taht the real error
  // in that moment is approximately half size
  double myDestRes = mDestXRes < mDestYRes ? mDestXRes : mDestYRes;
  mSqrTolerance = myDestRes * myDestRes;

  // Initialize the matrix by corners and middle points
  mCPCols = mCPRows = 3;
  for ( int i = 0; i < mCPRows; i++ )
  {
    QList<QgsPoint> myRow;
    myRow.append( QgsPoint() );
    myRow.append( QgsPoint() );
    myRow.append( QgsPoint() );
    mCPMatrix.insert( i,  myRow );
  }
  for ( int i = 0; i < mCPRows; i++ )
  {
    calcRow( i );
  }

  while ( true )
  {
    bool myColsOK = checkCols();
    if ( !myColsOK )
    {
      insertRows();
    }
    bool myRowsOK = checkRows();
    if ( !myRowsOK )
    {
      insertCols();
    }
    if ( myColsOK && myRowsOK )
    {
      QgsDebugMsg( "CP matrix within tolerance" );
      break;
    }
    // What is the maximum reasonable size of transformatio matrix?
    // TODO: consider better when to break - ratio
    if ( mCPRows * mCPCols > 0.25 * mDestRows * mDestCols )
    {
      QgsDebugMsg( "Too large CP matrix" );
      break;
    }

  }
  QgsDebugMsg( QString( "CPMatrix size: mCPRows = %1 mCPCols = %2" ).arg( mCPRows ).arg( mCPCols ) );
  mDestRowsPerMatrixRow = ( float )mDestRows / ( mCPRows - 1 );
  mDestColsPerMatrixCol = ( float )mDestCols / ( mCPCols - 1 );

  //QgsDebugMsg( "CPMatrix:\n" + cpToString() );

  // Calculate source dimensions
  calcSrcExtent();
  calcSrcRowsCols();
  mSrcXRes = mSrcExtent.height() / mSrcRows;
  mSrcYRes = mSrcExtent.width() / mSrcCols;

  // init helper points
  pHelperTop = new QgsPoint[mDestCols];
  pHelperBottom = new QgsPoint[mDestCols];
  calcHelper( 0, pHelperTop );
  calcHelper( 1, pHelperBottom );
  mHelperTopRow = 0;
}

QgsRasterProjector::~QgsRasterProjector()
{
  delete mCoordinateTransform;
}

void QgsRasterProjector::calcSrcExtent()
{
  /* Run around the mCPMatrix and find source extent */
  QgsPoint myPoint = mCPMatrix[0][0];
  mSrcExtent = QgsRectangle( myPoint.x(), myPoint.y(), myPoint.x(), myPoint.y() );
  for ( int i = 0; i < mCPRows; i++ )
  {
    myPoint = mCPMatrix[i][0];
    mSrcExtent.combineExtentWith( myPoint.x(), myPoint.y() );
    myPoint = mCPMatrix[i][mCPCols-1];
    mSrcExtent.combineExtentWith( myPoint.x(), myPoint.y() );
  }
  for ( int i = 1; i < mCPCols - 1; i++ )
  {
    myPoint = mCPMatrix[0][i];
    mSrcExtent.combineExtentWith( myPoint.x(), myPoint.y() );
    myPoint = mCPMatrix[mCPRows-1][i];
    mSrcExtent.combineExtentWith( myPoint.x(), myPoint.y() );
  }
  // Expand a bit to avoid possible approx coords falling out because of representation error?

  // If mMaxSrcXRes, mMaxSrcYRes are defined (fixed src resolution)
  // align extent to src resolution to avoid jumping reprojected pixels
  // because of shifting resampled grid
  // Important especially if we are over mMaxSrcXRes, mMaxSrcYRes limits

  QgsDebugMsg( "mSrcExtent = " + mSrcExtent.toString() );

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


  QgsDebugMsg( "mSrcExtent = " + mSrcExtent.toString() );
}

QString QgsRasterProjector::cpToString()
{
  QString myString;
  for ( int i = 0; i < mCPRows; i++ )
  {
    if ( i > 0 ) myString += "\n";
    for ( int j = 0; j < mCPCols; j++ )
    {
      if ( j > 0 ) myString += "  ";
      QgsPoint myPoint = mCPMatrix[i][j];
      myString += myPoint.toString();
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
  double myDestColsPerMatrixCell = mDestCols / mCPCols;
  double myDestRowsPerMatrixCell = mDestRows / mCPRows;
  QgsDebugMsg( QString( "myDestColsPerMatrixCell = %1 myDestRowsPerMatrixCell = %2" ).arg( myDestColsPerMatrixCell ).arg( myDestRowsPerMatrixCell ) );

  double myMinSize = DBL_MAX;

  for ( int i = 0; i < mCPRows - 1; i++ )
  {
    for ( int j = 0; j < mCPCols - 1; j++ )
    {
      QgsPoint myPointA = mCPMatrix[i][j];
      QgsPoint myPointB = mCPMatrix[i][j+1];
      QgsPoint myPointC = mCPMatrix[i+1][j];
      double mySize = sqrt( myPointA.sqrDist( myPointB ) ) / myDestColsPerMatrixCell;
      if ( mySize < myMinSize ) { myMinSize = mySize; }

      mySize = sqrt( myPointA.sqrDist( myPointC ) ) / myDestRowsPerMatrixCell;
      if ( mySize < myMinSize ) { myMinSize = mySize; }
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
  QgsPoint *tmp;
  tmp = pHelperTop;
  pHelperTop = pHelperBottom;
  pHelperBottom = tmp;
  calcHelper( mHelperTopRow + 2, pHelperBottom );
  mHelperTopRow++;
}

void QgsRasterProjector::srcRowCol( int theDestRow, int theDestCol, int *theSrcRow, int *theSrcCol )
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

  // TODO: check again cell selection (coor is in the middle)

  *theSrcRow = ( int ) floor(( mSrcExtent.yMaximum() - mySrcY ) / mSrcXRes );
  *theSrcCol = ( int ) floor(( mySrcX - mSrcExtent.xMinimum() ) / mSrcYRes );

  assert( *theSrcRow < mSrcRows );
  assert( *theSrcCol < mSrcCols );
}

void QgsRasterProjector::insertRows()
{
  for ( int r = 0; r < mCPRows - 1; r++ )
  {
    QList<QgsPoint> myRow;
    for ( int c = 0; c < mCPCols; c++ )
    {
      myRow.append( QgsPoint() );
    }
    QgsDebugMsg( QString( "insert new row at %1" ).arg( 1 + r*2 ) );
    mCPMatrix.insert( 1 + r*2,  myRow );
  }
  mCPRows += mCPRows - 1;
  for ( int r = 1; r < mCPRows - 1; r += 2 )
  {
    calcRow( r );
  }
}

void QgsRasterProjector::insertCols()
{
  for ( int r = 0; r < mCPRows; r++ )
  {
    QList<QgsPoint> myRow;
    for ( int c = 0; c < mCPCols - 1; c++ )
    {
      mCPMatrix[r].insert( 1 + c*2,  QgsPoint() );
    }
  }
  mCPCols += mCPCols - 1;
  for ( int c = 1; c < mCPCols - 1; c += 2 )
  {
    calcCol( c );
  }

}

void QgsRasterProjector::calcCP( int theRow, int theCol )
{
  double myDestX, myDestY;
  destPointOnCPMatrix( theRow, theCol, &myDestX, &myDestY );
  QgsPoint myDestPoint( myDestX, myDestY );

  mCPMatrix[theRow][theCol] = mCoordinateTransform->transform( myDestPoint );
}

bool QgsRasterProjector::calcRow( int theRow )
{
  QgsDebugMsg( QString( "theRow = %1" ).arg( theRow ) );
  for ( int i = 0; i < mCPCols; i++ )
  {
    calcCP( theRow, i );
  }

  return true;
}

bool QgsRasterProjector::calcCol( int theCol )
{
  QgsDebugMsgLevel( QString( "theCol = %1" ).arg( theCol ), 3 );
  for ( int i = 0; i < mCPRows; i++ )
  {
    calcCP( i, theCol );
  }

  return true;
}

bool QgsRasterProjector::checkCols()
{
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
      QgsPoint myDestApprox = mCoordinateTransform->transform( mySrcApprox, QgsCoordinateTransform::ReverseTransform );
      double mySqrDist = myDestApprox.sqrDist( myDestPoint );
      if ( mySqrDist > mSqrTolerance ) { return false; }
    }
  }
  return true;
}

bool QgsRasterProjector::checkRows()
{
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
      QgsPoint myDestApprox = mCoordinateTransform->transform( mySrcApprox, QgsCoordinateTransform::ReverseTransform );
      double mySqrDist = myDestApprox.sqrDist( myDestPoint );
      if ( mySqrDist > mSqrTolerance ) { return false; }
    }
  }
  return true;
}
