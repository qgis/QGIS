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
#include "qgslogger.h"
#include "qgsrasterprojector.h"
#include "qgscoordinatetransform.h"
#include "qgsexception.h"

Q_NOWARN_DEPRECATED_PUSH // because of deprecated members
QgsRasterProjector::QgsRasterProjector()
  : QgsRasterInterface( nullptr )
{
  QgsDebugMsgLevel( QStringLiteral( "Entered" ), 4 );
}
Q_NOWARN_DEPRECATED_POP


QgsRasterProjector *QgsRasterProjector::clone() const
{
  QgsDebugMsgLevel( QStringLiteral( "Entered" ), 4 );
  QgsRasterProjector *projector = new QgsRasterProjector;
  projector->mSrcCRS = mSrcCRS;
  projector->mDestCRS = mDestCRS;
  projector->mTransformContext = mTransformContext;

  Q_NOWARN_DEPRECATED_PUSH
  projector->mSrcDatumTransform = mSrcDatumTransform;
  projector->mDestDatumTransform = mDestDatumTransform;
  Q_NOWARN_DEPRECATED_POP

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

  return Qgis::DataType::UnknownDataType;
}


/// @cond PRIVATE

void QgsRasterProjector::setCrs( const QgsCoordinateReferenceSystem &srcCRS,
                                 const QgsCoordinateReferenceSystem &destCRS,
                                 int srcDatumTransform,
                                 int destDatumTransform )
{
  mSrcCRS = srcCRS;
  mDestCRS = destCRS;
  Q_NOWARN_DEPRECATED_PUSH
  mSrcDatumTransform = srcDatumTransform;
  mDestDatumTransform = destDatumTransform;
  Q_NOWARN_DEPRECATED_POP
}

void QgsRasterProjector::setCrs( const QgsCoordinateReferenceSystem &srcCRS, const QgsCoordinateReferenceSystem &destCRS, QgsCoordinateTransformContext transformContext )
{
  mSrcCRS = srcCRS;
  mDestCRS = destCRS;
  mTransformContext = transformContext;
  Q_NOWARN_DEPRECATED_PUSH
  mSrcDatumTransform = -1;
  mDestDatumTransform = -1;
  Q_NOWARN_DEPRECATED_POP
}


