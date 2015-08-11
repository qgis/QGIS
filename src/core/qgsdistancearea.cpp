/***************************************************************************
  qgsdistancearea.cpp - Distance and area calculations on the ellipsoid
 ---------------------------------------------------------------------------
  Date                 : September 2005
  Copyright            : (C) 2005 by Martin Dobias
  email                : won.der at centrum.sk
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <cmath>
#include <sqlite3.h>
#include <QDir>
#include <QString>
#include <QLocale>
#include <QObject>

#include "qgis.h"
#include "qgspoint.h"
#include "qgscoordinatetransform.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsgeometry.h"
#include "qgsgeometrycollectionv2.h"
#include "qgsdistancearea.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsmultisurfacev2.h"
#include "qgswkbptr.h"
#include "qgslinestringv2.h"
#include "qgspolygonv2.h"
#include "qgssurfacev2.h"

// MSVC compiler doesn't have defined M_PI in math.h
#ifndef M_PI
#define M_PI          3.14159265358979323846
#endif

#define DEG2RAD(x)    ((x)*M_PI/180)


QgsDistanceArea::QgsDistanceArea()
{
  // init with default settings
  mEllipsoidalMode = false;
  mCoordTransform = new QgsCoordinateTransform;
  setSourceCrs( GEOCRS_ID ); // WGS 84
  setEllipsoid( GEO_NONE );
}


//! Copy constructor
QgsDistanceArea::QgsDistanceArea( const QgsDistanceArea & origDA )
{
  _copy( origDA );
}

QgsDistanceArea::~QgsDistanceArea()
{
  delete mCoordTransform;
}

//! Assignment operator
QgsDistanceArea & QgsDistanceArea::operator=( const QgsDistanceArea & origDA )
{
  if ( this == & origDA )
  {
    // Do not copy unto self
    return *this;
  }
  _copy( origDA );
  return *this;
}

//! Copy helper. Avoid Sqlite3 accesses.
void QgsDistanceArea::_copy( const QgsDistanceArea & origDA )
{
  mEllipsoidalMode = origDA.mEllipsoidalMode;
  mEllipsoid = origDA.mEllipsoid;
  mSemiMajor = origDA.mSemiMajor;
  mSemiMinor = origDA.mSemiMinor;
  mInvFlattening = origDA.mInvFlattening;
  // Some calculations and trig. Should not be TOO time consuming.
  // Alternatively we could copy the temp vars?
  computeAreaInit();
  mCoordTransform = new QgsCoordinateTransform( origDA.mCoordTransform->sourceCrs(), origDA.mCoordTransform->destCRS() );
}

void QgsDistanceArea::setEllipsoidalMode( bool flag )
{
  mEllipsoidalMode = flag;
}

void QgsDistanceArea::setSourceCrs( long srsid )
{
  QgsCoordinateReferenceSystem srcCRS;
  srcCRS.createFromSrsId( srsid );
  mCoordTransform->setSourceCrs( srcCRS );
}

void QgsDistanceArea::setSourceCrs( const QgsCoordinateReferenceSystem& srcCRS )
{
  mCoordTransform->setSourceCrs( srcCRS );
}

void QgsDistanceArea::setSourceAuthId( QString authId )
{
  QgsCoordinateReferenceSystem srcCRS;
  srcCRS.createFromOgcWmsCrs( authId );
  mCoordTransform->setSourceCrs( srcCRS );
}

bool QgsDistanceArea::setEllipsoid( const QString& ellipsoid )
{
  QString radius, parameter2;
  //
  // SQLITE3 stuff - get parameters for selected ellipsoid
  //
  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;

  // Shortcut if ellipsoid is none.
  if ( ellipsoid == GEO_NONE )
  {
    mEllipsoid = GEO_NONE;
    return true;
  }

  // Check if we have a custom projection, and set from text string.
  // Format is "PARAMETER:<semi-major axis>:<semi minor axis>
  // Numbers must be with (optional) decimal point and no other separators (C locale)
  // Distances in meters.  Flattening is calculated.
  if ( ellipsoid.startsWith( "PARAMETER" ) )
  {
    QStringList paramList = ellipsoid.split( ":" );
    bool semiMajorOk, semiMinorOk;
    double semiMajor = paramList[1].toDouble( & semiMajorOk );
    double semiMinor = paramList[2].toDouble( & semiMinorOk );
    if ( semiMajorOk && semiMinorOk )
    {
      return setEllipsoid( semiMajor, semiMinor );
    }
    else
    {
      return false;
    }
  }

  // Continue with PROJ.4 list of ellipsoids.

  //check the db is available
  myResult = sqlite3_open_v2( QgsApplication::srsDbFilePath().toUtf8().data(), &myDatabase, SQLITE_OPEN_READONLY, NULL );
  if ( myResult )
  {
    QgsMessageLog::logMessage( QObject::tr( "Can't open database: %1" ).arg( sqlite3_errmsg( myDatabase ) ) );
    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist.
    return false;
  }
  // Set up the query to retrieve the projection information needed to populate the ELLIPSOID list
  QString mySql = "select radius, parameter2 from tbl_ellipsoid where acronym='" + ellipsoid + "'";
  myResult = sqlite3_prepare( myDatabase, mySql.toUtf8(), mySql.toUtf8().length(), &myPreparedStatement, &myTail );
  // XXX Need to free memory from the error msg if one is set
  if ( myResult == SQLITE_OK )
  {
    if ( sqlite3_step( myPreparedStatement ) == SQLITE_ROW )
    {
      radius = QString(( char * )sqlite3_column_text( myPreparedStatement, 0 ) );
      parameter2 = QString(( char * )sqlite3_column_text( myPreparedStatement, 1 ) );
    }
  }
  // close the sqlite3 statement
  sqlite3_finalize( myPreparedStatement );
  sqlite3_close( myDatabase );

  // row for this ellipsoid wasn't found?
  if ( radius.isEmpty() || parameter2.isEmpty() )
  {
    QgsDebugMsg( QString( "setEllipsoid: no row in tbl_ellipsoid for acronym '%1'" ).arg( ellipsoid ) );
    return false;
  }

  // get major semiaxis
  if ( radius.left( 2 ) == "a=" )
    mSemiMajor = radius.mid( 2 ).toDouble();
  else
  {
    QgsDebugMsg( QString( "setEllipsoid: wrong format of radius field: '%1'" ).arg( radius ) );
    return false;
  }

  // get second parameter
  // one of values 'b' or 'f' is in field parameter2
  // second one must be computed using formula: invf = a/(a-b)
  if ( parameter2.left( 2 ) == "b=" )
  {
    mSemiMinor = parameter2.mid( 2 ).toDouble();
    mInvFlattening = mSemiMajor / ( mSemiMajor - mSemiMinor );
  }
  else if ( parameter2.left( 3 ) == "rf=" )
  {
    mInvFlattening = parameter2.mid( 3 ).toDouble();
    mSemiMinor = mSemiMajor - ( mSemiMajor / mInvFlattening );
  }
  else
  {
    QgsDebugMsg( QString( "setEllipsoid: wrong format of parameter2 field: '%1'" ).arg( parameter2 ) );
    return false;
  }

  QgsDebugMsg( QString( "setEllipsoid: a=%1, b=%2, 1/f=%3" ).arg( mSemiMajor ).arg( mSemiMinor ).arg( mInvFlattening ) );


  // get spatial ref system for ellipsoid
  QString proj4 = "+proj=longlat +ellps=" + ellipsoid + " +no_defs";
  QgsCoordinateReferenceSystem destCRS;
  destCRS.createFromProj4( proj4 );
  //TODO: createFromProj4 used to save to the user database any new CRS
  // this behavior was changed in order to separate creation and saving.
  // Not sure if it necessary to save it here, should be checked by someone
  // familiar with the code (should also give a more descriptive name to the generated CRS)
  if ( destCRS.srsid() == 0 )
  {
    QString myName = QString( " * %1 (%2)" )
                     .arg( QObject::tr( "Generated CRS", "A CRS automatically generated from layer info get this prefix for description" ) )
                     .arg( destCRS.toProj4() );
    destCRS.saveAsUserCRS( myName );
  }
  //

  // set transformation from project CRS to ellipsoid coordinates
  mCoordTransform->setDestCRS( destCRS );

  mEllipsoid = ellipsoid;

  // precalculate some values for area calculations
  computeAreaInit();

  return true;
}

//! Sets ellipsoid by supplied radii
// Inverse flattening is calculated with invf = a/(a-b)
// Also, b = a-(a/invf)
bool  QgsDistanceArea::setEllipsoid( double semiMajor, double semiMinor )
{
  mEllipsoid = QString( "PARAMETER:%1:%2" ).arg( semiMajor ).arg( semiMinor );
  mSemiMajor = semiMajor;
  mSemiMinor = semiMinor;
  mInvFlattening = mSemiMajor / ( mSemiMajor - mSemiMinor );

  computeAreaInit();

  return true;
}

double QgsDistanceArea::measure( const QgsAbstractGeometryV2* geomV2 ) const
{
  if ( !geomV2 )
  {
    return 0.0;
  }

  int geomDimension = geomV2->dimension();
  if ( geomDimension <= 0 )
  {
    return 0.0;
  }

  if ( !mEllipsoidalMode )
  {
    if ( geomDimension == 1 )
    {
      return geomV2->length();
    }
    else
    {
      return geomV2->area();
    }
  }
  else
  {
    //multigeom is sum of measured parts
    const QgsGeometryCollectionV2* collection = dynamic_cast<const QgsGeometryCollectionV2*>( geomV2 );
    if ( collection )
    {
      double sum = 0;
      for ( int i = 0; i < collection->numGeometries(); ++i )
      {
        sum += measure( collection->geometryN( i ) );
      }
      return sum;
    }

    if ( geomDimension == 1 )
    {
      const QgsCurveV2* curve = dynamic_cast<const QgsCurveV2*>( geomV2 );
      if ( !curve )
      {
        return 0.0;
      }

      QgsLineStringV2* lineString = curve->curveToLine();
      double length = measureLine( lineString );
      delete lineString;
      return length;
    }
    else
    {
      const QgsSurfaceV2* surface = dynamic_cast<const QgsSurfaceV2*>( geomV2 );
      QgsPolygonV2* polygon = surface->surfaceToPolygon();

      double area = 0;
      const QgsCurveV2* outerRing = polygon->exteriorRing();
      area += measurePolygon( outerRing );

      for ( int i = 0; i < polygon->numInteriorRings(); ++i )
      {
        const QgsCurveV2* innerRing = polygon->interiorRing( i );
        area -= measurePolygon( innerRing );
      }
      delete polygon;
      return area;
    }
  }
  return 0.0;
}

double QgsDistanceArea::measure( const QgsGeometry *geometry ) const
{
  if ( !geometry )
    return 0.0;

  const QgsAbstractGeometryV2* geomV2 = geometry->geometry();
  return measure( geomV2 );
}

double QgsDistanceArea::measurePerimeter( const QgsGeometry* geometry ) const
{
  if ( !geometry )
    return 0.0;

  const QgsAbstractGeometryV2* geomV2 = geometry->geometry();
  if ( !geomV2 || geomV2->dimension() < 2 )
  {
    return 0.0;
  }

  if ( !mEllipsoidalMode )
  {
    return geomV2->length();
  }

  //create list with (single) surfaces
  QList< const QgsSurfaceV2* > surfaces;
  const QgsSurfaceV2* surf = dynamic_cast<const QgsSurfaceV2*>( geomV2 );
  if ( surf )
  {
    surfaces.append( surf );
  }
  const QgsMultiSurfaceV2* multiSurf = dynamic_cast<const QgsMultiSurfaceV2*>( geomV2 );
  if ( multiSurf )
  {
    for ( int i = 0; i  < multiSurf->numGeometries(); ++i )
    {
      surfaces.append( static_cast<const QgsSurfaceV2*>( multiSurf->geometryN( i ) ) );
    }
  }

  double length = 0;
  QList<QgsPointV2> pointList;
  QList<const QgsSurfaceV2*>::const_iterator surfaceIt = surfaces.constBegin();
  for ( ; surfaceIt != surfaces.constEnd(); ++surfaceIt )
  {
    if ( !*surfaceIt )
    {
      continue;
    }

    QgsPolygonV2* poly = ( *surfaceIt )->surfaceToPolygon();
    const QgsCurveV2* outerRing = poly->exteriorRing();
    if ( outerRing )
    {
      length += measure( outerRing );
    }
    int nInnerRings = poly->numInteriorRings();
    for ( int i = 0; i < nInnerRings; ++i )
    {
      length += measure( poly->interiorRing( i ) );
    }
    delete poly;
  }
  return length;
}

double QgsDistanceArea::measureLine( const QgsCurveV2* curve ) const
{
  if ( !curve )
  {
    return 0.0;
  }

  QList<QgsPointV2> linePointsV2;
  QList<QgsPoint> linePoints;
  curve->points( linePointsV2 );
  QgsGeometry::convertPointList( linePointsV2, linePoints );
  return measureLine( linePoints );
}

double QgsDistanceArea::measureLine( const QList<QgsPoint> &points ) const
{
  if ( points.size() < 2 )
    return 0;

  double total = 0;
  QgsPoint p1, p2;

  try
  {
    if ( mEllipsoidalMode && ( mEllipsoid != GEO_NONE ) )
      p1 = mCoordTransform->transform( points[0] );
    else
      p1 = points[0];

    for ( QList<QgsPoint>::const_iterator i = points.begin(); i != points.end(); ++i )
    {
      if ( mEllipsoidalMode && ( mEllipsoid != GEO_NONE ) )
      {
        p2 = mCoordTransform->transform( *i );
        total += computeDistanceBearing( p1, p2 );
      }
      else
      {
        p2 = *i;
        total += measureLine( p1, p2 );
      }

      p1 = p2;
    }

    return total;
  }
  catch ( QgsCsException &cse )
  {
    Q_UNUSED( cse );
    QgsMessageLog::logMessage( QObject::tr( "Caught a coordinate system exception while trying to transform a point. Unable to calculate line length." ) );
    return 0.0;
  }

}

double QgsDistanceArea::measureLine( const QgsPoint &p1, const QgsPoint &p2 ) const
{
  double result;

  try
  {
    QgsPoint pp1 = p1, pp2 = p2;

    QgsDebugMsgLevel( QString( "Measuring from %1 to %2" ).arg( p1.toString( 4 ) ).arg( p2.toString( 4 ) ), 3 );
    if ( mEllipsoidalMode && ( mEllipsoid != GEO_NONE ) )
    {
      QgsDebugMsgLevel( QString( "Ellipsoidal calculations is enabled, using ellipsoid %1" ).arg( mEllipsoid ), 4 );
      QgsDebugMsgLevel( QString( "From proj4 : %1" ).arg( mCoordTransform->sourceCrs().toProj4() ), 4 );
      QgsDebugMsgLevel( QString( "To   proj4 : %1" ).arg( mCoordTransform->destCRS().toProj4() ), 4 );
      pp1 = mCoordTransform->transform( p1 );
      pp2 = mCoordTransform->transform( p2 );
      QgsDebugMsgLevel( QString( "New points are %1 and %2, calculating..." ).arg( pp1.toString( 4 ) ).arg( pp2.toString( 4 ) ), 4 );
      result = computeDistanceBearing( pp1, pp2 );
    }
    else
    {
      QgsDebugMsgLevel( "Cartesian calculation on canvas coordinates", 4 );
      result = computeDistanceFlat( p1, p2 );
    }
  }
  catch ( QgsCsException &cse )
  {
    Q_UNUSED( cse );
    QgsMessageLog::logMessage( QObject::tr( "Caught a coordinate system exception while trying to transform a point. Unable to calculate line length." ) );
    result = 0.0;
  }
  QgsDebugMsgLevel( QString( "The result was %1" ).arg( result ), 3 );
  return result;
}


const unsigned char *QgsDistanceArea::measurePolygon( const unsigned char* feature, double* area, double* perimeter, bool hasZptr ) const
{
  if ( !feature )
  {
    QgsDebugMsg( "no feature to measure" );
    return 0;
  }

  QgsConstWkbPtr wkbPtr( feature + 1 + sizeof( int ) );

  // get number of rings in the polygon
  int numRings;
  wkbPtr >> numRings;

  if ( numRings == 0 )
  {
    QgsDebugMsg( "no rings to measure" );
    return 0;
  }

  // Set pointer to the first ring
  QList<QgsPoint> points;
  QgsPoint pnt;
  double x, y;
  if ( area )
    *area = 0;
  if ( perimeter )
    *perimeter = 0;

  try
  {
    for ( int idx = 0; idx < numRings; idx++ )
    {
      int nPoints;
      wkbPtr >> nPoints;

      // Extract the points from the WKB and store in a pair of
      // vectors.
      for ( int jdx = 0; jdx < nPoints; jdx++ )
      {
        wkbPtr >> x >> y;
        if ( hasZptr )
        {
          // totally ignore Z value
          wkbPtr += sizeof( double );
        }

        pnt = QgsPoint( x, y );

        if ( mEllipsoidalMode && ( mEllipsoid != GEO_NONE ) )
        {
          pnt = mCoordTransform->transform( pnt );
        }
        points.append( pnt );
      }

      if ( points.size() > 2 )
      {
        if ( area )
        {
          double areaTmp = computePolygonArea( points );
          if ( idx == 0 )
          {
            // exterior ring
            *area += areaTmp;
          }
          else
          {
            *area -= areaTmp; // interior rings
          }
        }

        if ( perimeter )
        {
          if ( idx == 0 )
          {
            // exterior ring
            *perimeter += computeDistance( points );
          }
        }
      }

      points.clear();
    }
  }
  catch ( QgsCsException &cse )
  {
    Q_UNUSED( cse );
    QgsMessageLog::logMessage( QObject::tr( "Caught a coordinate system exception while trying to transform a point. Unable to calculate polygon area or perimeter." ) );
  }

  return wkbPtr;
}

double QgsDistanceArea::measurePolygon( const QgsCurveV2* curve ) const
{
  if ( !curve )
  {
    return 0.0;
  }

  QList<QgsPointV2> linePointsV2;
  curve->points( linePointsV2 );

  QList<QgsPoint> linePoints;
  QList<QgsPointV2>::const_iterator ptIt = linePointsV2.constBegin();
  for ( ; ptIt != linePointsV2.constEnd(); ++ptIt )
  {
    linePoints.append( mCoordTransform->transform( QPoint( ptIt->x(), ptIt->y() ) ) );
  }

  return computePolygonArea( linePoints );
}


double QgsDistanceArea::measurePolygon( const QList<QgsPoint>& points ) const
{
  try
  {
    if ( mEllipsoidalMode && ( mEllipsoid != GEO_NONE ) )
    {
      QList<QgsPoint> pts;
      for ( QList<QgsPoint>::const_iterator i = points.begin(); i != points.end(); ++i )
      {
        pts.append( mCoordTransform->transform( *i ) );
      }
      return computePolygonArea( pts );
    }
    else
    {
      return computePolygonArea( points );
    }
  }
  catch ( QgsCsException &cse )
  {
    Q_UNUSED( cse );
    QgsMessageLog::logMessage( QObject::tr( "Caught a coordinate system exception while trying to transform a point. Unable to calculate polygon area." ) );
    return 0.0;
  }
}


double QgsDistanceArea::bearing( const QgsPoint& p1, const QgsPoint& p2 ) const
{
  QgsPoint pp1 = p1, pp2 = p2;
  double bearing;

  if ( mEllipsoidalMode && ( mEllipsoid != GEO_NONE ) )
  {
    pp1 = mCoordTransform->transform( p1 );
    pp2 = mCoordTransform->transform( p2 );
    computeDistanceBearing( pp1, pp2, &bearing );
  }
  else //compute simple planar azimuth
  {
    double dx = p2.x() - p1.x();
    double dy = p2.y() - p1.y();
    bearing = atan2( dx, dy );
  }

  return bearing;
}


///////////////////////////////////////////////////////////
// distance calculation

double QgsDistanceArea::computeDistanceBearing(
  const QgsPoint& p1, const QgsPoint& p2,
  double* course1, double* course2 ) const
{
  if ( p1.x() == p2.x() && p1.y() == p2.y() )
    return 0;

  // ellipsoid
  double a = mSemiMajor;
  double b = mSemiMinor;
  double f = 1 / mInvFlattening;

  double p1_lat = DEG2RAD( p1.y() ), p1_lon = DEG2RAD( p1.x() );
  double p2_lat = DEG2RAD( p2.y() ), p2_lon = DEG2RAD( p2.x() );

  double L = p2_lon - p1_lon;
  double U1 = atan(( 1 - f ) * tan( p1_lat ) );
  double U2 = atan(( 1 - f ) * tan( p2_lat ) );
  double sinU1 = sin( U1 ), cosU1 = cos( U1 );
  double sinU2 = sin( U2 ), cosU2 = cos( U2 );
  double lambda = L;
  double lambdaP = 2 * M_PI;

  double sinLambda = 0;
  double cosLambda = 0;
  double sinSigma = 0;
  double cosSigma = 0;
  double sigma = 0;
  double alpha = 0;
  double cosSqAlpha = 0;
  double cos2SigmaM = 0;
  double C = 0;
  double tu1 = 0;
  double tu2 = 0;

  int iterLimit = 20;
  while ( qAbs( lambda - lambdaP ) > 1e-12 && --iterLimit > 0 )
  {
    sinLambda = sin( lambda );
    cosLambda = cos( lambda );
    tu1 = ( cosU2 * sinLambda );
    tu2 = ( cosU1 * sinU2 - sinU1 * cosU2 * cosLambda );
    sinSigma = sqrt( tu1 * tu1 + tu2 * tu2 );
    cosSigma = sinU1 * sinU2 + cosU1 * cosU2 * cosLambda;
    sigma = atan2( sinSigma, cosSigma );
    alpha = asin( cosU1 * cosU2 * sinLambda / sinSigma );
    cosSqAlpha = cos( alpha ) * cos( alpha );
    cos2SigmaM = cosSigma - 2 * sinU1 * sinU2 / cosSqAlpha;
    C = f / 16 * cosSqAlpha * ( 4 + f * ( 4 - 3 * cosSqAlpha ) );
    lambdaP = lambda;
    lambda = L + ( 1 - C ) * f * sin( alpha ) *
             ( sigma + C * sinSigma * ( cos2SigmaM + C * cosSigma * ( -1 + 2 * cos2SigmaM * cos2SigmaM ) ) );
  }

  if ( iterLimit == 0 )
    return -1;  // formula failed to converge

  double uSq = cosSqAlpha * ( a * a - b * b ) / ( b * b );
  double A = 1 + uSq / 16384 * ( 4096 + uSq * ( -768 + uSq * ( 320 - 175 * uSq ) ) );
  double B = uSq / 1024 * ( 256 + uSq * ( -128 + uSq * ( 74 - 47 * uSq ) ) );
  double deltaSigma = B * sinSigma * ( cos2SigmaM + B / 4 * ( cosSigma * ( -1 + 2 * cos2SigmaM * cos2SigmaM ) -
                                       B / 6 * cos2SigmaM * ( -3 + 4 * sinSigma * sinSigma ) * ( -3 + 4 * cos2SigmaM * cos2SigmaM ) ) );
  double s = b * A * ( sigma - deltaSigma );

  if ( course1 )
  {
    *course1 = atan2( tu1, tu2 );
  }
  if ( course2 )
  {
    // PI is added to return azimuth from P2 to P1
    *course2 = atan2( cosU1 * sinLambda, -sinU1 * cosU2 + cosU1 * sinU2 * cosLambda ) + M_PI;
  }

  return s;
}

double QgsDistanceArea::computeDistanceFlat( const QgsPoint& p1, const QgsPoint& p2 ) const
{
  return sqrt(( p2.x() - p1.x() ) * ( p2.x() - p1.x() ) + ( p2.y() - p1.y() ) * ( p2.y() - p1.y() ) );
}

double QgsDistanceArea::computeDistance( const QList<QgsPoint>& points ) const
{
  if ( points.size() < 2 )
    return 0;

  double total = 0;
  QgsPoint p1, p2;

  try
  {
    p1 = points[0];

    for ( QList<QgsPoint>::const_iterator i = points.begin(); i != points.end(); ++i )
    {
      p2 = *i;
      if ( mEllipsoidalMode && ( mEllipsoid != GEO_NONE ) )
      {
        total += computeDistanceBearing( p1, p2 );
      }
      else
      {
        total += computeDistanceFlat( p1, p2 );
      }

      p1 = p2;
    }

    return total;
  }
  catch ( QgsCsException &cse )
  {
    Q_UNUSED( cse );
    QgsMessageLog::logMessage( QObject::tr( "Caught a coordinate system exception while trying to transform a point. Unable to calculate line length." ) );
    return 0.0;
  }
}



///////////////////////////////////////////////////////////
// stuff for measuring areas - copied from GRASS
// don't know how does it work, but it's working .)
// see G_begin_ellipsoid_polygon_area() in area_poly1.c

double QgsDistanceArea::getQ( double x ) const
{
  double sinx, sinx2;

  sinx = sin( x );
  sinx2 = sinx * sinx;

  return sinx *( 1 + sinx2 *( m_QA + sinx2 *( m_QB + sinx2 * m_QC ) ) );
}


double QgsDistanceArea::getQbar( double x ) const
{
  double cosx, cosx2;

  cosx = cos( x );
  cosx2 = cosx * cosx;

  return cosx *( m_QbarA + cosx2 *( m_QbarB + cosx2 *( m_QbarC + cosx2 * m_QbarD ) ) );
}


void QgsDistanceArea::computeAreaInit()
{
  //don't try to perform calculations if no ellipsoid
  if ( mEllipsoid == GEO_NONE )
  {
    return;
  }

  double a2 = ( mSemiMajor * mSemiMajor );
  double e2 = 1 - ( a2 / ( mSemiMinor * mSemiMinor ) );
  double e4, e6;

  m_TwoPI = M_PI + M_PI;

  e4 = e2 * e2;
  e6 = e4 * e2;

  m_AE = a2 * ( 1 - e2 );

  m_QA = ( 2.0 / 3.0 ) * e2;
  m_QB = ( 3.0 / 5.0 ) * e4;
  m_QC = ( 4.0 / 7.0 ) * e6;

  m_QbarA = -1.0 - ( 2.0 / 3.0 ) * e2 - ( 3.0 / 5.0 ) * e4  - ( 4.0 / 7.0 ) * e6;
  m_QbarB = ( 2.0 / 9.0 ) * e2 + ( 2.0 / 5.0 ) * e4  + ( 4.0 / 7.0 ) * e6;
  m_QbarC =                     - ( 3.0 / 25.0 ) * e4 - ( 12.0 / 35.0 ) * e6;
  m_QbarD = ( 4.0 / 49.0 ) * e6;

  m_Qp = getQ( M_PI / 2 );
  m_E  = 4 * M_PI * m_Qp * m_AE;
  if ( m_E < 0.0 )
    m_E = -m_E;
}


double QgsDistanceArea::computePolygonArea( const QList<QgsPoint>& points ) const
{
  double x1, y1, x2, y2, dx, dy;
  double Qbar1, Qbar2;
  double area;

  QgsDebugMsgLevel( "Ellipsoid: " + mEllipsoid, 3 );
  if (( ! mEllipsoidalMode ) || ( mEllipsoid == GEO_NONE ) )
  {
    return computePolygonFlatArea( points );
  }
  int n = points.size();
  x2 = DEG2RAD( points[n-1].x() );
  y2 = DEG2RAD( points[n-1].y() );
  Qbar2 = getQbar( y2 );

  area = 0.0;

  for ( int i = 0; i < n; i++ )
  {
    x1 = x2;
    y1 = y2;
    Qbar1 = Qbar2;

    x2 = DEG2RAD( points[i].x() );
    y2 = DEG2RAD( points[i].y() );
    Qbar2 = getQbar( y2 );

    if ( x1 > x2 )
      while ( x1 - x2 > M_PI )
        x2 += m_TwoPI;
    else if ( x2 > x1 )
      while ( x2 - x1 > M_PI )
        x1 += m_TwoPI;

    dx = x2 - x1;
    area += dx * ( m_Qp - getQ( y2 ) );

    if (( dy = y2 - y1 ) != 0.0 )
      area += dx * getQ( y2 ) - ( dx / dy ) * ( Qbar2 - Qbar1 );
  }
  if (( area *= m_AE ) < 0.0 )
    area = -area;

  /* kludge - if polygon circles the south pole the area will be
  * computed as if it cirlced the north pole. The correction is
  * the difference between total surface area of the earth and
  * the "north pole" area.
  */
  if ( area > m_E )
    area = m_E;
  if ( area > m_E / 2 )
    area = m_E - area;

  return area;
}

