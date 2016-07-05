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
#include "qgsapplication.h"
#include "qgscrscache.h"
#include "qgsmessagelog.h"
#include "qgslogger.h"

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

QgsCoordinateTransform::QgsCoordinateTransform()
    : QObject()
    , mShortCircuit( false )
    , mInitialisedFlag( false )
    , mSourceProjection( nullptr )
    , mDestinationProjection( nullptr )
    , mSourceDatumTransform( -1 )
    , mDestinationDatumTransform( -1 )
{
  setFinder();
}

QgsCoordinateTransform::QgsCoordinateTransform( const QgsCoordinateReferenceSystem& source, const QgsCoordinateReferenceSystem& dest )
    : QObject()
    , mShortCircuit( false )
    , mInitialisedFlag( false )
    , mSourceProjection( nullptr )
    , mDestinationProjection( nullptr )
    , mSourceDatumTransform( -1 )
    , mDestinationDatumTransform( -1 )
{
  setFinder();
  mSourceCRS = source;
  mDestCRS = dest;
  initialise();
}

QgsCoordinateTransform::QgsCoordinateTransform( long theSourceSrsId, long theDestSrsId )
    : QObject()
    , mInitialisedFlag( false )
    , mSourceCRS( QgsCRSCache::instance()->crsBySrsId( theSourceSrsId ) )
    , mDestCRS( QgsCRSCache::instance()->crsBySrsId( theDestSrsId ) )
    , mSourceProjection( nullptr )
    , mDestinationProjection( nullptr )
    , mSourceDatumTransform( -1 )
    , mDestinationDatumTransform( -1 )
{
  initialise();
}

QgsCoordinateTransform::QgsCoordinateTransform( const QString& theSourceCRS, const QString& theDestCRS )
    : QObject()
    , mInitialisedFlag( false )
    , mSourceProjection( nullptr )
    , mDestinationProjection( nullptr )
    , mSourceDatumTransform( -1 )
    , mDestinationDatumTransform( -1 )
{
  setFinder();
  mSourceCRS = QgsCRSCache::instance()->crsByWkt( theSourceCRS );
  mDestCRS = QgsCRSCache::instance()->crsByWkt( theDestCRS );
  // initialize the coordinate system data structures
  //XXX Who spells initialize initialise?
  //XXX A: Its the queen's english....
  //XXX  : Long live the queen! Lets get on with the initialization...
  initialise();
}

QgsCoordinateTransform::QgsCoordinateTransform( long theSourceSrid,
    const QString& theDestWkt,
    QgsCoordinateReferenceSystem::CrsType theSourceCRSType )
    : QObject()
    , mInitialisedFlag( false )
    , mSourceProjection( nullptr )
    , mDestinationProjection( nullptr )
    , mSourceDatumTransform( -1 )
    , mDestinationDatumTransform( -1 )
{
  setFinder();

  mSourceCRS.createFromId( theSourceSrid, theSourceCRSType );
  mDestCRS = QgsCRSCache::instance()->crsByWkt( theDestWkt );
  // initialize the coordinate system data structures
  //XXX Who spells initialize initialise?
  //XXX A: Its the queen's english....
  //XXX  : Long live the queen! Lets get on with the initialization...
  initialise();
}

QgsCoordinateTransform::~QgsCoordinateTransform()
{
  // free the proj objects
  if ( mSourceProjection )
  {
    pj_free( mSourceProjection );
  }
  if ( mDestinationProjection )
  {
    pj_free( mDestinationProjection );
  }
}

QgsCoordinateTransform* QgsCoordinateTransform::clone() const
{
  QgsCoordinateTransform* tr = new QgsCoordinateTransform( sourceCrs(), destCRS() );
  tr->setSourceDatumTransform( sourceDatumTransform() );
  tr->setDestinationDatumTransform( destinationDatumTransform() );
  tr->initialise();
  return tr;
}

