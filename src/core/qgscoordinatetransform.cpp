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
#include "qgsmessagelog.h"
#include "qgslogger.h"

//qt includes
#include <QDomNode>
#include <QDomElement>
#include <QApplication>

extern "C"
{
#include <proj_api.h>
}

// if defined shows all information about transform to stdout
// #define COORDINATE_TRANSFORM_VERBOSE

QgsCoordinateTransform::QgsCoordinateTransform()
    : QObject()
    , mInitialisedFlag( false )
    , mSourceProjection( 0 )
    , mDestinationProjection( 0 )
{
  setFinder();
}

QgsCoordinateTransform::QgsCoordinateTransform( const QgsCoordinateReferenceSystem& source, const QgsCoordinateReferenceSystem& dest )
    : QObject()
    , mInitialisedFlag( false )
    , mSourceProjection( 0 )
    , mDestinationProjection( 0 )
{
  setFinder();
  mSourceCRS = source;
  mDestCRS = dest;
  initialise();
}

QgsCoordinateTransform::QgsCoordinateTransform( long theSourceSrsId, long theDestSrsId )
    : QObject()
    , mInitialisedFlag( false )
    , mSourceCRS( theSourceSrsId, QgsCoordinateReferenceSystem::InternalCrsId )
    , mDestCRS( theDestSrsId, QgsCoordinateReferenceSystem::InternalCrsId )
    , mSourceProjection( 0 )
    , mDestinationProjection( 0 )
{
  initialise();
}

QgsCoordinateTransform::QgsCoordinateTransform( QString theSourceCRS, QString theDestCRS )
    : QObject()
    , mInitialisedFlag( false )
    , mSourceProjection( 0 )
    , mDestinationProjection( 0 )
{
  setFinder();
  mSourceCRS.createFromWkt( theSourceCRS );
  mDestCRS.createFromWkt( theDestCRS );
  // initialize the coordinate system data structures
  //XXX Who spells initialize initialise?
  //XXX A: Its the queen's english....
  //XXX  : Long live the queen! Lets get on with the initialisation...
  initialise();
}

QgsCoordinateTransform::QgsCoordinateTransform( long theSourceSrid,
    QString theDestWkt,
    QgsCoordinateReferenceSystem::CrsType theSourceCRSType )
    : QObject()
    , mInitialisedFlag( false )
    , mSourceProjection( 0 )
    , mDestinationProjection( 0 )
{
  setFinder();

  mSourceCRS.createFromId( theSourceSrid, theSourceCRSType );
  mDestCRS.createFromWkt( theDestWkt );
  // initialize the coordinate system data structures
  //XXX Who spells initialize initialise?
  //XXX A: Its the queen's english....
  //XXX  : Long live the queen! Lets get on with the initialisation...
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
  mDestCRS.createFromSrsId( theCRSID );
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
    //be the same as input proj. This only happens on the first layer loaded
    //whatever that may be...
    mDestCRS.createFromOgcWmsCrs( mSourceCRS.authid() );
  }

  // init the projections (destination and source)
  mDestinationProjection = pj_init_plus( mDestCRS.toProj4().toUtf8() );
  mSourceProjection = pj_init_plus( mSourceCRS.toProj4().toUtf8() );

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


QgsPoint QgsCoordinateTransform::transform( const QgsPoint thePoint, TransformDirection direction ) const
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
  catch ( QgsCsException &cse )
  {
    // rethrow the exception
    QgsDebugMsg( "rethrowing exception" );
    throw cse;
  }

  return QgsPoint( x, y );
}


QgsPoint QgsCoordinateTransform::transform( const double theX, const double theY = 0, TransformDirection direction ) const
{
  try
  {
    return transform( QgsPoint( theX, theY ), direction );
  }
  catch ( QgsCsException &cse )
  {
    // rethrow the exception
    QgsDebugMsg( "rethrowing exception" );
    throw cse;
  }
}

QgsRectangle QgsCoordinateTransform::transform( const QgsRectangle theRect, TransformDirection direction ) const
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
  catch ( QgsCsException &cse )
  {
    // rethrow the exception
    QgsDebugMsg( "rethrowing exception" );
    throw cse;
  }

#ifdef COORDINATE_TRANSFORM_VERBOSE
  QgsDebugMsg( "Rect projection..." );
  QgsLogger::debug( "Xmin : ", theRect.xMinimum(), 1, __FILE__, __FUNCTION__, __LINE__ );
  QgsLogger::debug( "-->", x1, 1, __FILE__, __FUNCTION__, __LINE__ );
  QgsLogger::debug( "Ymin : ", theRect.yMinimum(), 1, __FILE__, __FUNCTION__, __LINE__ );
  QgsLogger::debug( "-->", y1, 1, __FILE__, __FUNCTION__, __LINE__ );
  QgsLogger::debug( "Xmax : ", theRect.xMaximum(), 1, __FILE__, __FUNCTION__, __LINE__ );
  QgsLogger::debug( "-->", x2, 1, __FILE__, __FUNCTION__, __LINE__ );
  QgsLogger::debug( "Ymax : ", theRect.yMaximum(), 1, __FILE__, __FUNCTION__, __LINE__ );
  QgsLogger::debug( "-->", y2, 1, __FILE__, __FUNCTION__, __LINE__ );
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
  catch ( QgsCsException &cse )
  {
    // rethrow the exception
    QgsDebugMsg( "rethrowing exception" );
    throw cse;
  }
}

