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
#include "qgsgeometrycollection.h"
#include "qgspolygon.h"

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

void QgsGeometryValidator::checkRingIntersections( int p0, int i0, const QgsLineString *ring0, int p1, int i1, const QgsLineString *ring1 )
{
  for ( int i = 0; !mStop && i < ring0->numPoints() - 1; i++ )
  {
    QgsVector v( ring0->xAt( i + 1 ) - ring0->xAt( i ), ring0->yAt( i + 1 ) - ring0->yAt( i ) );

    for ( int j = 0; !mStop && j < ring1->numPoints() - 1; j++ )
    {
      QgsVector w( ring1->xAt( j + 1 ) - ring1->xAt( j ), ring1->yAt( j + 1 ) - ring1->yAt( j ) );

      QgsPoint s;
      if ( intersectLines( ring0->xAt( i ), ring0->yAt( i ), v, ring1->xAt( j ), ring1->yAt( j ), w, s ) )
      {
        double d = -distLine2Point( ring0->xAt( i ), ring0->yAt( i ), v.perpVector(), s );

        if ( d >= 0 && d <= v.length() )
        {
          d = -distLine2Point( ring1->xAt( j ), ring1->yAt( j ), w.perpVector(), s );
          if ( d > 0 && d < w.length() &&
               ring0[i + 1] != ring1[j + 1] && ring0[i + 1] != ring1[j] &&
               ring0[i + 0] != ring1[j + 1] && ring0[i + 0] != ring1[j] )
          {
            QString msg = QObject::tr( "segment %1 of ring %2 of polygon %3 intersects segment %4 of ring %5 of polygon %6 at %7, %8" )
                          .arg( i0 ).arg( i ).arg( p0 )
                          .arg( i1 ).arg( j ).arg( p1 )
                          .arg( s.x() ).arg( s.y() );
            QgsDebugMsg( msg );
            emit errorFound( QgsGeometry::Error( msg, s ) );
            mErrorCount++;
          }
        }
      }
    }
  }
}

void QgsGeometryValidator::validatePolyline( int i, const QgsLineString *line, bool ring )
{
  if ( !line )
    return;

  if ( ring )
  {
    if ( line->numPoints() < 4 )
    {
      QString msg = QObject::tr( "ring %1 with less than four points" ).arg( i );
      QgsDebugMsgLevel( msg, 2 );
      emit errorFound( QgsGeometry::Error( msg ) );
      mErrorCount++;
      return;
    }

    if ( !line->isClosed() )
    {
      QString msg = QObject::tr( "ring %1 not closed" ).arg( i );
      QgsDebugMsgLevel( msg, 2 );
      emit errorFound( QgsGeometry::Error( msg ) );
      mErrorCount++;
      return;
    }
  }
  else if ( line->numPoints() < 2 )
  {
    QString msg = QObject::tr( "line %1 with less than two points" ).arg( i );
    QgsDebugMsgLevel( msg, 2 );
    emit errorFound( QgsGeometry::Error( msg ) );
    mErrorCount++;
    return;
  }

  int j = 0;
  std::unique_ptr< QgsLineString > noDupes( line->clone() );
  while ( j < noDupes->numPoints() - 1 )
  {
    int n = 0;
    QgsPointXY dupeLocation;
    while ( j < noDupes->numPoints() - 1 && qgsDoubleNear( noDupes->xAt( j ), noDupes->xAt( j + 1 ), 1E-8 ) && qgsDoubleNear( noDupes->yAt( j ), noDupes->yAt( j + 1 ), 1E-8 ) )
    {
      dupeLocation = QgsPointXY( noDupes->pointN( j ) );
      noDupes->deleteVertex( QgsVertexId( -1, -1, j ) );
      n++;
    }

    if ( n > 0 )
    {
      QString msg = QObject::tr( "line %1 contains %n duplicate node(s) at %2", "number of duplicate nodes", n ).arg( i ).arg( j );
      QgsDebugMsgLevel( msg, 2 );
      emit errorFound( QgsGeometry::Error( msg, dupeLocation ) );
      mErrorCount++;
    }

    j++;
  }

  for ( j = 0; !mStop && j < noDupes->numPoints() - 3; j++ )
  {
    QgsVector v( noDupes->xAt( j + 1 ) - noDupes->xAt( j ), noDupes->yAt( j + 1 ) - noDupes->yAt( j ) );
    double vl = v.length();

    int n = ( j == 0 && ring ) ? noDupes->numPoints() - 2 : noDupes->numPoints() - 1;

    for ( int k = j + 2; !mStop && k < n; k++ )
    {
      QgsVector w( noDupes->xAt( k + 1 ) - noDupes->xAt( k ), noDupes->yAt( k + 1 ) - noDupes->yAt( k ) );

      QgsPoint s;
      if ( !intersectLines( noDupes->xAt( j ), noDupes->yAt( j ), v,
                            noDupes->xAt( k ), noDupes->yAt( k ), w, s ) )
        continue;

      double d = 0.0;
      try
      {
        d = -distLine2Point( noDupes->xAt( j ), noDupes->yAt( j ), v.perpVector(), s );
      }
      catch ( QgsException &e )
      {
        Q_UNUSED( e )
        QgsDebugMsg( "Error validating: " + e.what() );
        continue;
      }
      if ( d < 0 || d > vl )
        continue;

      try
      {
        d = -distLine2Point( noDupes->xAt( k ), noDupes->yAt( k ), w.perpVector(), s );
      }
      catch ( QgsException &e )
      {
        Q_UNUSED( e )
        QgsDebugMsg( "Error validating: " + e.what() );
        continue;
      }

      if ( d <= 0 || d >= w.length() )
        continue;

      QString msg = QObject::tr( "segments %1 and %2 of line %3 intersect at %4, %5" ).arg( j ).arg( k ).arg( i ).arg( s.x() ).arg( s.y() );
      QgsDebugMsgLevel( msg, 2 );
      emit errorFound( QgsGeometry::Error( msg, s ) );
      mErrorCount++;
    }
  }
}

