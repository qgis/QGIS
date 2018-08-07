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

//qt includes
#include <QDomNode>
#include <QDomElement>
#include <QApplication>
#include <QPolygonF>
#include <QStringList>
#include <QVector>

extern "C"
{
#include <proj_api.h>
}
#include <sqlite3.h>

// if defined shows all information about transform to stdout
// #define COORDINATE_TRANSFORM_VERBOSE

QReadWriteLock QgsCoordinateTransform::sCacheLock;
QMultiHash< QPair< QString, QString >, QgsCoordinateTransform > QgsCoordinateTransform::sTransforms; //same auth_id pairs might have different datum transformations

QgsCoordinateTransform::QgsCoordinateTransform()
{
  d = new QgsCoordinateTransformPrivate();
}

QgsCoordinateTransform::QgsCoordinateTransform( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &destination )
{
  d = new QgsCoordinateTransformPrivate( source, destination, QgsCoordinateTransformContext() );

  if ( !d->checkValidity() )
    return;

  if ( !setFromCache( d->mSourceCRS, d->mDestCRS, d->mSourceDatumTransform, d->mDestinationDatumTransform ) )
  {
    d->initialize();
    addToCache();
  }
}

QgsCoordinateTransform::QgsCoordinateTransform( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &destination, const QgsCoordinateTransformContext &context )
{
  mContext = context;
  d = new QgsCoordinateTransformPrivate( source, destination, mContext );
#ifdef QGISDEBUG
  mHasContext = true;
#endif

  if ( !d->checkValidity() )
    return;

  if ( !setFromCache( d->mSourceCRS, d->mDestCRS, d->mSourceDatumTransform, d->mDestinationDatumTransform ) )
  {
    d->initialize();
    addToCache();
  }
}

QgsCoordinateTransform::QgsCoordinateTransform( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &destination, const QgsProject *project )
{
  mContext = project ? project->transformContext() : QgsCoordinateTransformContext();
  d = new QgsCoordinateTransformPrivate( source, destination, mContext );
#ifdef QGISDEBUG
  if ( project )
    mHasContext = true;
#endif

  if ( !d->checkValidity() )
    return;

  if ( !setFromCache( d->mSourceCRS, d->mDestCRS, d->mSourceDatumTransform, d->mDestinationDatumTransform ) )
  {
    d->initialize();
    addToCache();
  }
}

QgsCoordinateTransform::QgsCoordinateTransform( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &destination, int sourceDatumTransform, int destinationDatumTransform )
{
  d = new QgsCoordinateTransformPrivate( source, destination, sourceDatumTransform, destinationDatumTransform );
#ifdef QGISDEBUG
  mHasContext = true; // not strictly true, but we don't need to worry if datums have been explicitly set
#endif

  if ( !d->checkValidity() )
    return;

  if ( !setFromCache( d->mSourceCRS, d->mDestCRS, d->mSourceDatumTransform, d->mDestinationDatumTransform ) )
  {
    d->initialize();
    addToCache();
  }
}

QgsCoordinateTransform::QgsCoordinateTransform( const QgsCoordinateTransform &o )
  : mContext( o.mContext )
#ifdef QGISDEBUG
  , mHasContext( o.mHasContext )
#endif
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
  return *this;
}

QgsCoordinateTransform::~QgsCoordinateTransform() {} //NOLINT

void QgsCoordinateTransform::setSourceCrs( const QgsCoordinateReferenceSystem &crs )
{
  d.detach();
  d->mSourceCRS = crs;
  if ( !d->checkValidity() )
    return;

  d->calculateTransforms( mContext );
  if ( !setFromCache( d->mSourceCRS, d->mDestCRS, d->mSourceDatumTransform, d->mDestinationDatumTransform ) )
  {
    d->initialize();
    addToCache();
  }
}
void QgsCoordinateTransform::setDestinationCrs( const QgsCoordinateReferenceSystem &crs )
{
  d.detach();
  d->mDestCRS = crs;
  if ( !d->checkValidity() )
    return;

  d->calculateTransforms( mContext );
  if ( !setFromCache( d->mSourceCRS, d->mDestCRS, d->mSourceDatumTransform, d->mDestinationDatumTransform ) )
  {
    d->initialize();
    addToCache();
  }
}

