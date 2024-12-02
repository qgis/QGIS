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
#include "moc_qgsgeometryvalidator.cpp"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsgeos.h"
#include "qgsgeometrycollection.h"
#include "qgscurvepolygon.h"
#include "qgscurve.h"
#include "qgsvertexid.h"

QgsGeometryValidator::QgsGeometryValidator( const QgsGeometry &geometry, QVector<QgsGeometry::Error> *errors, Qgis::GeometryValidationEngine method )
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

void QgsGeometryValidator::checkRingIntersections( int partIndex0, int ringIndex0, const QgsCurve *ring0, int partIndex1, int ringIndex1, const QgsCurve *ring1 )
{
  const QgsLineString *ringLine0 = qgsgeometry_cast< const QgsLineString * >( ring0 );
  std::unique_ptr< QgsLineString > segmentisedRing0;
  if ( !ringLine0 )
  {
    segmentisedRing0.reset( qgsgeometry_cast< QgsLineString * >( ring0->segmentize() ) );
    ringLine0 = segmentisedRing0.get();
  }

  const QgsLineString *ringLine1 = qgsgeometry_cast< const QgsLineString * >( ring1 );
  std::unique_ptr< QgsLineString > segmentisedRing1;
  if ( !ringLine1 )
  {
    segmentisedRing1.reset( qgsgeometry_cast< QgsLineString * >( ring1->segmentize() ) );
    ringLine1 = segmentisedRing1.get();
  }

  Q_ASSERT( ringLine0 );
  Q_ASSERT( ringLine1 );

  for ( int i = 0; !mStop && i < ringLine0->numPoints() - 1; i++ )
  {
    const double ring0XAti = ringLine0->xAt( i );
    const double ring0YAti = ringLine0->yAt( i );
    const QgsVector v( ringLine0->xAt( i + 1 ) - ring0XAti, ringLine0->yAt( i + 1 ) - ring0YAti );

    for ( int j = 0; !mStop && j < ringLine1->numPoints() - 1; j++ )
    {
      const double ring1XAtj = ringLine1->xAt( j );
      const double ring1YAtj = ringLine1->yAt( j );
      const QgsVector w( ringLine1->xAt( j + 1 ) - ring1XAtj, ringLine1->yAt( j + 1 ) - ring1YAtj );

      double sX;
      double sY;
      if ( intersectLines( ring0XAti, ring0YAti, v, ring1XAtj, ring1YAtj, w, sX, sY ) )
      {
        double d = -distLine2Point( ring0XAti, ring0YAti, v.perpVector(), sX, sY );

        if ( d >= 0 && d <= v.length() )
        {
          d = -distLine2Point( ring1XAtj, ring1YAtj, w.perpVector(), sX, sY );
          if ( d > 0 && d < w.length() &&
               ringLine0->pointN( i + 1 ) != ringLine1->pointN( j + 1 ) && ringLine0->pointN( i + 1 ) != ringLine1->pointN( j ) &&
               ringLine0->pointN( i + 0 ) != ringLine1->pointN( j + 1 ) && ringLine0->pointN( i + 0 ) != ringLine1->pointN( j ) )
          {
            const QString msg = QObject::tr( "segment %1 of ring %2 of polygon %3 intersects segment %4 of ring %5 of polygon %6 at %7, %8" )
                                .arg( i ).arg( ringIndex0 ).arg( partIndex0 )
                                .arg( j ).arg( ringIndex1 ).arg( partIndex1 )
                                .arg( sX ).arg( sY );
            emit errorFound( QgsGeometry::Error( msg, QgsPointXY( sX, sY ) ) );
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
      const QString msg = QObject::tr( "ring %1 with less than four points" ).arg( i );
      QgsDebugMsgLevel( msg, 2 );
      emit errorFound( QgsGeometry::Error( msg ) );
      mErrorCount++;
      return;
    }

    if ( !line->isClosed() )
    {
      const QgsPoint startPoint = line->startPoint();
      const QgsPoint endPoint = line->endPoint();
      QString msg;
      if ( line->is3D() && line->isClosed2D() )
      {
        msg = QObject::tr( "ring %1 not closed, Z mismatch: %2 vs %3" ).arg( i ).arg( startPoint.z() ).arg( endPoint.z() );
      }
      else
      {
        msg = QObject::tr( "ring %1 not closed" ).arg( i );
        QgsDebugMsgLevel( msg, 2 );
      }
      emit errorFound( QgsGeometry::Error( msg, QgsPointXY( startPoint.x(), startPoint.y() ) ) );
      mErrorCount++;
      return;
    }
  }
  else if ( line->numPoints() < 2 )
  {
    const QString msg = QObject::tr( "line %1 with less than two points" ).arg( i );
    QgsDebugMsgLevel( msg, 2 );
    emit errorFound( QgsGeometry::Error( msg ) );
    mErrorCount++;
    return;
  }

  std::unique_ptr< QgsLineString > noDupes;

  // test for duplicate nodes, and if we find any flag errors and then remove them so that the subsequent
  // tests work OK.
  const QVector< QgsVertexId > duplicateNodes = line->collectDuplicateNodes( 1E-8 );
  if ( !duplicateNodes.empty() )
  {
    noDupes.reset( line->clone() );
    for ( int j = duplicateNodes.size() - 1; j >= 0; j-- )
    {
      const QgsVertexId duplicateVertex = duplicateNodes.at( j );
      const QgsPointXY duplicationLocation = noDupes->vertexAt( duplicateVertex );
      noDupes->deleteVertex( duplicateVertex );
      int n = 1;

      // count how many other points exist at this location too
      for ( int k = j - 1; k >= 0; k-- )
      {
        const QgsVertexId prevDupe = duplicateNodes.at( k );
        const QgsPoint prevPoint = noDupes->vertexAt( prevDupe );
        if ( qgsDoubleNear( duplicationLocation.x(), prevPoint.x(), 1E-8 ) && qgsDoubleNear( duplicationLocation.y(), prevPoint.y(), 1E-8 ) )
        {
          noDupes->deleteVertex( prevDupe );
          n++;
        }
        else
        {
          break;
        }
      }

      j -= n - 1;

      const QString msg = QObject::tr( "line %1 contains %n duplicate node(s) starting at vertex %2", "number of duplicate nodes", n + 1 ).arg( i + 1 ).arg( duplicateVertex.vertex - n + 1 );
      QgsDebugMsgLevel( msg, 2 );
      emit errorFound( QgsGeometry::Error( msg, duplicationLocation ) );
      mErrorCount++;
    }
    line = noDupes.get();
  }

  // segment Intersection
  for ( int j = 0; !mStop && j < line->numPoints() - 3; j++ )
  {
    const int n = ( j == 0 && ring ) ? line->numPoints() - 2 : line->numPoints() - 1;
    for ( int k = j + 2; !mStop && k < n; k++ )
    {
      double intersectionPointX = std::numeric_limits<double>::quiet_NaN();
      double intersectionPointY = std::numeric_limits<double>::quiet_NaN();
      bool isIntersection = false;
      if ( QgsGeometryUtilsBase::segmentIntersection( line->xAt( j ), line->yAt( j ), line->xAt( j + 1 ), line->yAt( j + 1 ), line->xAt( k ), line->yAt( k ), line->xAt( k + 1 ), line->yAt( k + 1 ), intersectionPointX, intersectionPointY, isIntersection ) )
      {
        const QString msg = QObject::tr( "segments %1 and %2 of line %3 intersect at %4, %5" ).arg( j ).arg( k ).arg( i ).arg( intersectionPointX ).arg( intersectionPointY );
        QgsDebugMsgLevel( msg, 2 );
        emit errorFound( QgsGeometry::Error( msg, QgsPointXY( intersectionPointX, intersectionPointY ) ) );
        mErrorCount++;
      }
    }
  }
}

void QgsGeometryValidator::validatePolygon( int partIndex, const QgsCurvePolygon *polygon )
{
  // check if holes are inside polygon
  for ( int i = 0; !mStop && i < polygon->numInteriorRings(); ++i )
  {
    if ( !ringInRing( polygon->interiorRing( i ), polygon->exteriorRing() ) )
    {
      const QString msg = QObject::tr( "ring %1 of polygon %2 not in exterior ring" ).arg( i + 1 ).arg( partIndex );
      QgsDebugMsgLevel( msg, 2 );
      emit errorFound( QgsGeometry::Error( msg ) );
      mErrorCount++;
    }
  }

  // check holes for intersections
  for ( int i = 0; !mStop && i < polygon->numInteriorRings(); i++ )
  {
    for ( int j = i + 1; !mStop && j < polygon->numInteriorRings(); j++ )
    {
      checkRingIntersections( partIndex, i + 1, polygon->interiorRing( i ),
                              partIndex, j + 1, polygon->interiorRing( j ) );
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
  if ( mGeometry.isNull() )
  {
    return;
  }

  switch ( mMethod )
  {
    case Qgis::GeometryValidationEngine::Geos:
    {
      // avoid calling geos for trivial point geometries
      if ( QgsWkbTypes::geometryType( mGeometry.wkbType() ) == Qgis::GeometryType::Point )
      {
        return;
      }

      const QgsGeos geos( mGeometry.constGet(), 0, Qgis::GeosCreationFlags() );
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

    case Qgis::GeometryValidationEngine::QgisInternal:
    {
      switch ( QgsWkbTypes::flatType( mGeometry.constGet()->wkbType() ) )
      {
        case Qgis::WkbType::Point:
        case Qgis::WkbType::MultiPoint:
          break;

        case Qgis::WkbType::LineString:
          validatePolyline( 0, qgsgeometry_cast< const QgsLineString * >( mGeometry.constGet() ) );
          break;

        case Qgis::WkbType::MultiLineString:
        {
          const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( mGeometry.constGet() );
          for ( int i = 0; !mStop && i < collection->numGeometries(); i++ )
            validatePolyline( i, qgsgeometry_cast< const QgsLineString * >( collection->geometryN( i ) ) );
          break;
        }

        case Qgis::WkbType::Polygon:
        case Qgis::WkbType::CurvePolygon:
          validatePolygon( 0, qgsgeometry_cast< const QgsCurvePolygon * >( mGeometry.constGet() ) );
          break;

        case Qgis::WkbType::MultiPolygon:
        case Qgis::WkbType::MultiSurface:
        {
          const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( mGeometry.constGet() );
          for ( int i = 0; !mStop && i < collection->numGeometries(); i++ )
            validatePolygon( i, qgsgeometry_cast< const QgsCurvePolygon * >( collection->geometryN( i ) ) );

          for ( int i = 0; !mStop && i < collection->numGeometries(); i++ )
          {
            const QgsCurvePolygon *poly = qgsgeometry_cast< const QgsCurvePolygon * >( collection->geometryN( i ) );
            if ( !poly->exteriorRing() || poly->exteriorRing()->isEmpty() )
            {
              emit errorFound( QgsGeometry::Error( QObject::tr( "Polygon %1 has no rings" ).arg( i ) ) );
              mErrorCount++;
              continue;
            }

            for ( int j = i + 1;  !mStop && j < collection->numGeometries(); j++ )
            {
              const QgsCurvePolygon *poly2 = qgsgeometry_cast< const QgsCurvePolygon * >( collection->geometryN( j ) );
              if ( !poly2->exteriorRing() || poly2->exteriorRing()->isEmpty() )
                continue;

              if ( ringInRing( poly->exteriorRing(),
                               poly2->exteriorRing() ) )
              {
                emit errorFound( QgsGeometry::Error( QObject::tr( "Polygon %1 lies inside polygon %2" ).arg( i ).arg( j ) ) );
                mErrorCount++;
              }
              else if ( ringInRing( poly2->exteriorRing(),
                                    poly->exteriorRing() ) )
              {
                emit errorFound( QgsGeometry::Error( QObject::tr( "Polygon %1 lies inside polygon %2" ).arg( j ).arg( i ) ) );
                mErrorCount++;
              }
              else
              {
                checkRingIntersections( i, 0, poly->exteriorRing(),
                                        j, 0, poly2->exteriorRing() );
              }
            }
          }
          break;
        }

        case Qgis::WkbType::Unknown:
        {
          emit errorFound( QgsGeometry::Error( QObject::tr( "Unknown geometry type %1" ).arg( qgsEnumValueToKey( mGeometry.wkbType() ) ) ) );
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
        emit validationFinished( QObject::tr( "Geometry has %n error(s).", nullptr, mErrorCount ) );
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

void QgsGeometryValidator::validateGeometry( const QgsGeometry &geometry, QVector<QgsGeometry::Error> &errors, Qgis::GeometryValidationEngine method )
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
double QgsGeometryValidator::distLine2Point( double px, double py, QgsVector v, double qX, double qY )
{
  const double l = v.length();
  if ( qgsDoubleNear( l, 0 ) )
  {
    throw QgsException( QObject::tr( "invalid line" ) );
  }

  return ( v.x() * ( qY - py ) - v.y() * ( qX - px ) ) / l;
}

bool QgsGeometryValidator::intersectLines( double px, double py, QgsVector v, double qx, double qy, QgsVector w, double &sX, double &sY )
{
  const double d = v.y() * w.x() - v.x() * w.y();

  if ( qgsDoubleNear( d, 0 ) )
    return false;

  const double dx = qx - px;
  const double dy = qy - py;
  const double k = ( dy * w.x() - dx * w.y() ) / d;

  sX = px  + v.x() * k;
  sY = py + v.y() * k;

  return true;
}

bool QgsGeometryValidator::pointInRing( const QgsCurve *ring, double pX, double pY )
{
  if ( !ring->boundingBox().contains( pX, pY ) )
    return false;

  bool inside = false;
  int j = ring->numPoints() - 1;

  for ( int i = 0; !mStop && i < ring->numPoints(); i++ )
  {
    const double xAti = ring->xAt( i );
    const double yAti = ring->yAt( i );
    const double xAtj = ring->xAt( j );
    const double yAtj = ring->yAt( j );

    if ( qgsDoubleNear( xAti, pX ) && qgsDoubleNear( yAti, pY ) )
      return true;

    if ( ( yAti < pY && yAtj >= pY ) ||
         ( yAtj < pY && yAti >= pY ) )
    {
      if ( xAti + ( pY - yAti ) / ( yAtj - yAti ) * ( xAtj - xAti ) <= pX )
        inside = !inside;
    }

    j = i;
  }

  return inside;
}

bool QgsGeometryValidator::ringInRing( const QgsCurve *inside, const QgsCurve *outside )
{
  if ( !outside->boundingBox().contains( inside->boundingBox() ) )
    return false;

  for ( int i = 0; !mStop && i < inside->numPoints(); i++ )
  {
    if ( !pointInRing( outside, inside->xAt( i ), inside->yAt( i ) ) )
      return false;
  }

  return true;
}
