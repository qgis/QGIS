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
#include <QString>
#include <QObject>

#include "qgsdistancearea.h"
#include "qgis.h"
#include "qgspointxy.h"
#include "qgscoordinatetransform.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsgeometry.h"
#include "qgsgeometrycollection.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsmultisurface.h"
#include "qgswkbptr.h"
#include "qgslinestring.h"
#include "qgspolygon.h"
#include "qgssurface.h"
#include "qgsunittypes.h"
#include "qgsexception.h"
#include "qgsmultilinestring.h"

#include <geodesic.h>

#define DEG2RAD(x)    ((x)*M_PI/180)
#define RAD2DEG(r) (180.0 * (r) / M_PI)
#define POW2(x) ((x)*(x))

QgsDistanceArea::QgsDistanceArea()
{
  // init with default settings
  mSemiMajor = -1.0;
  mSemiMinor = -1.0;
  mInvFlattening = -1.0;
  const QgsCoordinateTransformContext context; // this is ok - by default we have a source/dest of WGS84, so no reprojection takes place
  setSourceCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ), context ); // WGS 84
  setEllipsoid( geoNone() );
}

QgsDistanceArea::~QgsDistanceArea() = default;

QgsDistanceArea::QgsDistanceArea( const QgsDistanceArea &other )
  : mCoordTransform( other.mCoordTransform )
  , mEllipsoid( other.mEllipsoid )
  , mSemiMajor( other.mSemiMajor )
  , mSemiMinor( other.mSemiMinor )
  , mInvFlattening( other.mInvFlattening )
{
  computeAreaInit();
}

QgsDistanceArea &QgsDistanceArea::operator=( const QgsDistanceArea &other )
{
  mCoordTransform = other.mCoordTransform;
  mEllipsoid = other.mEllipsoid;
  mSemiMajor = other.mSemiMajor;
  mSemiMinor = other.mSemiMinor;
  mInvFlattening = other.mInvFlattening;
  computeAreaInit();
  return *this;
}

bool QgsDistanceArea::willUseEllipsoid() const
{
  return mEllipsoid != geoNone();
}

void QgsDistanceArea::setSourceCrs( const QgsCoordinateReferenceSystem &srcCRS, const QgsCoordinateTransformContext &context )
{
  mCoordTransform.setContext( context );
  mCoordTransform.setSourceCrs( srcCRS );
}

bool QgsDistanceArea::setEllipsoid( const QString &ellipsoid )
{
  // Shortcut if ellipsoid is none.
  if ( ellipsoid == geoNone() )
  {
    mEllipsoid = geoNone();
    mGeod.reset();
    return true;
  }

  const QgsEllipsoidUtils::EllipsoidParameters params = QgsEllipsoidUtils::ellipsoidParameters( ellipsoid );
  if ( !params.valid )
  {
    mGeod.reset();
    return false;
  }
  else
  {
    mEllipsoid = ellipsoid;
    setFromParams( params );
    return true;
  }
}

// Inverse flattening is calculated with invf = a/(a-b)
// Also, b = a-(a/invf)
bool QgsDistanceArea::setEllipsoid( double semiMajor, double semiMinor )
{
  mEllipsoid = QStringLiteral( "PARAMETER:%1:%2" ).arg( qgsDoubleToString( semiMajor ), qgsDoubleToString( semiMinor ) );
  mSemiMajor = semiMajor;
  mSemiMinor = semiMinor;
  mInvFlattening = mSemiMajor / ( mSemiMajor - mSemiMinor );

  computeAreaInit();

  return true;
}