void QgsCoordinateTransform::setContext( const QgsCoordinateTransformContext &context )
{
  d.detach();
  mContext = context;
#ifdef QGISDEBUG
  mHasContext = true;
#endif
  if ( !d->checkValidity() )
    return;

  d->calculateTransforms( mContext );
  if ( !setFromCache( d->mSourceCRS, d->mDestCRS, d->mSourceDatumTransform, d->mDestinationDatumTransform ) )
  {
    d->initialize();
    addToCache();
  }
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

QgsPointXY QgsCoordinateTransform::transform( const QgsPointXY &point, TransformDirection direction ) const
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
    QgsDebugMsg( "rethrowing exception" );
    throw;
  }

  return QgsPointXY( x, y );
}


QgsPointXY QgsCoordinateTransform::transform( const double theX, const double theY = 0.0, TransformDirection direction ) const
{
  try
  {
    return transform( QgsPointXY( theX, theY ), direction );
  }
  catch ( const QgsCsException & )
  {
    // rethrow the exception
    QgsDebugMsg( "rethrowing exception" );
    throw;
  }
}

QgsRectangle QgsCoordinateTransform::transform( const QgsRectangle &rect, TransformDirection direction ) const
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
    QgsDebugMsg( "rethrowing exception" );
    throw;
  }

#ifdef COORDINATE_TRANSFORM_VERBOSE
  QgsDebugMsg( "Rect projection..." );
  QgsDebugMsg( QString( "Xmin : %1 --> %2" ).arg( rect.xMinimum() ).arg( x1 ) );
  QgsDebugMsg( QString( "Ymin : %1 --> %2" ).arg( rect.yMinimum() ).arg( y1 ) );
  QgsDebugMsg( QString( "Xmax : %1 --> %2" ).arg( rect.xMaximum() ).arg( x2 ) );
  QgsDebugMsg( QString( "Ymax : %1 --> %2" ).arg( rect.yMaximum() ).arg( y2 ) );
#endif
  return QgsRectangle( x1, y1, x2, y2 );
}

void QgsCoordinateTransform::transformInPlace( double &x, double &y, double &z,
    TransformDirection direction ) const
{
  if ( !d->mIsValid || d->mShortCircuit )
    return;
#ifdef QGISDEBUG
// QgsDebugMsg(QString("Using transform in place %1 %2").arg(__FILE__).arg(__LINE__));
#endif
  // transform x
  try
  {
    transformCoords( 1, &x, &y, &z, direction );
  }
  catch ( const QgsCsException & )
  {
    // rethrow the exception
    QgsDebugMsg( "rethrowing exception" );
    throw;
  }
}

void QgsCoordinateTransform::transformInPlace( float &x, float &y, double &z,
    TransformDirection direction ) const
{
  double xd = static_cast< double >( x ), yd = static_cast< double >( y );
  transformInPlace( xd, yd, z, direction );
  x = xd;
  y = yd;
}

void QgsCoordinateTransform::transformInPlace( float &x, float &y, float &z,
    TransformDirection direction ) const
{
  if ( !d->mIsValid || d->mShortCircuit )
    return;
#ifdef QGISDEBUG
  // QgsDebugMsg(QString("Using transform in place %1 %2").arg(__FILE__).arg(__LINE__));
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
    QgsDebugMsg( "rethrowing exception" );
    throw;
  }
}

void QgsCoordinateTransform::transformPolygon( QPolygonF &poly, TransformDirection direction ) const
{
  if ( !d->mIsValid || d->mShortCircuit )
  {
    return;
  }

  //create x, y arrays
  int nVertices = poly.size();

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

  try
  {
    transformCoords( nVertices, x.data(), y.data(), z.data(), direction );
  }
  catch ( const QgsCsException & )
  {
    // rethrow the exception
    QgsDebugMsg( "rethrowing exception" );
    throw;
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
}

void QgsCoordinateTransform::transformInPlace(
  QVector<double> &x, QVector<double> &y, QVector<double> &z,
  TransformDirection direction ) const
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
    QgsDebugMsg( "rethrowing exception" );
    throw;
  }
}


void QgsCoordinateTransform::transformInPlace(
  QVector<float> &x, QVector<float> &y, QVector<float> &z,
  TransformDirection direction ) const
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
    int vectorSize = x.size();
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
    QgsDebugMsg( "rethrowing exception" );
    throw;
  }
}