void QgsCoordinateTransform::transformInPlace( std::vector<double>& x,
    std::vector<double>& y, std::vector<double>& z,
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
  catch ( QgsCsException &cse )
  {
    // rethrow the exception
    QgsDebugMsg( "rethrowing exception" );
    throw cse;
  }
}

#ifdef ANDROID
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
  catch ( QgsCsException &cse )
  {
    // rethrow the exception
    QgsDebugMsg( "rethrowing exception" );
    throw cse;
  }
}

void QgsCoordinateTransform::transformInPlace( std::vector<float>& x,
    std::vector<float>& y, std::vector<float>& z,
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
    std::vector<double> xd( x.size() );
    std::vector<double> yd( y.size() );
    std::vector<double> zd( z.size() );
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
  catch ( QgsCsException &cse )
  {
    // rethrow the exception
    QgsDebugMsg( "rethrowing exception" );
    throw cse;
  }
}
#endif //ANDROID


QgsRectangle QgsCoordinateTransform::transformBoundingBox( const QgsRectangle rect, TransformDirection direction ) const
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

  static const int numP = 8;

  QgsRectangle bb_rect;
  bb_rect.setMinimal();

  // We're interfacing with C-style vectors in the
  // end, so let's do C-style vectors here too.

  double x[numP * numP];
  double y[numP * numP];
  double z[numP * numP];

  QgsDebugMsg( "Entering transformBoundingBox..." );

  // Populate the vectors

  double dx = rect.width()  / ( double )( numP - 1 );
  double dy = rect.height() / ( double )( numP - 1 );

  double pointY = rect.yMinimum();

  for ( int i = 0; i < numP ; i++ )
  {

    // Start at right edge
    double pointX = rect.xMinimum();

    for ( int j = 0; j < numP; j++ )
    {
      x[( i*numP ) + j] = pointX;
      y[( i*numP ) + j] = pointY;
      // and the height...
      z[( i*numP ) + j] = 0.0;
      // QgsDebugMsg(QString("BBox coord: (%1, %2)").arg(x[(i*numP) + j]).arg(y[(i*numP) + j]));
      pointX += dx;
    }
    pointY += dy;
  }

  // Do transformation. Any exception generated must
  // be handled in above layers.
  try
  {
    transformCoords( numP * numP, x, y, z, direction );
  }
  catch ( QgsCsException &cse )
  {
    // rethrow the exception
    QgsDebugMsg( "rethrowing exception" );
    throw cse;
  }

  // Calculate the bounding box and use that for the extent

  for ( int i = 0; i < numP * numP; i++ )
  {
    if ( qIsFinite( x[i] ) && qIsFinite( y[i] ) )
      bb_rect.combineExtentWith( x[i], y[i] );
  }

  QgsDebugMsg( "Projected extent: " + bb_rect.toString() );

  if ( bb_rect.isEmpty() )
  {
    QgsDebugMsg( "Original extent: " + rect.toString() );
  }

  return bb_rect;
}

void QgsCoordinateTransform::transformCoords( const int& numPoints, double *x, double *y, double *z, TransformDirection direction ) const
{
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
      z[i] *= DEG_TO_RAD;
    }

  }
  int projResult;
  if ( direction == ReverseTransform )
  {
    projResult = pj_transform( mDestinationProjection, mSourceProjection, numPoints, 0, x, y, z );
    dir = tr( "inverse transform" );
  }
  else
  {
    Q_ASSERT( mSourceProjection != 0 );
    Q_ASSERT( mDestinationProjection != 0 );
    projResult = pj_transform( mSourceProjection, mDestinationProjection, numPoints, 0, x, y, z );
    dir = tr( "forward transform" );
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

    QString msg = tr( "%1 of\n"
                      "%2"
                      "PROJ.4: %3 +to %4\n"
                      "Error: %5" )
                  .arg( dir )
                  .arg( points )
                  .arg( mSourceCRS.toProj4() ).arg( mDestCRS.toProj4() )
                  .arg( QString::fromUtf8( pj_strerrno( projResult ) ) );

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
      z[i] *= RAD_TO_DEG;
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

  initialise();

  return true;
}

bool QgsCoordinateTransform::writeXML( QDomNode & theNode, QDomDocument & theDoc )
{
  QDomElement myNodeElement = theNode.toElement();
  QDomElement myTransformElement = theDoc.createElement( "coordinatetransform" );

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
#ifdef WIN32
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