double QgsDistanceArea::measure( const QgsAbstractGeometry *geomV2, MeasureType type ) const
{
  if ( !geomV2 )
  {
    return 0.0;
  }

  const int geomDimension = geomV2->dimension();
  if ( geomDimension <= 0 )
  {
    return 0.0;
  }

  MeasureType measureType = type;
  if ( measureType == Default )
  {
    measureType = ( geomDimension == 1 ? Length : Area );
  }

  if ( !willUseEllipsoid() )
  {
    //no transform required
    if ( measureType == Length )
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
    const QgsGeometryCollection *collection = qgsgeometry_cast<const QgsGeometryCollection *>( geomV2 );
    if ( collection )
    {
      double sum = 0;
      for ( int i = 0; i < collection->numGeometries(); ++i )
      {
        sum += measure( collection->geometryN( i ), measureType );
      }
      return sum;
    }

    if ( measureType == Length )
    {
      const QgsCurve *curve = qgsgeometry_cast<const QgsCurve *>( geomV2 );
      if ( !curve )
      {
        return 0.0;
      }

      QgsLineString *lineString = curve->curveToLine();
      const double length = measureLine( lineString );
      delete lineString;
      return length;
    }
    else
    {
      const QgsSurface *surface = qgsgeometry_cast<const QgsSurface *>( geomV2 );
      if ( !surface )
        return 0.0;

      QgsPolygon *polygon = surface->surfaceToPolygon();

      double area = 0;
      const QgsCurve *outerRing = polygon->exteriorRing();
      area += measurePolygon( outerRing );

      for ( int i = 0; i < polygon->numInteriorRings(); ++i )
      {
        const QgsCurve *innerRing = polygon->interiorRing( i );
        area -= measurePolygon( innerRing );
      }
      delete polygon;
      return area;
    }
  }
}

double QgsDistanceArea::measureArea( const QgsGeometry &geometry ) const
{
  if ( geometry.isNull() )
    return 0.0;

  const QgsAbstractGeometry *geomV2 = geometry.constGet();
  return measure( geomV2, Area );
}

double QgsDistanceArea::measureLength( const QgsGeometry &geometry ) const
{
  if ( geometry.isNull() )
    return 0.0;

  const QgsAbstractGeometry *geomV2 = geometry.constGet();
  return measure( geomV2, Length );
}

double QgsDistanceArea::measurePerimeter( const QgsGeometry &geometry ) const
{
  if ( geometry.isNull() )
    return 0.0;

  const QgsAbstractGeometry *geomV2 = geometry.constGet();
  if ( !geomV2 || geomV2->dimension() < 2 )
  {
    return 0.0;
  }

  if ( !willUseEllipsoid() )
  {
    return geomV2->perimeter();
  }

  //create list with (single) surfaces
  QVector< const QgsSurface * > surfaces;
  const QgsSurface *surf = qgsgeometry_cast<const QgsSurface *>( geomV2 );
  if ( surf )
  {
    surfaces.append( surf );
  }
  const QgsMultiSurface *multiSurf = qgsgeometry_cast<const QgsMultiSurface *>( geomV2 );
  if ( multiSurf )
  {
    surfaces.reserve( ( surf ? 1 : 0 ) + multiSurf->numGeometries() );
    for ( int i = 0; i  < multiSurf->numGeometries(); ++i )
    {
      surfaces.append( static_cast<const QgsSurface *>( multiSurf->geometryN( i ) ) );
    }
  }

  double length = 0;
  QVector<const QgsSurface *>::const_iterator surfaceIt = surfaces.constBegin();
  for ( ; surfaceIt != surfaces.constEnd(); ++surfaceIt )
  {
    if ( !*surfaceIt )
    {
      continue;
    }

    QgsPolygon *poly = ( *surfaceIt )->surfaceToPolygon();
    const QgsCurve *outerRing = poly->exteriorRing();
    if ( outerRing )
    {
      length += measure( outerRing );
    }
    const int nInnerRings = poly->numInteriorRings();
    for ( int i = 0; i < nInnerRings; ++i )
    {
      length += measure( poly->interiorRing( i ) );
    }
    delete poly;
  }
  return length;
}

double QgsDistanceArea::measureLine( const QgsCurve *curve ) const
{
  if ( !curve )
  {
    return 0.0;
  }

  QgsPointSequence linePointsV2;
  QVector<QgsPointXY> linePoints;
  curve->points( linePointsV2 );
  QgsGeometry::convertPointList( linePointsV2, linePoints );
  return measureLine( linePoints );
}

