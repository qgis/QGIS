/***************************************************************************
               QgsCoordinateTransform.cpp  - Coordinate Transforms
                             -------------------
    begin                : Dec 2004
    copyright            : (C) 2004 Tim Sutton
    email                : tim at linfiniti.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgscoordinatetransform.h"
#include "qgscoordinatetransform_p.h"
#include "qgsapplication.h"
#include "qgsmessagelog.h"
#include "qgslogger.h"
#include "qgspointxy.h"
#include "qgsrectangle.h"
#include "qgsexception.h"
#include "qgsproject.h"
#include "qgsreadwritelocker.h"
#include "qgsvector3d.h"
#include "qgis.h"

//qt includes
#include <QDomNode>
#include <QDomElement>
#include <QApplication>
#include <QPolygonF>
#include <QStringList>
#include <QVector>

#include <proj.h>
#include "qgsprojutils.h"

#include <sqlite3.h>
#include <qlogging.h>
#include <vector>
#include <algorithm>

// if defined shows all information about transform to stdout
// #define COORDINATE_TRANSFORM_VERBOSE

QReadWriteLock QgsCoordinateTransform::sCacheLock;
QMultiHash< QPair< QString, QString >, QgsCoordinateTransform > QgsCoordinateTransform::sTransforms; //same auth_id pairs might have different datum transformations
bool QgsCoordinateTransform::sDisableCache = false;

std::function< void( const QgsCoordinateReferenceSystem &sourceCrs,
                     const QgsCoordinateReferenceSystem &destinationCrs,
                     const QString &desiredOperation )> QgsCoordinateTransform::sFallbackOperationOccurredHandler = nullptr;

QgsCoordinateTransform::QgsCoordinateTransform()
{
  d = new QgsCoordinateTransformPrivate();
}

QgsCoordinateTransform::QgsCoordinateTransform( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &destination, const QgsCoordinateTransformContext &context, Qgis::CoordinateTransformationFlags flags )
{
  mContext = context;
  d = new QgsCoordinateTransformPrivate( source, destination, mContext );

  if ( flags & Qgis::CoordinateTransformationFlag::IgnoreImpossibleTransformations )
    mIgnoreImpossible = true;

#ifdef QGISDEBUG
  mHasContext = true;
#endif

  if ( mIgnoreImpossible && !isTransformationPossible( source, destination ) )
  {
    d->invalidate();
    return;
  }

  if ( !d->checkValidity() )
    return;

  Q_NOWARN_DEPRECATED_PUSH
  if ( !setFromCache( d->mSourceCRS, d->mDestCRS, d->mProjCoordinateOperation, d->mAllowFallbackTransforms ) )
  {
    d->initialize();
    addToCache();
  }
  Q_NOWARN_DEPRECATED_POP

  if ( flags & Qgis::CoordinateTransformationFlag::BallparkTransformsAreAppropriate )
    mBallparkTransformsAreAppropriate = true;
}

QgsCoordinateTransform::QgsCoordinateTransform( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &destination, const QgsProject *project, Qgis::CoordinateTransformationFlags flags )
{
  mContext = project ? project->transformContext() : QgsCoordinateTransformContext();
  d = new QgsCoordinateTransformPrivate( source, destination, mContext );
#ifdef QGISDEBUG
  if ( project )
    mHasContext = true;
#endif

  if ( flags & Qgis::CoordinateTransformationFlag::IgnoreImpossibleTransformations )
    mIgnoreImpossible = true;

  if ( mIgnoreImpossible && !isTransformationPossible( source, destination ) )
  {
    d->invalidate();
    return;
  }

  if ( !d->checkValidity() )
    return;

  Q_NOWARN_DEPRECATED_PUSH
  if ( !setFromCache( d->mSourceCRS, d->mDestCRS, d->mProjCoordinateOperation, d->mAllowFallbackTransforms ) )
  {
    d->initialize();
    addToCache();
  }
  Q_NOWARN_DEPRECATED_POP

  if ( flags & Qgis::CoordinateTransformationFlag::BallparkTransformsAreAppropriate )
    mBallparkTransformsAreAppropriate = true;
}

QgsCoordinateTransform::QgsCoordinateTransform( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &destination, int sourceDatumTransform, int destinationDatumTransform )
{
  d = new QgsCoordinateTransformPrivate( source, destination, sourceDatumTransform, destinationDatumTransform );
#ifdef QGISDEBUG
  mHasContext = true; // not strictly true, but we don't need to worry if datums have been explicitly set
#endif

  if ( !d->checkValidity() )
    return;

  Q_NOWARN_DEPRECATED_PUSH
  if ( !setFromCache( d->mSourceCRS, d->mDestCRS, d->mProjCoordinateOperation, d->mAllowFallbackTransforms ) )
  {
    d->initialize();
    addToCache();
  }
  Q_NOWARN_DEPRECATED_POP
}

QgsCoordinateTransform::QgsCoordinateTransform( const QgsCoordinateTransform &o )
  : mContext( o.mContext )
#ifdef QGISDEBUG
  , mHasContext( o.mHasContext )
#endif
  , mLastError()
    // none of these should be copied -- they must be set manually for every object instead, or
    // we risk contaminating the cache and copies retrieved from cache with settings which should NOT
    // be applied to all transforms
  , mIgnoreImpossible( false )
  , mBallparkTransformsAreAppropriate( false )
  , mDisableFallbackHandler( false )
  , mFallbackOperationOccurred( false )
{
  d = o.d;
}

QgsCoordinateTransform &QgsCoordinateTransform::operator=( const QgsCoordinateTransform &o )  //NOLINT
{
  d = o.d;
#ifdef QGISDEBUG
  mHasContext = o.mHasContext;
#endif
  mContext = o.mContext;
  mLastError = QString();
  return *this;
}

QgsCoordinateTransform::~QgsCoordinateTransform() {} //NOLINT

bool QgsCoordinateTransform::isTransformationPossible( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &destination )
{
  if ( !source.isValid() || !destination.isValid() )
    return false;

#if PROJ_VERSION_MAJOR>8 || (PROJ_VERSION_MAJOR==8 && PROJ_VERSION_MINOR>=1)
  if ( source.celestialBodyName() != destination.celestialBodyName() )
    return false;
#endif

  return true;
}

void QgsCoordinateTransform::setSourceCrs( const QgsCoordinateReferenceSystem &crs )
{
  d.detach();
  d->mSourceCRS = crs;

  if ( mIgnoreImpossible && !isTransformationPossible( d->mSourceCRS, d->mDestCRS ) )
  {
    d->invalidate();
    return;
  }

  if ( !d->checkValidity() )
    return;

  d->calculateTransforms( mContext );
  Q_NOWARN_DEPRECATED_PUSH
  if ( !setFromCache( d->mSourceCRS, d->mDestCRS, d->mProjCoordinateOperation, d->mAllowFallbackTransforms ) )
  {
    d->initialize();
    addToCache();
  }
  Q_NOWARN_DEPRECATED_POP
}
void QgsCoordinateTransform::setDestinationCrs( const QgsCoordinateReferenceSystem &crs )
{
  d.detach();
  d->mDestCRS = crs;

  if ( mIgnoreImpossible && !isTransformationPossible( d->mSourceCRS, d->mDestCRS ) )
  {
    d->invalidate();
    return;
  }

  if ( !d->checkValidity() )
    return;

  d->calculateTransforms( mContext );
  Q_NOWARN_DEPRECATED_PUSH
  if ( !setFromCache( d->mSourceCRS, d->mDestCRS, d->mProjCoordinateOperation, d->mAllowFallbackTransforms ) )
  {
    d->initialize();
    addToCache();
  }
  Q_NOWARN_DEPRECATED_POP
}

void QgsCoordinateTransform::setContext( const QgsCoordinateTransformContext &context )
{
  d.detach();
  mContext = context;
#ifdef QGISDEBUG
  mHasContext = true;
#endif

  if ( mIgnoreImpossible && !isTransformationPossible( d->mSourceCRS, d->mDestCRS ) )
  {
    d->invalidate();
    return;
  }

  if ( !d->checkValidity() )
    return;

  d->calculateTransforms( mContext );
  Q_NOWARN_DEPRECATED_PUSH
  if ( !setFromCache( d->mSourceCRS, d->mDestCRS, d->mProjCoordinateOperation, d->mAllowFallbackTransforms ) )
  {
    d->initialize();
    addToCache();
  }
  Q_NOWARN_DEPRECATED_POP
}

QgsCoordinateTransformContext QgsCoordinateTransform::context() const
{
  return mContext;
}

QgsCoordinateReferenceSystem QgsCoordinateTransform::sourceCrs() const
{
  return d->mSourceCRS;
}

QgsCoordinateReferenceSystem QgsCoordinateTransform::destinationCrs() const
{
  return d->mDestCRS;
}

QgsPointXY QgsCoordinateTransform::transform( const QgsPointXY &point, Qgis::TransformDirection direction ) const
{
  if ( !d->mIsValid || d->mShortCircuit )
    return point;

  // transform x
  double x = point.x();
  double y = point.y();
  double z = 0.0;
  try
  {
    transformCoords( 1, &x, &y, &z, direction );
  }
  catch ( const QgsCsException & )
  {
    // rethrow the exception
    QgsDebugMsgLevel( QStringLiteral( "rethrowing exception" ), 2 );
    throw;
  }

  return QgsPointXY( x, y );
}


QgsPointXY QgsCoordinateTransform::transform( const double theX, const double theY = 0.0, Qgis::TransformDirection direction ) const
{
  try
  {
    return transform( QgsPointXY( theX, theY ), direction );
  }
  catch ( const QgsCsException & )
  {
    // rethrow the exception
    QgsDebugMsgLevel( QStringLiteral( "rethrowing exception" ), 2 );
    throw;
  }
}

QgsRectangle QgsCoordinateTransform::transform( const QgsRectangle &rect, Qgis::TransformDirection direction ) const
{
  if ( !d->mIsValid || d->mShortCircuit )
    return rect;
  // transform x
  double x1 = rect.xMinimum();
  double y1 = rect.yMinimum();
  double x2 = rect.xMaximum();
  double y2 = rect.yMaximum();

  // Number of points to reproject------+
  //                                    |
  //                                    V
  try
  {
    double z = 0.0;
    transformCoords( 1, &x1, &y1, &z, direction );
    transformCoords( 1, &x2, &y2, &z, direction );
  }
  catch ( const QgsCsException & )
  {
    // rethrow the exception
    QgsDebugMsgLevel( QStringLiteral( "rethrowing exception" ), 2 );
    throw;
  }

#ifdef COORDINATE_TRANSFORM_VERBOSE
  QgsDebugMsgLevel( QStringLiteral( "Rect projection..." ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "Xmin : %1 --> %2" ).arg( rect.xMinimum() ).arg( x1 ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "Ymin : %1 --> %2" ).arg( rect.yMinimum() ).arg( y1 ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "Xmax : %1 --> %2" ).arg( rect.xMaximum() ).arg( x2 ), 2 );
  QgsDebugMsgLevel( QStringLiteral( "Ymax : %1 --> %2" ).arg( rect.yMaximum() ).arg( y2 ), 2 );
#endif
  return QgsRectangle( x1, y1, x2, y2 );
}

QgsVector3D QgsCoordinateTransform::transform( const QgsVector3D &point, Qgis::TransformDirection direction ) const
{
  double x = point.x();
  double y = point.y();
  double z = point.z();
  try
  {
    transformCoords( 1, &x, &y, &z, direction );
  }
  catch ( const QgsCsException & )
  {
    // rethrow the exception
    QgsDebugMsgLevel( QStringLiteral( "rethrowing exception" ), 2 );
    throw;
  }
  return QgsVector3D( x, y, z );
}

void QgsCoordinateTransform::transformInPlace( double &x, double &y, double &z,
    Qgis::TransformDirection direction ) const
{
  if ( !d->mIsValid || d->mShortCircuit )
    return;
#ifdef QGISDEBUG
// QgsDebugMsgLevel(QString("Using transform in place %1 %2").arg(__FILE__).arg(__LINE__), 2);
#endif
  // transform x
  try
  {
    transformCoords( 1, &x, &y, &z, direction );
  }
  catch ( const QgsCsException & )
  {
    // rethrow the exception
    QgsDebugMsgLevel( QStringLiteral( "rethrowing exception" ), 2 );
    throw;
  }
}

void QgsCoordinateTransform::transformInPlace( float &x, float &y, double &z,
    Qgis::TransformDirection direction ) const
{
  double xd = static_cast< double >( x ), yd = static_cast< double >( y );
  transformInPlace( xd, yd, z, direction );
  x = xd;
  y = yd;
}

void QgsCoordinateTransform::transformInPlace( float &x, float &y, float &z,
    Qgis::TransformDirection direction ) const
{
  if ( !d->mIsValid || d->mShortCircuit )
    return;
#ifdef QGISDEBUG
  // QgsDebugMsgLevel(QString("Using transform in place %1 %2").arg(__FILE__).arg(__LINE__), 2);
#endif
  // transform x
  try
  {
    double xd = x;
    double yd = y;
    double zd = z;
    transformCoords( 1, &xd, &yd, &zd, direction );
    x = xd;
    y = yd;
    z = zd;
  }
  catch ( QgsCsException & )
  {
    // rethrow the exception
    QgsDebugMsgLevel( QStringLiteral( "rethrowing exception" ), 2 );
    throw;
  }
}

void QgsCoordinateTransform::transformPolygon( QPolygonF &poly, Qgis::TransformDirection direction ) const
{
  if ( !d->mIsValid || d->mShortCircuit )
  {
    return;
  }

  //create x, y arrays
  const int nVertices = poly.size();

  QVector<double> x( nVertices );
  QVector<double> y( nVertices );
  QVector<double> z( nVertices );
  double *destX = x.data();
  double *destY = y.data();
  double *destZ = z.data();

  const QPointF *polyData = poly.constData();
  for ( int i = 0; i < nVertices; ++i )
  {
    *destX++ = polyData->x();
    *destY++ = polyData->y();
    *destZ++ = 0;
    polyData++;
  }

  QString err;
  try
  {
    transformCoords( nVertices, x.data(), y.data(), z.data(), direction );
  }
  catch ( const QgsCsException &e )
  {
    // record the exception, but don't rethrow it until we've recorded the coordinates we *could* transform
    err = e.what();
  }

  QPointF *destPoint = poly.data();
  const double *srcX = x.constData();
  const double *srcY = y.constData();
  for ( int i = 0; i < nVertices; ++i )
  {
    destPoint->rx() = *srcX++;
    destPoint->ry() = *srcY++;
    destPoint++;
  }

  // rethrow the exception
  if ( !err.isEmpty() )
    throw QgsCsException( err );
}

void QgsCoordinateTransform::transformInPlace( QVector<double> &x, QVector<double> &y, QVector<double> &z,
    Qgis::TransformDirection direction ) const
{

  if ( !d->mIsValid || d->mShortCircuit )
    return;

  Q_ASSERT( x.size() == y.size() );

  // Apparently, if one has a std::vector, it is valid to use the
  // address of the first element in the vector as a pointer to an
  // array of the vectors data, and hence easily interface with code
  // that wants C-style arrays.

  try
  {
    transformCoords( x.size(), &x[0], &y[0], &z[0], direction );
  }
  catch ( const QgsCsException & )
  {
    // rethrow the exception
    QgsDebugMsgLevel( QStringLiteral( "rethrowing exception" ), 2 );
    throw;
  }
}


void QgsCoordinateTransform::transformInPlace( QVector<float> &x, QVector<float> &y, QVector<float> &z,
    Qgis::TransformDirection direction ) const
{
  if ( !d->mIsValid || d->mShortCircuit )
    return;

  Q_ASSERT( x.size() == y.size() );

  // Apparently, if one has a std::vector, it is valid to use the
  // address of the first element in the vector as a pointer to an
  // array of the vectors data, and hence easily interface with code
  // that wants C-style arrays.

  try
  {
    //copy everything to double vectors since proj needs double
    const int vectorSize = x.size();
    QVector<double> xd( x.size() );
    QVector<double> yd( y.size() );
    QVector<double> zd( z.size() );

    double *destX = xd.data();
    double *destY = yd.data();
    double *destZ = zd.data();

    const float *srcX = x.constData();
    const float *srcY = y.constData();
    const float *srcZ = z.constData();

    for ( int i = 0; i < vectorSize; ++i )
    {
      *destX++ = static_cast< double >( *srcX++ );
      *destY++ = static_cast< double >( *srcY++ );
      *destZ++ = static_cast< double >( *srcZ++ );
    }

    transformCoords( x.size(), &xd[0], &yd[0], &zd[0], direction );

    //copy back
    float *destFX = x.data();
    float *destFY = y.data();
    float *destFZ = z.data();
    const double *srcXD = xd.constData();
    const double *srcYD = yd.constData();
    const double *srcZD = zd.constData();
    for ( int i = 0; i < vectorSize; ++i )
    {
      *destFX++ = static_cast< float >( *srcXD++ );
      *destFY++ = static_cast< float >( *srcYD++ );
      *destFZ++ = static_cast< float >( *srcZD++ );
    }
  }
  catch ( QgsCsException & )
  {
    // rethrow the exception
    QgsDebugMsgLevel( QStringLiteral( "rethrowing exception" ), 2 );
    throw;
  }
}

QgsRectangle QgsCoordinateTransform::transformBoundingBox( const QgsRectangle &rect, Qgis::TransformDirection direction, const bool handle180Crossover ) const
{
  // Calculate the bounding box of a QgsRectangle in the source CRS
  // when projected to the destination CRS (or the inverse).
  // This is done by looking at a number of points spread evenly
  // across the rectangle

  if ( !d->mIsValid || d->mShortCircuit )
    return rect;

  if ( rect.isEmpty() )
  {
    const QgsPointXY p = transform( rect.xMinimum(), rect.yMinimum(), direction );
    return QgsRectangle( p, p );
  }

  double yMin = rect.yMinimum();
  double yMax = rect.yMaximum();
  if ( d->mGeographicToWebMercator &&
       ( ( direction == Qgis::TransformDirection::Forward && !d->mIsReversed ) ||
         ( direction == Qgis::TransformDirection::Reverse && d->mIsReversed ) ) )
  {
    // Latitudes close to 90 degree project to infinite northing in theory.
    // We limit to 90 - 1e-1 which reproject to northing of ~ 44e6 m (about twice
    // the maximum easting of ~20e6 m).
    // For reference, GoogleMercator tiles are limited to a northing ~85 deg / ~20e6 m
    // so limiting to 90 - 1e-1 is reasonable.
    constexpr double EPS = 1e-1;
    if ( yMin < -90 + EPS )
    {
      if ( yMax < -90 + EPS )
        throw QgsCsException( QObject::tr( "Could not transform bounding box to target CRS" ) );
      yMin = -90 + EPS;
    }
    if ( yMax > 90 - EPS )
    {
      if ( yMin > 90 - EPS )
        throw QgsCsException( QObject::tr( "Could not transform bounding box to target CRS" ) );
      yMax = 90 - EPS;
    }
  }

  // 64 points (<=2.12) is not enough, see #13665, for EPSG:4326 -> EPSG:3574 (say that it is a hard one),
  // are decent result from about 500 points and more. This method is called quite often, but
  // even with 1000 points it takes < 1ms.
  // TODO: how to effectively and precisely reproject bounding box?
  const int nPoints = 1000;
  const double dst = std::sqrt( ( rect.width() * ( yMax - yMin ) ) / std::pow( std::sqrt( static_cast< double >( nPoints ) ) - 1, 2.0 ) );
  const int nXPoints = std::min( static_cast< int >( std::ceil( rect.width() / dst ) ) + 1, 1000 );
  const int nYPoints = std::min( static_cast< int >( std::ceil( ( yMax - yMin ) / dst ) ) + 1, 1000 );

  QgsRectangle bb_rect;
  bb_rect.setNull();

  // We're interfacing with C-style vectors in the
  // end, so let's do C-style vectors here too.
  QVector<double> x( nXPoints * nYPoints );
  QVector<double> y( nXPoints * nYPoints );
  QVector<double> z( nXPoints * nYPoints );

  QgsDebugMsgLevel( QStringLiteral( "Entering transformBoundingBox..." ), 4 );

  // Populate the vectors

  const double dx = rect.width()  / static_cast< double >( nXPoints - 1 );
  const double dy = ( yMax - yMin ) / static_cast< double >( nYPoints - 1 );

  double pointY = yMin;

  for ( int i = 0; i < nYPoints ; i++ )
  {

    // Start at right edge
    double pointX = rect.xMinimum();

    for ( int j = 0; j < nXPoints; j++ )
    {
      x[( i * nXPoints ) + j] = pointX;
      y[( i * nXPoints ) + j] = pointY;
      // and the height...
      z[( i * nXPoints ) + j] = 0.0;
      // QgsDebugMsgLevel(QString("BBox coord: (%1, %2)").arg(x[(i*numP) + j]).arg(y[(i*numP) + j]), 2);
      pointX += dx;
    }
    pointY += dy;
  }

  // Do transformation. Any exception generated must
  // be handled in above layers.
  try
  {
    transformCoords( nXPoints * nYPoints, x.data(), y.data(), z.data(), direction );
  }
  catch ( const QgsCsException & )
  {
    // rethrow the exception
    QgsDebugMsgLevel( QStringLiteral( "rethrowing exception" ), 2 );
    throw;
  }

  // check if result bbox is geographic and is crossing 180/-180 line: ie. min X is before the 180° and max X is after the -180°
  bool doHandle180Crossover = false;
  if ( nXPoints > 0 )
  {
    const double xMin = std::fmod( x[0], 180.0 );
    const double xMax = std::fmod( x[nXPoints - 1], 180.0 );
    if ( handle180Crossover
         && ( ( direction == Qgis::TransformDirection::Forward && d->mDestCRS.isGeographic() ) ||
              ( direction == Qgis::TransformDirection::Reverse && d->mSourceCRS.isGeographic() ) )
         && xMin > 0.0 && xMin <= 180.0 && xMax < 0.0 && xMax >= -180.0 )
    {
      doHandle180Crossover = true;
    }
  }

  // Calculate the bounding box and use that for the extent
  for ( int i = 0; i < nXPoints * nYPoints; i++ )
  {
    if ( !std::isfinite( x[i] ) || !std::isfinite( y[i] ) )
    {
      continue;
    }

    if ( doHandle180Crossover )
    {
      //if crossing the date line, temporarily add 360 degrees to -ve longitudes
      bb_rect.combineExtentWith( x[i] >= 0.0 ? x[i] : x[i] + 360.0, y[i] );
    }
    else
    {
      bb_rect.combineExtentWith( x[i], y[i] );
    }
  }

  if ( bb_rect.isNull() )
  {
    // something bad happened when reprojecting the filter rect... no finite points were left!
    throw QgsCsException( QObject::tr( "Could not transform bounding box to target CRS" ) );
  }

  if ( doHandle180Crossover )
  {
    //subtract temporary addition of 360 degrees from longitudes
    if ( bb_rect.xMinimum() > 180.0 )
      bb_rect.setXMinimum( bb_rect.xMinimum() - 360.0 );
    if ( bb_rect.xMaximum() > 180.0 )
      bb_rect.setXMaximum( bb_rect.xMaximum() - 360.0 );
  }

  QgsDebugMsgLevel( "Projected extent: " + bb_rect.toString(), 4 );

  if ( bb_rect.isEmpty() )
  {
    QgsDebugMsgLevel( "Original extent: " + rect.toString(), 4 );
  }

  return bb_rect;
}

void QgsCoordinateTransform::transformCoords( int numPoints, double *x, double *y, double *z, Qgis::TransformDirection direction ) const
{
  if ( !d->mIsValid || d->mShortCircuit )
    return;
  // Refuse to transform the points if the srs's are invalid
  if ( !d->mSourceCRS.isValid() )
  {
    QgsMessageLog::logMessage( QObject::tr( "The source spatial reference system (CRS) is not valid. "
                                            "The coordinates can not be reprojected. The CRS is: %1" )
                               .arg( d->mSourceCRS.toProj() ), QObject::tr( "CRS" ) );
    return;
  }
  if ( !d->mDestCRS.isValid() )
  {
    QgsMessageLog::logMessage( QObject::tr( "The destination spatial reference system (CRS) is not valid. "
                                            "The coordinates can not be reprojected. The CRS is: %1" ).arg( d->mDestCRS.toProj() ), QObject::tr( "CRS" ) );
    return;
  }

  std::vector< int > zNanPositions;
  for ( int i = 0; i < numPoints; i++ )
  {
    if ( std::isnan( z[i] ) )
    {
      zNanPositions.push_back( i );
      z[i] = 0.0;
    }
  }

  std::vector< double > xprev( numPoints );
  memcpy( xprev.data(), x, sizeof( double ) * numPoints );
  std::vector< double > yprev( numPoints );
  memcpy( yprev.data(), y, sizeof( double ) * numPoints );
  std::vector< double > zprev( numPoints );
  memcpy( zprev.data(), z, sizeof( double ) * numPoints );

  const bool useTime = !std::isnan( d->mDefaultTime );
  std::vector< double > t( useTime ? numPoints : 0, d->mDefaultTime );

#ifdef COORDINATE_TRANSFORM_VERBOSE
  double xorg = *x;
  double yorg = *y;
  QgsDebugMsgLevel( QStringLiteral( "[[[[[[ Number of points to transform: %1 ]]]]]]" ).arg( numPoints ), 2 );
#endif

#ifdef QGISDEBUG
  if ( !mHasContext )
    QgsDebugMsgLevel( QStringLiteral( "No QgsCoordinateTransformContext context set for transform" ), 4 );
#endif

  // use proj4 to do the transform

  // if the source/destination projection is lat/long, convert the points to radians
  // prior to transforming
  ProjData projData = d->threadLocalProjData();

  int projResult = 0;

  proj_errno_reset( projData );
  proj_trans_generic( projData, ( direction == Qgis::TransformDirection::Forward && !d->mIsReversed ) || ( direction == Qgis::TransformDirection::Reverse && d->mIsReversed ) ? PJ_FWD : PJ_INV,
                      x, sizeof( double ), numPoints,
                      y, sizeof( double ), numPoints,
                      z, sizeof( double ), numPoints,
                      useTime ? t.data() : nullptr, sizeof( double ), useTime ? numPoints : 0 );
  // Try to - approximately - emulate the behavior of pj_transform()...
  // In the case of a single point transform, and a transformation error occurs,
  // pj_transform() would return the errno. In cases of multiple point transform,
  // it would continue (for non-transient errors, that is pipeline definition
  // errors) and just set the resulting x,y to infinity. This is in fact a
  // bit more subtle than that, and I'm not completely sure the logic in
  // pj_transform() was really sane & fully bullet proof
  // So here just check proj_errno() for single point transform
  int actualRes = 0;
  if ( numPoints == 1 )
  {
    projResult = proj_errno( projData );
    actualRes = projResult;
  }
  else
  {
    actualRes =  proj_errno( projData );
  }
  if ( actualRes == 0 )
  {
    // proj_errno is sometimes not an accurate method to test for transform failures - so we need to
    // manually scan for nan values
    if ( std::any_of( x, x + numPoints, []( double v ) { return std::isinf( v ); } )
    || std::any_of( y, y + numPoints, []( double v ) { return std::isinf( v ); } )
    || std::any_of( z, z + numPoints, []( double v ) { return std::isinf( v ); } ) )
    {
      actualRes = 1;
    }
  }

  mFallbackOperationOccurred = false;
  if ( actualRes != 0
       && ( d->mAvailableOpCount > 1 || d->mAvailableOpCount == -1 ) // only use fallbacks if more than one operation is possible -- otherwise we've already tried it and it failed
       && ( d->mAllowFallbackTransforms || mBallparkTransformsAreAppropriate ) )
  {
    // fail #1 -- try with getting proj to auto-pick an appropriate coordinate operation for the points
    if ( PJ *transform = d->threadLocalFallbackProjData() )
    {
      projResult = 0;
      proj_errno_reset( transform );
      proj_trans_generic( transform, direction == Qgis::TransformDirection::Forward ? PJ_FWD : PJ_INV,
                          xprev.data(), sizeof( double ), numPoints,
                          yprev.data(), sizeof( double ), numPoints,
                          zprev.data(), sizeof( double ), numPoints,
                          useTime ? t.data() : nullptr, sizeof( double ), useTime ? numPoints : 0 );
      // Try to - approximately - emulate the behavior of pj_transform()...
      // In the case of a single point transform, and a transformation error occurs,
      // pj_transform() would return the errno. In cases of multiple point transform,
      // it would continue (for non-transient errors, that is pipeline definition
      // errors) and just set the resulting x,y to infinity. This is in fact a
      // bit more subtle than that, and I'm not completely sure the logic in
      // pj_transform() was really sane & fully bullet proof
      // So here just check proj_errno() for single point transform
      if ( numPoints == 1 )
      {
        // hmm - something very odd here. We can't trust proj_errno( transform ), as that's giving us incorrect error numbers
        // (such as "failed to load datum shift file", which is definitely incorrect for a default proj created operation!)
        // so we resort to testing values ourselves...
        projResult = std::isinf( xprev[0] ) || std::isinf( yprev[0] ) || std::isinf( zprev[0] ) ? 1 : 0;
      }

      if ( projResult == 0 )
      {
        memcpy( x, xprev.data(), sizeof( double ) * numPoints );
        memcpy( y, yprev.data(), sizeof( double ) * numPoints );
        memcpy( z, zprev.data(), sizeof( double ) * numPoints );
        mFallbackOperationOccurred = true;
      }

      if ( !mBallparkTransformsAreAppropriate && !mDisableFallbackHandler && sFallbackOperationOccurredHandler )
      {
        sFallbackOperationOccurredHandler( d->mSourceCRS, d->mDestCRS, d->mProjCoordinateOperation );
#if 0
        const QString warning = QStringLiteral( "A fallback coordinate operation was used between %1 and %2" ).arg( d->mSourceCRS.authid(),
                                d->mDestCRS.authid() );
        qWarning( "%s", warning.toLatin1().constData() );
#endif
      }
    }
  }

  for ( const int &pos : zNanPositions )
  {
    z[pos] = std::numeric_limits<double>::quiet_NaN();
  }

  if ( projResult != 0 )
  {
    //something bad happened....
    QString points;

    for ( int i = 0; i < numPoints; ++i )
    {
      points += QStringLiteral( "(%1, %2)\n" ).arg( x[i], 0, 'f' ).arg( y[i], 0, 'f' );
    }

    const QString dir = ( direction == Qgis::TransformDirection::Forward ) ? QObject::tr( "forward transform" ) : QObject::tr( "inverse transform" );

    const QString msg = QObject::tr( "%1 of\n"
                                     "%2"
                                     "Error: %3" )
                        .arg( dir,
                              points,
                              projResult < 0 ? QString::fromUtf8( proj_errno_string( projResult ) ) : QObject::tr( "Fallback transform failed" ) );


    // don't flood console with thousands of duplicate transform error messages
    if ( msg != mLastError )
    {
      QgsDebugError( "Projection failed emitting invalid transform signal: " + msg );
      mLastError = msg;
    }
    QgsDebugMsgLevel( QStringLiteral( "rethrowing exception" ), 2 );

    throw QgsCsException( msg );
  }

#ifdef COORDINATE_TRANSFORM_VERBOSE
  QgsDebugMsgLevel( QStringLiteral( "[[[[[[ Projected %1, %2 to %3, %4 ]]]]]]" )
                    .arg( xorg, 0, 'g', 15 ).arg( yorg, 0, 'g', 15 )
                    .arg( *x, 0, 'g', 15 ).arg( *y, 0, 'g', 15 ), 2 );
#endif
}

bool QgsCoordinateTransform::isValid() const
{
  return d->mIsValid;
}

bool QgsCoordinateTransform::isShortCircuited() const
{
  return !d->mIsValid || d->mShortCircuit;
}

QString QgsCoordinateTransform::coordinateOperation() const
{
  return d->mProjCoordinateOperation;
}

QgsDatumTransform::TransformDetails QgsCoordinateTransform::instantiatedCoordinateOperationDetails() const
{
  ProjData projData = d->threadLocalProjData();
  return QgsDatumTransform::transformDetailsFromPj( projData );
}

void QgsCoordinateTransform::setCoordinateOperation( const QString &operation ) const
{
  d.detach();
  d->mProjCoordinateOperation = operation;
  d->mShouldReverseCoordinateOperation = false;
}

void QgsCoordinateTransform::setAllowFallbackTransforms( bool allowed )
{
  d.detach();
  d->mAllowFallbackTransforms = allowed;
}

bool QgsCoordinateTransform::allowFallbackTransforms() const
{
  return d->mAllowFallbackTransforms;
}

void QgsCoordinateTransform::setBallparkTransformsAreAppropriate( bool appropriate )
{
  mBallparkTransformsAreAppropriate = appropriate;
}

void QgsCoordinateTransform::disableFallbackOperationHandler( bool disabled )
{
  mDisableFallbackHandler = disabled;
}

bool QgsCoordinateTransform::fallbackOperationOccurred() const
{
  return mFallbackOperationOccurred;
}

const char *finder( const char *name )
{
  QString proj;
#ifdef Q_OS_WIN
  proj = QApplication::applicationDirPath()
         + "/share/proj/" + QString( name );
#else
  Q_UNUSED( name )
#endif
  return proj.toUtf8();
}

bool QgsCoordinateTransform::setFromCache( const QgsCoordinateReferenceSystem &src, const QgsCoordinateReferenceSystem &dest, const QString &coordinateOperationProj, bool allowFallback )
{
  if ( !src.isValid() || !dest.isValid() )
    return false;

  const QString sourceKey = src.authid().isEmpty() ?
                            src.toWkt( Qgis::CrsWktVariant::Preferred ) : src.authid();
  const QString destKey = dest.authid().isEmpty() ?
                          dest.toWkt( Qgis::CrsWktVariant::Preferred ) : dest.authid();

  if ( sourceKey.isEmpty() || destKey.isEmpty() )
    return false;

  QgsReadWriteLocker locker( sCacheLock, QgsReadWriteLocker::Read );
  if ( sDisableCache )
    return false;

  const QList< QgsCoordinateTransform > values = sTransforms.values( qMakePair( sourceKey, destKey ) );
  for ( auto valIt = values.constBegin(); valIt != values.constEnd(); ++valIt )
  {
    if ( ( *valIt ).coordinateOperation() == coordinateOperationProj
         && ( *valIt ).allowFallbackTransforms() == allowFallback
         && qgsNanCompatibleEquals( src.coordinateEpoch(), ( *valIt ).sourceCrs().coordinateEpoch() )
         && qgsNanCompatibleEquals( dest.coordinateEpoch(), ( *valIt ).destinationCrs().coordinateEpoch() )
       )
    {
      // need to save, and then restore the context... we don't want this to be cached or to use the values from the cache
      const QgsCoordinateTransformContext context = mContext;
#ifdef QGISDEBUG
      const bool hasContext = mHasContext;
#endif
      *this = *valIt;
      locker.unlock();

      mContext = context;
#ifdef QGISDEBUG
      mHasContext = hasContext;
#endif

      return true;
    }
  }
  return false;
}

void QgsCoordinateTransform::addToCache()
{
  if ( !d->mSourceCRS.isValid() || !d->mDestCRS.isValid() )
    return;

  const QString sourceKey = d->mSourceCRS.authid().isEmpty() ?
                            d->mSourceCRS.toWkt( Qgis::CrsWktVariant::Preferred ) : d->mSourceCRS.authid();
  const QString destKey = d->mDestCRS.authid().isEmpty() ?
                          d->mDestCRS.toWkt( Qgis::CrsWktVariant::Preferred ) : d->mDestCRS.authid();

  if ( sourceKey.isEmpty() || destKey.isEmpty() )
    return;

  const QgsReadWriteLocker locker( sCacheLock, QgsReadWriteLocker::Write );
  if ( sDisableCache )
    return;

  sTransforms.insert( qMakePair( sourceKey, destKey ), *this );
}

int QgsCoordinateTransform::sourceDatumTransformId() const
{
  Q_NOWARN_DEPRECATED_PUSH
  return d->mSourceDatumTransform;
  Q_NOWARN_DEPRECATED_POP
}

void QgsCoordinateTransform::setSourceDatumTransformId( int dt )
{
  d.detach();
  Q_NOWARN_DEPRECATED_PUSH
  d->mSourceDatumTransform = dt;
  Q_NOWARN_DEPRECATED_POP
}

int QgsCoordinateTransform::destinationDatumTransformId() const
{
  Q_NOWARN_DEPRECATED_PUSH
  return d->mDestinationDatumTransform;
  Q_NOWARN_DEPRECATED_POP
}

void QgsCoordinateTransform::setDestinationDatumTransformId( int dt )
{
  d.detach();
  Q_NOWARN_DEPRECATED_PUSH
  d->mDestinationDatumTransform = dt;
  Q_NOWARN_DEPRECATED_POP
}

void QgsCoordinateTransform::invalidateCache( bool disableCache )
{
  const QgsReadWriteLocker locker( sCacheLock, QgsReadWriteLocker::Write );
  if ( sDisableCache )
    return;

  if ( disableCache )
  {
    sDisableCache = true;
  }

  sTransforms.clear();
}

void QgsCoordinateTransform::removeFromCacheObjectsBelongingToCurrentThread( void *pj_context )
{
  // Not completely sure about object order destruction after main() has
  // exited. So it is safer to check sDisableCache before using sCacheLock
  // in case sCacheLock would have been destroyed before the current TLS
  // QgsProjContext object that has called us...
  if ( sDisableCache )
    return;

  const QgsReadWriteLocker locker( sCacheLock, QgsReadWriteLocker::Write );
  // cppcheck-suppress identicalConditionAfterEarlyExit
  if ( sDisableCache )
    return;

  for ( auto it = sTransforms.begin(); it != sTransforms.end(); )
  {
    auto &v = it.value();
    if ( v.d->removeObjectsBelongingToCurrentThread( pj_context ) )
      it = sTransforms.erase( it );
    else
      ++it;
  }
}

double QgsCoordinateTransform::scaleFactor( const QgsRectangle &ReferenceExtent ) const
{
  const QgsPointXY source1( ReferenceExtent.xMinimum(), ReferenceExtent.yMinimum() );
  const QgsPointXY source2( ReferenceExtent.xMaximum(), ReferenceExtent.yMaximum() );
  const double distSourceUnits = std::sqrt( source1.sqrDist( source2 ) );
  const QgsPointXY dest1 = transform( source1 );
  const QgsPointXY dest2 = transform( source2 );
  const double distDestUnits = std::sqrt( dest1.sqrDist( dest2 ) );
  return distDestUnits / distSourceUnits;
}

void QgsCoordinateTransform::setCustomMissingRequiredGridHandler( const std::function<void ( const QgsCoordinateReferenceSystem &, const QgsCoordinateReferenceSystem &, const QgsDatumTransform::GridDetails & )> &handler )
{
  QgsCoordinateTransformPrivate::setCustomMissingRequiredGridHandler( handler );
}

void QgsCoordinateTransform::setCustomMissingPreferredGridHandler( const std::function<void ( const QgsCoordinateReferenceSystem &, const QgsCoordinateReferenceSystem &, const QgsDatumTransform::TransformDetails &, const QgsDatumTransform::TransformDetails & )> &handler )
{
  QgsCoordinateTransformPrivate::setCustomMissingPreferredGridHandler( handler );
}

void QgsCoordinateTransform::setCustomCoordinateOperationCreationErrorHandler( const std::function<void ( const QgsCoordinateReferenceSystem &, const QgsCoordinateReferenceSystem &, const QString & )> &handler )
{
  QgsCoordinateTransformPrivate::setCustomCoordinateOperationCreationErrorHandler( handler );
}

void QgsCoordinateTransform::setCustomMissingGridUsedByContextHandler( const std::function<void ( const QgsCoordinateReferenceSystem &, const QgsCoordinateReferenceSystem &, const QgsDatumTransform::TransformDetails & )> &handler )
{
  QgsCoordinateTransformPrivate::setCustomMissingGridUsedByContextHandler( handler );
}

void QgsCoordinateTransform::setFallbackOperationOccurredHandler( const std::function<void ( const QgsCoordinateReferenceSystem &, const QgsCoordinateReferenceSystem &, const QString & )> &handler )
{
  sFallbackOperationOccurredHandler = handler;
}

void QgsCoordinateTransform::setDynamicCrsToDynamicCrsWarningHandler( const std::function<void ( const QgsCoordinateReferenceSystem &, const QgsCoordinateReferenceSystem & )> &handler )
{
  QgsCoordinateTransformPrivate::setDynamicCrsToDynamicCrsWarningHandler( handler );
}