QgsRectangle QgsCoordinateTransform::transformBoundingBox( const QgsRectangle &rect, TransformDirection direction, const bool handle180Crossover ) const
{
  // Calculate the bounding box of a QgsRectangle in the source CRS
  // when projected to the destination CRS (or the inverse).
  // This is done by looking at a number of points spread evenly
  // across the rectangle

  if ( !d->mIsValid || d->mShortCircuit )
    return rect;

  if ( rect.isEmpty() )
  {
    QgsPointXY p = transform( rect.xMinimum(), rect.yMinimum(), direction );
    return QgsRectangle( p, p );
  }

  // 64 points (<=2.12) is not enough, see #13665, for EPSG:4326 -> EPSG:3574 (say that it is a hard one),
  // are decent result from about 500 points and more. This method is called quite often, but
  // even with 1000 points it takes < 1ms
  // TODO: how to effectively and precisely reproject bounding box?
  const int nPoints = 1000;
  double d = std::sqrt( ( rect.width() * rect.height() ) / std::pow( std::sqrt( static_cast< double >( nPoints ) ) - 1, 2.0 ) );
  int nXPoints = static_cast< int >( std::ceil( rect.width() / d ) ) + 1;
  int nYPoints = static_cast< int >( std::ceil( rect.height() / d ) ) + 1;

  QgsRectangle bb_rect;
  bb_rect.setMinimal();

  // We're interfacing with C-style vectors in the
  // end, so let's do C-style vectors here too.

  QVector<double> x( nXPoints * nYPoints );
  QVector<double> y( nXPoints * nYPoints );
  QVector<double> z( nXPoints * nYPoints );

  QgsDebugMsgLevel( "Entering transformBoundingBox...", 4 );

  // Populate the vectors

  double dx = rect.width()  / static_cast< double >( nXPoints - 1 );
  double dy = rect.height() / static_cast< double >( nYPoints - 1 );

  double pointY = rect.yMinimum();

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
      // QgsDebugMsg(QString("BBox coord: (%1, %2)").arg(x[(i*numP) + j]).arg(y[(i*numP) + j]));
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
    QgsDebugMsg( "rethrowing exception" );
    throw;
  }

  // Calculate the bounding box and use that for the extent

  for ( int i = 0; i < nXPoints * nYPoints; i++ )
  {
    if ( !std::isfinite( x[i] ) || !std::isfinite( y[i] ) )
    {
      continue;
    }

    if ( handle180Crossover )
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

  if ( handle180Crossover )
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

void QgsCoordinateTransform::transformCoords( int numPoints, double *x, double *y, double *z, TransformDirection direction ) const
{
  if ( !d->mIsValid || d->mShortCircuit )
    return;
  // Refuse to transform the points if the srs's are invalid
  if ( !d->mSourceCRS.isValid() )
  {
    QgsMessageLog::logMessage( QObject::tr( "The source spatial reference system (CRS) is not valid. "
                                            "The coordinates can not be reprojected. The CRS is: %1" )
                               .arg( d->mSourceCRS.toProj4() ), QObject::tr( "CRS" ) );
    return;
  }
  if ( !d->mDestCRS.isValid() )
  {
    QgsMessageLog::logMessage( QObject::tr( "The destination spatial reference system (CRS) is not valid. "
                                            "The coordinates can not be reprojected. The CRS is: %1" ).arg( d->mDestCRS.toProj4() ), QObject::tr( "CRS" ) );
    return;
  }

#ifdef COORDINATE_TRANSFORM_VERBOSE
  double xorg = *x;
  double yorg = *y;
  QgsDebugMsg( QString( "[[[[[[ Number of points to transform: %1 ]]]]]]" ).arg( numPoints ) );
#endif

#ifdef QGISDEBUG
  if ( !mHasContext )
    QgsDebugMsgLevel( "No QgsCoordinateTransformContext context set for transform", 4 );
#endif

  // use proj4 to do the transform

  // if the source/destination projection is lat/long, convert the points to radians
  // prior to transforming
  QPair<projPJ, projPJ> projData = d->threadLocalProjData();
  projPJ sourceProj = projData.first;
  projPJ destProj = projData.second;

  if ( ( pj_is_latlong( destProj ) && ( direction == ReverseTransform ) )
       || ( pj_is_latlong( sourceProj ) && ( direction == ForwardTransform ) ) )
  {
    for ( int i = 0; i < numPoints; ++i )
    {
      x[i] *= DEG_TO_RAD;
      y[i] *= DEG_TO_RAD;
    }

  }
  int projResult;
  if ( direction == ReverseTransform )
  {
    projResult = pj_transform( destProj, sourceProj, numPoints, 0, x, y, z );
  }
  else
  {
    Q_ASSERT( sourceProj );
    Q_ASSERT( destProj );
    projResult = pj_transform( sourceProj, destProj, numPoints, 0, x, y, z );
  }

  if ( projResult != 0 )
  {
    //something bad happened....
    QString points;

    for ( int i = 0; i < numPoints; ++i )
    {
      if ( direction == ForwardTransform )
      {
        points += QStringLiteral( "(%1, %2)\n" ).arg( x[i], 0, 'f' ).arg( y[i], 0, 'f' );
      }
      else
      {
        points += QStringLiteral( "(%1, %2)\n" ).arg( x[i] * RAD_TO_DEG, 0, 'f' ).arg( y[i] * RAD_TO_DEG, 0, 'f' );
      }
    }

    QString dir = ( direction == ForwardTransform ) ? QObject::tr( "forward transform" ) : QObject::tr( "inverse transform" );

    char *srcdef = pj_get_def( sourceProj, 0 );
    char *dstdef = pj_get_def( destProj, 0 );

    QString msg = QObject::tr( "%1 of\n"
                               "%2"
                               "PROJ: %3 +to %4\n"
                               "Error: %5" )
                  .arg( dir,
                        points,
                        srcdef, dstdef,
                        QString::fromUtf8( pj_strerrno( projResult ) ) );

    pj_dalloc( srcdef );
    pj_dalloc( dstdef );

    QgsDebugMsg( "Projection failed emitting invalid transform signal: " + msg );
    QgsDebugMsg( "throwing exception" );

    throw QgsCsException( msg );
  }

  // if the result is lat/long, convert the results from radians back
  // to degrees
  if ( ( pj_is_latlong( destProj ) && ( direction == ForwardTransform ) )
       || ( pj_is_latlong( sourceProj ) && ( direction == ReverseTransform ) ) )
  {
    for ( int i = 0; i < numPoints; ++i )
    {
      x[i] *= RAD_TO_DEG;
      y[i] *= RAD_TO_DEG;
    }
  }
#ifdef COORDINATE_TRANSFORM_VERBOSE
  QgsDebugMsg( QString( "[[[[[[ Projected %1, %2 to %3, %4 ]]]]]]" )
               .arg( xorg, 0, 'g', 15 ).arg( yorg, 0, 'g', 15 )
               .arg( *x, 0, 'g', 15 ).arg( *y, 0, 'g', 15 ) );
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

const char *finder( const char *name )
{
  QString proj;
#ifdef Q_OS_WIN
  proj = QApplication::applicationDirPath()
         + "/share/proj/" + QString( name );
#else
  Q_UNUSED( name );
#endif
  return proj.toUtf8();
}

bool QgsCoordinateTransform::setFromCache( const QgsCoordinateReferenceSystem &src, const QgsCoordinateReferenceSystem &dest, int srcDatumTransform, int destDatumTransform )
{
  if ( !src.isValid() || !dest.isValid() )
    return false;

  sCacheLock.lockForRead();
  const QList< QgsCoordinateTransform > values = sTransforms.values( qMakePair( src.authid(), dest.authid() ) );
  for ( auto valIt = values.constBegin(); valIt != values.constEnd(); ++valIt )
  {
    if ( ( *valIt ).sourceDatumTransformId() == srcDatumTransform &&
         ( *valIt ).destinationDatumTransformId() == destDatumTransform )
    {
      // need to save, and then restore the context... we don't want this to be cached or to use the values from the cache
      QgsCoordinateTransformContext context = mContext;
#ifdef QGISDEBUG
      bool hasContext = mHasContext;
#endif
      *this = *valIt;
      sCacheLock.unlock();

      mContext = context;
#ifdef QGISDEBUG
      mHasContext = hasContext;
#endif

      return true;
    }
  }
  sCacheLock.unlock();
  return false;
}

void QgsCoordinateTransform::addToCache()
{
  if ( !d->mSourceCRS.isValid() || !d->mDestCRS.isValid() )
    return;

  sCacheLock.lockForWrite();
  sTransforms.insertMulti( qMakePair( d->mSourceCRS.authid(), d->mDestCRS.authid() ), *this );
  sCacheLock.unlock();
}

int QgsCoordinateTransform::sourceDatumTransformId() const
{
  return d->mSourceDatumTransform;
}

void QgsCoordinateTransform::setSourceDatumTransformId( int dt )
{
  d.detach();
  d->mSourceDatumTransform = dt;
}

int QgsCoordinateTransform::destinationDatumTransformId() const
{
  return d->mDestinationDatumTransform;
}

void QgsCoordinateTransform::setDestinationDatumTransformId( int dt )
{
  d.detach();
  d->mDestinationDatumTransform = dt;
}

void QgsCoordinateTransform::invalidateCache()
{
  sCacheLock.lockForWrite();
  sTransforms.clear();
  sCacheLock.unlock();
}
