/***************************************************************************
  qgsgeometryvalidator.cpp - geometry validation thread
  -------------------------------------------------------------------
Date                 : 03.01.2012
Copyright            : (C) 2012 by Juergen E. Fischer
email                : jef at norbit dot de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgis.h"
#include "qgsgeometryvalidator.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsgeos.h"

QgsGeometryValidator::QgsGeometryValidator( const QgsGeometry &geometry, QVector<QgsGeometry::Error> *errors, QgsGeometry::ValidationMethod method )
  : mGeometry( geometry )
  , mErrors( errors )
  , mStop( false )
  , mErrorCount( 0 )
  , mMethod( method )
{
}

QgsGeometryValidator::~QgsGeometryValidator()
{
  stop();
  wait();
}

void QgsGeometryValidator::stop()
{
  mStop = true;
}

void QgsGeometryValidator::checkRingIntersections(
  int p0, int i0, const QgsPolylineXY &ring0,
  int p1, int i1, const QgsPolylineXY &ring1 )
{
  for ( int i = 0; !mStop && i < ring0.size() - 1; i++ )
  {
    QgsVector v = ring0[i + 1] - ring0[i];

    for ( int j = 0; !mStop && j < ring1.size() - 1; j++ )
    {
      QgsVector w = ring1[j + 1] - ring1[j];

      QgsPointXY s;
      if ( intersectLines( ring0[i], v, ring1[j], w, s ) )
      {
        double d = -distLine2Point( ring0[i], v.perpVector(), s );

        if ( d >= 0 && d <= v.length() )
        {
          d = -distLine2Point( ring1[j], w.perpVector(), s );
          if ( d > 0 && d < w.length() &&
               ring0[i + 1] != ring1[j + 1] && ring0[i + 1] != ring1[j] &&
               ring0[i + 0] != ring1[j + 1] && ring0[i + 0] != ring1[j] )
          {
            QString msg = QObject::tr( "segment %1 of ring %2 of polygon %3 intersects segment %4 of ring %5 of polygon %6 at %7" )
                          .arg( i0 ).arg( i ).arg( p0 )
                          .arg( i1 ).arg( j ).arg( p1 )
                          .arg( s.toString() );
            QgsDebugMsg( msg );
            emit errorFound( QgsGeometry::Error( msg, s ) );
            mErrorCount++;
          }
        }
      }
    }
  }
}

void QgsGeometryValidator::validatePolyline( int i, QgsPolylineXY line, bool ring )
{
  if ( ring )
  {
    if ( line.size() < 4 )
    {
      QString msg = QObject::tr( "ring %1 with less than four points" ).arg( i );
      QgsDebugMsg( msg );
      emit errorFound( QgsGeometry::Error( msg ) );
      mErrorCount++;
      return;
    }

    if ( line[0] != line[ line.size() - 1 ] )
    {
      QString msg = QObject::tr( "ring %1 not closed" ).arg( i );
      QgsDebugMsg( msg );
      emit errorFound( QgsGeometry::Error( msg ) );
      mErrorCount++;
      return;
    }
  }
  else if ( line.size() < 2 )
  {
    QString msg = QObject::tr( "line %1 with less than two points" ).arg( i );
    QgsDebugMsg( msg );
    emit errorFound( QgsGeometry::Error( msg ) );
    mErrorCount++;
    return;
  }

  int j = 0;
  while ( j < line.size() - 1 )
  {
    int n = 0;
    while ( j < line.size() - 1 && line[j] == line[j + 1] )
    {
      line.remove( j );
      n++;
    }

    if ( n > 0 )
    {
      QString msg = QObject::tr( "line %1 contains %n duplicate node(s) at %2", "number of duplicate nodes", n ).arg( i ).arg( j );
      QgsDebugMsg( msg );
      emit errorFound( QgsGeometry::Error( msg, line[j] ) );
      mErrorCount++;
    }

    j++;
  }

  for ( j = 0; !mStop && j < line.size() - 3; j++ )
  {
    QgsVector v = line[j + 1] - line[j];
    double vl = v.length();

    int n = ( j == 0 && ring ) ? line.size() - 2 : line.size() - 1;

    for ( int k = j + 2; !mStop && k < n; k++ )
    {
      QgsVector w = line[k + 1] - line[k];

      QgsPointXY s;
      if ( !intersectLines( line[j], v, line[k], w, s ) )
        continue;

      double d = 0.0;
      try
      {
        d = -distLine2Point( line[j], v.perpVector(), s );
      }
      catch ( QgsException &e )
      {
        Q_UNUSED( e );
        QgsDebugMsg( "Error validating: " + e.what() );
        continue;
      }
      if ( d < 0 || d > vl )
        continue;

      try
      {
        d = -distLine2Point( line[k], w.perpVector(), s );
      }
      catch ( QgsException &e )
      {
        Q_UNUSED( e );
        QgsDebugMsg( "Error validating: " + e.what() );
        continue;
      }

      if ( d <= 0 || d >= w.length() )
        continue;

      QString msg = QObject::tr( "segments %1 and %2 of line %3 intersect at %4" ).arg( j ).arg( k ).arg( i ).arg( s.toString() );
      QgsDebugMsg( msg );
      emit errorFound( QgsGeometry::Error( msg, s ) );
      mErrorCount++;
    }
  }
}

void QgsGeometryValidator::validatePolygon( int idx, const QgsPolygonXY &polygon )
{
  // check if holes are inside polygon
  for ( int i = 1; !mStop && i < polygon.size(); i++ )
  {
    if ( !ringInRing( polygon[i], polygon[0] ) )
    {
      QString msg = QObject::tr( "Ring %1 of polygon %2 not in exterior ring" ).arg( i ).arg( idx );
      QgsDebugMsg( msg );
      emit errorFound( QgsGeometry::Error( msg ) );
      mErrorCount++;
    }
  }

  // check holes for intersections
  for ( int i = 1; !mStop && i < polygon.size(); i++ )
  {
    for ( int j = i + 1; !mStop && j < polygon.size(); j++ )
    {
      checkRingIntersections( idx, i, polygon[i], idx, j, polygon[j] );
    }
  }

  // check if rings are self-intersecting
  for ( int i = 0; !mStop && i < polygon.size(); i++ )
  {
    validatePolyline( i, polygon[i], true );
  }
}

void QgsGeometryValidator::run()
{
  mErrorCount = 0;
  switch ( mMethod )
  {
    case QgsGeometry::ValidatorGeos:
    {
      char *r = nullptr;
      geos::unique_ptr g0 = QgsGeos::asGeos( mGeometry );
      GEOSContextHandle_t handle = QgsGeos::getGEOSHandler();
      if ( !g0 )
      {
        emit errorFound( QgsGeometry::Error( QObject::tr( "GEOS error: could not produce geometry for GEOS (check log window)" ) ) );
      }
      else
      {
        GEOSGeometry *g1 = nullptr;
        char res = GEOSisValidDetail_r( handle, g0.get(), GEOSVALID_ALLOW_SELFTOUCHING_RING_FORMING_HOLE, &r, &g1 );
        if ( res != 1 )
        {
          static QgsStringMap translatedErrors;

          if ( translatedErrors.empty() )
          {
            // Copied from https://git.osgeo.org/gitea/geos/geos/src/branch/master/src/operation/valid/TopologyValidationError.cpp
            translatedErrors.insert( QStringLiteral( "Topology Validation Error" ), QObject::tr( "Topology validation error", "GEOS Error" ) );
            translatedErrors.insert( QStringLiteral( "Repeated Point" ), QObject::tr( "Repeated point", "GEOS Error" ) );
            translatedErrors.insert( QStringLiteral( "Hole lies outside shell" ), QObject::tr( "Hole lies outside shell", "GEOS Error" ) );
            translatedErrors.insert( QStringLiteral( "Holes are nested" ), QObject::tr( "Holes are nested", "GEOS Error" ) );
            translatedErrors.insert( QStringLiteral( "Interior is disconnected" ), QObject::tr( "Interior is disconnected", "GEOS Error" ) );
            translatedErrors.insert( QStringLiteral( "Self-intersection" ), QObject::tr( "Self-intersection", "GEOS Error" ) );
            translatedErrors.insert( QStringLiteral( "Ring Self-intersection" ), QObject::tr( "Ring self-intersection", "GEOS Error" ) );
            translatedErrors.insert( QStringLiteral( "Nested shells" ), QObject::tr( "Nested shells", "GEOS Error" ) );
            translatedErrors.insert( QStringLiteral( "Duplicate Rings" ), QObject::tr( "Duplicate rings", "GEOS Error" ) );
            translatedErrors.insert( QStringLiteral( "Too few points in geometry component" ), QObject::tr( "Too few points in geometry component", "GEOS Error" ) );
            translatedErrors.insert( QStringLiteral( "Invalid Coordinate" ), QObject::tr( "Invalid coordinate", "GEOS Error" ) );
            translatedErrors.insert( QStringLiteral( "Ring is not closed" ), QObject::tr( "Ring is not closed", "GEOS Error" ) );
          }

          const QString errorMsg( r );
          const QString translatedErrorMsg = translatedErrors.value( errorMsg, errorMsg );

          if ( g1 )
          {
            const GEOSCoordSequence *cs = GEOSGeom_getCoordSeq_r( handle, g1 );

            unsigned int n;
            if ( GEOSCoordSeq_getSize_r( handle, cs, &n ) && n == 1 )
            {
              double x, y;
              GEOSCoordSeq_getX_r( handle, cs, 0, &x );
              GEOSCoordSeq_getY_r( handle, cs, 0, &y );

              emit errorFound( QgsGeometry::Error( translatedErrorMsg, QgsPointXY( x, y ) ) );
              mErrorCount++;
            }

            GEOSGeom_destroy_r( handle, g1 );
          }
          else
          {
            emit errorFound( QgsGeometry::Error( translatedErrorMsg ) );
            mErrorCount++;
          }

          GEOSFree_r( handle, r );
        }
      }

      break;
    }

    case QgsGeometry::ValidatorQgisInternal:
    {
      QgsWkbTypes::Type flatType = QgsWkbTypes::flatType( mGeometry.wkbType() );
      //if ( flatType == QgsWkbTypes::Point || flatType == QgsWkbTypes::MultiPoint )
      //    break;
      if ( flatType == QgsWkbTypes::LineString )
      {
        validatePolyline( 0, mGeometry.asPolyline() );
      }
      else if ( flatType == QgsWkbTypes::MultiLineString )
      {
        QgsMultiPolylineXY mp = mGeometry.asMultiPolyline();
        for ( int i = 0; !mStop && i < mp.size(); i++ )
          validatePolyline( i, mp[i] );
      }
      else if ( flatType == QgsWkbTypes::Polygon )
      {
        validatePolygon( 0, mGeometry.asPolygon() );
      }
      else if ( flatType == QgsWkbTypes::MultiPolygon )
      {
        QgsMultiPolygonXY mp = mGeometry.asMultiPolygon();
        for ( int i = 0; !mStop && i < mp.size(); i++ )
        {
          validatePolygon( i, mp[i] );
        }

        for ( int i = 0; !mStop && i < mp.size(); i++ )
        {
          if ( mp[i].isEmpty() )
          {
            emit errorFound( QgsGeometry::Error( QObject::tr( "Polygon %1 has no rings" ).arg( i ) ) );
            mErrorCount++;
            continue;
          }

          for ( int j = i + 1;  !mStop && j < mp.size(); j++ )
          {
            if ( mp[j].isEmpty() )
              continue;

            if ( ringInRing( mp[i][0], mp[j][0] ) )
            {
              emit errorFound( QgsGeometry::Error( QObject::tr( "Polygon %1 lies inside polygon %2" ).arg( i ).arg( j ) ) );
              mErrorCount++;
            }
            else if ( ringInRing( mp[j][0], mp[i][0] ) )
            {
              emit errorFound( QgsGeometry::Error( QObject::tr( "Polygon %1 lies inside polygon %2" ).arg( j ).arg( i ) ) );
              mErrorCount++;
            }
            else
            {
              checkRingIntersections( i, 0, mp[i][0], j, 0, mp[j][0] );
            }
          }
        }
      }

      else if ( flatType == QgsWkbTypes::Unknown )
      {
        emit errorFound( QgsGeometry::Error( QObject::tr( "Unknown geometry type %1" ).arg( mGeometry.wkbType() ) ) );
        mErrorCount++;
      }

      if ( mStop )
      {
        emit errorFound( QgsGeometry::Error( QObject::tr( "Geometry validation was aborted." ) ) );
      }
      else if ( mErrorCount > 0 )
      {
        emit errorFound( QgsGeometry::Error( QObject::tr( "Geometry has %1 errors." ).arg( mErrorCount ) ) );
      }
#if 0
      else
      {
        emit errorFound( QgsGeometry::Error( QObject::tr( "Geometry is valid." ) ) );
      }
#endif
      break;
    }
  }
}

void QgsGeometryValidator::addError( const QgsGeometry::Error &e )
{
  if ( mErrors )
    *mErrors << e;
}

void QgsGeometryValidator::validateGeometry( const QgsGeometry &geometry, QVector<QgsGeometry::Error> &errors, QgsGeometry::ValidationMethod method )
{
  QgsGeometryValidator *gv = new QgsGeometryValidator( geometry, &errors, method );
  connect( gv, &QgsGeometryValidator::errorFound, gv, &QgsGeometryValidator::addError );
  gv->run();
  gv->wait();
}

//
// distance of point q from line through p in direction v
// return >0  => q lies left of the line
//        <0  => q lies right of the line
//
double QgsGeometryValidator::distLine2Point( const QgsPointXY &p, QgsVector v, const QgsPointXY &q )
{
  if ( qgsDoubleNear( v.length(), 0 ) )
  {
    throw QgsException( QObject::tr( "invalid line" ) );
  }

  return ( v.x() * ( q.y() - p.y() ) - v.y() * ( q.x() - p.x() ) ) / v.length();
}

bool QgsGeometryValidator::intersectLines( const QgsPointXY &p, QgsVector v, const QgsPointXY &q, QgsVector w, QgsPointXY &s )
{
  double d = v.y() * w.x() - v.x() * w.y();

  if ( qgsDoubleNear( d, 0 ) )
    return false;

  double dx = q.x() - p.x();
  double dy = q.y() - p.y();
  double k = ( dy * w.x() - dx * w.y() ) / d;

  s = p + v * k;

  return true;
}

bool QgsGeometryValidator::pointInRing( const QgsPolylineXY &ring, const QgsPointXY &p )
{
  bool inside = false;
  int j = ring.size() - 1;

  for ( int i = 0; !mStop && i < ring.size(); i++ )
  {
    if ( qgsDoubleNear( ring[i].x(), p.x() ) && qgsDoubleNear( ring[i].y(), p.y() ) )
      return true;

    if ( ( ring[i].y() < p.y() && ring[j].y() >= p.y() ) ||
         ( ring[j].y() < p.y() && ring[i].y() >= p.y() ) )
    {
      if ( ring[i].x() + ( p.y() - ring[i].y() ) / ( ring[j].y() - ring[i].y() ) * ( ring[j].x() - ring[i].x() ) <= p.x() )
        inside = !inside;
    }

    j = i;
  }

  return inside;
}

bool QgsGeometryValidator::ringInRing( const QgsPolylineXY &inside, const QgsPolylineXY &outside )
{
  for ( int i = 0; !mStop && i < inside.size(); i++ )
  {
    if ( !pointInRing( outside, inside[i] ) )
      return false;
  }

  return true;
}