void QgsCoordinateTransform::setSourceCrs( const QgsCoordinateReferenceSystem& theCRS )
{
  mSourceCRS = theCRS;
  initialise();
}
void QgsCoordinateTransform::setDestCRS( const QgsCoordinateReferenceSystem& theCRS )
{
  mDestCRS = theCRS;
  initialise();
}

void QgsCoordinateTransform::setDestCRSID( long theCRSID )
{
  //!todo Add some logic here to determine if the srsid is a system or user one
  mDestCRS = QgsCRSCache::instance()->crsBySrsId( theCRSID );
  initialise();
}

// XXX This whole function is full of multiple return statements!!!
// And probably shouldn't be a void
void QgsCoordinateTransform::initialise()
{
  // XXX Warning - multiple return paths in this block!!
  if ( !mSourceCRS.isValid() )
  {
    //mSourceCRS = defaultWkt;
    // Pass through with no projection since we have no idea what the layer
    // coordinates are and projecting them may not be appropriate
    mShortCircuit = true;
    QgsDebugMsg( "SourceCRS seemed invalid!" );
    return;
  }

  if ( !mDestCRS.isValid() )
  {
    //No destination projection is set so we set the default output projection to
    //be the same as input proj.
    mDestCRS = QgsCRSCache::instance()->crsByOgcWmsCrs( mSourceCRS.authid() );
  }

  bool useDefaultDatumTransform = ( mSourceDatumTransform == - 1 && mDestinationDatumTransform == -1 );

  // init the projections (destination and source)

  pj_free( mSourceProjection );
  QString sourceProjString = mSourceCRS.toProj4();
  if ( !useDefaultDatumTransform )
  {
    sourceProjString = stripDatumTransform( sourceProjString );
  }
  if ( mSourceDatumTransform != -1 )
  {
    sourceProjString += ( ' ' + datumTransformString( mSourceDatumTransform ) );
  }

  pj_free( mDestinationProjection );
  QString destProjString = mDestCRS.toProj4();
  if ( !useDefaultDatumTransform )
  {
    destProjString = stripDatumTransform( destProjString );
  }
  if ( mDestinationDatumTransform != -1 )
  {
    destProjString += ( ' ' +  datumTransformString( mDestinationDatumTransform ) );
  }

  if ( !useDefaultDatumTransform )
  {
    addNullGridShifts( sourceProjString, destProjString );
  }

  mSourceProjection = pj_init_plus( sourceProjString.toUtf8() );
  mDestinationProjection = pj_init_plus( destProjString.toUtf8() );

#ifdef COORDINATE_TRANSFORM_VERBOSE
  QgsDebugMsg( "From proj : " + mSourceCRS.toProj4() );
  QgsDebugMsg( "To proj   : " + mDestCRS.toProj4() );
#endif

  mInitialisedFlag = true;
  if ( !mDestinationProjection )
  {
    mInitialisedFlag = false;
  }
  if ( !mSourceProjection )
  {
    mInitialisedFlag = false;
  }
#ifdef COORDINATE_TRANSFORM_VERBOSE
  if ( mInitialisedFlag )
  {
    QgsDebugMsg( "------------------------------------------------------------" );
    QgsDebugMsg( "The OGR Coordinate transformation for this layer was set to" );
    QgsLogger::debug<QgsCoordinateReferenceSystem>( "Input", mSourceCRS, __FILE__, __FUNCTION__, __LINE__ );
    QgsLogger::debug<QgsCoordinateReferenceSystem>( "Output", mDestCRS, __FILE__, __FUNCTION__, __LINE__ );
    QgsDebugMsg( "------------------------------------------------------------" );
  }
  else
  {
    QgsDebugMsg( "------------------------------------------------------------" );
    QgsDebugMsg( "The OGR Coordinate transformation FAILED TO INITIALISE!" );
    QgsDebugMsg( "------------------------------------------------------------" );
  }
#else
  if ( !mInitialisedFlag )
  {
    QgsDebugMsg( "Coordinate transformation failed to initialize!" );
  }
#endif

  //XXX todo overload == operator for QgsCoordinateReferenceSystem
  //at the moment srs.parameters contains the whole proj def...soon it wont...
  //if (mSourceCRS->toProj4() == mDestCRS->toProj4())
  if ( mSourceCRS == mDestCRS )
  {
    // If the source and destination projection are the same, set the short
    // circuit flag (no transform takes place)
    mShortCircuit = true;
    QgsDebugMsgLevel( "Source/Dest CRS equal, shortcircuit is set.", 3 );
  }
  else
  {
    // Transform must take place
    mShortCircuit = false;
    QgsDebugMsgLevel( "Source/Dest CRS UNequal, shortcircuit is NOt set.", 3 );
  }

}