void QgsGeometryValidator::validatePolygon( int idx, const QgsPolygon *polygon )
{
  // check if holes are inside polygon
  for ( int i = 0; !mStop && i < polygon->numInteriorRings(); ++i )
  {
    if ( !ringInRing( static_cast< const QgsLineString * >( polygon->interiorRing( i ) ), static_cast< const QgsLineString * >( polygon->exteriorRing() ) ) )
    {
      QString msg = QObject::tr( "ring %1 of polygon %2 not in exterior ring" ).arg( i + 1 ).arg( idx );
      QgsDebugMsg( msg );
      emit errorFound( QgsGeometry::Error( msg ) );
      mErrorCount++;
    }
  }

  // check holes for intersections
  for ( int i = 0; !mStop && i < polygon->numInteriorRings(); i++ )
  {
    for ( int j = i + 1; !mStop && j < polygon->numInteriorRings(); j++ )
    {
      checkRingIntersections( idx, i + 1, qgsgeometry_cast< QgsLineString * >( polygon->interiorRing( i ) ),
                              idx, j + 1, qgsgeometry_cast< QgsLineString * >( polygon->interiorRing( j ) ) );
    }
  }

  // check if rings are self-intersecting
  validatePolyline( 0, qgsgeometry_cast< const QgsLineString * >( polygon->exteriorRing() ), true );
  for ( int i = 0; !mStop && i < polygon->numInteriorRings(); i++ )
  {
    validatePolyline( i + 1, qgsgeometry_cast< const QgsLineString * >( polygon->interiorRing( i ) ), true );
  }
}