ProjectorData::ProjectorData( const QgsRectangle &extent, int width, int height, QgsRasterInterface *input, const QgsCoordinateTransform &inverseCt, QgsRasterProjector::Precision precision, QgsRasterBlockFeedback *feedback )
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
  QgsDebugMsgLevel( QStringLiteral( "Entered" ), 4 );

  // Get max source resolution and extent if possible
  if ( input )
  {
    QgsRasterDataProvider *provider = dynamic_cast<QgsRasterDataProvider *>( input->sourceInput() );
    if ( provider )
    {
      // If provider-side resampling is possible, we will get a much better looking
      // result by not requesting at the maximum resolution and then doing nearest
      // resampling here. A real fix would be to do resampling during reprojection
      // however.
      if ( !( provider->providerCapabilities() & QgsRasterDataProvider::ProviderHintCanPerformProviderResampling ) &&
           ( provider->capabilities() & QgsRasterDataProvider::Size ) )
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
  const double myDestRes = mDestXRes < mDestYRes ? mDestXRes : mDestYRes;
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

  // For WebMercator to geographic, if the bounding box is symmetric in Y,
  // and we only use 3 sample points, we would believe that there is perfect
  // linear approximation, resulting in wrong reprojection
  // (see https://github.com/qgis/QGIS/issues/34518).
  // But if we use 5 sample points, we will detect the non-linearity and will
  // refine the CPMatrix appropriately.
  if ( std::fabs( -mDestExtent.yMinimum() - mDestExtent.yMaximum() ) / height < 0.5 * mDestYRes )
    mCPRows = 5;

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
    const bool myColsOK = checkCols( inverseCt );
    if ( !myColsOK )
    {
      insertRows( inverseCt );
    }
    const bool myRowsOK = checkRows( inverseCt );
    if ( !myRowsOK )
    {
      insertCols( inverseCt );
    }
    if ( myColsOK && myRowsOK )
    {
      QgsDebugMsgLevel( QStringLiteral( "CP matrix within tolerance" ), 4 );
      break;
    }
    // What is the maximum reasonable size of transformatio matrix?
    // TODO: consider better when to break - ratio
    if ( mCPRows * mCPCols > 0.25 * mDestRows * mDestCols )
      //if ( mCPRows * mCPCols > mDestRows * mDestCols )
    {
      QgsDebugMsgLevel( QStringLiteral( "Too large CP matrix" ), 4 );
      mApproximate = false;
      break;
    }
    if ( feedback && feedback->isCanceled() )
    {
      return;
    }
  }
  QgsDebugMsgLevel( QStringLiteral( "CPMatrix size: mCPRows = %1 mCPCols = %2" ).arg( mCPRows ).arg( mCPCols ), 4 );
  mDestRowsPerMatrixRow = static_cast< double >( mDestRows ) / ( mCPRows - 1 );
  mDestColsPerMatrixCol = static_cast< double >( mDestCols ) / ( mCPCols - 1 );

#if 0
  QgsDebugMsgLevel( QStringLiteral( "CPMatrix:" ), 5 );
  QgsDebugMsgLevel( cpToString(), 5 );
#endif

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
  mSrcExtent = mSrcExtent.intersect( mExtent );

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
      const QgsPointXY myPoint = mCPMatrix[i][j];
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
    double myMaxSize = 0;

    // For now, we take cell sizes projected to source but not to source axes
    const double myDestColsPerMatrixCell = static_cast< double >( mDestCols ) / mCPCols;
    const double myDestRowsPerMatrixCell = static_cast< double >( mDestRows ) / mCPRows;
    QgsDebugMsgLevel( QStringLiteral( "myDestColsPerMatrixCell = %1 myDestRowsPerMatrixCell = %2" ).arg( myDestColsPerMatrixCell ).arg( myDestRowsPerMatrixCell ), 4 );
    for ( int i = 0; i < mCPRows - 1; i++ )
    {
      for ( int j = 0; j < mCPCols - 1; j++ )
      {
        const QgsPointXY myPointA = mCPMatrix[i][j];
        const QgsPointXY myPointB = mCPMatrix[i][j + 1];
        const QgsPointXY myPointC = mCPMatrix[i + 1][j];
        if ( mCPLegalMatrix[i][j] && mCPLegalMatrix[i][j + 1] && mCPLegalMatrix[i + 1][j] )
        {
          double mySize = std::sqrt( myPointA.sqrDist( myPointB ) ) / myDestColsPerMatrixCell;
          if ( mySize < myMinSize )
            myMinSize = mySize;
          if ( mySize > myMaxSize )
            myMaxSize = mySize;

          mySize = std::sqrt( myPointA.sqrDist( myPointC ) ) / myDestRowsPerMatrixCell;
          if ( mySize < myMinSize )
            myMinSize = mySize;
          if ( mySize > myMaxSize )
            myMaxSize = mySize;
        }
      }
    }
    // Limit resolution to 1/10th of the maximum resolution to avoid issues
    // with for example WebMercator at high northings that result in very small
    // latitude differences.
    if ( myMinSize < 0.1 * myMaxSize )
      myMinSize = 0.1 * myMaxSize;
  }
  else
  {
    // take highest from corners, points in in the middle of corners and center (3 x 3 )
    //double
    QgsRectangle srcExtent;
    int srcXSize, srcYSize;
    if ( QgsRasterProjector::extentSize( mInverseCt, mDestExtent, mDestCols, mDestRows, srcExtent, srcXSize, srcYSize ) )
    {
      const double srcXRes = srcExtent.width() / srcXSize;
      const double srcYRes = srcExtent.height() / srcYSize;
      myMinSize = std::min( srcXRes, srcYRes );
    }
    else
    {
      QgsDebugMsg( QStringLiteral( "Cannot get src extent/size" ) );
    }
  }

  // Make it a bit higher resolution
  // TODO: find the best coefficient, attention, increasing resolution for WMS
  // is changing WMS content
  myMinSize *= 0.75;

  QgsDebugMsgLevel( QStringLiteral( "mMaxSrcXRes = %1 mMaxSrcYRes = %2" ).arg( mMaxSrcXRes ).arg( mMaxSrcYRes ), 4 );
  // mMaxSrcXRes, mMaxSrcYRes may be 0 - no limit (WMS)
  const double myMinXSize = mMaxSrcXRes > myMinSize ? mMaxSrcXRes : myMinSize;
  const double myMinYSize = mMaxSrcYRes > myMinSize ? mMaxSrcYRes : myMinSize;
  QgsDebugMsgLevel( QStringLiteral( "myMinXSize = %1 myMinYSize = %2" ).arg( myMinXSize ).arg( myMinYSize ), 4 );
  QgsDebugMsgLevel( QStringLiteral( "mSrcExtent.width = %1 mSrcExtent.height = %2" ).arg( mSrcExtent.width() ).arg( mSrcExtent.height() ), 4 );

  // we have to round to keep alignment set in calcSrcExtent
  // Limit to 10x the source dimensions to avoid excessive memory allocation
  // and processing time.
  double dblSrcRows = mSrcExtent.height() / myMinYSize;
  if ( dblSrcRows > mDestRows * 10 )
    mSrcRows = mDestRows * 10;
  else
    mSrcRows = static_cast< int >( std::round( dblSrcRows ) );

  double dblSrcCols = mSrcExtent.width() / myMinXSize;
  if ( dblSrcCols > mDestCols * 10 )
    mSrcCols = mDestCols * 10;
  else
    mSrcCols = static_cast< int >( std::round( dblSrcCols ) );

  QgsDebugMsgLevel( QStringLiteral( "mSrcRows = %1 mSrcCols = %2" ).arg( mSrcRows ).arg( mSrcCols ), 4 );
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
    const double myDestX = mDestExtent.xMinimum() + ( myDestCol + 0.5 ) * mDestXRes;

    const int myMatrixCol = matrixCol( myDestCol );

    double myDestXMin, myDestYMin, myDestXMax, myDestYMax;

    destPointOnCPMatrix( matrixRow, myMatrixCol, &myDestXMin, &myDestYMin );
    destPointOnCPMatrix( matrixRow, myMatrixCol + 1, &myDestXMax, &myDestYMax );

    const double xfrac = ( myDestX - myDestXMin ) / ( myDestXMax - myDestXMin );

    const QgsPointXY &mySrcPoint0 = mCPMatrix[matrixRow][myMatrixCol];
    const QgsPointXY &mySrcPoint1 = mCPMatrix[matrixRow][myMatrixCol + 1];
    const double s = mySrcPoint0.x() + ( mySrcPoint1.x() - mySrcPoint0.x() ) * xfrac;
    const double t = mySrcPoint0.y() + ( mySrcPoint1.y() - mySrcPoint0.y() ) * xfrac;

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
#if 0 // too slow, even if we only run it on debug builds!
  QgsDebugMsgLevel( QStringLiteral( "theDestRow = %1" ).arg( destRow ), 5 );
  QgsDebugMsgLevel( QStringLiteral( "theDestRow = %1 mDestExtent.yMaximum() = %2 mDestYRes = %3" ).arg( destRow ).arg( mDestExtent.yMaximum() ).arg( mDestYRes ), 5 );