double QgsDistanceArea::measureLine( const QVector<QgsPointXY> &points ) const
{
  if ( points.size() < 2 )
    return 0;

  double total = 0;
  QgsPointXY p1, p2;

  if ( willUseEllipsoid() )
  {
    if ( !mGeod )
      computeAreaInit();
    Q_ASSERT_X( static_cast<bool>( mGeod ), "QgsDistanceArea::measureLine()", "Error creating geod_geodesic object" );
    if ( !mGeod )
      return 0;
  }

  try
  {
    if ( willUseEllipsoid() )
      p1 = mCoordTransform.transform( points[0] );
    else
      p1 = points[0];

    for ( QVector<QgsPointXY>::const_iterator i = points.constBegin(); i != points.constEnd(); ++i )
    {
      if ( willUseEllipsoid() )
      {
        p2 = mCoordTransform.transform( *i );

        double distance = 0;
        double azimuth1 = 0;
        double azimuth2 = 0;
        geod_inverse( mGeod.get(), p1.y(), p1.x(), p2.y(), p2.x(), &distance, &azimuth1, &azimuth2 );
        total += distance;
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
    Q_UNUSED( cse )
    QgsMessageLog::logMessage( QObject::tr( "Caught a coordinate system exception while trying to transform a point. Unable to calculate line length." ) );
    return 0.0;
  }

}

double QgsDistanceArea::measureLine( const QgsPointXY &p1, const QgsPointXY &p2 ) const
{
  double result;

  if ( willUseEllipsoid() )
  {
    if ( !mGeod )
      computeAreaInit();
    Q_ASSERT_X( static_cast<bool>( mGeod ), "QgsDistanceArea::measureLine()", "Error creating geod_geodesic object" );
    if ( !mGeod )
      return 0;
  }

  try
  {
    QgsPointXY pp1 = p1, pp2 = p2;

    QgsDebugMsgLevel( QStringLiteral( "Measuring from %1 to %2" ).arg( p1.toString( 4 ), p2.toString( 4 ) ), 3 );
    if ( willUseEllipsoid() )
    {
      QgsDebugMsgLevel( QStringLiteral( "Ellipsoidal calculations is enabled, using ellipsoid %1" ).arg( mEllipsoid ), 4 );
      QgsDebugMsgLevel( QStringLiteral( "From proj4 : %1" ).arg( mCoordTransform.sourceCrs().toProj() ), 4 );
      QgsDebugMsgLevel( QStringLiteral( "To   proj4 : %1" ).arg( mCoordTransform.destinationCrs().toProj() ), 4 );
      pp1 = mCoordTransform.transform( p1 );
      pp2 = mCoordTransform.transform( p2 );
      QgsDebugMsgLevel( QStringLiteral( "New points are %1 and %2, calculating..." ).arg( pp1.toString( 4 ), pp2.toString( 4 ) ), 4 );

      double azimuth1 = 0;
      double azimuth2 = 0;
      geod_inverse( mGeod.get(), pp1.y(), pp1.x(), pp2.y(), pp2.x(), &result, &azimuth1, &azimuth2 );
    }
    else
    {
      QgsDebugMsgLevel( QStringLiteral( "Cartesian calculation on canvas coordinates" ), 4 );
      result = p2.distance( p1 );
    }
  }
  catch ( QgsCsException &cse )
  {
    Q_UNUSED( cse )
    QgsMessageLog::logMessage( QObject::tr( "Caught a coordinate system exception while trying to transform a point. Unable to calculate line length." ) );
    result = 0.0;
  }
  QgsDebugMsgLevel( QStringLiteral( "The result was %1" ).arg( result ), 3 );
  return result;
}

double QgsDistanceArea::measureLineProjected( const QgsPointXY &p1, double distance, double azimuth, QgsPointXY *projectedPoint ) const
{
  double result = 0.0;
  QgsPointXY p2;
  if ( mCoordTransform.sourceCrs().isGeographic() && willUseEllipsoid() )
  {
    p2 = computeSpheroidProject( p1, distance, azimuth );
    result = p1.distance( p2 );
  }
  else // Cartesian coordinates
  {
    result = distance; // Avoid rounding errors when using meters [return as sent]
    if ( sourceCrs().mapUnits() != QgsUnitTypes::DistanceMeters )
    {
      distance = ( distance * QgsUnitTypes::fromUnitToUnitFactor( QgsUnitTypes::DistanceMeters, sourceCrs().mapUnits() ) );
      result = p1.distance( p2 );
    }
    p2 = p1.project( distance, azimuth );
  }
  QgsDebugMsgLevel( QStringLiteral( "Converted distance of %1 %2 to %3 distance %4 %5, using azimuth[%6] from point[%7] to point[%8] sourceCrs[%9] mEllipsoid[%10] isGeographic[%11] [%12]" )
                    .arg( QString::number( distance, 'f', 7 ),
                          QgsUnitTypes::toString( QgsUnitTypes::DistanceMeters ),
                          QString::number( result, 'f', 7 ),
                          mCoordTransform.sourceCrs().isGeographic() ? QStringLiteral( "Geographic" ) : QStringLiteral( "Cartesian" ),
                          QgsUnitTypes::toString( sourceCrs().mapUnits() ) )
                    .arg( azimuth )
                    .arg( p1.asWkt(),
                          p2.asWkt(),
                          sourceCrs().description(),
                          mEllipsoid )
                    .arg( sourceCrs().isGeographic() )
                    .arg( QStringLiteral( "SemiMajor[%1] SemiMinor[%2] InvFlattening[%3] " ).arg( QString::number( mSemiMajor, 'f', 7 ), QString::number( mSemiMinor, 'f', 7 ), QString::number( mInvFlattening, 'f', 7 ) ) ), 4 );
  if ( projectedPoint )
  {
    *projectedPoint = QgsPointXY( p2 );
  }
  return result;
}

QgsPointXY QgsDistanceArea::computeSpheroidProject(
  const QgsPointXY &p1, double distance, double azimuth ) const
{
  if ( !mGeod )
    computeAreaInit();
  if ( !mGeod )
    return QgsPointXY();

  double lat2 = 0;
  double lon2 = 0;
  double azimuth2 = 0;
  geod_direct( mGeod.get(), p1.y(), p1.x(), RAD2DEG( azimuth ), distance, &lat2, &lon2, &azimuth2 );

  return QgsPointXY( lon2, lat2 );
}

double QgsDistanceArea::latitudeGeodesicCrossesAntimeridian( const QgsPointXY &pp1, const QgsPointXY &pp2, double &fractionAlongLine ) const
{
  QgsPointXY p1 = pp1;
  QgsPointXY p2 = pp2;
  if ( p1.x() < -120 )
    p1.setX( p1.x() + 360 );
  if ( p2.x() < -120 )
    p2.setX( p2.x() + 360 );

  // we need p2.x() > 180 and p1.x() < 180
  double p1x = p1.x() < 180 ? p1.x() : p2.x();
  double p1y = p1.x() < 180 ? p1.y() : p2.y();
  double p2x = p1.x() < 180 ? p2.x() : p1.x();
  double p2y = p1.x() < 180 ? p2.y() : p1.y();
  // lat/lon are our candidate intersection position - we want this to get as close to 180 as possible
  // the first candidate is p2
  double lat = p2y;
  double lon = p2x;

  if ( mEllipsoid == geoNone() )
  {
    fractionAlongLine = ( 180 - p1x ) / ( p2x - p1x );
    if ( p1.x() >= 180 )
      fractionAlongLine = 1 - fractionAlongLine;
    return p1y + ( 180 - p1x ) / ( p2x - p1x ) * ( p2y - p1y );
  }

  if ( !mGeod )
    computeAreaInit();
  Q_ASSERT_X( static_cast<bool>( mGeod ), "QgsDistanceArea::latitudeGeodesicCrossesAntimeridian()", "Error creating geod_geodesic object" );
  if ( !mGeod )
    return 0;

  geod_geodesicline line;
  geod_inverseline( &line, mGeod.get(), p1y, p1x, p2y, p2x, GEOD_ALL );

  const double totalDist = line.s13;
  double intersectionDist = line.s13;

  int iterations = 0;
  double t = 0;
  // iterate until our intersection candidate is within ~1 mm of the antimeridian (or too many iterations happened)
  while ( std::fabs( lon - 180.0 ) > 0.00000001 && iterations < 100 )
  {
    if ( iterations > 0 && std::fabs( p2x - p1x ) > 5 )
    {
      // if we have too large a range of longitudes, we use a binary search to narrow the window -- this ensures we will converge
      if ( lon < 180 )
      {
        p1x = lon;
        p1y = lat;
      }
      else
      {
        p2x = lon;
        p2y = lat;
      }
      QgsDebugMsgLevel( QStringLiteral( "Narrowed window to %1, %2 - %3, %4" ).arg( p1x ).arg( p1y ).arg( p2x ).arg( p2y ), 4 );

      geod_inverseline( &line, mGeod.get(), p1y, p1x, p2y, p2x, GEOD_ALL );
      intersectionDist = line.s13 * 0.5;
    }
    else
    {
      // we have a sufficiently narrow window -- use Newton's method
      // adjust intersection distance by fraction of how close the previous candidate was to 180 degrees longitude -
      // this helps us close in to the correct longitude quickly
      intersectionDist *= ( 180.0 - p1x ) / ( lon - p1x );
    }

    // now work out the point on the geodesic this far from p1 - this becomes our new candidate for crossing the antimeridian

    geod_position( &line, intersectionDist, &lat, &lon, &t );
    // we don't want to wrap longitudes > 180 around)
    if ( lon < 0 )
      lon += 360;

    iterations++;
    QgsDebugMsgLevel( QStringLiteral( "After %1 iterations lon is %2, lat is %3, dist from p1: %4" ).arg( iterations ).arg( lon ).arg( lat ).arg( intersectionDist ), 4 );
  }

  fractionAlongLine = intersectionDist / totalDist;
  if ( p1.x() >= 180 )
    fractionAlongLine = 1 - fractionAlongLine;

  // either converged on 180 longitude or hit too many iterations
  return lat;
}

QgsGeometry QgsDistanceArea::splitGeometryAtAntimeridian( const QgsGeometry &geometry ) const
{
  if ( QgsWkbTypes::geometryType( geometry.wkbType() ) != QgsWkbTypes::LineGeometry )
    return geometry;

  QgsGeometry g = geometry;
  // TODO - avoid segmentization of curved geometries (if this is even possible!)
  if ( QgsWkbTypes::isCurvedType( g.wkbType() ) )
    g.convertToStraightSegment();

  std::unique_ptr< QgsMultiLineString > res = std::make_unique< QgsMultiLineString >();
  for ( auto part = g.const_parts_begin(); part != g.const_parts_end(); ++part )
  {
    const QgsLineString *line = qgsgeometry_cast< const QgsLineString * >( *part );
    if ( !line )
      continue;
    if ( line->isEmpty() )
    {
      continue;
    }

    const std::unique_ptr< QgsLineString > l = std::make_unique< QgsLineString >();
    try
    {
      double x = 0;
      double y = 0;
      double z = 0;
      double m = 0;
      QVector< QgsPoint > newPoints;
      newPoints.reserve( line->numPoints() );
      double prevLon = 0;
      double prevLat = 0;
      double lon = 0;
      double lat = 0;
      double prevZ = 0;
      double prevM = 0;
      for ( int i = 0; i < line->numPoints(); i++ )
      {
        QgsPoint p = line->pointN( i );
        x = p.x();
        if ( mCoordTransform.sourceCrs().isGeographic() )
        {
          x = std::fmod( x, 360.0 );
          if ( x > 180 )
            x -= 360;
          p.setX( x );
        }
        y = p.y();
        lon = x;
        lat = y;
        mCoordTransform.transformInPlace( lon, lat, z );

        //test if we crossed the antimeridian in this segment
        if ( i > 0 && ( ( prevLon < -120 && lon > 120 ) || ( prevLon > 120 && lon  < -120 ) ) )
        {
          // we did!
          // when crossing the antimeridian, we need to calculate the latitude
          // at which the geodesic intersects the antimeridian
          double fract = 0;
          const double lat180 = latitudeGeodesicCrossesAntimeridian( QgsPointXY( prevLon, prevLat ), QgsPointXY( lon, lat ), fract );
          if ( line->is3D() )
          {
            z = prevZ + ( p.z() - prevZ ) * fract;
          }
          if ( line->isMeasure() )
          {
            m = prevM + ( p.m() - prevM ) * fract;
          }

          QgsPointXY antiMeridianPoint;
          if ( prevLon < -120 )
            antiMeridianPoint = mCoordTransform.transform( QgsPointXY( -180, lat180 ), Qgis::TransformDirection::Reverse );
          else
            antiMeridianPoint = mCoordTransform.transform( QgsPointXY( 180, lat180 ), Qgis::TransformDirection::Reverse );

          QgsPoint newPoint( antiMeridianPoint );
          if ( line->is3D() )
            newPoint.addZValue( z );
          if ( line->isMeasure() )
            newPoint.addMValue( m );

          if ( std::isfinite( newPoint.x() ) && std::isfinite( newPoint.y() ) )
          {
            newPoints << newPoint;
          }
          res->addGeometry( new QgsLineString( newPoints ) );

          newPoints.clear();
          newPoints.reserve( line->numPoints() - i + 1 );

          if ( lon < -120 )
            antiMeridianPoint = mCoordTransform.transform( QgsPointXY( -180, lat180 ), Qgis::TransformDirection::Reverse );
          else
            antiMeridianPoint = mCoordTransform.transform( QgsPointXY( 180, lat180 ), Qgis::TransformDirection::Reverse );

          if ( std::isfinite( antiMeridianPoint.x() ) && std::isfinite( antiMeridianPoint.y() ) )
          {
            // we want to keep the previously calculated z/m value for newPoint, if present. They're the same each
            // of the antimeridian split
            newPoint.setX( antiMeridianPoint.x() );
            newPoint.setY( antiMeridianPoint.y() );
            newPoints << newPoint;
          }
        }
        newPoints << p;

        prevLon = lon;
        prevLat = lat;
        if ( line->is3D() )
          prevZ = p.z();
        if ( line->isMeasure() )
          prevM = p.m();
      }
      res->addGeometry( new QgsLineString( newPoints ) );
    }
    catch ( QgsCsException & )
    {
      QgsMessageLog::logMessage( QObject::tr( "Caught a coordinate system exception while trying to transform linestring. Unable to calculate break point." ) );
      res->addGeometry( line->clone() );
      break;
    }
  }

  return QgsGeometry( std::move( res ) );
}


QVector< QVector<QgsPointXY> > QgsDistanceArea::geodesicLine( const QgsPointXY &p1, const QgsPointXY &p2, const double interval, const bool breakLine ) const
{
  if ( !willUseEllipsoid() )
  {
    return QVector< QVector< QgsPointXY > >() << ( QVector< QgsPointXY >() << p1 << p2 );
  }

  if ( !mGeod )
    computeAreaInit();
  if ( !mGeod )
    return QVector< QVector< QgsPointXY > >();

  QgsPointXY pp1, pp2;
  try
  {
    pp1 = mCoordTransform.transform( p1 );
    pp2 = mCoordTransform.transform( p2 );
  }
  catch ( QgsCsException & )
  {
    QgsMessageLog::logMessage( QObject::tr( "Caught a coordinate system exception while trying to transform a point. Unable to calculate geodesic line." ) );
    return QVector< QVector< QgsPointXY > >();
  }

  geod_geodesicline line;
  geod_inverseline( &line, mGeod.get(), pp1.y(), pp1.x(), pp2.y(), pp2.x(), GEOD_ALL );
  const double totalDist = line.s13;

  QVector< QVector< QgsPointXY > > res;
  QVector< QgsPointXY > currentPart;
  currentPart << p1;
  double d = interval;
  double prevLon = pp1.x();
  double prevLat = pp1.y();
  bool lastRun = false;
  double t = 0;
  while ( true )
  {
    double lat, lon;
    if ( lastRun )
    {
      lat = pp2.y();
      lon = pp2.x();
      if ( lon > 180 )
        lon -= 360;
    }
    else
    {
      geod_position( &line, d, &lat, &lon, &t );
    }

    if ( breakLine && ( ( prevLon < -120 && lon > 120 ) || ( prevLon > 120 && lon < -120 ) ) )
    {
      // when breaking the geodesic at the antimeridian, we need to calculate the latitude
      // at which the geodesic intersects the antimeridian, and add points to both line segments at this latitude
      // on the antimeridian.
      double fraction;
      const double lat180 = latitudeGeodesicCrossesAntimeridian( QgsPointXY( prevLon, prevLat ), QgsPointXY( lon, lat ), fraction );

      try
      {
        QgsPointXY p;
        if ( prevLon < -120 )
          p = mCoordTransform.transform( QgsPointXY( -180, lat180 ), Qgis::TransformDirection::Reverse );
        else
          p = mCoordTransform.transform( QgsPointXY( 180, lat180 ), Qgis::TransformDirection::Reverse );

        if ( std::isfinite( p.x() ) && std::isfinite( p.y() ) )
          currentPart << p;
      }
      catch ( QgsCsException & )
      {
        QgsMessageLog::logMessage( QObject::tr( "Caught a coordinate system exception while trying to transform a point." ) );
      }

      res << currentPart;
      currentPart.clear();
      try
      {
        QgsPointXY p;
        if ( lon < -120 )
          p = mCoordTransform.transform( QgsPointXY( -180, lat180 ), Qgis::TransformDirection::Reverse );
        else
          p = mCoordTransform.transform( QgsPointXY( 180, lat180 ), Qgis::TransformDirection::Reverse );

        if ( std::isfinite( p.x() ) && std::isfinite( p.y() ) )
          currentPart << p;
      }
      catch ( QgsCsException & )
      {
        QgsMessageLog::logMessage( QObject::tr( "Caught a coordinate system exception while trying to transform a point." ) );
      }

    }

    prevLon = lon;
    prevLat = lat;

    try
    {
      currentPart << mCoordTransform.transform( QgsPointXY( lon, lat ), Qgis::TransformDirection::Reverse );
    }
    catch ( QgsCsException & )
    {
      QgsMessageLog::logMessage( QObject::tr( "Caught a coordinate system exception while trying to transform a point." ) );
    }

    if ( lastRun )
      break;

    d += interval;
    if ( d >= totalDist )
      lastRun = true;
  }
  res << currentPart;
  return res;
}

QgsUnitTypes::DistanceUnit QgsDistanceArea::lengthUnits() const
{
  return willUseEllipsoid() ? QgsUnitTypes::DistanceMeters : mCoordTransform.sourceCrs().mapUnits();
}

QgsUnitTypes::AreaUnit QgsDistanceArea::areaUnits() const
{
  return willUseEllipsoid() ? QgsUnitTypes::AreaSquareMeters :
         QgsUnitTypes::distanceToAreaUnit( mCoordTransform.sourceCrs().mapUnits() );
}

double QgsDistanceArea::measurePolygon( const QgsCurve *curve ) const
{
  if ( !curve )
  {
    return 0.0;
  }

  QgsPointSequence linePointsV2;
  curve->points( linePointsV2 );
  QVector<QgsPointXY> linePoints;
  QgsGeometry::convertPointList( linePointsV2, linePoints );
  return measurePolygon( linePoints );
}


double QgsDistanceArea::measurePolygon( const QVector<QgsPointXY> &points ) const
{
  try
  {
    if ( willUseEllipsoid() )
    {
      QVector<QgsPointXY> pts;
      for ( QVector<QgsPointXY>::const_iterator i = points.constBegin(); i != points.constEnd(); ++i )
      {
        pts.append( mCoordTransform.transform( *i ) );
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
    Q_UNUSED( cse )
    QgsMessageLog::logMessage( QObject::tr( "Caught a coordinate system exception while trying to transform a point. Unable to calculate polygon area." ) );
    return 0.0;
  }
}


double QgsDistanceArea::bearing( const QgsPointXY &p1, const QgsPointXY &p2 ) const
{
  QgsPointXY pp1 = p1, pp2 = p2;
  double bearing;

  if ( willUseEllipsoid() )
  {
    pp1 = mCoordTransform.transform( p1 );
    pp2 = mCoordTransform.transform( p2 );

    if ( !mGeod )
      computeAreaInit();
    Q_ASSERT_X( static_cast<bool>( mGeod ), "QgsDistanceArea::bearing()", "Error creating geod_geodesic object" );
    if ( !mGeod )
      return 0;

    double distance = 0;
    double azimuth1 = 0;
    double azimuth2 = 0;
    geod_inverse( mGeod.get(), pp1.y(), pp1.x(), pp2.y(), pp2.x(), &distance, &azimuth1, &azimuth2 );

    bearing = DEG2RAD( azimuth1 );
  }
  else //compute simple planar azimuth
  {
    const double dx = p2.x() - p1.x();
    const double dy = p2.y() - p1.y();
    // Note: the prototype of std::atan2 is (y,x), to return the angle of
    // vector (x,y) from the horizontal axis in counter-clock-wise orientation.
    // But a bearing is expressed in clock-wise order from the vertical axis, so
    // M_PI / 2 - std::atan2( dy, dx ) == std::atan2( dx, dy )
    bearing = std::atan2( dx, dy );
  }

  return bearing;
}

void QgsDistanceArea::computeAreaInit() const
{
  //don't try to perform calculations if no ellipsoid
  if ( mEllipsoid == geoNone() )
  {
    mGeod.reset();
    return;
  }

  mGeod.reset( new geod_geodesic() );
  geod_init( mGeod.get(), mSemiMajor, 1 / mInvFlattening );
}

void QgsDistanceArea::setFromParams( const QgsEllipsoidUtils::EllipsoidParameters &params )
{
  if ( params.useCustomParameters )
  {
    setEllipsoid( params.semiMajor, params.semiMinor );
  }
  else
  {
    mSemiMajor = params.semiMajor;
    mSemiMinor = params.semiMinor;
    mInvFlattening = params.inverseFlattening;
    mCoordTransform.setDestinationCrs( params.crs );
    computeAreaInit();
  }
}

double QgsDistanceArea::computePolygonArea( const QVector<QgsPointXY> &points ) const
{
  if ( points.isEmpty() )
  {
    return 0;
  }

  QgsDebugMsgLevel( "Ellipsoid: " + mEllipsoid, 3 );
  if ( !willUseEllipsoid() )
  {
    return computePolygonFlatArea( points );
  }

  if ( !mGeod )
    computeAreaInit();
  Q_ASSERT_X( static_cast<bool>( mGeod ), "QgsDistanceArea::computePolygonArea()", "Error creating geod_geodesic object" );
  if ( !mGeod )
    return 0;

  struct geod_polygon p;
  geod_polygon_init( &p, 0 );

  const bool isClosed = points.constFirst() == points.constLast();

  /* GeographicLib does not need a closed ring,
   * see example for geod_polygonarea() in geodesic.h */
  /* add points in reverse order */
  int i = points.size();
  while ( ( isClosed && --i ) || ( !isClosed && --i >= 0 ) )
    geod_polygon_addpoint( mGeod.get(), &p, points.at( i ).y(), points.at( i ).x() );

  double area = 0;
  double perimeter = 0;
  geod_polygon_compute( mGeod.get(), &p, 0, 1, &area, &perimeter );

  return std::fabs( area );
}

double QgsDistanceArea::computePolygonFlatArea( const QVector<QgsPointXY> &points ) const
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
    area = area + points[i].x() * points[( i + 1 ) % size].y() - points[( i + 1 ) % size].x() * points[i].y();
  }
  // QgsDebugMsg("Area from point: " + (points[i % size]).toString(2));
  area = area / 2.0;
  return std::fabs( area ); // All areas are positive!
}

QString QgsDistanceArea::formatDistance( double distance, int decimals, QgsUnitTypes::DistanceUnit unit, bool keepBaseUnit )
{
  return QgsUnitTypes::formatDistance( distance, decimals, unit, keepBaseUnit );
}

QString QgsDistanceArea::formatArea( double area, int decimals, QgsUnitTypes::AreaUnit unit, bool keepBaseUnit )
{
  return QgsUnitTypes::formatArea( area, decimals, unit, keepBaseUnit );
}

double QgsDistanceArea::convertLengthMeasurement( double length, QgsUnitTypes::DistanceUnit toUnits ) const
{
  // get the conversion factor between the specified units
  const QgsUnitTypes::DistanceUnit measureUnits = lengthUnits();
  const double factorUnits = QgsUnitTypes::fromUnitToUnitFactor( measureUnits, toUnits );

  const double result = length * factorUnits;
  QgsDebugMsgLevel( QStringLiteral( "Converted length of %1 %2 to %3 %4" ).arg( length )
                    .arg( QgsUnitTypes::toString( measureUnits ) )
                    .arg( result )
                    .arg( QgsUnitTypes::toString( toUnits ) ), 3 );
  return result;
}

double QgsDistanceArea::convertAreaMeasurement( double area, QgsUnitTypes::AreaUnit toUnits ) const
{
  // get the conversion factor between the specified units
  const QgsUnitTypes::AreaUnit measureUnits = areaUnits();
  const double factorUnits = QgsUnitTypes::fromUnitToUnitFactor( measureUnits, toUnits );

  const double result = area * factorUnits;
  QgsDebugMsgLevel( QStringLiteral( "Converted area of %1 %2 to %3 %4" ).arg( area )
                    .arg( QgsUnitTypes::toString( measureUnits ) )
                    .arg( result )
                    .arg( QgsUnitTypes::toString( toUnits ) ), 3 );
  return result;
}