//
//
// TRANSFORMERS BELOW THIS POINT .........
//
//
//


QgsPoint QgsCoordinateTransform::transform( const QgsPoint &thePoint, TransformDirection direction ) const
{
  if ( mShortCircuit || !mInitialisedFlag )
    return thePoint;
  // transform x
  double x = thePoint.x();
  double y = thePoint.y();
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

  return QgsPoint( x, y );
}


QgsPoint QgsCoordinateTransform::transform( const double theX, const double theY = 0.0, TransformDirection direction ) const
{
  try
  {
    return transform( QgsPoint( theX, theY ), direction );
  }
  catch ( const QgsCsException & )
  {
    // rethrow the exception
    QgsDebugMsg( "rethrowing exception" );
    throw;
  }
}

QgsRectangle QgsCoordinateTransform::transform( const QgsRectangle &theRect, TransformDirection direction ) const
{
  if ( mShortCircuit || !mInitialisedFlag )
    return theRect;
  // transform x
  double x1 = theRect.xMinimum();
  double y1 = theRect.yMinimum();
  double x2 = theRect.xMaximum();
  double y2 = theRect.yMaximum();

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
  QgsDebugMsg( QString( "Xmin : %1 --> %2" ).arg( theRect.xMinimum() ).arg( x1 ) );
  QgsDebugMsg( QString( "Ymin : %1 --> %2" ).arg( theRect.yMinimum() ).arg( y1 ) );
  QgsDebugMsg( QString( "Xmax : %1 --> %2" ).arg( theRect.xMaximum() ).arg( x2 ) );
  QgsDebugMsg( QString( "Ymax : %1 --> %2" ).arg( theRect.yMaximum() ).arg( y2 ) );
#endif
  return QgsRectangle( x1, y1, x2, y2 );
}