#endif

  // Get coordinate of center of destination cell
  double x = mDestExtent.xMinimum() + ( destCol + 0.5 ) * mDestXRes;
  double y = mDestExtent.yMaximum() - ( destRow + 0.5 ) * mDestYRes;
  double z = 0;

#if 0
  QgsDebugMsgLevel( QStringLiteral( "x = %1 y = %2" ).arg( x ).arg( y ), 5 );
#endif

  if ( mInverseCt.isValid() )
  {
    try
    {
      mInverseCt.transformInPlace( x, y, z );
    }
    catch ( QgsCsException & )
    {
      return false;
    }
  }

#if 0
  QgsDebugMsgLevel( QStringLiteral( "x = %1 y = %2" ).arg( x ).arg( y ), 5 );
#endif

  if ( !mExtent.contains( x, y ) )
  {
    return false;
  }
  // Get source row col
  *srcRow = static_cast< int >( std::floor( ( mSrcExtent.yMaximum() - y ) / mSrcYRes ) );
  *srcCol = static_cast< int >( std::floor( ( x - mSrcExtent.xMinimum() ) / mSrcXRes ) );
#if 0
  QgsDebugMsgLevel( QStringLiteral( "mSrcExtent.yMinimum() = %1 mSrcExtent.yMaximum() = %2 mSrcYRes = %3" ).arg( mSrcExtent.yMinimum() ).arg( mSrcExtent.yMaximum() ).arg( mSrcYRes ), 5 );
  QgsDebugMsgLevel( QStringLiteral( "theSrcRow = %1 srcCol = %2" ).arg( *srcRow ).arg( *srcCol ), 5 );
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
  const int myMatrixRow = matrixRow( destRow );
  const int myMatrixCol = matrixCol( destCol );

  if ( myMatrixRow > mHelperTopRow )
  {
    // TODO: make it more robust (for random, not sequential reading)
    nextHelper();
  }

  const double myDestY = mDestExtent.yMaximum() - ( destRow + 0.5 ) * mDestYRes;

  // See the schema in javax.media.jai.WarpGrid doc (but up side down)
  // TODO: use some kind of cache of values which can be reused
  double myDestXMin, myDestYMin, myDestXMax, myDestYMax;

  destPointOnCPMatrix( myMatrixRow + 1, myMatrixCol, &myDestXMin, &myDestYMin );
  destPointOnCPMatrix( myMatrixRow, myMatrixCol + 1, &myDestXMax, &myDestYMax );

  const double yfrac = ( myDestY - myDestYMin ) / ( myDestYMax - myDestYMin );

  const QgsPointXY &myTop = pHelperTop[destCol];
  const QgsPointXY &myBot = pHelperBottom[destCol];

  // Warning: this is very SLOW compared to the following code!:
  //double mySrcX = myBot.x() + (myTop.x() - myBot.x()) * yfrac;
  //double mySrcY = myBot.y() + (myTop.y() - myBot.y()) * yfrac;

  const double tx = myTop.x();
  const double ty = myTop.y();
  const double bx = myBot.x();
  const double by = myBot.y();
  const double mySrcX = bx + ( tx - bx ) * yfrac;
  const double mySrcY = by + ( ty - by ) * yfrac;

  if ( !mExtent.contains( mySrcX, mySrcY ) )
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
    QgsDebugMsgLevel( QStringLiteral( "insert new row at %1" ).arg( 1 + r * 2 ), 3 );
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
  const QgsPointXY myDestPoint( myDestX, myDestY );
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
    Q_UNUSED( e )
    // Caught an error in transform
    mCPLegalMatrix[row][col] = false;
  }
}