double QgsDistanceArea::computePolygonFlatArea( const QList<QgsPoint>& points ) const
{
  // Normal plane area calculations.
  double area = 0.0;
  int i, size;

  size = points.size();

  // QgsDebugMsg("New area calc, nr of points: " + QString::number(size));
  for ( i = 0; i < size; i++ )
  {
    // QgsDebugMsg("Area from point: " + (points[i]).toString(2));
    // Using '% size', so that we always end with the starting point
    // and thus close the polygon.
    area = area + points[i].x() * points[( i+1 ) % size].y() - points[( i+1 ) % size].x() * points[i].y();
  }
  // QgsDebugMsg("Area from point: " + (points[i % size]).toString(2));
  area = area / 2.0;
  return qAbs( area ); // All areas are positive!
}

QString QgsDistanceArea::textUnit( double value, int decimals, QGis::UnitType u, bool isArea, bool keepBaseUnit )
{
  QString unitLabel;

  switch ( u )
  {
    case QGis::Meters:
      if ( isArea )
      {
        if ( keepBaseUnit )
        {
          unitLabel = QObject::trUtf8( " m²" );
        }
        else if ( qAbs( value ) > 1000000.0 )
        {
          unitLabel = QObject::trUtf8( " km²" );
          value = value / 1000000.0;
        }
        else if ( qAbs( value ) > 10000.0 )
        {
          unitLabel = QObject::tr( " ha" );
          value = value / 10000.0;
        }
        else
        {
          unitLabel = QObject::trUtf8( " m²" );
        }
      }
      else
      {
        if ( keepBaseUnit || qAbs( value ) == 0.0 )
        {
          unitLabel = QObject::tr( " m" );
        }
        else if ( qAbs( value ) > 1000.0 )
        {
          unitLabel = QObject::tr( " km" );
          value = value / 1000;
        }
        else if ( qAbs( value ) < 0.01 )
        {
          unitLabel = QObject::tr( " mm" );
          value = value * 1000;
        }
        else if ( qAbs( value ) < 0.1 )
        {
          unitLabel = QObject::tr( " cm" );
          value = value * 100;
        }
        else
        {
          unitLabel = QObject::tr( " m" );
        }
      }
      break;
    case QGis::Feet:
      if ( isArea )
      {
        if ( keepBaseUnit  || qAbs( value ) <= 0.5*43560.0 )
        {
          // < 0.5 acre show sq ft
          unitLabel = QObject::tr( " sq ft" );
        }
        else if ( qAbs( value ) <= 0.5*5280.0*5280.0 )
        {
          // < 0.5 sq mile show acre
          unitLabel = QObject::tr( " acres" );
          value /= 43560.0;
        }
        else
        {
          // above 0.5 acre show sq mi
          unitLabel = QObject::tr( " sq mile" );
          value /= 5280.0 * 5280.0;
        }
      }
      else
      {
        if ( qAbs( value ) <= 528.0 || keepBaseUnit )
        {
          if ( qAbs( value ) == 1.0 )
          {
            unitLabel = QObject::tr( " foot" );
          }
          else
          {
            unitLabel = QObject::tr( " feet" );
          }
        }
        else
        {
          unitLabel = QObject::tr( " mile" );
          value /= 5280.0;
        }
      }
      break;
    case QGis::NauticalMiles:
      if ( isArea )
      {
        unitLabel = QObject::tr( " sq. NM" );
      }
      else
      {
        unitLabel = QObject::tr( " NM" );
      }
      break;
    case QGis::Degrees:
      if ( isArea )
      {
        unitLabel = QObject::tr( " sq.deg." );
      }
      else
      {
        if ( qAbs( value ) == 1.0 )
          unitLabel = QObject::tr( " degree" );
        else
          unitLabel = QObject::tr( " degrees" );
      }
      break;
    case QGis::UnknownUnit:
      unitLabel = QObject::tr( " unknown" );
      //intentional fall-through
    default:
      QgsDebugMsg( QString( "Error: not picked up map units - actual value = %1" ).arg( u ) );
  }

  return QLocale::system().toString( value, 'f', decimals ) + unitLabel;
}