void QgsCoordinateTransform::transformInPlace( double& x, double& y, double& z,
    TransformDirection direction ) const
{
  if ( mShortCircuit || !mInitialisedFlag )
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

void QgsCoordinateTransform::transformInPlace( float& x, float& y, double& z,
    TransformDirection direction ) const
{
  double xd = static_cast< double >( x ), yd = static_cast< double >( y );
  transformInPlace( xd, yd, z, direction );
  x = xd;
  y = yd;
}

void QgsCoordinateTransform::transformInPlace( float& x, float& y, float& z,
    TransformDirection direction ) const
{
  if ( mShortCircuit || !mInitialisedFlag )
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

void QgsCoordinateTransform::transformPolygon( QPolygonF& poly, TransformDirection direction ) const
{
  if ( mShortCircuit || !mInitialisedFlag )
  {
    return;
  }

  //create x, y arrays
  int nVertices = poly.size();

  QVector<double> x( nVertices );
  QVector<double> y( nVertices );
  QVector<double> z( nVertices );

  for ( int i = 0; i < nVertices; ++i )
  {
    const QPointF& pt = poly.at( i );
    x[i] = pt.x();
    y[i] = pt.y();
    z[i] = 0;
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

  for ( int i = 0; i < nVertices; ++i )
  {
    QPointF& pt = poly[i];
    pt.rx() = x[i];
    pt.ry() = y[i];
  }
}

void QgsCoordinateTransform::transformInPlace(
  QVector<double>& x, QVector<double>& y, QVector<double>& z,
  TransformDirection direction ) const
{
  if ( mShortCircuit || !mInitialisedFlag )
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
  QVector<float>& x, QVector<float>& y, QVector<float>& z,
  TransformDirection direction ) const
{
  if ( mShortCircuit || !mInitialisedFlag )
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
    for ( int i = 0; i < vectorSize; ++i )
    {
      xd[i] = x[i];
      yd[i] = y[i];
      zd[i] = z[i];
    }
    transformCoords( x.size(), &xd[0], &yd[0], &zd[0], direction );

    //copy back
    for ( int i = 0; i < vectorSize; ++i )
    {
      x[i] = xd[i];
      y[i] = yd[i];
      z[i] = zd[i];
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

  if ( mShortCircuit || !mInitialisedFlag )
    return rect;

  if ( rect.isEmpty() )
  {
    QgsPoint p = transform( rect.xMinimum(), rect.yMinimum(), direction );
    return QgsRectangle( p, p );
  }

  // 64 points (<=2.12) is not enough, see #13665, for EPSG:4326 -> EPSG:3574 (say that it is a hard one),
  // are decent result from about 500 points and more. This method is called quite often, but
  // even with 1000 points it takes < 1ms
  // TODO: how to effectively and precisely reproject bounding box?
  const int nPoints = 1000;
  double d = sqrt(( rect.width() * rect.height() ) / pow( sqrt( static_cast< double >( nPoints ) ) - 1, 2.0 ) );
  int nXPoints = static_cast< int >( ceil( rect.width() / d ) ) + 1;
  int nYPoints = static_cast< int >( ceil( rect.height() / d ) ) + 1;

  QgsRectangle bb_rect;
  bb_rect.setMinimal();

  // We're interfacing with C-style vectors in the
  // end, so let's do C-style vectors here too.

  QVector<double> x( nXPoints * nYPoints );
  QVector<double> y( nXPoints * nYPoints );
  QVector<double> z( nXPoints * nYPoints );

  QgsDebugMsg( "Entering transformBoundingBox..." );

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
      x[( i*nXPoints ) + j] = pointX;
      y[( i*nXPoints ) + j] = pointY;
      // and the height...
      z[( i*nXPoints ) + j] = 0.0;
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
    if ( !qIsFinite( x[i] ) || !qIsFinite( y[i] ) )
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

  if ( handle180Crossover )
  {
    //subtract temporary addition of 360 degrees from longitudes
    if ( bb_rect.xMinimum() > 180.0 )
      bb_rect.setXMinimum( bb_rect.xMinimum() - 360.0 );
    if ( bb_rect.xMaximum() > 180.0 )
      bb_rect.setXMaximum( bb_rect.xMaximum() - 360.0 );
  }

  QgsDebugMsg( "Projected extent: " + bb_rect.toString() );

  if ( bb_rect.isEmpty() )
  {
    QgsDebugMsg( "Original extent: " + rect.toString() );
  }

  return bb_rect;
}

void QgsCoordinateTransform::transformCoords( int numPoints, double *x, double *y, double *z, TransformDirection direction ) const
{
  if ( mShortCircuit || !mInitialisedFlag )
    return;
  // Refuse to transform the points if the srs's are invalid
  if ( !mSourceCRS.isValid() )
  {
    QgsMessageLog::logMessage( tr( "The source spatial reference system (CRS) is not valid. "
                                   "The coordinates can not be reprojected. The CRS is: %1" )
                               .arg( mSourceCRS.toProj4() ), tr( "CRS" ) );
    return;
  }
  if ( !mDestCRS.isValid() )
  {
    QgsMessageLog::logMessage( tr( "The destination spatial reference system (CRS) is not valid. "
                                   "The coordinates can not be reprojected. The CRS is: %1" ).arg( mDestCRS.toProj4() ), tr( "CRS" ) );
    return;
  }

#ifdef COORDINATE_TRANSFORM_VERBOSE
  double xorg = *x;
  double yorg = *y;
  QgsDebugMsg( QString( "[[[[[[ Number of points to transform: %1 ]]]]]]" ).arg( numPoints ) );
#endif

  // use proj4 to do the transform
  QString dir;
  // if the source/destination projection is lat/long, convert the points to radians
  // prior to transforming
  if (( pj_is_latlong( mDestinationProjection ) && ( direction == ReverseTransform ) )
      || ( pj_is_latlong( mSourceProjection ) && ( direction == ForwardTransform ) ) )
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
    projResult = pj_transform( mDestinationProjection, mSourceProjection, numPoints, 0, x, y, z );
  }
  else
  {
    Q_ASSERT( mSourceProjection );
    Q_ASSERT( mDestinationProjection );
    projResult = pj_transform( mSourceProjection, mDestinationProjection, numPoints, 0, x, y, z );
  }

  if ( projResult != 0 )
  {
    //something bad happened....
    QString points;

    for ( int i = 0; i < numPoints; ++i )
    {
      if ( direction == ForwardTransform )
      {
        points += QString( "(%1, %2)\n" ).arg( x[i], 0, 'f' ).arg( y[i], 0, 'f' );
      }
      else
      {
        points += QString( "(%1, %2)\n" ).arg( x[i] * RAD_TO_DEG, 0, 'f' ).arg( y[i] * RAD_TO_DEG, 0, 'f' );
      }
    }

    dir = ( direction == ForwardTransform ) ? tr( "forward transform" ) : tr( "inverse transform" );

    char *srcdef = pj_get_def( mSourceProjection, 0 );
    char *dstdef = pj_get_def( mDestinationProjection, 0 );

    QString msg = tr( "%1 of\n"
                      "%2"
                      "PROJ.4: %3 +to %4\n"
                      "Error: %5" )
                  .arg( dir,
                        points,
                        srcdef, dstdef,
                        QString::fromUtf8( pj_strerrno( projResult ) ) );

    pj_dalloc( srcdef );
    pj_dalloc( dstdef );

    QgsDebugMsg( "Projection failed emitting invalid transform signal: " + msg );

    emit invalidTransformInput();

    QgsDebugMsg( "throwing exception" );

    throw QgsCsException( msg );
  }

  // if the result is lat/long, convert the results from radians back
  // to degrees
  if (( pj_is_latlong( mDestinationProjection ) && ( direction == ForwardTransform ) )
      || ( pj_is_latlong( mSourceProjection ) && ( direction == ReverseTransform ) ) )
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

bool QgsCoordinateTransform::readXML( QDomNode & theNode )
{

  QgsDebugMsg( "Reading Coordinate Transform from xml ------------------------!" );

  QDomNode mySrcNode = theNode.namedItem( "sourcesrs" );
  mSourceCRS.readXML( mySrcNode );

  QDomNode myDestNode = theNode.namedItem( "destinationsrs" );
  mDestCRS.readXML( myDestNode );

  mSourceDatumTransform = theNode.toElement().attribute( "sourceDatumTransform", "-1" ).toInt();
  mDestinationDatumTransform = theNode.toElement().attribute( "destinationDatumTransform", "-1" ).toInt();

  initialise();

  return true;
}

bool QgsCoordinateTransform::writeXML( QDomNode & theNode, QDomDocument & theDoc )
{
  QDomElement myNodeElement = theNode.toElement();
  QDomElement myTransformElement = theDoc.createElement( "coordinatetransform" );
  myTransformElement.setAttribute( "sourceDatumTransform", QString::number( mSourceDatumTransform ) );
  myTransformElement.setAttribute( "destinationDatumTransform", QString::number( mDestinationDatumTransform ) );

  QDomElement mySourceElement = theDoc.createElement( "sourcesrs" );
  mSourceCRS.writeXML( mySourceElement, theDoc );
  myTransformElement.appendChild( mySourceElement );

  QDomElement myDestElement = theDoc.createElement( "destinationsrs" );
  mDestCRS.writeXML( myDestElement, theDoc );
  myTransformElement.appendChild( myDestElement );

  myNodeElement.appendChild( myTransformElement );

  return true;
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

void QgsCoordinateTransform::setFinder()
{
#if 0
  // Attention! It should be possible to set PROJ_LIB
  // but it can happen that it was previously set by installer
  // (version 0.7) and the old installation was deleted

  // Another problem: PROJ checks if pj_finder was set before
  // PROJ_LIB environment variable. pj_finder is probably set in
  // GRASS gproj library when plugin is loaded, consequently
  // PROJ_LIB is ignored

  pj_set_finder( finder );
#endif
}

QList< QList< int > > QgsCoordinateTransform::datumTransformations( const QgsCoordinateReferenceSystem& srcCRS, const QgsCoordinateReferenceSystem& destCRS )
{
  QList< QList< int > > transformations;

  QString srcGeoId = srcCRS.geographicCRSAuthId();
  QString destGeoId = destCRS.geographicCRSAuthId();

  if ( srcGeoId.isEmpty() || destGeoId.isEmpty() )
  {
    return transformations;
  }

  QStringList srcSplit = srcGeoId.split( ':' );
  QStringList destSplit = destGeoId.split( ':' );

  if ( srcSplit.size() < 2 || destSplit.size() < 2 )
  {
    return transformations;
  }

  int srcAuthCode = srcSplit.at( 1 ).toInt();
  int destAuthCode = destSplit.at( 1 ).toInt();

  if ( srcAuthCode == destAuthCode )
  {
    return transformations; //crs have the same datum
  }

  QList<int> directTransforms;
  searchDatumTransform( QString( "SELECT coord_op_code FROM tbl_datum_transform WHERE source_crs_code=%1 AND target_crs_code=%2 ORDER BY deprecated ASC,preferred DESC" ).arg( srcAuthCode ).arg( destAuthCode ),
                        directTransforms );
  QList<int> reverseDirectTransforms;
  searchDatumTransform( QString( "SELECT coord_op_code FROM tbl_datum_transform WHERE source_crs_code = %1 AND target_crs_code=%2 ORDER BY deprecated ASC,preferred DESC" ).arg( destAuthCode ).arg( srcAuthCode ),
                        reverseDirectTransforms );
  QList<int> srcToWgs84;
  searchDatumTransform( QString( "SELECT coord_op_code FROM tbl_datum_transform WHERE (source_crs_code=%1 AND target_crs_code=%2) OR (source_crs_code=%2 AND target_crs_code=%1) ORDER BY deprecated ASC,preferred DESC" ).arg( srcAuthCode ).arg( 4326 ),
                        srcToWgs84 );
  QList<int> destToWgs84;
  searchDatumTransform( QString( "SELECT coord_op_code FROM tbl_datum_transform WHERE (source_crs_code=%1 AND target_crs_code=%2) OR (source_crs_code=%2 AND target_crs_code=%1) ORDER BY deprecated ASC,preferred DESC" ).arg( destAuthCode ).arg( 4326 ),
                        destToWgs84 );

  //add direct datum transformations
  QList<int>::const_iterator directIt = directTransforms.constBegin();
  for ( ; directIt != directTransforms.constEnd(); ++directIt )
  {
    transformations.push_back( QList<int>() << *directIt << -1 );
  }

  //add direct datum transformations
  directIt = reverseDirectTransforms.constBegin();
  for ( ; directIt != reverseDirectTransforms.constEnd(); ++directIt )
  {
    transformations.push_back( QList<int>() << -1 << *directIt );
  }

  QList<int>::const_iterator srcWgsIt = srcToWgs84.constBegin();
  for ( ; srcWgsIt != srcToWgs84.constEnd(); ++srcWgsIt )
  {
    QList<int>::const_iterator dstWgsIt = destToWgs84.constBegin();
    for ( ; dstWgsIt != destToWgs84.constEnd(); ++dstWgsIt )
    {
      transformations.push_back( QList<int>() << *srcWgsIt << *dstWgsIt );
    }
  }

  return transformations;
}

QString QgsCoordinateTransform::stripDatumTransform( const QString& proj4 )
{
  QStringList parameterSplit = proj4.split( '+', QString::SkipEmptyParts );
  QString currentParameter;
  QString newProjString;

  for ( int i = 0; i < parameterSplit.size(); ++i )
  {
    currentParameter = parameterSplit.at( i );
    if ( !currentParameter.startsWith( "towgs84", Qt::CaseInsensitive )
         && !currentParameter.startsWith( "nadgrids", Qt::CaseInsensitive ) )
    {
      newProjString.append( '+' );
      newProjString.append( currentParameter );
      newProjString.append( ' ' );
    }
  }
  return newProjString;
}

void QgsCoordinateTransform::searchDatumTransform( const QString& sql, QList< int >& transforms )
{
  sqlite3* db;
  int openResult = sqlite3_open_v2( QgsApplication::srsDbFilePath().toUtf8().constData(), &db, SQLITE_OPEN_READONLY, 0 );
  if ( openResult != SQLITE_OK )
  {
    sqlite3_close( db );
    return;
  }

  sqlite3_stmt* stmt;
  int prepareRes = sqlite3_prepare( db, sql.toAscii(), sql.size(), &stmt, nullptr );
  if ( prepareRes != SQLITE_OK )
  {
    sqlite3_finalize( stmt );
    sqlite3_close( db );
    return;
  }

  QString cOpCode;
  while ( sqlite3_step( stmt ) == SQLITE_ROW )
  {
    cOpCode = reinterpret_cast< const char * >( sqlite3_column_text( stmt, 0 ) );
    transforms.push_back( cOpCode.toInt() );
  }
  sqlite3_finalize( stmt );
  sqlite3_close( db );
}

QString QgsCoordinateTransform::datumTransformString( int datumTransform )
{
  QString transformString;

  sqlite3* db;
  int openResult = sqlite3_open_v2( QgsApplication::srsDbFilePath().toUtf8().constData(), &db, SQLITE_OPEN_READONLY, 0 );
  if ( openResult != SQLITE_OK )
  {
    sqlite3_close( db );
    return transformString;
  }

  sqlite3_stmt* stmt;
  QString sql = QString( "SELECT coord_op_method_code,p1,p2,p3,p4,p5,p6,p7 FROM tbl_datum_transform WHERE coord_op_code=%1" ).arg( datumTransform );
  int prepareRes = sqlite3_prepare( db, sql.toAscii(), sql.size(), &stmt, nullptr );
  if ( prepareRes != SQLITE_OK )
  {
    sqlite3_finalize( stmt );
    sqlite3_close( db );
    return transformString;
  }

  if ( sqlite3_step( stmt ) == SQLITE_ROW )
  {
    //coord_op_methode_code
    int methodCode = sqlite3_column_int( stmt, 0 );
    if ( methodCode == 9615 ) //ntv2
    {
      transformString = "+nadgrids=" + QString( reinterpret_cast< const char * >( sqlite3_column_text( stmt, 1 ) ) );
    }
    else if ( methodCode == 9603 || methodCode == 9606 || methodCode == 9607 )
    {
      transformString += "+towgs84=";
      double p1 = sqlite3_column_double( stmt, 1 );
      double p2 = sqlite3_column_double( stmt, 2 );
      double p3 = sqlite3_column_double( stmt, 3 );
      double p4 = sqlite3_column_double( stmt, 4 );
      double p5 = sqlite3_column_double( stmt, 5 );
      double p6 = sqlite3_column_double( stmt, 6 );
      double p7 = sqlite3_column_double( stmt, 7 );
      if ( methodCode == 9603 ) //3 parameter transformation
      {
        transformString += QString( "%1,%2,%3" ).arg( p1 ).arg( p2 ).arg( p3 );
      }
      else //7 parameter transformation
      {
        transformString += QString( "%1,%2,%3,%4,%5,%6,%7" ).arg( p1 ).arg( p2 ).arg( p3 ).arg( p4 ).arg( p5 ).arg( p6 ).arg( p7 );
      }
    }
  }

  sqlite3_finalize( stmt );
  sqlite3_close( db );
  return transformString;
}

bool QgsCoordinateTransform::datumTransformCrsInfo( int datumTransform, int& epsgNr, QString& srcProjection, QString& dstProjection, QString &remarks, QString &scope, bool &preferred, bool &deprecated )
{
  sqlite3* db;
  int openResult = sqlite3_open_v2( QgsApplication::srsDbFilePath().toUtf8().constData(), &db, SQLITE_OPEN_READONLY, 0 );
  if ( openResult != SQLITE_OK )
  {
    sqlite3_close( db );
    return false;
  }

  sqlite3_stmt* stmt;
  QString sql = QString( "SELECT epsg_nr,source_crs_code,target_crs_code,remarks,scope,preferred,deprecated FROM tbl_datum_transform WHERE coord_op_code=%1" ).arg( datumTransform );
  int prepareRes = sqlite3_prepare( db, sql.toAscii(), sql.size(), &stmt, nullptr );
  if ( prepareRes != SQLITE_OK )
  {
    sqlite3_finalize( stmt );
    sqlite3_close( db );
    return false;
  }

  int srcCrsId, destCrsId;
  if ( sqlite3_step( stmt ) != SQLITE_ROW )
  {
    sqlite3_finalize( stmt );
    sqlite3_close( db );
    return false;
  }

  epsgNr = sqlite3_column_int( stmt, 0 );
  srcCrsId = sqlite3_column_int( stmt, 1 );
  destCrsId = sqlite3_column_int( stmt, 2 );
  remarks = QString::fromUtf8( reinterpret_cast< const char * >( sqlite3_column_text( stmt, 3 ) ) );
  scope = QString::fromUtf8( reinterpret_cast< const char * >( sqlite3_column_text( stmt, 4 ) ) );
  preferred = sqlite3_column_int( stmt, 5 ) != 0;
  deprecated = sqlite3_column_int( stmt, 6 ) != 0;

  QgsCoordinateReferenceSystem srcCrs = QgsCRSCache::instance()->crsByOgcWmsCrs( QString( "EPSG:%1" ).arg( srcCrsId ) );
  srcProjection = srcCrs.description();
  QgsCoordinateReferenceSystem destCrs = QgsCRSCache::instance()->crsByOgcWmsCrs( QString( "EPSG:%1" ).arg( destCrsId ) );
  dstProjection = destCrs.description();

  sqlite3_finalize( stmt );
  sqlite3_close( db );
  return true;
}

void QgsCoordinateTransform::addNullGridShifts( QString& srcProjString, QString& destProjString )
{
  //if one transformation uses ntv2, the other one needs to be null grid shift
  if ( mDestinationDatumTransform == -1 && srcProjString.contains( "+nadgrids" ) ) //add null grid if source transformation is ntv2
  {
    destProjString += " +nadgrids=@null";
    return;
  }
  if ( mSourceDatumTransform == -1 && destProjString.contains( "+nadgrids" ) )
  {
    srcProjString += " +nadgrids=@null";
    return;
  }

  //add null shift grid for google mercator
  //(see e.g. http://trac.osgeo.org/proj/wiki/FAQ#ChangingEllipsoidWhycantIconvertfromWGS84toGoogleEarthVirtualGlobeMercator)
  if ( mSourceCRS.authid().compare( "EPSG:3857", Qt::CaseInsensitive ) == 0 && mSourceDatumTransform == -1 )
  {
    srcProjString += " +nadgrids=@null";
  }
  if ( mDestCRS.authid().compare( "EPSG:3857", Qt::CaseInsensitive ) == 0 && mDestinationDatumTransform == -1 )
  {
    destProjString += " +nadgrids=@null";
  }
}