void QgsGeometryValidator::run()
{
  mErrorCount = 0;
  switch ( mMethod )
  {
    case QgsGeometry::ValidatorGeos:
    {
      if ( mGeometry.isNull() )
      {
        return;
      }

      // avoid calling geos for trivial point geometries
      if ( QgsWkbTypes::geometryType( mGeometry.wkbType() ) == QgsWkbTypes::PointGeometry )
      {
        return;
      }

      QgsGeos geos( mGeometry.constGet() );
      QString error;
      QgsGeometry errorLoc;
      if ( !geos.isValid( &error, true, &errorLoc ) )
      {
        if ( errorLoc.isNull() )
        {
          emit errorFound( QgsGeometry::Error( error ) );
          mErrorCount++;
        }
        else
        {
          const QgsPointXY point = errorLoc.asPoint();
          emit errorFound( QgsGeometry::Error( error, point ) );
          mErrorCount++;
        }
      }

      break;
    }

    case QgsGeometry::ValidatorQgisInternal:
    {
      switch ( QgsWkbTypes::flatType( mGeometry.constGet()->wkbType() ) )
      {
        case QgsWkbTypes::Point:
        case QgsWkbTypes::MultiPoint:
          break;

        case QgsWkbTypes::LineString:
          validatePolyline( 0, qgsgeometry_cast< const QgsLineString * >( mGeometry.constGet() ) );
          break;

        case QgsWkbTypes::MultiLineString:
        {
          const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( mGeometry.constGet() );
          for ( int i = 0; !mStop && i < collection->numGeometries(); i++ )
            validatePolyline( i, qgsgeometry_cast< const QgsLineString * >( collection->geometryN( i ) ) );
          break;
        }

        case QgsWkbTypes::Polygon:
          validatePolygon( 0, qgsgeometry_cast< const QgsPolygon * >( mGeometry.constGet() ) );
          break;

        case QgsWkbTypes::MultiPolygon:
        {
          const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( mGeometry.constGet() );
          for ( int i = 0; !mStop && i < collection->numGeometries(); i++ )
            validatePolygon( i, qgsgeometry_cast< const QgsPolygon * >( collection->geometryN( i ) ) );

          for ( int i = 0; !mStop && i < collection->numGeometries(); i++ )
          {
            const QgsPolygon *poly = qgsgeometry_cast< const QgsPolygon * >( collection->geometryN( i ) );
            if ( !poly->exteriorRing() || poly->exteriorRing()->isEmpty() )
            {
              emit errorFound( QgsGeometry::Error( QObject::tr( "Polygon %1 has no rings" ).arg( i ) ) );
              mErrorCount++;
              continue;
            }

            for ( int j = i + 1;  !mStop && j < collection->numGeometries(); j++ )
            {
              const QgsPolygon *poly2 = qgsgeometry_cast< const QgsPolygon * >( collection->geometryN( j ) );
              if ( !poly2->exteriorRing() || poly2->exteriorRing()->isEmpty() )
                continue;

              if ( ringInRing( qgsgeometry_cast< const QgsLineString * >( poly->exteriorRing() ),
                               qgsgeometry_cast< const QgsLineString * >( poly2->exteriorRing() ) ) )
              {
                emit errorFound( QgsGeometry::Error( QObject::tr( "Polygon %1 lies inside polygon %2" ).arg( i ).arg( j ) ) );
                mErrorCount++;
              }
              else if ( ringInRing( static_cast< const QgsLineString * >( poly2->exteriorRing() ),
                                    static_cast< const QgsLineString * >( poly->exteriorRing() ) ) )
              {
                emit errorFound( QgsGeometry::Error( QObject::tr( "Polygon %1 lies inside polygon %2" ).arg( j ).arg( i ) ) );
                mErrorCount++;
              }
              else
              {
                checkRingIntersections( i, 0, qgsgeometry_cast< const QgsLineString * >( poly->exteriorRing() ),
                                        j, 0, qgsgeometry_cast< const QgsLineString * >( poly2->exteriorRing() ) );
              }
            }
          }
          break;
        }

        case QgsWkbTypes::Unknown:
        {
          emit errorFound( QgsGeometry::Error( QObject::tr( "Unknown geometry type %1" ).arg( mGeometry.wkbType() ) ) );
          mErrorCount++;
          break;
        }

        default:
          break;
      }

      if ( mStop )
      {
        emit validationFinished( QObject::tr( "Geometry validation was aborted." ) );
      }
      else if ( mErrorCount > 0 )
      {
        emit validationFinished( QObject::tr( "Geometry has %1 errors." ).arg( mErrorCount ) );
      }
      else
      {
        emit validationFinished( QObject::tr( "Geometry is valid." ) );
      }
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
double QgsGeometryValidator::distLine2Point( double px, double py, QgsVector v, const QgsPoint &q )
{
  if ( qgsDoubleNear( v.length(), 0 ) )
  {
    throw QgsException( QObject::tr( "invalid line" ) );
  }

  return ( v.x() * ( q.y() - py ) - v.y() * ( q.x() - px ) ) / v.length();
}

bool QgsGeometryValidator::intersectLines( double px, double py, QgsVector v, double qx, double qy, QgsVector w, QgsPoint &s )
{
  double d = v.y() * w.x() - v.x() * w.y();

  if ( qgsDoubleNear( d, 0 ) )
    return false;

  double dx = qx - px;
  double dy = qy - py;
  double k = ( dy * w.x() - dx * w.y() ) / d;

  s = QgsPoint( px  + v.x() * k, py + v.y() * k );

  return true;
}

bool QgsGeometryValidator::pointInRing( const QgsLineString *ring, const QgsPoint &p )
{
  if ( !ring->boundingBox().contains( p ) )
    return false;

  bool inside = false;
  int j = ring->numPoints() - 1;

  for ( int i = 0; !mStop && i < ring->numPoints(); i++ )
  {
    if ( qgsDoubleNear( ring->xAt( i ), p.x() ) && qgsDoubleNear( ring->yAt( i ), p.y() ) )
      return true;

    if ( ( ring->yAt( i ) < p.y() && ring->yAt( j ) >= p.y() ) ||
         ( ring->yAt( j ) < p.y() && ring->yAt( i ) >= p.y() ) )
    {
      if ( ring->xAt( i ) + ( p.y() - ring->yAt( i ) ) / ( ring->yAt( j ) - ring->yAt( i ) ) * ( ring->xAt( j ) - ring->xAt( i ) ) <= p.x() )
        inside = !inside;
    }

    j = i;
  }

  return inside;
}

bool QgsGeometryValidator::ringInRing( const QgsLineString *inside, const QgsLineString *outside )
{
  if ( !outside->boundingBox().contains( inside->boundingBox() ) )
    return false;

  for ( int i = 0; !mStop && i < inside->numPoints(); i++ )
  {
    if ( !pointInRing( outside, inside->pointN( i ) ) )
      return false;
  }

  return true;
}