bool ProjectorData::calcRow( int row, const QgsCoordinateTransform &ct )
{
  QgsDebugMsgLevel( QStringLiteral( "theRow = %1" ).arg( row ), 3 );
  for ( int i = 0; i < mCPCols; i++ )
  {
    calcCP( row, i, ct );
  }

  return true;
}

bool ProjectorData::calcCol( int col, const QgsCoordinateTransform &ct )
{
  QgsDebugMsgLevel( QStringLiteral( "theCol = %1" ).arg( col ), 3 );
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
      const QgsPointXY myDestPoint( myDestX, myDestY );

      const QgsPointXY mySrcPoint1 = mCPMatrix[r - 1][c];
      const QgsPointXY mySrcPoint2 = mCPMatrix[r][c];
      const QgsPointXY mySrcPoint3 = mCPMatrix[r + 1][c];

      const QgsPointXY mySrcApprox( ( mySrcPoint1.x() + mySrcPoint3.x() ) / 2, ( mySrcPoint1.y() + mySrcPoint3.y() ) / 2 );
      if ( !mCPLegalMatrix[r - 1][c] || !mCPLegalMatrix[r][c] || !mCPLegalMatrix[r + 1][c] )
      {
        // There was an error earlier in transform, just abort
        return false;
      }
      try
      {
        const QgsPointXY myDestApprox = ct.transform( mySrcApprox, Qgis::TransformDirection::Reverse );
        const double mySqrDist = myDestApprox.sqrDist( myDestPoint );
        if ( mySqrDist > mSqrTolerance )
        {
          return false;
        }
      }
      catch ( QgsCsException &e )
      {
        Q_UNUSED( e )
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

      const QgsPointXY myDestPoint( myDestX, myDestY );
      const QgsPointXY mySrcPoint1 = mCPMatrix[r][c - 1];
      const QgsPointXY mySrcPoint2 = mCPMatrix[r][c];
      const QgsPointXY mySrcPoint3 = mCPMatrix[r][c + 1];

      const QgsPointXY mySrcApprox( ( mySrcPoint1.x() + mySrcPoint3.x() ) / 2, ( mySrcPoint1.y() + mySrcPoint3.y() ) / 2 );
      if ( !mCPLegalMatrix[r][c - 1] || !mCPLegalMatrix[r][c] || !mCPLegalMatrix[r][c + 1] )
      {
        // There was an error earlier in transform, just abort
        return false;
      }
      try
      {
        const QgsPointXY myDestApprox = ct.transform( mySrcApprox, Qgis::TransformDirection::Reverse );
        const double mySqrDist = myDestApprox.sqrDist( myDestPoint );
        if ( mySqrDist > mSqrTolerance )
        {
          return false;
        }
      }
      catch ( QgsCsException &e )
      {
        Q_UNUSED( e )
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
  QgsDebugMsgLevel( QStringLiteral( "extent:\n%1" ).arg( extent.toString() ), 4 );
  QgsDebugMsgLevel( QStringLiteral( "width = %1 height = %2" ).arg( width ).arg( height ), 4 );
  if ( !mInput )
  {
    QgsDebugMsgLevel( QStringLiteral( "Input not set" ), 4 );
    return new QgsRasterBlock();
  }

  if ( feedback && feedback->isCanceled() )
    return new QgsRasterBlock();

  if ( ! mSrcCRS.isValid() || ! mDestCRS.isValid() || mSrcCRS == mDestCRS )
  {
    QgsDebugMsgLevel( QStringLiteral( "No projection necessary" ), 4 );
    return mInput->block( bandNo, extent, width, height, feedback );
  }

  Q_NOWARN_DEPRECATED_PUSH
  const QgsCoordinateTransform inverseCt = mSrcDatumTransform != -1 || mDestDatumTransform != -1 ?
      QgsCoordinateTransform( mDestCRS, mSrcCRS, mDestDatumTransform, mSrcDatumTransform ) : QgsCoordinateTransform( mDestCRS, mSrcCRS, mTransformContext ) ;
  Q_NOWARN_DEPRECATED_POP

  ProjectorData pd( extent, width, height, mInput, inverseCt, mPrecision, feedback );

  if ( feedback && feedback->isCanceled() )
    return new QgsRasterBlock();

  QgsDebugMsgLevel( QStringLiteral( "srcExtent:\n%1" ).arg( pd.srcExtent().toString() ), 4 );
  QgsDebugMsgLevel( QStringLiteral( "srcCols = %1 srcRows = %2" ).arg( pd.srcCols() ).arg( pd.srcRows() ), 4 );

  // If we zoom out too much, projector srcRows / srcCols maybe 0, which can cause problems in providers
  if ( pd.srcRows() <= 0 || pd.srcCols() <= 0 )
  {
    QgsDebugMsgLevel( QStringLiteral( "Zero srcRows or srcCols" ), 4 );
    return new QgsRasterBlock();
  }

  std::unique_ptr< QgsRasterBlock > inputBlock( mInput->block( bandNo, pd.srcExtent(), pd.srcCols(), pd.srcRows(), feedback ) );
  if ( !inputBlock || inputBlock->isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "No raster data!" ) );
    return new QgsRasterBlock();
  }

  const qgssize pixelSize = static_cast<qgssize>( QgsRasterBlock::typeSize( mInput->dataType( bandNo ) ) );

  std::unique_ptr< QgsRasterBlock > outputBlock( new QgsRasterBlock( inputBlock->dataType(), width, height ) );
  if ( inputBlock->hasNoDataValue() )
  {
    outputBlock->setNoDataValue( inputBlock->noDataValue() );
  }
  if ( !outputBlock->isValid() )
  {
    QgsDebugMsg( QStringLiteral( "Cannot create block" ) );
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
  const bool doNoData = !QgsRasterBlock::typeIsNumeric( inputBlock->dataType() ) && inputBlock->hasNoData() && !inputBlock->hasNoDataValue();

  outputBlock->setIsNoData();

  int srcRow, srcCol;
  for ( int i = 0; i < height; ++i )
  {
    if ( feedback && feedback->isCanceled() )
      break;
    for ( int j = 0; j < width; ++j )
    {
      const bool inside = pd.srcRowCol( i, j, &srcRow, &srcCol );
      if ( !inside ) continue; // we have everything set to no data

      const qgssize srcIndex = static_cast< qgssize >( srcRow ) * pd.srcCols() + srcCol;

      // isNoData() may be slow so we check doNoData first
      if ( doNoData && inputBlock->isNoData( srcRow, srcCol ) )
      {
        outputBlock->setIsNoData( i, j );
        continue;
      }

      const qgssize destIndex = static_cast< qgssize >( i ) * width + j;
      char *srcBits = inputBlock->bits( srcIndex );
      char *destBits = outputBlock->bits( destIndex );
      if ( !srcBits )
      {
        // QgsDebugMsg( QStringLiteral( "Cannot get input block data: row = %1 col = %2" ).arg( i ).arg( j ) );
        continue;
      }
      if ( !destBits )
      {
        // QgsDebugMsg( QStringLiteral( "Cannot set output block data: srcRow = %1 srcCol = %2" ).arg( srcRow ).arg( srcCol ) );
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

  Q_NOWARN_DEPRECATED_PUSH
  const QgsCoordinateTransform ct = mSrcDatumTransform != -1 || mDestDatumTransform != -1 ?
                                    QgsCoordinateTransform( mSrcCRS, mDestCRS, mSrcDatumTransform, mDestDatumTransform ) : QgsCoordinateTransform( mSrcCRS, mDestCRS, mTransformContext ) ;
  Q_NOWARN_DEPRECATED_POP

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

  QgsCoordinateTransform extentTransform = ct;
  extentTransform.setBallparkTransformsAreAppropriate( true );

  destExtent = extentTransform.transformBoundingBox( srcExtent );

  // We reproject pixel rectangle from 9 points matrix of source extent, of course, it gives
  // bigger xRes,yRes than reprojected edges (envelope)
  constexpr int steps = 3;
  const double srcXStep = srcExtent.width() / steps;
  const double srcYStep = srcExtent.height() / steps;
  const double srcXRes = srcExtent.width() / srcXSize;
  const double srcYRes = srcExtent.height() / srcYSize;
  double destXRes = std::numeric_limits<double>::max();
  double destYRes = std::numeric_limits<double>::max();
  double maxXRes = 0;
  double maxYRes = 0;

  for ( int i = 0; i < steps; i++ )
  {
    const double x = srcExtent.xMinimum() + i * srcXStep;
    for ( int j = 0; j < steps; j++ )
    {
      const double y = srcExtent.yMinimum() + j * srcYStep;
      const QgsRectangle srcRectangle( x - srcXRes / 2, y - srcYRes / 2, x + srcXRes / 2, y + srcYRes / 2 );
      try
      {
        const QgsRectangle destRectangle = extentTransform.transformBoundingBox( srcRectangle );
        if ( destRectangle.width() > 0 )
        {
          destXRes = std::min( destXRes, destRectangle.width() );
          if ( destRectangle.width() > maxXRes )
            maxXRes = destRectangle.width();
        }
        if ( destRectangle.height() > 0 )
        {
          destYRes = std::min( destYRes, destRectangle.height() );
          if ( destRectangle.height() > maxYRes )
            maxYRes = destRectangle.height();
        }
      }
      catch ( QgsCsException & )
      {

      }
    }
  }

  // Limit resolution to 1/10th of the maximum resolution to avoid issues
  // with for example WebMercator at high northings that result in very small
  // latitude differences.
  if ( destXRes < 0.1 * maxXRes )
  {
    destXRes = 0.1 * maxXRes;
  }
  if ( destYRes < 0.1 * maxYRes )
  {
    destYRes = 0.1 * maxYRes;
  }
  if ( destXRes == 0 || destExtent.width() / destXRes  > std::numeric_limits<int>::max() )
    return false;
  if ( destYRes == 0 || destExtent.height() / destYRes  > std::numeric_limits<int>::max() )
    return false;

  destXSize = std::max( 1, static_cast< int >( destExtent.width() / destXRes ) );
  destYSize = std::max( 1, static_cast< int >( destExtent.height() / destYRes ) );

  return true;
}