void QgsDistanceArea::convertMeasurement( double &measure, QGis::UnitType &measureUnits, QGis::UnitType displayUnits, bool isArea ) const
{
  // Helper for converting between meters and feet and degrees and NauticalMiles...
  // The parameters measure and measureUnits are in/out

  if (( measureUnits == QGis::Degrees || measureUnits == QGis::Feet || measureUnits == QGis::NauticalMiles ) &&
      mEllipsoid != GEO_NONE &&
      mEllipsoidalMode )
  {
    // Measuring on an ellipsoid returned meters. Force!
    measureUnits = QGis::Meters;
    QgsDebugMsg( "We're measuring on an ellipsoid or using projections, the system is returning meters" );
  }
  else if ( mEllipsoidalMode && mEllipsoid == GEO_NONE )
  {
    // Measuring in plane within the source CRS. Force its map units
    measureUnits = mCoordTransform->sourceCrs().mapUnits();
    QgsDebugMsg( "We're measuing on planimetric distance/area on given CRS, measured value is in CRS units" );
  }

  // Gets the conversion factor between the specified units
  double factorUnits = QGis::fromUnitToUnitFactor( measureUnits, displayUnits );
  if ( isArea )
    factorUnits *= factorUnits;

  QgsDebugMsg( QString( "Converting %1 %2" ).arg( QString::number( measure ), QGis::toLiteral( measureUnits ) ) );
  measure *= factorUnits;
  QgsDebugMsg( QString( "to %1 %2" ).arg( QString::number( measure ), QGis::toLiteral( displayUnits ) ) );
  measureUnits = displayUnits;
}

