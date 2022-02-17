/***************************************************************************
  qgsgeometry.cpp - Geometry (stored as Open Geospatial Consortium WKB)
  -------------------------------------------------------------------
Date                 : 02 May 2005
Copyright            : (C) 2005 by Brendan Morley
email                : morb at ozemail dot com dot au
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <limits>
#include <cstdarg>
#include <cstdio>
#include <cmath>
#include <nlohmann/json.hpp>
#include <QCache>

#include "qgis.h"
#include "qgsgeometry.h"
#include "qgsgeometryeditutils.h"
#include "qgsgeometryfactory.h"

#include <geos_c.h>

#if ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR<8 )
#include "qgsgeometrymakevalid.h"
#endif

#include "qgsgeometryutils.h"
#include "qgsinternalgeometryengine.h"
#include "qgsgeos.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsmaptopixel.h"
#include "qgsmessagelog.h"
#include "qgspointxy.h"
#include "qgsrectangle.h"

#include "qgsvectorlayer.h"
#include "qgsgeometryvalidator.h"

#include "qgsmulticurve.h"
#include "qgsmultilinestring.h"
#include "qgsmultipoint.h"
#include "qgsmultipolygon.h"
#include "qgsmultisurface.h"
#include "qgspoint.h"
#include "qgspolygon.h"
#include "qgslinestring.h"
#include "qgscircle.h"
#include "qgscurve.h"
#include "qgsreadwritelocker.h"

struct QgsGeometryPrivate
{
  QgsGeometryPrivate(): ref( 1 ) {}
  QAtomicInt ref;
  std::unique_ptr< QgsAbstractGeometry > geometry;
};

QgsGeometry::QgsGeometry()
  : d( new QgsGeometryPrivate() )
{
}

QgsGeometry::~QgsGeometry()
{
  if ( !d->ref.deref() )
    delete d;
}

QgsGeometry::QgsGeometry( QgsAbstractGeometry *geom )
  : d( new QgsGeometryPrivate() )
{
  d->geometry.reset( geom );
  d->ref = QAtomicInt( 1 );
}

QgsGeometry::QgsGeometry( std::unique_ptr<QgsAbstractGeometry> geom )
  : d( new QgsGeometryPrivate() )
{
  d->geometry = std::move( geom );
  d->ref = QAtomicInt( 1 );
}

QgsGeometry::QgsGeometry( const QgsGeometry &other )
  : d( other.d )
{
  mLastError = other.mLastError;
  d->ref.ref();
}

QgsGeometry &QgsGeometry::operator=( QgsGeometry const &other )
{
  if ( this != &other )
  {
    if ( !d->ref.deref() )
    {
      delete d;
    }

    mLastError = other.mLastError;
    d = other.d;
    d->ref.ref();
  }
  return *this;
}

void QgsGeometry::detach()
{
  if ( d->ref <= 1 )
    return;

  std::unique_ptr< QgsAbstractGeometry > cGeom;
  if ( d->geometry )
    cGeom.reset( d->geometry->clone() );

  reset( std::move( cGeom ) );
}

void QgsGeometry::reset( std::unique_ptr<QgsAbstractGeometry> newGeometry )
{
  if ( d->ref > 1 )
  {
    ( void )d->ref.deref();
    d = new QgsGeometryPrivate();
  }
  d->geometry = std::move( newGeometry );
}

const QgsAbstractGeometry *QgsGeometry::constGet() const
{
  return d->geometry.get();
}

QgsAbstractGeometry *QgsGeometry::get()
{
  detach();
  return d->geometry.get();
}

void QgsGeometry::set( QgsAbstractGeometry *geometry )
{
  if ( d->geometry.get() == geometry )
  {
    return;
  }

  reset( std::unique_ptr< QgsAbstractGeometry >( geometry ) );
}

bool QgsGeometry::isNull() const
{
  return !d->geometry;
}

typedef QCache< QString, QgsGeometry > WktCache;
Q_GLOBAL_STATIC_WITH_ARGS( WktCache, sWktCache, ( 2000 ) )  // store up to 2000 geometries
Q_GLOBAL_STATIC( QMutex, sWktMutex )

QgsGeometry QgsGeometry::fromWkt( const QString &wkt )
{
  QMutexLocker lock( sWktMutex() );
  if ( const QgsGeometry *cached = sWktCache()->object( wkt ) )
    return *cached;
  const QgsGeometry result( QgsGeometryFactory::geomFromWkt( wkt ) );
  sWktCache()->insert( wkt, new QgsGeometry( result ), 1 );
  return result;
}

QgsGeometry QgsGeometry::fromPointXY( const QgsPointXY &point )
{
  std::unique_ptr< QgsAbstractGeometry > geom( QgsGeometryFactory::fromPointXY( point ) );
  if ( geom )
  {
    return QgsGeometry( geom.release() );
  }
  return QgsGeometry();
}

QgsGeometry QgsGeometry::fromPolylineXY( const QgsPolylineXY &polyline )
{
  std::unique_ptr< QgsAbstractGeometry > geom = QgsGeometryFactory::fromPolylineXY( polyline );
  if ( geom )
  {
    return QgsGeometry( std::move( geom ) );
  }
  return QgsGeometry();
}

QgsGeometry QgsGeometry::fromPolyline( const QgsPolyline &polyline )
{
  return QgsGeometry( std::make_unique< QgsLineString >( polyline ) );
}

QgsGeometry QgsGeometry::fromPolygonXY( const QgsPolygonXY &polygon )
{
  std::unique_ptr< QgsPolygon > geom = QgsGeometryFactory::fromPolygonXY( polygon );
  if ( geom )
  {
    return QgsGeometry( std::move( geom ) );
  }
  return QgsGeometry();
}

QgsGeometry QgsGeometry::fromMultiPointXY( const QgsMultiPointXY &multipoint )
{
  std::unique_ptr< QgsMultiPoint > geom = QgsGeometryFactory::fromMultiPointXY( multipoint );
  if ( geom )
  {
    return QgsGeometry( std::move( geom ) );
  }
  return QgsGeometry();
}

QgsGeometry QgsGeometry::fromMultiPolylineXY( const QgsMultiPolylineXY &multiline )
{
  std::unique_ptr< QgsMultiLineString > geom = QgsGeometryFactory::fromMultiPolylineXY( multiline );
  if ( geom )
  {
    return QgsGeometry( std::move( geom ) );
  }
  return QgsGeometry();
}

QgsGeometry QgsGeometry::fromMultiPolygonXY( const QgsMultiPolygonXY &multipoly )
{
  std::unique_ptr< QgsMultiPolygon > geom = QgsGeometryFactory::fromMultiPolygonXY( multipoly );
  if ( geom )
  {
    return QgsGeometry( std::move( geom ) );
  }
  return QgsGeometry();
}

QgsGeometry QgsGeometry::fromRect( const QgsRectangle &rect )
{
  std::unique_ptr< QgsLineString > ext = std::make_unique< QgsLineString >(
      QVector< double >() << rect.xMinimum()
      << rect.xMaximum()
      << rect.xMaximum()
      << rect.xMinimum()
      << rect.xMinimum(),
      QVector< double >() << rect.yMinimum()
      << rect.yMinimum()
      << rect.yMaximum()
      << rect.yMaximum()
      << rect.yMinimum() );
  std::unique_ptr< QgsPolygon > polygon = std::make_unique< QgsPolygon >();
  polygon->setExteriorRing( ext.release() );
  return QgsGeometry( std::move( polygon ) );
}

QgsGeometry QgsGeometry::collectGeometry( const QVector< QgsGeometry > &geometries )
{
  QgsGeometry collected;

  for ( const QgsGeometry &g : geometries )
  {
    if ( collected.isNull() )
    {
      collected = g;
      collected.convertToMultiType();
    }
    else
    {
      if ( g.isMultipart() )
      {
        for ( auto p = g.const_parts_begin(); p != g.const_parts_end(); ++p )
        {
          collected.addPart( ( *p )->clone() );
        }
      }
      else
      {
        collected.addPart( g );
      }
    }
  }
  return collected;
}

QgsGeometry QgsGeometry::createWedgeBuffer( const QgsPoint &center, const double azimuth, const double angularWidth, const double outerRadius, const double innerRadius )
{
  if ( std::abs( angularWidth ) >= 360.0 )
  {
    std::unique_ptr< QgsCompoundCurve > outerCc = std::make_unique< QgsCompoundCurve >();

    QgsCircle outerCircle = QgsCircle( center, outerRadius );
    outerCc->addCurve( outerCircle.toCircularString() );

    std::unique_ptr< QgsCurvePolygon > cp = std::make_unique< QgsCurvePolygon >();
    cp->setExteriorRing( outerCc.release() );

    if ( !qgsDoubleNear( innerRadius, 0.0 ) && innerRadius > 0 )
    {
      std::unique_ptr< QgsCompoundCurve > innerCc = std::make_unique< QgsCompoundCurve >();

      QgsCircle innerCircle = QgsCircle( center, innerRadius );
      innerCc->addCurve( innerCircle.toCircularString() );

      cp->setInteriorRings( { innerCc.release() } );
    }

    return QgsGeometry( std::move( cp ) );
  }

  std::unique_ptr< QgsCompoundCurve > wedge = std::make_unique< QgsCompoundCurve >();

  const double startAngle = azimuth - angularWidth * 0.5;
  const double endAngle = azimuth + angularWidth * 0.5;

  const QgsPoint outerP1 = center.project( outerRadius, startAngle );
  const QgsPoint outerP2 = center.project( outerRadius, endAngle );

  const bool useShortestArc = angularWidth <= 180.0;

  wedge->addCurve( new QgsCircularString( QgsCircularString::fromTwoPointsAndCenter( outerP1, outerP2, center, useShortestArc ) ) );

  if ( !qgsDoubleNear( innerRadius, 0.0 ) && innerRadius > 0 )
  {
    const QgsPoint innerP1 = center.project( innerRadius, startAngle );
    const QgsPoint innerP2 = center.project( innerRadius, endAngle );
    wedge->addCurve( new QgsLineString( outerP2, innerP2 ) );
    wedge->addCurve( new QgsCircularString( QgsCircularString::fromTwoPointsAndCenter( innerP2, innerP1, center, useShortestArc ) ) );
    wedge->addCurve( new QgsLineString( innerP1, outerP1 ) );
  }
  else
  {
    wedge->addCurve( new QgsLineString( outerP2, center ) );
    wedge->addCurve( new QgsLineString( center, outerP1 ) );
  }

  std::unique_ptr< QgsCurvePolygon > cp = std::make_unique< QgsCurvePolygon >();
  cp->setExteriorRing( wedge.release() );
  return QgsGeometry( std::move( cp ) );
}

void QgsGeometry::fromWkb( unsigned char *wkb, int length )
{
  QgsConstWkbPtr ptr( wkb, length );
  reset( QgsGeometryFactory::geomFromWkb( ptr ) );
  delete [] wkb;
}

void QgsGeometry::fromWkb( const QByteArray &wkb )
{
  QgsConstWkbPtr ptr( wkb );
  reset( QgsGeometryFactory::geomFromWkb( ptr ) );
}

QgsWkbTypes::Type QgsGeometry::wkbType() const
{
  if ( !d->geometry )
  {
    return QgsWkbTypes::Unknown;
  }
  else
  {
    return d->geometry->wkbType();
  }
}


QgsWkbTypes::GeometryType QgsGeometry::type() const
{
  if ( !d->geometry )
  {
    return QgsWkbTypes::UnknownGeometry;
  }
  return static_cast< QgsWkbTypes::GeometryType >( QgsWkbTypes::geometryType( d->geometry->wkbType() ) );
}

bool QgsGeometry::isEmpty() const
{
  if ( !d->geometry )
  {
    return true;
  }

  return d->geometry->isEmpty();
}

bool QgsGeometry::isMultipart() const
{
  if ( !d->geometry )
  {
    return false;
  }
  return QgsWkbTypes::isMultiType( d->geometry->wkbType() );
}
QgsPointXY QgsGeometry::closestVertex( const QgsPointXY &point, int &closestVertexIndex, int &previousVertexIndex, int &nextVertexIndex, double &sqrDist ) const
{
  if ( !d->geometry )
  {
    sqrDist = -1;
    return QgsPointXY();
  }

  QgsPoint pt( point );
  QgsVertexId id;

  QgsPoint vp = QgsGeometryUtils::closestVertex( *( d->geometry ), pt, id );
  if ( !id.isValid() )
  {
    sqrDist = -1;
    return QgsPointXY();
  }
  sqrDist = QgsGeometryUtils::sqrDistance2D( pt, vp );

  QgsVertexId prevVertex;
  QgsVertexId nextVertex;
  d->geometry->adjacentVertices( id, prevVertex, nextVertex );
  closestVertexIndex = vertexNrFromVertexId( id );
  previousVertexIndex = vertexNrFromVertexId( prevVertex );
  nextVertexIndex = vertexNrFromVertexId( nextVertex );
  return QgsPointXY( vp.x(), vp.y() );
}

double QgsGeometry::distanceToVertex( int vertex ) const
{
  if ( !d->geometry )
  {
    return -1;
  }

  QgsVertexId id;
  if ( !vertexIdFromVertexNr( vertex, id ) )
  {
    return -1;
  }

  return QgsGeometryUtils::distanceToVertex( *( d->geometry ), id );
}

double QgsGeometry::angleAtVertex( int vertex ) const
{
  if ( !d->geometry )
  {
    return 0;
  }

  QgsVertexId v2;
  if ( !vertexIdFromVertexNr( vertex, v2 ) )
  {
    return 0;
  }

  return d->geometry->vertexAngle( v2 );
}

void QgsGeometry::adjacentVertices( int atVertex, int &beforeVertex, int &afterVertex ) const
{
  if ( !d->geometry )
  {
    return;
  }

  QgsVertexId id;
  if ( !vertexIdFromVertexNr( atVertex, id ) )
  {
    beforeVertex = -1;
    afterVertex = -1;
    return;
  }

  QgsVertexId beforeVertexId, afterVertexId;
  d->geometry->adjacentVertices( id, beforeVertexId, afterVertexId );
  beforeVertex = vertexNrFromVertexId( beforeVertexId );
  afterVertex = vertexNrFromVertexId( afterVertexId );
}

bool QgsGeometry::moveVertex( double x, double y, int atVertex )
{
  if ( !d->geometry )
  {
    return false;
  }

  QgsVertexId id;
  if ( !vertexIdFromVertexNr( atVertex, id ) )
  {
    return false;
  }

  detach();

  return d->geometry->moveVertex( id, QgsPoint( x, y ) );
}

bool QgsGeometry::moveVertex( const QgsPoint &p, int atVertex )
{
  if ( !d->geometry )
  {
    return false;
  }

  QgsVertexId id;
  if ( !vertexIdFromVertexNr( atVertex, id ) )
  {
    return false;
  }

  detach();

  return d->geometry->moveVertex( id, p );
}

bool QgsGeometry::deleteVertex( int atVertex )
{
  if ( !d->geometry )
  {
    return false;
  }

  //maintain compatibility with < 2.10 API
  if ( QgsWkbTypes::flatType( d->geometry->wkbType() ) == QgsWkbTypes::MultiPoint )
  {
    detach();
    //delete geometry instead of point
    return static_cast< QgsGeometryCollection * >( d->geometry.get() )->removeGeometry( atVertex );
  }

  //if it is a point, set the geometry to nullptr
  if ( QgsWkbTypes::flatType( d->geometry->wkbType() ) == QgsWkbTypes::Point )
  {
    reset( nullptr );
    return true;
  }

  QgsVertexId id;
  if ( !vertexIdFromVertexNr( atVertex, id ) )
  {
    return false;
  }

  detach();

  return d->geometry->deleteVertex( id );
}

bool QgsGeometry::toggleCircularAtVertex( int atVertex )
{

  if ( !d->geometry )
    return false;

  QgsVertexId id;
  if ( !vertexIdFromVertexNr( atVertex, id ) )
    return false;

  detach();

  QgsAbstractGeometry *geom = d->geometry.get();

  // If the geom is a collection, we get the concerned part, otherwise, the part is just the whole geom
  QgsAbstractGeometry *part = nullptr;
  QgsGeometryCollection *owningCollection = qgsgeometry_cast<QgsGeometryCollection *>( geom );
  if ( owningCollection != nullptr )
    part = owningCollection->geometryN( id.part );
  else
    part = geom;

  // If the part is a polygon, we get the concerned ring, otherwise, the ring is just the whole part
  QgsAbstractGeometry *ring = nullptr;
  QgsCurvePolygon *owningPolygon = qgsgeometry_cast<QgsCurvePolygon *>( part );
  if ( owningPolygon != nullptr )
    ring = ( id.ring == 0 ) ? owningPolygon->exteriorRing() : owningPolygon->interiorRing( id.ring - 1 );
  else
    ring = part;

  // If the ring is not a curve, we're probably on a point geometry
  QgsCurve *curve = qgsgeometry_cast<QgsCurve *>( ring );
  if ( curve == nullptr )
    return false;

  bool success = false;
  QgsCompoundCurve *cpdCurve  = qgsgeometry_cast<QgsCompoundCurve *>( curve );
  if ( cpdCurve != nullptr )
  {
    // If the geom is a already compound curve, we convert inplace, and we're done
    success = cpdCurve->toggleCircularAtVertex( id );
  }
  else
  {
    // TODO : move this block before the above, so we call toggleCircularAtVertex only in one place
    // If the geom is a linestring or cirularstring, we create a compound curve
    std::unique_ptr<QgsCompoundCurve> cpdCurve = std::make_unique<QgsCompoundCurve>();
    cpdCurve->addCurve( curve->clone() );
    success = cpdCurve->toggleCircularAtVertex( QgsVertexId( -1, -1, id.vertex ) );

    // In that case, we must also reassign the instances
    if ( success )
    {

      if ( owningPolygon == nullptr && owningCollection == nullptr )
      {
        // Standalone linestring
        reset( std::make_unique<QgsCompoundCurve>( *cpdCurve ) ); // <- REVIEW PLZ
      }
      else if ( owningPolygon != nullptr )
      {
        // Replace the ring in the owning polygon
        if ( id.ring == 0 )
        {
          owningPolygon->setExteriorRing( cpdCurve.release() );
        }
        else
        {
          owningPolygon->removeInteriorRing( id.ring - 1 );
          owningPolygon->addInteriorRing( cpdCurve.release() );
        }
      }
      else if ( owningCollection != nullptr )
      {
        // Replace the curve in the owning collection
        owningCollection->removeGeometry( id.part );
        owningCollection->insertGeometry( cpdCurve.release(), id.part );
      }
    }
  }

  return success;
}

bool QgsGeometry::insertVertex( double x, double y, int beforeVertex )
{
  if ( !d->geometry )
  {
    return false;
  }

  //maintain compatibility with < 2.10 API
  if ( QgsWkbTypes::flatType( d->geometry->wkbType() ) == QgsWkbTypes::MultiPoint )
  {
    detach();
    //insert geometry instead of point
    return static_cast< QgsGeometryCollection * >( d->geometry.get() )->insertGeometry( new QgsPoint( x, y ), beforeVertex );
  }

  QgsVertexId id;
  if ( !vertexIdFromVertexNr( beforeVertex, id ) )
  {
    return false;
  }

  detach();

  return d->geometry->insertVertex( id, QgsPoint( x, y ) );
}

bool QgsGeometry::insertVertex( const QgsPoint &point, int beforeVertex )
{
  if ( !d->geometry )
  {
    return false;
  }

  //maintain compatibility with < 2.10 API
  if ( QgsWkbTypes::flatType( d->geometry->wkbType() ) == QgsWkbTypes::MultiPoint )
  {
    detach();
    //insert geometry instead of point
    return static_cast< QgsGeometryCollection * >( d->geometry.get() )->insertGeometry( new QgsPoint( point ), beforeVertex );
  }

  QgsVertexId id;
  if ( !vertexIdFromVertexNr( beforeVertex, id ) )
  {
    return false;
  }

  detach();

  return d->geometry->insertVertex( id, point );
}

QgsPoint QgsGeometry::vertexAt( int atVertex ) const
{
  if ( !d->geometry )
  {
    return QgsPoint();
  }

  QgsVertexId vId;
  ( void )vertexIdFromVertexNr( atVertex, vId );
  if ( vId.vertex < 0 )
  {
    return QgsPoint();
  }
  return d->geometry->vertexAt( vId );
}

double QgsGeometry::sqrDistToVertexAt( QgsPointXY &point, int atVertex ) const
{
  QgsPointXY vertexPoint = vertexAt( atVertex );
  return QgsGeometryUtils::sqrDistance2D( QgsPoint( vertexPoint ), QgsPoint( point ) );
}

QgsGeometry QgsGeometry::nearestPoint( const QgsGeometry &other ) const
{
  // avoid calling geos for trivial point calculations
  if ( d->geometry && QgsWkbTypes::flatType( d->geometry->wkbType() ) == QgsWkbTypes::Point )
  {
    return QgsGeometry( qgsgeometry_cast< const QgsPoint * >( d->geometry.get() )->clone() );
  }

  QgsGeos geos( d->geometry.get() );
  mLastError.clear();
  QgsGeometry result = geos.closestPoint( other );
  result.mLastError = mLastError;
  return result;
}

QgsGeometry QgsGeometry::shortestLine( const QgsGeometry &other ) const
{
  // avoid calling geos for trivial point-to-point line calculations
  if ( d->geometry && QgsWkbTypes::flatType( d->geometry->wkbType() ) == QgsWkbTypes::Point && QgsWkbTypes::flatType( other.wkbType() ) == QgsWkbTypes::Point )
  {
    return QgsGeometry( std::make_unique< QgsLineString >( *qgsgeometry_cast< const QgsPoint * >( d->geometry.get() ), *qgsgeometry_cast< const QgsPoint * >( other.constGet() ) ) );
  }

  QgsGeos geos( d->geometry.get() );
  mLastError.clear();
  QgsGeometry result = geos.shortestLine( other, &mLastError );
  result.mLastError = mLastError;
  return result;
}

double QgsGeometry::closestVertexWithContext( const QgsPointXY &point, int &atVertex ) const
{
  if ( !d->geometry )
  {
    return -1;
  }

  QgsVertexId vId;
  QgsPoint pt( point );
  QgsPoint closestPoint = QgsGeometryUtils::closestVertex( *( d->geometry ), pt, vId );
  if ( !vId.isValid() )
    return -1;
  atVertex = vertexNrFromVertexId( vId );
  return QgsGeometryUtils::sqrDistance2D( closestPoint, pt );
}

double QgsGeometry::closestSegmentWithContext( const QgsPointXY &point,
    QgsPointXY &minDistPoint,
    int &nextVertexIndex,
    int *leftOrRightOfSegment,
    double epsilon ) const
{
  if ( !d->geometry )
  {
    return -1;
  }

  QgsPoint segmentPt;
  QgsVertexId vertexAfter;

  double sqrDist = d->geometry->closestSegment( QgsPoint( point ), segmentPt,  vertexAfter, leftOrRightOfSegment, epsilon );
  if ( sqrDist < 0 )
    return -1;

  minDistPoint.setX( segmentPt.x() );
  minDistPoint.setY( segmentPt.y() );
  nextVertexIndex = vertexNrFromVertexId( vertexAfter );
  return sqrDist;
}

Qgis::GeometryOperationResult QgsGeometry::addRing( const QVector<QgsPointXY> &ring )
{
  std::unique_ptr< QgsLineString > ringLine = std::make_unique< QgsLineString >( ring );
  return addRing( ringLine.release() );
}

Qgis::GeometryOperationResult QgsGeometry::addRing( QgsCurve *ring )
{
  std::unique_ptr< QgsCurve > r( ring );
  if ( !d->geometry )
  {
    return Qgis::GeometryOperationResult::InvalidInputGeometryType;
  }

  detach();

  return QgsGeometryEditUtils::addRing( d->geometry.get(), std::move( r ) );
}

Qgis::GeometryOperationResult QgsGeometry::addPart( const QVector<QgsPointXY> &points, QgsWkbTypes::GeometryType geomType )
{
  QgsPointSequence l;
  convertPointList( points, l );
  return addPart( l, geomType );
}

Qgis::GeometryOperationResult QgsGeometry::addPart( const QgsPointSequence &points, QgsWkbTypes::GeometryType geomType )
{
  std::unique_ptr< QgsAbstractGeometry > partGeom;
  if ( points.size() == 1 )
  {
    partGeom = std::make_unique< QgsPoint >( points[0] );
  }
  else if ( points.size() > 1 )
  {
    std::unique_ptr< QgsLineString > ringLine = std::make_unique< QgsLineString >();
    ringLine->setPoints( points );
    partGeom = std::move( ringLine );
  }
  return addPart( partGeom.release(), geomType );
}

Qgis::GeometryOperationResult QgsGeometry::addPart( QgsAbstractGeometry *part, QgsWkbTypes::GeometryType geomType )
{
  std::unique_ptr< QgsAbstractGeometry > p( part );
  if ( !d->geometry )
  {
    switch ( geomType )
    {
      case QgsWkbTypes::PointGeometry:
        reset( std::make_unique< QgsMultiPoint >() );
        break;
      case QgsWkbTypes::LineGeometry:
        reset( std::make_unique< QgsMultiLineString >() );
        break;
      case QgsWkbTypes::PolygonGeometry:
        reset( std::make_unique< QgsMultiPolygon >() );
        break;
      default:
        reset( nullptr );
        return Qgis::GeometryOperationResult::AddPartNotMultiGeometry;
    }
  }
  else
  {
    detach();
  }

  convertToMultiType();
  return QgsGeometryEditUtils::addPart( d->geometry.get(), std::move( p ) );
}

Qgis::GeometryOperationResult QgsGeometry::addPart( const QgsGeometry &newPart )
{
  if ( !d->geometry )
  {
    return Qgis::GeometryOperationResult::InvalidBaseGeometry;
  }
  if ( newPart.isNull() || !newPart.d->geometry )
  {
    return Qgis::GeometryOperationResult::AddPartNotMultiGeometry;
  }

  return addPart( newPart.d->geometry->clone() );
}

QgsGeometry QgsGeometry::removeInteriorRings( double minimumRingArea ) const
{
  if ( !d->geometry || type() != QgsWkbTypes::PolygonGeometry )
  {
    return QgsGeometry();
  }

  if ( QgsWkbTypes::isMultiType( d->geometry->wkbType() ) )
  {
    const QVector<QgsGeometry> parts = asGeometryCollection();
    QVector<QgsGeometry> results;
    results.reserve( parts.count() );
    for ( const QgsGeometry &part : parts )
    {
      QgsGeometry result = part.removeInteriorRings( minimumRingArea );
      if ( !result.isNull() )
        results << result;
    }
    if ( results.isEmpty() )
      return QgsGeometry();

    QgsGeometry first = results.takeAt( 0 );
    for ( const QgsGeometry &result : std::as_const( results ) )
    {
      first.addPart( result );
    }
    return first;
  }
  else
  {
    std::unique_ptr< QgsCurvePolygon > newPoly( static_cast< QgsCurvePolygon * >( d->geometry->clone() ) );
    newPoly->removeInteriorRings( minimumRingArea );
    return QgsGeometry( std::move( newPoly ) );
  }
}

Qgis::GeometryOperationResult QgsGeometry::translate( double dx, double dy, double dz, double dm )
{
  if ( !d->geometry )
  {
    return Qgis::GeometryOperationResult::InvalidBaseGeometry;
  }

  detach();

  d->geometry->transform( QTransform::fromTranslate( dx, dy ), dz, 1.0, dm );
  return Qgis::GeometryOperationResult::Success;
}

Qgis::GeometryOperationResult QgsGeometry::rotate( double rotation, const QgsPointXY &center )
{
  if ( !d->geometry )
  {
    return Qgis::GeometryOperationResult::InvalidBaseGeometry;
  }

  detach();

  QTransform t = QTransform::fromTranslate( center.x(), center.y() );
  t.rotate( -rotation );
  t.translate( -center.x(), -center.y() );
  d->geometry->transform( t );
  return Qgis::GeometryOperationResult::Success;
}

Qgis::GeometryOperationResult QgsGeometry::splitGeometry( const QVector<QgsPointXY> &splitLine, QVector<QgsGeometry> &newGeometries, bool topological, QVector<QgsPointXY> &topologyTestPoints, bool splitFeature )
{
  QgsPointSequence split, topology;
  convertPointList( splitLine, split );
  convertPointList( topologyTestPoints, topology );
  Qgis::GeometryOperationResult result = splitGeometry( split, newGeometries, topological, topology, splitFeature );
  convertPointList( topology, topologyTestPoints );
  return result;
}
Qgis::GeometryOperationResult QgsGeometry::splitGeometry( const QgsPointSequence &splitLine, QVector<QgsGeometry> &newGeometries, bool topological, QgsPointSequence &topologyTestPoints, bool splitFeature, bool skipIntersectionTest )
{
  if ( !d->geometry )
  {
    return Qgis::GeometryOperationResult::InvalidBaseGeometry;
  }

  QVector<QgsGeometry > newGeoms;
  QgsLineString splitLineString( splitLine );

  /**
   * QGIS uses GEOS algorithm to split geometries.
   * Using 3D points in GEOS will returns an interpolation value which is the
   * mean between geometries.
   * On the contrary, in our logic, the interpolation is a linear interpolation
   * on the split point. By dropping Z/M value, GEOS will returns the expected
   * result. See https://github.com/qgis/QGIS/issues/33489
   */
  splitLineString.dropZValue();
  splitLineString.dropMValue();

  QgsGeos geos( d->geometry.get() );
  mLastError.clear();
  QgsGeometryEngine::EngineOperationResult result = geos.splitGeometry( splitLineString, newGeoms, topological, topologyTestPoints, &mLastError, skipIntersectionTest );

  if ( result == QgsGeometryEngine::Success )
  {
    if ( splitFeature )
      *this = newGeoms.takeAt( 0 );
    newGeometries = newGeoms;
  }

  switch ( result )
  {
    case QgsGeometryEngine::Success:
      return Qgis::GeometryOperationResult::Success;
    case QgsGeometryEngine::MethodNotImplemented:
    case QgsGeometryEngine::EngineError:
    case QgsGeometryEngine::NodedGeometryError:
      return Qgis::GeometryOperationResult::GeometryEngineError;
    case QgsGeometryEngine::InvalidBaseGeometry:
      return Qgis::GeometryOperationResult::InvalidBaseGeometry;
    case QgsGeometryEngine::InvalidInput:
      return Qgis::GeometryOperationResult::InvalidInputGeometryType;
    case QgsGeometryEngine::SplitCannotSplitPoint:
      return Qgis::GeometryOperationResult::SplitCannotSplitPoint;
    case QgsGeometryEngine::NothingHappened:
      return Qgis::GeometryOperationResult::NothingHappened;
      //default: do not implement default to handle properly all cases
  }

  // this should never be reached
  Q_ASSERT( false );
  return Qgis::GeometryOperationResult::NothingHappened;
}

Qgis::GeometryOperationResult QgsGeometry::splitGeometry( const QgsCurve *curve, QVector<QgsGeometry> &newGeometries, bool preserveCircular, bool topological, QgsPointSequence &topologyTestPoints, bool splitFeature )
{
  std::unique_ptr<QgsLineString> segmentizedLine( curve->curveToLine() );
  QgsPointSequence points;
  segmentizedLine->points( points );
  Qgis::GeometryOperationResult result = splitGeometry( points, newGeometries, topological, topologyTestPoints, splitFeature );

  if ( result == Qgis::GeometryOperationResult::Success )
  {
    if ( preserveCircular )
    {
      for ( int i = 0; i < newGeometries.count(); ++i )
        newGeometries[i] = newGeometries[i].convertToCurves();
      *this = convertToCurves();
    }
  }

  return result;
}

Qgis::GeometryOperationResult QgsGeometry::reshapeGeometry( const QgsLineString &reshapeLineString )
{
  if ( !d->geometry )
  {
    return Qgis::GeometryOperationResult::InvalidBaseGeometry;
  }

  QgsGeos geos( d->geometry.get() );
  QgsGeometryEngine::EngineOperationResult errorCode = QgsGeometryEngine::Success;
  mLastError.clear();
  std::unique_ptr< QgsAbstractGeometry > geom( geos.reshapeGeometry( reshapeLineString, &errorCode, &mLastError ) );
  if ( errorCode == QgsGeometryEngine::Success && geom )
  {
    reset( std::move( geom ) );
    return Qgis::GeometryOperationResult::Success;
  }

  switch ( errorCode )
  {
    case QgsGeometryEngine::Success:
      return Qgis::GeometryOperationResult::Success;
    case QgsGeometryEngine::MethodNotImplemented:
    case QgsGeometryEngine::EngineError:
    case QgsGeometryEngine::NodedGeometryError:
      return Qgis::GeometryOperationResult::GeometryEngineError;
    case QgsGeometryEngine::InvalidBaseGeometry:
      return Qgis::GeometryOperationResult::InvalidBaseGeometry;
    case QgsGeometryEngine::InvalidInput:
      return Qgis::GeometryOperationResult::InvalidInputGeometryType;
    case QgsGeometryEngine::SplitCannotSplitPoint: // should not happen
      return Qgis::GeometryOperationResult::GeometryEngineError;
    case QgsGeometryEngine::NothingHappened:
      return Qgis::GeometryOperationResult::NothingHappened;
  }

  // should not be reached
  return Qgis::GeometryOperationResult::GeometryEngineError;
}

int QgsGeometry::makeDifferenceInPlace( const QgsGeometry &other )
{
  if ( !d->geometry || !other.d->geometry )
  {
    return 0;
  }

  QgsGeos geos( d->geometry.get() );

  mLastError.clear();
  std::unique_ptr< QgsAbstractGeometry > diffGeom( geos.intersection( other.constGet(), &mLastError ) );
  if ( !diffGeom )
  {
    return 1;
  }

  reset( std::move( diffGeom ) );
  return 0;
}

QgsGeometry QgsGeometry::makeDifference( const QgsGeometry &other ) const
{
  if ( !d->geometry || other.isNull() )
  {
    return QgsGeometry();
  }

  QgsGeos geos( d->geometry.get() );

  mLastError.clear();
  std::unique_ptr< QgsAbstractGeometry > diffGeom( geos.intersection( other.constGet(), &mLastError ) );
  if ( !diffGeom )
  {
    QgsGeometry result;
    result.mLastError = mLastError;
    return result;
  }

  return QgsGeometry( diffGeom.release() );
}

QgsRectangle QgsGeometry::boundingBox() const
{
  if ( d->geometry )
  {
    return d->geometry->boundingBox();
  }
  return QgsRectangle();
}

QgsGeometry QgsGeometry::orientedMinimumBoundingBox( double &area, double &angle, double &width, double &height ) const
{
  mLastError.clear();
  QgsInternalGeometryEngine engine( *this );
  const QgsGeometry res = engine.orientedMinimumBoundingBox( area, angle, width, height );
  if ( res.isNull() )
    mLastError = engine.lastError();
  return res;
}

QgsGeometry QgsGeometry::orientedMinimumBoundingBox() const
{
  double area, angle, width, height;
  return orientedMinimumBoundingBox( area, angle, width, height );
}

static QgsCircle __recMinimalEnclosingCircle( QgsMultiPointXY points, QgsMultiPointXY boundary )
{
  auto l_boundary = boundary.length();
  QgsCircle circ_mec;
  if ( ( points.length() == 0 ) || ( l_boundary == 3 ) )
  {
    switch ( l_boundary )
    {
      case 0:
        circ_mec = QgsCircle();
        break;
      case 1:
        circ_mec = QgsCircle( QgsPoint( boundary.last() ), 0 );
        boundary.pop_back();
        break;
      case 2:
      {
        QgsPointXY p1 = boundary.last();
        boundary.pop_back();
        QgsPointXY p2 = boundary.last();
        boundary.pop_back();
        circ_mec = QgsCircle::from2Points( QgsPoint( p1 ), QgsPoint( p2 ) );
      }
      break;
      default:
        QgsPoint p1( boundary.at( 0 ) );
        QgsPoint p2( boundary.at( 1 ) );
        QgsPoint p3( boundary.at( 2 ) );
        circ_mec = QgsCircle::minimalCircleFrom3Points( p1, p2, p3 );
        break;
    }
    return circ_mec;
  }
  else
  {
    QgsPointXY pxy = points.last();
    points.pop_back();
    circ_mec = __recMinimalEnclosingCircle( points, boundary );
    QgsPoint p( pxy );
    if ( !circ_mec.contains( p ) )
    {
      boundary.append( pxy );
      circ_mec = __recMinimalEnclosingCircle( points, boundary );
    }
  }
  return circ_mec;
}

QgsGeometry QgsGeometry::minimalEnclosingCircle( QgsPointXY &center, double &radius, unsigned int segments ) const
{
  center = QgsPointXY();
  radius = 0;

  if ( isEmpty() )
  {
    return QgsGeometry();
  }

  /* optimization */
  QgsGeometry hull = convexHull();
  if ( hull.isNull() )
    return QgsGeometry();

  QgsMultiPointXY P = hull.convertToPoint( true ).asMultiPoint();
  QgsMultiPointXY R;

  QgsCircle circ = __recMinimalEnclosingCircle( P, R );
  center = QgsPointXY( circ.center() );
  radius = circ.radius();
  QgsGeometry geom;
  geom.set( circ.toPolygon( segments ) );
  return geom;

}

QgsGeometry QgsGeometry::minimalEnclosingCircle( unsigned int segments ) const
{
  QgsPointXY center;
  double radius;
  return minimalEnclosingCircle( center, radius, segments );

}

QgsGeometry QgsGeometry::orthogonalize( double tolerance, int maxIterations, double angleThreshold ) const
{
  QgsInternalGeometryEngine engine( *this );

  return engine.orthogonalize( tolerance, maxIterations, angleThreshold );
}

QgsGeometry QgsGeometry::triangularWaves( double wavelength, double amplitude, bool strictWavelength ) const
{
  QgsInternalGeometryEngine engine( *this );
  return engine.triangularWaves( wavelength, amplitude, strictWavelength );
}

QgsGeometry QgsGeometry::triangularWavesRandomized( double minimumWavelength, double maximumWavelength, double minimumAmplitude, double maximumAmplitude, unsigned long seed ) const
{
  QgsInternalGeometryEngine engine( *this );
  return engine.triangularWavesRandomized( minimumWavelength, maximumWavelength, minimumAmplitude, maximumAmplitude, seed );
}

QgsGeometry QgsGeometry::squareWaves( double wavelength, double amplitude, bool strictWavelength ) const
{
  QgsInternalGeometryEngine engine( *this );
  return engine.squareWaves( wavelength, amplitude, strictWavelength );
}

QgsGeometry QgsGeometry::squareWavesRandomized( double minimumWavelength, double maximumWavelength, double minimumAmplitude, double maximumAmplitude, unsigned long seed ) const
{
  QgsInternalGeometryEngine engine( *this );
  return engine.squareWavesRandomized( minimumWavelength, maximumWavelength, minimumAmplitude, maximumAmplitude, seed );
}

QgsGeometry QgsGeometry::roundWaves( double wavelength, double amplitude, bool strictWavelength ) const
{
  QgsInternalGeometryEngine engine( *this );
  return engine.roundWaves( wavelength, amplitude, strictWavelength );
}

QgsGeometry QgsGeometry::roundWavesRandomized( double minimumWavelength, double maximumWavelength, double minimumAmplitude, double maximumAmplitude, unsigned long seed ) const
{
  QgsInternalGeometryEngine engine( *this );
  return engine.roundWavesRandomized( minimumWavelength, maximumWavelength, minimumAmplitude, maximumAmplitude, seed );
}

QgsGeometry QgsGeometry::applyDashPattern( const QVector<double> &pattern, Qgis::DashPatternLineEndingRule startRule, Qgis::DashPatternLineEndingRule endRule, Qgis::DashPatternSizeAdjustment adjustment, double patternOffset ) const
{
  QgsInternalGeometryEngine engine( *this );
  return engine.applyDashPattern( pattern, startRule, endRule, adjustment, patternOffset );
}

QgsGeometry QgsGeometry::snappedToGrid( double hSpacing, double vSpacing, double dSpacing, double mSpacing ) const
{
  if ( !d->geometry )
  {
    return QgsGeometry();
  }
  return QgsGeometry( d->geometry->snappedToGrid( hSpacing, vSpacing, dSpacing, mSpacing ) );
}

bool QgsGeometry::removeDuplicateNodes( double epsilon, bool useZValues )
{
  if ( !d->geometry )
    return false;

  detach();
  return d->geometry->removeDuplicateNodes( epsilon, useZValues );
}

bool QgsGeometry::intersects( const QgsRectangle &r ) const
{
  // fast case, check bounding boxes
  if ( !boundingBoxIntersects( r ) )
    return false;

  // optimise trivial case for point intersections -- the bounding box test has already given us the answer
  if ( QgsWkbTypes::flatType( d->geometry->wkbType() ) == QgsWkbTypes::Point )
  {
    return true;
  }

  QgsGeometry g = fromRect( r );
  return intersects( g );
}

bool QgsGeometry::intersects( const QgsGeometry &geometry ) const
{
  if ( !d->geometry || geometry.isNull() )
  {
    return false;
  }

  QgsGeos geos( d->geometry.get() );
  mLastError.clear();
  return geos.intersects( geometry.d->geometry.get(), &mLastError );
}

bool QgsGeometry::boundingBoxIntersects( const QgsRectangle &rectangle ) const
{
  if ( !d->geometry )
  {
    return false;
  }

  return d->geometry->boundingBoxIntersects( rectangle );
}

bool QgsGeometry::boundingBoxIntersects( const QgsGeometry &geometry ) const
{
  if ( !d->geometry || geometry.isNull() )
  {
    return false;
  }

  return d->geometry->boundingBoxIntersects( geometry.constGet()->boundingBox() );
}

bool QgsGeometry::contains( const QgsPointXY *p ) const
{
  if ( !d->geometry || !p )
  {
    return false;
  }

  QgsPoint pt( p->x(), p->y() );
  QgsGeos geos( d->geometry.get() );
  mLastError.clear();
  return geos.contains( &pt, &mLastError );
}

bool QgsGeometry::contains( const QgsGeometry &geometry ) const
{
  if ( !d->geometry || geometry.isNull() )
  {
    return false;
  }

  QgsGeos geos( d->geometry.get() );
  mLastError.clear();
  return geos.contains( geometry.d->geometry.get(), &mLastError );
}

bool QgsGeometry::disjoint( const QgsGeometry &geometry ) const
{
  if ( !d->geometry || geometry.isNull() )
  {
    return false;
  }

  QgsGeos geos( d->geometry.get() );
  mLastError.clear();
  return geos.disjoint( geometry.d->geometry.get(), &mLastError );
}

bool QgsGeometry::equals( const QgsGeometry &geometry ) const
{
  if ( !d->geometry || geometry.isNull() )
  {
    return false;
  }

  // fast check - are they shared copies of the same underlying geometry?
  if ( d == geometry.d )
    return true;

  // fast check - distinct geometry types?
  if ( type() != geometry.type() )
    return false;

  // slower check - actually test the geometries
  return *d->geometry == *geometry.d->geometry;
}

bool QgsGeometry::touches( const QgsGeometry &geometry ) const
{
  if ( !d->geometry || geometry.isNull() )
  {
    return false;
  }

  QgsGeos geos( d->geometry.get() );
  mLastError.clear();
  return geos.touches( geometry.d->geometry.get(), &mLastError );
}

bool QgsGeometry::overlaps( const QgsGeometry &geometry ) const
{
  if ( !d->geometry || geometry.isNull() )
  {
    return false;
  }

  QgsGeos geos( d->geometry.get() );
  mLastError.clear();
  return geos.overlaps( geometry.d->geometry.get(), &mLastError );
}

bool QgsGeometry::within( const QgsGeometry &geometry ) const
{
  if ( !d->geometry || geometry.isNull() )
  {
    return false;
  }

  QgsGeos geos( d->geometry.get() );
  mLastError.clear();
  return geos.within( geometry.d->geometry.get(), &mLastError );
}

bool QgsGeometry::crosses( const QgsGeometry &geometry ) const
{
  if ( !d->geometry || geometry.isNull() )
  {
    return false;
  }

  QgsGeos geos( d->geometry.get() );
  mLastError.clear();
  return geos.crosses( geometry.d->geometry.get(), &mLastError );
}

QString QgsGeometry::asWkt( int precision ) const
{
  if ( !d->geometry )
  {
    return QString();
  }
  return d->geometry->asWkt( precision );
}

QString QgsGeometry::asJson( int precision ) const
{
  return QString::fromStdString( asJsonObject( precision ).dump() );
}

json QgsGeometry::asJsonObject( int precision ) const
{
  if ( !d->geometry )
  {
    return nullptr;
  }
  return d->geometry->asJsonObject( precision );

}

QVector<QgsGeometry> QgsGeometry::coerceToType( const QgsWkbTypes::Type type, double defaultZ, double defaultM ) const
{
  QVector< QgsGeometry > res;
  if ( isNull() )
    return res;

  if ( wkbType() == type || type == QgsWkbTypes::Unknown )
  {
    res << *this;
    return res;
  }

  if ( type == QgsWkbTypes::NoGeometry )
  {
    return res;
  }

  QgsGeometry newGeom = *this;

  // Curved -> straight
  if ( !QgsWkbTypes::isCurvedType( type ) && QgsWkbTypes::isCurvedType( newGeom.wkbType() ) )
  {
    newGeom = QgsGeometry( d->geometry.get()->segmentize() );
  }

  // polygon -> line
  if ( QgsWkbTypes::geometryType( type ) == QgsWkbTypes::LineGeometry &&
       newGeom.type() == QgsWkbTypes::PolygonGeometry )
  {
    // boundary gives us a (multi)line string of exterior + interior rings
    newGeom = QgsGeometry( newGeom.constGet()->boundary() );
  }
  // line -> polygon
  if ( QgsWkbTypes::geometryType( type ) == QgsWkbTypes::PolygonGeometry &&
       newGeom.type() == QgsWkbTypes::LineGeometry )
  {
    std::unique_ptr< QgsGeometryCollection > gc( QgsGeometryFactory::createCollectionOfType( type ) );
    const QgsGeometry source = newGeom;
    for ( auto part = source.const_parts_begin(); part != source.const_parts_end(); ++part )
    {
      std::unique_ptr< QgsAbstractGeometry > exterior( ( *part )->clone() );
      if ( QgsCurve *curve = qgsgeometry_cast< QgsCurve * >( exterior.get() ) )
      {
        if ( QgsWkbTypes::isCurvedType( type ) )
        {
          std::unique_ptr< QgsCurvePolygon > cp = std::make_unique< QgsCurvePolygon >();
          cp->setExteriorRing( curve );
          exterior.release();
          gc->addGeometry( cp.release() );
        }
        else
        {
          std::unique_ptr< QgsPolygon > p = std::make_unique< QgsPolygon  >();
          p->setExteriorRing( qgsgeometry_cast< QgsLineString * >( curve ) );
          exterior.release();
          gc->addGeometry( p.release() );
        }
      }
    }
    newGeom = QgsGeometry( std::move( gc ) );
  }

  // line/polygon -> points
  if ( QgsWkbTypes::geometryType( type ) == QgsWkbTypes::PointGeometry &&
       ( newGeom.type() == QgsWkbTypes::LineGeometry ||
         newGeom.type() == QgsWkbTypes::PolygonGeometry ) )
  {
    // lines/polygons to a point layer, extract all vertices
    std::unique_ptr< QgsMultiPoint > mp = std::make_unique< QgsMultiPoint >();
    const QgsGeometry source = newGeom;
    QSet< QgsPoint > added;
    for ( auto vertex = source.vertices_begin(); vertex != source.vertices_end(); ++vertex )
    {
      if ( added.contains( *vertex ) )
        continue; // avoid duplicate points, e.g. start/end of rings
      mp->addGeometry( ( *vertex ).clone() );
      added.insert( *vertex );
    }
    newGeom = QgsGeometry( std::move( mp ) );
  }

  // Single -> multi
  if ( QgsWkbTypes::isMultiType( type ) && ! newGeom.isMultipart( ) )
  {
    newGeom.convertToMultiType();
  }
  // Drop Z/M
  if ( newGeom.constGet()->is3D() && ! QgsWkbTypes::hasZ( type ) )
  {
    newGeom.get()->dropZValue();
  }
  if ( newGeom.constGet()->isMeasure() && ! QgsWkbTypes::hasM( type ) )
  {
    newGeom.get()->dropMValue();
  }
  // Add Z/M back, set to 0
  if ( ! newGeom.constGet()->is3D() && QgsWkbTypes::hasZ( type ) )
  {
    newGeom.get()->addZValue( defaultZ );
  }
  if ( ! newGeom.constGet()->isMeasure() && QgsWkbTypes::hasM( type ) )
  {
    newGeom.get()->addMValue( defaultM );
  }

  // Multi -> single
  if ( ! QgsWkbTypes::isMultiType( type ) && newGeom.isMultipart( ) )
  {
    const QgsGeometryCollection *parts( static_cast< const QgsGeometryCollection * >( newGeom.constGet() ) );
    res.reserve( parts->partCount() );
    for ( int i = 0; i < parts->partCount( ); i++ )
    {
      res << QgsGeometry( parts->geometryN( i )->clone() );
    }
  }
  else
  {
    res << newGeom;
  }
  return res;
}

QgsGeometry QgsGeometry::convertToType( QgsWkbTypes::GeometryType destType, bool destMultipart ) const
{
  switch ( destType )
  {
    case QgsWkbTypes::PointGeometry:
      return convertToPoint( destMultipart );

    case QgsWkbTypes::LineGeometry:
      return convertToLine( destMultipart );

    case QgsWkbTypes::PolygonGeometry:
      return convertToPolygon( destMultipart );

    default:
      return QgsGeometry();
  }
}

bool QgsGeometry::convertToMultiType()
{
  if ( !d->geometry )
  {
    return false;
  }

  if ( isMultipart() ) //already multitype, no need to convert
  {
    return true;
  }

  std::unique_ptr< QgsAbstractGeometry >geom = QgsGeometryFactory::geomFromWkbType( QgsWkbTypes::multiType( d->geometry->wkbType() ) );
  QgsGeometryCollection *multiGeom = qgsgeometry_cast<QgsGeometryCollection *>( geom.get() );
  if ( !multiGeom )
  {
    return false;
  }

  //try to avoid cloning existing geometry whenever we can

  //want to see a magic trick?... gather round kiddies...
  detach(); // maybe a clone, hopefully not if we're the only ref to the private data
  // now we cheat a bit and steal the private geometry and add it direct to the multigeom
  // we can do this because we're the only ref to this geometry, guaranteed by the detach call above
  multiGeom->addGeometry( d->geometry.release() );
  // and replace it with the multi geometry.
  // TADA! a clone free conversion in some cases
  d->geometry = std::move( geom );
  return true;
}

bool QgsGeometry::convertToSingleType()
{
  if ( !d->geometry )
  {
    return false;
  }

  if ( !isMultipart() ) //already single part, no need to convert
  {
    return true;
  }

  QgsGeometryCollection *multiGeom = qgsgeometry_cast<QgsGeometryCollection *>( d->geometry.get() );
  if ( !multiGeom || multiGeom->partCount() < 1 )
    return false;

  std::unique_ptr< QgsAbstractGeometry > firstPart( multiGeom->geometryN( 0 )->clone() );
  reset( std::move( firstPart ) );
  return true;
}


bool QgsGeometry::convertGeometryCollectionToSubclass( QgsWkbTypes::GeometryType geomType )
{
  const QgsGeometryCollection *origGeom = qgsgeometry_cast<const QgsGeometryCollection *>( constGet() );
  if ( !origGeom )
    return false;

  std::unique_ptr<QgsGeometryCollection> resGeom;
  switch ( geomType )
  {
    case QgsWkbTypes::PointGeometry:
      resGeom = std::make_unique<QgsMultiPoint>();
      break;
    case QgsWkbTypes::LineGeometry:
      resGeom = std::make_unique<QgsMultiLineString>();
      break;
    case QgsWkbTypes::PolygonGeometry:
      resGeom = std::make_unique<QgsMultiPolygon>();
      break;
    default:
      break;
  }
  if ( !resGeom )
    return false;

  resGeom->reserve( origGeom->numGeometries() );
  for ( int i = 0; i < origGeom->numGeometries(); ++i )
  {
    const QgsAbstractGeometry *g = origGeom->geometryN( i );
    if ( QgsWkbTypes::geometryType( g->wkbType() ) == geomType )
      resGeom->addGeometry( g->clone() );
  }

  set( resGeom.release() );
  return true;
}


QgsPointXY QgsGeometry::asPoint() const
{
  if ( !d->geometry )
  {
    return QgsPointXY();
  }
  if ( QgsPoint *pt = qgsgeometry_cast<QgsPoint *>( d->geometry->simplifiedTypeRef() ) )
  {
    return QgsPointXY( pt->x(), pt->y() );
  }
  else
  {
    return QgsPointXY();
  }
}

QgsPolylineXY QgsGeometry::asPolyline() const
{
  QgsPolylineXY polyLine;
  if ( !d->geometry )
  {
    return polyLine;
  }

  bool doSegmentation = ( QgsWkbTypes::flatType( d->geometry->wkbType() ) == QgsWkbTypes::CompoundCurve
                          || QgsWkbTypes::flatType( d->geometry->wkbType() ) == QgsWkbTypes::CircularString );
  std::unique_ptr< QgsLineString > segmentizedLine;
  QgsLineString *line = nullptr;
  if ( doSegmentation )
  {
    QgsCurve *curve = qgsgeometry_cast<QgsCurve *>( d->geometry.get() );
    if ( !curve )
    {
      return polyLine;
    }
    segmentizedLine.reset( curve->curveToLine() );
    line = segmentizedLine.get();
  }
  else
  {
    line = qgsgeometry_cast<QgsLineString *>( d->geometry.get() );
    if ( !line )
    {
      return polyLine;
    }
  }

  int nVertices = line->numPoints();
  polyLine.resize( nVertices );
  QgsPointXY *data = polyLine.data();
  const double *xData = line->xData();
  const double *yData = line->yData();
  for ( int i = 0; i < nVertices; ++i )
  {
    data->setX( *xData++ );
    data->setY( *yData++ );
    data++;
  }

  return polyLine;
}

QgsPolygonXY QgsGeometry::asPolygon() const
{
  if ( !d->geometry )
    return QgsPolygonXY();

  bool doSegmentation = ( QgsWkbTypes::flatType( d->geometry->wkbType() ) == QgsWkbTypes::CurvePolygon );

  QgsPolygon *p = nullptr;
  std::unique_ptr< QgsPolygon > segmentized;
  if ( doSegmentation )
  {
    QgsCurvePolygon *curvePoly = qgsgeometry_cast<QgsCurvePolygon *>( d->geometry.get() );
    if ( !curvePoly )
    {
      return QgsPolygonXY();
    }
    segmentized.reset( curvePoly->toPolygon() );
    p = segmentized.get();
  }
  else
  {
    p = qgsgeometry_cast<QgsPolygon *>( d->geometry.get() );
  }

  if ( !p )
  {
    return QgsPolygonXY();
  }

  QgsPolygonXY polygon;
  convertPolygon( *p, polygon );

  return polygon;
}

QgsMultiPointXY QgsGeometry::asMultiPoint() const
{
  if ( !d->geometry || QgsWkbTypes::flatType( d->geometry->wkbType() ) != QgsWkbTypes::MultiPoint )
  {
    return QgsMultiPointXY();
  }

  const QgsMultiPoint *mp = qgsgeometry_cast<QgsMultiPoint *>( d->geometry.get() );
  if ( !mp )
  {
    return QgsMultiPointXY();
  }

  int nPoints = mp->numGeometries();
  QgsMultiPointXY multiPoint( nPoints );
  for ( int i = 0; i < nPoints; ++i )
  {
    const QgsPoint *pt = mp->pointN( i );
    multiPoint[i].setX( pt->x() );
    multiPoint[i].setY( pt->y() );
  }
  return multiPoint;
}

QgsMultiPolylineXY QgsGeometry::asMultiPolyline() const
{
  if ( !d->geometry )
  {
    return QgsMultiPolylineXY();
  }

  QgsGeometryCollection *geomCollection = qgsgeometry_cast<QgsGeometryCollection *>( d->geometry.get() );
  if ( !geomCollection )
  {
    return QgsMultiPolylineXY();
  }

  int nLines = geomCollection->numGeometries();
  if ( nLines < 1 )
  {
    return QgsMultiPolylineXY();
  }

  QgsMultiPolylineXY mpl;
  mpl.reserve( nLines );
  for ( int i = 0; i < nLines; ++i )
  {
    const QgsLineString *line = qgsgeometry_cast<const QgsLineString *>( geomCollection->geometryN( i ) );
    std::unique_ptr< QgsLineString > segmentized;
    if ( !line )
    {
      const QgsCurve *curve = qgsgeometry_cast<const QgsCurve *>( geomCollection->geometryN( i ) );
      if ( !curve )
      {
        continue;
      }
      segmentized.reset( curve->curveToLine() );
      line = segmentized.get();
    }

    QgsPolylineXY polyLine;
    int nVertices = line->numPoints();
    polyLine.resize( nVertices );
    QgsPointXY *data = polyLine.data();
    const double *xData = line->xData();
    const double *yData = line->yData();
    for ( int i = 0; i < nVertices; ++i )
    {
      data->setX( *xData++ );
      data->setY( *yData++ );
      data++;
    }
    mpl.append( polyLine );
  }
  return mpl;
}

QgsMultiPolygonXY QgsGeometry::asMultiPolygon() const
{
  if ( !d->geometry )
  {
    return QgsMultiPolygonXY();
  }

  const QgsGeometryCollection *geomCollection = qgsgeometry_cast<const QgsGeometryCollection *>( d->geometry.get() );
  if ( !geomCollection )
  {
    return QgsMultiPolygonXY();
  }

  const int nPolygons = geomCollection->numGeometries();
  if ( nPolygons < 1 )
  {
    return QgsMultiPolygonXY();
  }

  QgsMultiPolygonXY mp;
  mp.reserve( nPolygons );
  for ( int i = 0; i < nPolygons; ++i )
  {
    const QgsPolygon *polygon = qgsgeometry_cast<const QgsPolygon *>( geomCollection->geometryN( i ) );
    if ( !polygon )
    {
      const QgsCurvePolygon *cPolygon = qgsgeometry_cast<const QgsCurvePolygon *>( geomCollection->geometryN( i ) );
      if ( cPolygon )
      {
        polygon = cPolygon->toPolygon();
      }
      else
      {
        continue;
      }
    }

    QgsPolygonXY poly;
    convertPolygon( *polygon, poly );
    mp.push_back( poly );
  }
  return mp;
}

double QgsGeometry::area() const
{
  if ( !d->geometry )
  {
    return -1.0;
  }

  return d->geometry->area();
}

double QgsGeometry::length() const
{
  if ( !d->geometry )
  {
    return -1.0;
  }

  switch ( QgsWkbTypes::geometryType( d->geometry->wkbType() ) )
  {
    case QgsWkbTypes::PointGeometry:
      return 0.0;

    case QgsWkbTypes::LineGeometry:
      return d->geometry->length();

    case QgsWkbTypes::PolygonGeometry:
      return d->geometry->perimeter();

    case QgsWkbTypes::UnknownGeometry:
    case QgsWkbTypes::NullGeometry:
      return d->geometry->length();
  }
  return -1;
}

double QgsGeometry::distance( const QgsGeometry &geom ) const
{
  if ( !d->geometry || !geom.d->geometry )
  {
    return -1.0;
  }

  // avoid calling geos for trivial point-to-point distance calculations
  if ( QgsWkbTypes::flatType( d->geometry->wkbType() ) == QgsWkbTypes::Point && QgsWkbTypes::flatType( geom.wkbType() ) == QgsWkbTypes::Point )
  {
    return qgsgeometry_cast< const QgsPoint * >( d->geometry.get() )->distance( *qgsgeometry_cast< const QgsPoint * >( geom.constGet() ) );
  }

  QgsGeos g( d->geometry.get() );
  mLastError.clear();
  return g.distance( geom.d->geometry.get(), &mLastError );
}

double QgsGeometry::hausdorffDistance( const QgsGeometry &geom ) const
{
  if ( !d->geometry || !geom.d->geometry )
  {
    return -1.0;
  }

  QgsGeos g( d->geometry.get() );
  mLastError.clear();
  return g.hausdorffDistance( geom.d->geometry.get(), &mLastError );
}

double QgsGeometry::hausdorffDistanceDensify( const QgsGeometry &geom, double densifyFraction ) const
{
  if ( !d->geometry || !geom.d->geometry )
  {
    return -1.0;
  }

  QgsGeos g( d->geometry.get() );
  mLastError.clear();
  return g.hausdorffDistanceDensify( geom.d->geometry.get(), densifyFraction, &mLastError );
}


double QgsGeometry::frechetDistance( const QgsGeometry &geom ) const
{
  if ( !d->geometry || !geom.d->geometry )
  {
    return -1.0;
  }

  QgsGeos g( d->geometry.get() );
  mLastError.clear();
  return g.frechetDistance( geom.d->geometry.get(), &mLastError );
}

double QgsGeometry::frechetDistanceDensify( const QgsGeometry &geom, double densifyFraction ) const
{
  if ( !d->geometry || !geom.d->geometry )
  {
    return -1.0;
  }

  QgsGeos g( d->geometry.get() );
  mLastError.clear();
  return g.frechetDistanceDensify( geom.d->geometry.get(), densifyFraction, &mLastError );
}

QgsAbstractGeometry::vertex_iterator QgsGeometry::vertices_begin() const
{
  if ( !d->geometry || d->geometry.get()->isEmpty() )
    return QgsAbstractGeometry::vertex_iterator();
  return d->geometry->vertices_begin();
}

QgsAbstractGeometry::vertex_iterator QgsGeometry::vertices_end() const
{
  if ( !d->geometry || d->geometry.get()->isEmpty() )
    return QgsAbstractGeometry::vertex_iterator();
  return d->geometry->vertices_end();
}

QgsVertexIterator QgsGeometry::vertices() const
{
  if ( !d->geometry || d->geometry.get()->isEmpty() )
    return QgsVertexIterator();
  return QgsVertexIterator( d->geometry.get() );
}

QgsAbstractGeometry::part_iterator QgsGeometry::parts_begin()
{
  if ( !d->geometry )
    return QgsAbstractGeometry::part_iterator();

  detach();
  return d->geometry->parts_begin();
}

QgsAbstractGeometry::part_iterator QgsGeometry::parts_end()
{
  if ( !d->geometry )
    return QgsAbstractGeometry::part_iterator();
  return d->geometry->parts_end();
}

QgsAbstractGeometry::const_part_iterator QgsGeometry::const_parts_begin() const
{
  if ( !d->geometry )
    return QgsAbstractGeometry::const_part_iterator();
  return d->geometry->const_parts_begin();
}

QgsAbstractGeometry::const_part_iterator QgsGeometry::const_parts_end() const
{
  if ( !d->geometry )
    return QgsAbstractGeometry::const_part_iterator();
  return d->geometry->const_parts_end();
}

QgsGeometryPartIterator QgsGeometry::parts()
{
  if ( !d->geometry )
    return QgsGeometryPartIterator();

  detach();
  return QgsGeometryPartIterator( d->geometry.get() );
}

QgsGeometryConstPartIterator QgsGeometry::constParts() const
{
  if ( !d->geometry )
    return QgsGeometryConstPartIterator();

  return QgsGeometryConstPartIterator( d->geometry.get() );
}

QgsGeometry QgsGeometry::buffer( double distance, int segments ) const
{
  if ( !d->geometry )
  {
    return QgsGeometry();
  }

  QgsGeos g( d->geometry.get() );
  mLastError.clear();
  std::unique_ptr<QgsAbstractGeometry> geom( g.buffer( distance, segments, &mLastError ) );
  if ( !geom )
  {
    QgsGeometry result;
    result.mLastError = mLastError;
    return result;
  }
  return QgsGeometry( std::move( geom ) );
}

QgsGeometry QgsGeometry::buffer( double distance, int segments, Qgis::EndCapStyle endCapStyle, Qgis::JoinStyle joinStyle, double miterLimit ) const
{
  if ( !d->geometry )
  {
    return QgsGeometry();
  }

  QgsGeos g( d->geometry.get() );
  mLastError.clear();
  QgsAbstractGeometry *geom = g.buffer( distance, segments, endCapStyle, joinStyle, miterLimit, &mLastError );
  if ( !geom )
  {
    QgsGeometry result;
    result.mLastError = mLastError;
    return result;
  }
  return QgsGeometry( geom );
}

QgsGeometry QgsGeometry::offsetCurve( double distance, int segments, Qgis::JoinStyle joinStyle, double miterLimit ) const
{
  if ( !d->geometry || type() != QgsWkbTypes::LineGeometry )
  {
    return QgsGeometry();
  }

  if ( QgsWkbTypes::isMultiType( d->geometry->wkbType() ) )
  {
    const QVector<QgsGeometry> parts = asGeometryCollection();
    QVector<QgsGeometry> results;
    results.reserve( parts.count() );
    for ( const QgsGeometry &part : parts )
    {
      QgsGeometry result = part.offsetCurve( distance, segments, joinStyle, miterLimit );
      if ( !result.isNull() )
        results << result;
    }
    if ( results.isEmpty() )
      return QgsGeometry();

    QgsGeometry first = results.takeAt( 0 );
    for ( const QgsGeometry &result : std::as_const( results ) )
    {
      first.addPart( result );
    }
    return first;
  }
  else
  {
    QgsGeos geos( d->geometry.get() );
    mLastError.clear();

    // GEOS can flip the curve orientation in some circumstances. So record previous orientation and correct if required
    const Qgis::AngularDirection prevOrientation = qgsgeometry_cast< const QgsCurve * >( d->geometry.get() )->orientation();

    std::unique_ptr< QgsAbstractGeometry > offsetGeom( geos.offsetCurve( distance, segments, joinStyle, miterLimit, &mLastError ) );
    if ( !offsetGeom )
    {
      QgsGeometry result;
      result.mLastError = mLastError;
      return result;
    }

    if ( const QgsCurve *offsetCurve = qgsgeometry_cast< const QgsCurve * >( offsetGeom.get() ) )
    {
      const Qgis::AngularDirection newOrientation = offsetCurve->orientation();
      if ( newOrientation != prevOrientation )
      {
        // GEOS has flipped line orientation, flip it back
        std::unique_ptr< QgsAbstractGeometry > flipped( offsetCurve->reversed() );
        offsetGeom = std::move( flipped );
      }
    }
    return QgsGeometry( std::move( offsetGeom ) );
  }
}

QgsGeometry QgsGeometry::singleSidedBuffer( double distance, int segments, Qgis::BufferSide side, Qgis::JoinStyle joinStyle, double miterLimit ) const
{
  if ( !d->geometry || type() != QgsWkbTypes::LineGeometry )
  {
    return QgsGeometry();
  }

  if ( QgsWkbTypes::isMultiType( d->geometry->wkbType() ) )
  {
    const QVector<QgsGeometry> parts = asGeometryCollection();
    QVector<QgsGeometry> results;
    results.reserve( parts.count() );
    for ( const QgsGeometry &part : parts )
    {
      QgsGeometry result = part.singleSidedBuffer( distance, segments, side, joinStyle, miterLimit );
      if ( !result.isNull() )
        results << result;
    }
    if ( results.isEmpty() )
      return QgsGeometry();

    QgsGeometry first = results.takeAt( 0 );
    for ( const QgsGeometry &result : std::as_const( results ) )
    {
      first.addPart( result );
    }
    return first;
  }
  else
  {
    QgsGeos geos( d->geometry.get() );
    mLastError.clear();
    std::unique_ptr< QgsAbstractGeometry > bufferGeom = geos.singleSidedBuffer( distance, segments, side,
        joinStyle, miterLimit, &mLastError );
    if ( !bufferGeom )
    {
      QgsGeometry result;
      result.mLastError = mLastError;
      return result;
    }
    return QgsGeometry( std::move( bufferGeom ) );
  }
}

QgsGeometry QgsGeometry::taperedBuffer( double startWidth, double endWidth, int segments ) const
{
  QgsInternalGeometryEngine engine( *this );

  return engine.taperedBuffer( startWidth, endWidth, segments );
}

QgsGeometry QgsGeometry::variableWidthBufferByM( int segments ) const
{
  QgsInternalGeometryEngine engine( *this );

  return engine.variableWidthBufferByM( segments );
}

QgsGeometry QgsGeometry::extendLine( double startDistance, double endDistance ) const
{
  if ( !d->geometry || type() != QgsWkbTypes::LineGeometry )
  {
    return QgsGeometry();
  }

  if ( QgsWkbTypes::isMultiType( d->geometry->wkbType() ) )
  {
    const QVector<QgsGeometry> parts = asGeometryCollection();
    QVector<QgsGeometry> results;
    results.reserve( parts.count() );
    for ( const QgsGeometry &part : parts )
    {
      QgsGeometry result = part.extendLine( startDistance, endDistance );
      if ( !result.isNull() )
        results << result;
    }
    if ( results.isEmpty() )
      return QgsGeometry();

    QgsGeometry first = results.takeAt( 0 );
    for ( const QgsGeometry &result : std::as_const( results ) )
    {
      first.addPart( result );
    }
    return first;
  }
  else
  {
    QgsLineString *line = qgsgeometry_cast< QgsLineString * >( d->geometry.get() );
    if ( !line )
      return QgsGeometry();

    std::unique_ptr< QgsLineString > newLine( line->clone() );
    newLine->extend( startDistance, endDistance );
    return QgsGeometry( std::move( newLine ) );
  }
}

QgsGeometry QgsGeometry::simplify( double tolerance ) const
{
  if ( !d->geometry )
  {
    return QgsGeometry();
  }

  QgsGeos geos( d->geometry.get() );
  mLastError.clear();
  std::unique_ptr< QgsAbstractGeometry > simplifiedGeom( geos.simplify( tolerance, &mLastError ) );
  if ( !simplifiedGeom )
  {
    QgsGeometry result;
    result.mLastError = mLastError;
    return result;
  }
  return QgsGeometry( std::move( simplifiedGeom ) );
}

QgsGeometry QgsGeometry::densifyByCount( int extraNodesPerSegment ) const
{
  QgsInternalGeometryEngine engine( *this );

  return engine.densifyByCount( extraNodesPerSegment );
}

QgsGeometry QgsGeometry::densifyByDistance( double distance ) const
{
  QgsInternalGeometryEngine engine( *this );

  return engine.densifyByDistance( distance );
}

QgsGeometry QgsGeometry::convertToCurves( double distanceTolerance, double angleTolerance ) const
{
  QgsInternalGeometryEngine engine( *this );

  return engine.convertToCurves( distanceTolerance, angleTolerance );
}

QgsGeometry QgsGeometry::centroid() const
{
  if ( !d->geometry )
  {
    return QgsGeometry();
  }

  // avoid calling geos for trivial point centroids
  if ( QgsWkbTypes::flatType( d->geometry->wkbType() ) == QgsWkbTypes::Point )
  {
    QgsGeometry c = *this;
    c.get()->dropZValue();
    c.get()->dropMValue();
    return c;
  }

  QgsGeos geos( d->geometry.get() );

  mLastError.clear();
  QgsGeometry result( geos.centroid( &mLastError ) );
  result.mLastError = mLastError;
  return result;
}

QgsGeometry QgsGeometry::pointOnSurface() const
{
  if ( !d->geometry )
  {
    return QgsGeometry();
  }

  QgsGeos geos( d->geometry.get() );

  mLastError.clear();
  QgsGeometry result( geos.pointOnSurface( &mLastError ) );
  result.mLastError = mLastError;
  return result;
}

QgsGeometry QgsGeometry::poleOfInaccessibility( double precision, double *distanceToBoundary ) const
{
  QgsInternalGeometryEngine engine( *this );

  return engine.poleOfInaccessibility( precision, distanceToBoundary );
}

QgsGeometry QgsGeometry::largestEmptyCircle( double tolerance,  const QgsGeometry &boundary ) const
{
  if ( !d->geometry )
  {
    return QgsGeometry();
  }

  QgsGeos geos( d->geometry.get() );

  mLastError.clear();
  QgsGeometry result( geos.largestEmptyCircle( tolerance, boundary.constGet(), &mLastError ) );
  result.mLastError = mLastError;
  return result;
}

QgsGeometry QgsGeometry::minimumWidth() const
{
  if ( !d->geometry )
  {
    return QgsGeometry();
  }

  QgsGeos geos( d->geometry.get() );

  mLastError.clear();
  QgsGeometry result( geos.minimumWidth( &mLastError ) );
  result.mLastError = mLastError;
  return result;
}

double QgsGeometry::minimumClearance() const
{
  if ( !d->geometry )
  {
    return std::numeric_limits< double >::quiet_NaN();
  }

  QgsGeos geos( d->geometry.get() );

  mLastError.clear();
  return geos.minimumClearance( &mLastError );
}

QgsGeometry QgsGeometry::minimumClearanceLine() const
{
  if ( !d->geometry )
  {
    return QgsGeometry();
  }

  QgsGeos geos( d->geometry.get() );

  mLastError.clear();
  QgsGeometry result( geos.minimumClearanceLine( &mLastError ) );
  result.mLastError = mLastError;
  return result;
}

QgsGeometry QgsGeometry::convexHull() const
{
  if ( !d->geometry )
  {
    return QgsGeometry();
  }
  QgsGeos geos( d->geometry.get() );
  mLastError.clear();
  std::unique_ptr< QgsAbstractGeometry > cHull( geos.convexHull( &mLastError ) );
  if ( !cHull )
  {
    QgsGeometry geom;
    geom.mLastError = mLastError;
    return geom;
  }
  return QgsGeometry( std::move( cHull ) );
}

QgsGeometry QgsGeometry::voronoiDiagram( const QgsGeometry &extent, double tolerance, bool edgesOnly ) const
{
  if ( !d->geometry )
  {
    return QgsGeometry();
  }

  QgsGeos geos( d->geometry.get() );
  mLastError.clear();
  QgsGeometry result = geos.voronoiDiagram( extent.constGet(), tolerance, edgesOnly, &mLastError );
  result.mLastError = mLastError;
  return result;
}

QgsGeometry QgsGeometry::delaunayTriangulation( double tolerance, bool edgesOnly ) const
{
  if ( !d->geometry )
  {
    return QgsGeometry();
  }

  QgsGeos geos( d->geometry.get() );
  mLastError.clear();
  QgsGeometry result = geos.delaunayTriangulation( tolerance, edgesOnly );
  result.mLastError = mLastError;
  return result;
}

QgsGeometry QgsGeometry::node() const
{
  if ( !d->geometry )
  {
    return QgsGeometry();
  }

  QgsGeos geos( d->geometry.get() );
  mLastError.clear();
  QgsGeometry result( geos.node( &mLastError ) );
  result.mLastError = mLastError;
  return result;
}

QgsGeometry QgsGeometry::sharedPaths( const QgsGeometry &other ) const
{
  if ( !d->geometry )
  {
    return QgsGeometry();
  }

  QgsGeos geos( d->geometry.get() );
  mLastError.clear();
  QgsGeometry result( geos.sharedPaths( other.constGet(), &mLastError ) );
  result.mLastError = mLastError;
  return result;
}

QgsGeometry QgsGeometry::subdivide( int maxNodes ) const
{
  if ( !d->geometry )
  {
    return QgsGeometry();
  }

  const QgsAbstractGeometry *geom = d->geometry.get();
  std::unique_ptr< QgsAbstractGeometry > segmentizedCopy;
  if ( QgsWkbTypes::isCurvedType( d->geometry->wkbType() ) )
  {
    segmentizedCopy.reset( d->geometry->segmentize() );
    geom = segmentizedCopy.get();
  }

  QgsGeos geos( geom );
  mLastError.clear();
  std::unique_ptr< QgsAbstractGeometry > result( geos.subdivide( maxNodes, &mLastError ) );
  if ( !result )
  {
    QgsGeometry geom;
    geom.mLastError = mLastError;
    return geom;
  }
  return QgsGeometry( std::move( result ) );
}

QgsGeometry QgsGeometry::interpolate( double distance ) const
{
  if ( !d->geometry )
  {
    return QgsGeometry();
  }

  QgsGeometry line = *this;
  if ( type() == QgsWkbTypes::PointGeometry )
    return QgsGeometry();
  else if ( type() == QgsWkbTypes::PolygonGeometry )
  {
    line = QgsGeometry( d->geometry->boundary() );
  }

  const QgsCurve *curve = nullptr;
  if ( line.isMultipart() )
  {
    // if multi part, iterate through parts to find target part
    const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( line.constGet() );
    for ( int part = 0; part < collection->numGeometries(); ++part )
    {
      const QgsCurve *candidate = qgsgeometry_cast< const QgsCurve * >( collection->geometryN( part ) );
      if ( !candidate )
        continue;
      const double candidateLength = candidate->length();
      if ( candidateLength >= distance )
      {
        curve = candidate;
        break;
      }

      distance -= candidateLength;
    }
  }
  else
  {
    curve = qgsgeometry_cast< const QgsCurve * >( line.constGet() );
  }
  if ( !curve )
    return QgsGeometry();

  std::unique_ptr< QgsPoint > result( curve->interpolatePoint( distance ) );
  if ( !result )
  {
    return QgsGeometry();
  }
  return QgsGeometry( std::move( result ) );
}

double QgsGeometry::lineLocatePoint( const QgsGeometry &point ) const
{
  if ( type() != QgsWkbTypes::LineGeometry )
    return -1;

  if ( QgsWkbTypes::flatType( point.wkbType() ) != QgsWkbTypes::Point )
    return -1;

  QgsGeometry segmentized = *this;
  if ( QgsWkbTypes::isCurvedType( wkbType() ) )
  {
    segmentized = QgsGeometry( static_cast< QgsCurve * >( d->geometry.get() )->segmentize() );
  }

  QgsGeos geos( d->geometry.get() );
  mLastError.clear();
  return geos.lineLocatePoint( *( static_cast< QgsPoint * >( point.d->geometry.get() ) ), &mLastError );
}

double QgsGeometry::interpolateAngle( double distance ) const
{
  if ( !d->geometry )
    return 0.0;

  const QgsAbstractGeometry *geom = d->geometry->simplifiedTypeRef();
  if ( QgsWkbTypes::geometryType( geom->wkbType() ) == QgsWkbTypes::PointGeometry )
    return 0.0;

  // always operate on segmentized geometries
  QgsGeometry segmentized = *this;
  if ( QgsWkbTypes::isCurvedType( geom->wkbType() ) )
  {
    segmentized = QgsGeometry( static_cast< const QgsCurve * >( geom )->segmentize() );
  }

  QgsVertexId previous;
  QgsVertexId next;
  if ( !QgsGeometryUtils::verticesAtDistance( *segmentized.constGet(), distance, previous, next ) )
    return 0.0;

  if ( previous == next )
  {
    // distance coincided exactly with a vertex
    QgsVertexId v2 = previous;
    QgsVertexId v1;
    QgsVertexId v3;
    segmentized.constGet()->adjacentVertices( v2, v1, v3 );
    if ( v1.isValid() && v3.isValid() )
    {
      QgsPoint p1 = segmentized.constGet()->vertexAt( v1 );
      QgsPoint p2 = segmentized.constGet()->vertexAt( v2 );
      QgsPoint p3 = segmentized.constGet()->vertexAt( v3 );
      double angle1 = QgsGeometryUtils::lineAngle( p1.x(), p1.y(), p2.x(), p2.y() );
      double angle2 = QgsGeometryUtils::lineAngle( p2.x(), p2.y(), p3.x(), p3.y() );
      return QgsGeometryUtils::averageAngle( angle1, angle2 );
    }
    else if ( v3.isValid() )
    {
      QgsPoint p1 = segmentized.constGet()->vertexAt( v2 );
      QgsPoint p2 = segmentized.constGet()->vertexAt( v3 );
      return QgsGeometryUtils::lineAngle( p1.x(), p1.y(), p2.x(), p2.y() );
    }
    else
    {
      QgsPoint p1 = segmentized.constGet()->vertexAt( v1 );
      QgsPoint p2 = segmentized.constGet()->vertexAt( v2 );
      return QgsGeometryUtils::lineAngle( p1.x(), p1.y(), p2.x(), p2.y() );
    }
  }
  else
  {
    QgsPoint p1 = segmentized.constGet()->vertexAt( previous );
    QgsPoint p2 = segmentized.constGet()->vertexAt( next );
    return QgsGeometryUtils::lineAngle( p1.x(), p1.y(), p2.x(), p2.y() );
  }
}

QgsGeometry QgsGeometry::intersection( const QgsGeometry &geometry ) const
{
  if ( !d->geometry || geometry.isNull() )
  {
    return QgsGeometry();
  }

  QgsGeos geos( d->geometry.get() );

  mLastError.clear();
  std::unique_ptr< QgsAbstractGeometry > resultGeom( geos.intersection( geometry.d->geometry.get(), &mLastError ) );

  if ( !resultGeom )
  {
    QgsGeometry geom;
    geom.mLastError = mLastError;
    return geom;
  }

  return QgsGeometry( std::move( resultGeom ) );
}

QgsGeometry QgsGeometry::combine( const QgsGeometry &geometry ) const
{
  if ( !d->geometry || geometry.isNull() )
  {
    return QgsGeometry();
  }

  QgsGeos geos( d->geometry.get() );
  mLastError.clear();
  std::unique_ptr< QgsAbstractGeometry > resultGeom( geos.combine( geometry.d->geometry.get(), &mLastError ) );
  if ( !resultGeom )
  {
    QgsGeometry geom;
    geom.mLastError = mLastError;
    return geom;
  }
  return QgsGeometry( std::move( resultGeom ) );
}

QgsGeometry QgsGeometry::mergeLines() const
{
  if ( !d->geometry )
  {
    return QgsGeometry();
  }

  if ( QgsWkbTypes::flatType( d->geometry->wkbType() ) == QgsWkbTypes::LineString )
  {
    // special case - a single linestring was passed
    return QgsGeometry( *this );
  }

  QgsGeos geos( d->geometry.get() );
  mLastError.clear();
  QgsGeometry result = geos.mergeLines( &mLastError );
  result.mLastError = mLastError;
  return result;
}

QgsGeometry QgsGeometry::difference( const QgsGeometry &geometry ) const
{
  if ( !d->geometry || geometry.isNull() )
  {
    return QgsGeometry();
  }

  QgsGeos geos( d->geometry.get() );

  mLastError.clear();
  std::unique_ptr< QgsAbstractGeometry > resultGeom( geos.difference( geometry.d->geometry.get(), &mLastError ) );
  if ( !resultGeom )
  {
    QgsGeometry geom;
    geom.mLastError = mLastError;
    return geom;
  }
  return QgsGeometry( std::move( resultGeom ) );
}

QgsGeometry QgsGeometry::symDifference( const QgsGeometry &geometry ) const
{
  if ( !d->geometry || geometry.isNull() )
  {
    return QgsGeometry();
  }

  QgsGeos geos( d->geometry.get() );

  mLastError.clear();
  std::unique_ptr< QgsAbstractGeometry > resultGeom( geos.symDifference( geometry.d->geometry.get(), &mLastError ) );
  if ( !resultGeom )
  {
    QgsGeometry geom;
    geom.mLastError = mLastError;
    return geom;
  }
  return QgsGeometry( std::move( resultGeom ) );
}

QgsGeometry QgsGeometry::extrude( double x, double y )
{
  QgsInternalGeometryEngine engine( *this );

  return engine.extrude( x, y );
}

///@cond PRIVATE // avoid dox warning

QVector<QgsPointXY> QgsGeometry::randomPointsInPolygon( int count, const std::function< bool( const QgsPointXY & ) > &acceptPoint, unsigned long seed, QgsFeedback *feedback, int maxTriesPerPoint ) const
{
  if ( type() != QgsWkbTypes::PolygonGeometry )
    return QVector< QgsPointXY >();

  return QgsInternalGeometryEngine::randomPointsInPolygon( *this, count, acceptPoint, seed, feedback, maxTriesPerPoint );
}

QVector<QgsPointXY> QgsGeometry::randomPointsInPolygon( int count, unsigned long seed, QgsFeedback *feedback ) const
{
  if ( type() != QgsWkbTypes::PolygonGeometry )
    return QVector< QgsPointXY >();

  return QgsInternalGeometryEngine::randomPointsInPolygon( *this, count, []( const QgsPointXY & ) { return true; }, seed, feedback, 0 );
}
///@endcond

int QgsGeometry::wkbSize( QgsAbstractGeometry::WkbFlags flags ) const
{
  return d->geometry ? d->geometry->wkbSize( flags ) : 0;
}

QByteArray QgsGeometry::asWkb( QgsAbstractGeometry::WkbFlags flags ) const
{
  return d->geometry ? d->geometry->asWkb( flags ) : QByteArray();
}

QVector<QgsGeometry> QgsGeometry::asGeometryCollection() const
{
  QVector<QgsGeometry> geometryList;
  if ( !d->geometry )
  {
    return geometryList;
  }

  QgsGeometryCollection *gc = qgsgeometry_cast<QgsGeometryCollection *>( d->geometry.get() );
  if ( gc )
  {
    int numGeom = gc->numGeometries();
    geometryList.reserve( numGeom );
    for ( int i = 0; i < numGeom; ++i )
    {
      geometryList.append( QgsGeometry( gc->geometryN( i )->clone() ) );
    }
  }
  else //a singlepart geometry
  {
    geometryList.append( *this );
  }

  return geometryList;
}

QPointF QgsGeometry::asQPointF() const
{
  QgsPointXY point = asPoint();
  return point.toQPointF();
}

QPolygonF QgsGeometry::asQPolygonF() const
{
  const QgsAbstractGeometry *part = constGet();

  // if a geometry collection, get first part only
  if ( const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection *>( part ) )
  {
    if ( collection->numGeometries() > 0 )
      part = collection->geometryN( 0 );
    else
      return QPolygonF();
  }

  if ( const QgsCurve *curve = qgsgeometry_cast< const QgsCurve * >( part ) )
    return curve->asQPolygonF();
  else if ( const QgsCurvePolygon *polygon = qgsgeometry_cast< const QgsCurvePolygon * >( part ) )
    return polygon->exteriorRing() ? polygon->exteriorRing()->asQPolygonF() : QPolygonF();
  return QPolygonF();
}

bool QgsGeometry::deleteRing( int ringNum, int partNum )
{
  if ( !d->geometry )
  {
    return false;
  }

  detach();
  bool ok = QgsGeometryEditUtils::deleteRing( d->geometry.get(), ringNum, partNum );
  return ok;
}

bool QgsGeometry::deletePart( int partNum )
{
  if ( !d->geometry )
  {
    return false;
  }

  if ( !isMultipart() && partNum < 1 )
  {
    set( nullptr );
    return true;
  }

  detach();
  bool ok = QgsGeometryEditUtils::deletePart( d->geometry.get(), partNum );
  return ok;
}

int QgsGeometry::avoidIntersections( const QList<QgsVectorLayer *> &avoidIntersectionsLayers, const QHash<QgsVectorLayer *, QSet<QgsFeatureId> > &ignoreFeatures )
{
  if ( !d->geometry )
  {
    return 1;
  }

  QgsWkbTypes::Type geomTypeBeforeModification = wkbType();

  bool haveInvalidGeometry = false;
  std::unique_ptr< QgsAbstractGeometry > diffGeom = QgsGeometryEditUtils::avoidIntersections( *( d->geometry ), avoidIntersectionsLayers, haveInvalidGeometry, ignoreFeatures );
  if ( diffGeom )
  {
    reset( std::move( diffGeom ) );
  }

  if ( geomTypeBeforeModification != wkbType() )
    return 2;
  if ( haveInvalidGeometry )
    return 4;

  return 0;
}


QgsGeometry QgsGeometry::makeValid() const
{
  if ( !d->geometry )
    return QgsGeometry();

  mLastError.clear();
#if GEOS_VERSION_MAJOR>3 || ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR>=8 )
  QgsGeos geos( d->geometry.get() );
  std::unique_ptr< QgsAbstractGeometry > g( geos.makeValid( &mLastError ) );
#else
  std::unique_ptr< QgsAbstractGeometry > g( _qgis_lwgeom_make_valid( d->geometry.get(), mLastError ) );
#endif

  QgsGeometry result = QgsGeometry( std::move( g ) );
  result.mLastError = mLastError;
  return result;
}

QgsGeometry QgsGeometry::forceRHR() const
{
  return forcePolygonClockwise();
}

QgsGeometry QgsGeometry::forcePolygonClockwise() const
{
  if ( !d->geometry )
    return QgsGeometry();

  if ( isMultipart() )
  {
    const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( d->geometry.get() );
    std::unique_ptr< QgsGeometryCollection > newCollection( collection->createEmptyWithSameType() );
    newCollection->reserve( collection->numGeometries() );
    for ( int i = 0; i < collection->numGeometries(); ++i )
    {
      const QgsAbstractGeometry *g = collection->geometryN( i );
      if ( const QgsCurvePolygon *cp = qgsgeometry_cast< const QgsCurvePolygon * >( g ) )
      {
        std::unique_ptr< QgsCurvePolygon > corrected( cp->clone() );
        corrected->forceClockwise();
        newCollection->addGeometry( corrected.release() );
      }
      else
      {
        newCollection->addGeometry( g->clone() );
      }
    }
    return QgsGeometry( std::move( newCollection ) );
  }
  else
  {
    if ( const QgsCurvePolygon *cp = qgsgeometry_cast< const QgsCurvePolygon * >( d->geometry.get() ) )
    {
      std::unique_ptr< QgsCurvePolygon > corrected( cp->clone() );
      corrected->forceClockwise();
      return QgsGeometry( std::move( corrected ) );
    }
    else
    {
      // not a curve polygon, so return unchanged
      return *this;
    }
  }
}

QgsGeometry QgsGeometry::forcePolygonCounterClockwise() const
{
  if ( !d->geometry )
    return QgsGeometry();

  if ( isMultipart() )
  {
    const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( d->geometry.get() );
    std::unique_ptr< QgsGeometryCollection > newCollection( collection->createEmptyWithSameType() );
    newCollection->reserve( collection->numGeometries() );
    for ( int i = 0; i < collection->numGeometries(); ++i )
    {
      const QgsAbstractGeometry *g = collection->geometryN( i );
      if ( const QgsCurvePolygon *cp = qgsgeometry_cast< const QgsCurvePolygon * >( g ) )
      {
        std::unique_ptr< QgsCurvePolygon > corrected( cp->clone() );
        corrected->forceCounterClockwise();
        newCollection->addGeometry( corrected.release() );
      }
      else
      {
        newCollection->addGeometry( g->clone() );
      }
    }
    return QgsGeometry( std::move( newCollection ) );
  }
  else
  {
    if ( const QgsCurvePolygon *cp = qgsgeometry_cast< const QgsCurvePolygon * >( d->geometry.get() ) )
    {
      std::unique_ptr< QgsCurvePolygon > corrected( cp->clone() );
      corrected->forceCounterClockwise();
      return QgsGeometry( std::move( corrected ) );
    }
    else
    {
      // not a curve polygon, so return unchanged
      return *this;
    }
  }
}


void QgsGeometry::validateGeometry( QVector<QgsGeometry::Error> &errors, const Qgis::GeometryValidationEngine method, const Qgis::GeometryValidityFlags flags ) const
{
  errors.clear();
  if ( !d->geometry )
    return;

  // avoid expensive calcs for trivial point geometries
  if ( QgsWkbTypes::geometryType( d->geometry->wkbType() ) == QgsWkbTypes::PointGeometry )
  {
    return;
  }

  switch ( method )
  {
    case Qgis::GeometryValidationEngine::QgisInternal:
      QgsGeometryValidator::validateGeometry( *this, errors, method );
      return;

    case Qgis::GeometryValidationEngine::Geos:
    {
      QgsGeos geos( d->geometry.get() );
      QString error;
      QgsGeometry errorLoc;
      if ( !geos.isValid( &error, flags & Qgis::GeometryValidityFlag::AllowSelfTouchingHoles, &errorLoc ) )
      {
        if ( errorLoc.isNull() )
        {
          errors.append( QgsGeometry::Error( error ) );
        }
        else
        {
          const QgsPointXY point = errorLoc.asPoint();
          errors.append( QgsGeometry::Error( error, point ) );
        }
        return;
      }
    }
  }
}

void QgsGeometry::normalize()
{
  if ( !d->geometry )
  {
    return;
  }

  detach();
  d->geometry->normalize();
}

bool QgsGeometry::isGeosValid( Qgis::GeometryValidityFlags flags ) const
{
  if ( !d->geometry )
  {
    return false;
  }

  return d->geometry->isValid( mLastError, flags );
}

bool QgsGeometry::isSimple() const
{
  if ( !d->geometry )
    return false;

  QgsGeos geos( d->geometry.get() );
  mLastError.clear();
  return geos.isSimple( &mLastError );
}

bool QgsGeometry::isAxisParallelRectangle( double maximumDeviation, bool simpleRectanglesOnly ) const
{
  if ( !d->geometry )
    return false;

  QgsInternalGeometryEngine engine( *this );
  return engine.isAxisParallelRectangle( maximumDeviation, simpleRectanglesOnly );
}

bool QgsGeometry::isGeosEqual( const QgsGeometry &g ) const
{
  if ( !d->geometry || !g.d->geometry )
  {
    return false;
  }

  // fast check - are they shared copies of the same underlying geometry?
  if ( d == g.d )
    return true;

  // fast check - distinct geometry types?
  if ( type() != g.type() )
    return false;

  // avoid calling geos for trivial point case
  if ( QgsWkbTypes::flatType( d->geometry->wkbType() ) == QgsWkbTypes::Point
       && QgsWkbTypes::flatType( g.d->geometry->wkbType() ) == QgsWkbTypes::Point )
  {
    return equals( g );
  }

  //  another nice fast check upfront -- if the bounding boxes aren't equal, the geometries themselves can't be equal!
  if ( d->geometry->boundingBox() != g.d->geometry->boundingBox() )
    return false;

  QgsGeos geos( d->geometry.get() );
  mLastError.clear();
  return geos.isEqual( g.d->geometry.get(), &mLastError );
}

QgsGeometry QgsGeometry::unaryUnion( const QVector<QgsGeometry> &geometries )
{
  QgsGeos geos( nullptr );

  QString error;
  std::unique_ptr< QgsAbstractGeometry > geom( geos.combine( geometries, &error ) );
  QgsGeometry result( std::move( geom ) );
  result.mLastError = error;
  return result;
}

QgsGeometry QgsGeometry::polygonize( const QVector<QgsGeometry> &geometryList )
{
  QVector<const QgsAbstractGeometry *> geomV2List;
  for ( const QgsGeometry &g : geometryList )
  {
    if ( !( g.isNull() ) )
    {
      geomV2List.append( g.constGet() );
    }
  }

  QString error;
  QgsGeometry result = QgsGeos::polygonize( geomV2List, &error );
  result.mLastError = error;
  return result;
}

void QgsGeometry::convertToStraightSegment( double tolerance, QgsAbstractGeometry::SegmentationToleranceType toleranceType )
{
  if ( !d->geometry || !requiresConversionToStraightSegments() )
  {
    return;
  }

  std::unique_ptr< QgsAbstractGeometry > straightGeom( d->geometry->segmentize( tolerance, toleranceType ) );
  reset( std::move( straightGeom ) );
}

bool QgsGeometry::requiresConversionToStraightSegments() const
{
  if ( !d->geometry )
  {
    return false;
  }

  return d->geometry->hasCurvedSegments();
}

Qgis::GeometryOperationResult QgsGeometry::transform( const QgsCoordinateTransform &ct, const Qgis::TransformDirection direction, const bool transformZ )
{
  if ( !d->geometry )
  {
    return Qgis::GeometryOperationResult::InvalidBaseGeometry;
  }

  detach();
  d->geometry->transform( ct, direction, transformZ );
  return Qgis::GeometryOperationResult::Success;
}

Qgis::GeometryOperationResult QgsGeometry::transform( const QTransform &ct, double zTranslate, double zScale, double mTranslate, double mScale )
{
  if ( !d->geometry )
  {
    return Qgis::GeometryOperationResult::InvalidBaseGeometry;
  }

  detach();
  d->geometry->transform( ct, zTranslate, zScale, mTranslate, mScale );
  return Qgis::GeometryOperationResult::Success;
}

void QgsGeometry::mapToPixel( const QgsMapToPixel &mtp )
{
  if ( d->geometry )
  {
    detach();
    d->geometry->transform( mtp.transform() );
  }
}

QgsGeometry QgsGeometry::clipped( const QgsRectangle &rectangle )
{
  if ( !d->geometry || rectangle.isNull() || rectangle.isEmpty() )
  {
    return QgsGeometry();
  }

  QgsGeos geos( d->geometry.get() );
  mLastError.clear();
  std::unique_ptr< QgsAbstractGeometry > resultGeom = geos.clip( rectangle, &mLastError );
  if ( !resultGeom )
  {
    QgsGeometry result;
    result.mLastError = mLastError;
    return result;
  }
  return QgsGeometry( std::move( resultGeom ) );
}

void QgsGeometry::draw( QPainter &p ) const
{
  if ( d->geometry )
  {
    d->geometry->draw( p );
  }
}

static bool vertexIndexInfo( const QgsAbstractGeometry *g, int vertexIndex, int &partIndex, int &ringIndex, int &vertex )
{
  if ( vertexIndex < 0 )
    return false;  // clearly something wrong

  if ( const QgsGeometryCollection *geomCollection = qgsgeometry_cast<const QgsGeometryCollection *>( g ) )
  {
    partIndex = 0;
    for ( int i = 0; i < geomCollection->numGeometries(); ++i )
    {
      const QgsAbstractGeometry *part = geomCollection->geometryN( i );

      // count total number of vertices in the part
      int numPoints = 0;
      for ( int k = 0; k < part->ringCount(); ++k )
        numPoints += part->vertexCount( 0, k );

      if ( vertexIndex < numPoints )
      {
        int nothing;
        return vertexIndexInfo( part, vertexIndex, nothing, ringIndex, vertex ); // set ring_index + index
      }
      vertexIndex -= numPoints;
      partIndex++;
    }
  }
  else if ( const QgsCurvePolygon *curvePolygon = qgsgeometry_cast<const QgsCurvePolygon *>( g ) )
  {
    const QgsCurve *ring = curvePolygon->exteriorRing();
    if ( vertexIndex < ring->numPoints() )
    {
      partIndex = 0;
      ringIndex = 0;
      vertex = vertexIndex;
      return true;
    }
    vertexIndex -= ring->numPoints();
    ringIndex = 1;
    for ( int i = 0; i < curvePolygon->numInteriorRings(); ++i )
    {
      const QgsCurve *ring = curvePolygon->interiorRing( i );
      if ( vertexIndex < ring->numPoints() )
      {
        partIndex = 0;
        vertex = vertexIndex;
        return true;
      }
      vertexIndex -= ring->numPoints();
      ringIndex += 1;
    }
  }
  else if ( const QgsCurve *curve = qgsgeometry_cast<const QgsCurve *>( g ) )
  {
    if ( vertexIndex < curve->numPoints() )
    {
      partIndex = 0;
      ringIndex = 0;
      vertex = vertexIndex;
      return true;
    }
  }
  else if ( qgsgeometry_cast<const QgsPoint *>( g ) )
  {
    if ( vertexIndex == 0 )
    {
      partIndex = 0;
      ringIndex = 0;
      vertex = 0;
      return true;
    }
  }

  return false;
}

bool QgsGeometry::vertexIdFromVertexNr( int nr, QgsVertexId &id ) const
{
  if ( !d->geometry )
  {
    return false;
  }

  id.type = Qgis::VertexType::Segment;

  bool res = vertexIndexInfo( d->geometry.get(), nr, id.part, id.ring, id.vertex );
  if ( !res )
    return false;

  // now let's find out if it is a straight or circular segment
  const QgsAbstractGeometry *g = d->geometry.get();
  if ( const QgsGeometryCollection *geomCollection = qgsgeometry_cast<const QgsGeometryCollection *>( g ) )
  {
    g = geomCollection->geometryN( id.part );
  }

  if ( const QgsCurvePolygon *curvePolygon = qgsgeometry_cast<const QgsCurvePolygon *>( g ) )
  {
    g = id.ring == 0 ? curvePolygon->exteriorRing() : curvePolygon->interiorRing( id.ring - 1 );
  }

  if ( const QgsCurve *curve = qgsgeometry_cast<const QgsCurve *>( g ) )
  {
    QgsPoint p;
    res = curve->pointAt( id.vertex, p, id.type );
    if ( !res )
      return false;
  }

  return true;
}

int QgsGeometry::vertexNrFromVertexId( QgsVertexId id ) const
{
  if ( !d->geometry )
  {
    return -1;
  }
  return d->geometry->vertexNumberFromVertexId( id );
}

QString QgsGeometry::lastError() const
{
  return mLastError;
}

void QgsGeometry::filterVertices( const std::function<bool ( const QgsPoint & )> &filter )
{
  if ( !d->geometry )
    return;

  detach();

  d->geometry->filterVertices( filter );
}

void QgsGeometry::transformVertices( const std::function<QgsPoint( const QgsPoint & )> &transform )
{
  if ( !d->geometry )
    return;

  detach();

  d->geometry->transformVertices( transform );
}

void QgsGeometry::convertPointList( const QVector<QgsPointXY> &input, QgsPointSequence &output )
{
  output.clear();
  for ( const QgsPointXY &p : input )
  {
    output.append( QgsPoint( p ) );
  }
}

void QgsGeometry::convertPointList( const QgsPointSequence &input, QVector<QgsPointXY> &output )
{
  output.clear();
  for ( const QgsPoint &p : input )
  {
    output.append( QgsPointXY( p.x(), p.y() ) );
  }
}

void QgsGeometry::convertPolygon( const QgsPolygon &input, QgsPolygonXY &output )
{
  output.clear();

  auto convertRing = []( const QgsCurve * ring ) -> QgsPolylineXY
  {
    QgsPolylineXY res;
    bool doSegmentation = ( QgsWkbTypes::flatType( ring->wkbType() ) == QgsWkbTypes::CompoundCurve
                            || QgsWkbTypes::flatType( ring->wkbType() ) == QgsWkbTypes::CircularString );
    std::unique_ptr< QgsLineString > segmentizedLine;
    const QgsLineString *line = nullptr;
    if ( doSegmentation )
    {
      segmentizedLine.reset( ring->curveToLine() );
      line = segmentizedLine.get();
    }
    else
    {
      line = qgsgeometry_cast<const QgsLineString *>( ring );
      if ( !line )
      {
        return res;
      }
    }

    int nVertices = line->numPoints();
    res.resize( nVertices );
    QgsPointXY *data = res.data();
    const double *xData = line->xData();
    const double *yData = line->yData();
    for ( int i = 0; i < nVertices; ++i )
    {
      data->setX( *xData++ );
      data->setY( *yData++ );
      data++;
    }
    return res;
  };

  if ( const QgsCurve *exterior = input.exteriorRing() )
  {
    output.push_back( convertRing( exterior ) );
  }

  const int interiorRingCount = input.numInteriorRings();
  output.reserve( output.size() + interiorRingCount );
  for ( int n = 0; n < interiorRingCount; ++n )
  {
    output.push_back( convertRing( input.interiorRing( n ) ) );
  }
}

QgsGeometry QgsGeometry::fromQPointF( QPointF point )
{
  return QgsGeometry( std::make_unique< QgsPoint >( point.x(), point.y() ) );
}

QgsGeometry QgsGeometry::fromQPolygonF( const QPolygonF &polygon )
{
  std::unique_ptr < QgsLineString > ring( QgsLineString::fromQPolygonF( polygon ) );

  if ( polygon.isClosed() )
  {
    std::unique_ptr< QgsPolygon > poly = std::make_unique< QgsPolygon >();
    poly->setExteriorRing( ring.release() );
    return QgsGeometry( std::move( poly ) );
  }
  else
  {
    return QgsGeometry( std::move( ring ) );
  }
}

QgsPolygonXY QgsGeometry::createPolygonFromQPolygonF( const QPolygonF &polygon )
{
  Q_NOWARN_DEPRECATED_PUSH
  QgsPolygonXY result;
  result << createPolylineFromQPolygonF( polygon );
  return result;
  Q_NOWARN_DEPRECATED_POP
}

QgsPolylineXY QgsGeometry::createPolylineFromQPolygonF( const QPolygonF &polygon )
{
  QgsPolylineXY result;
  result.reserve( polygon.count() );
  for ( const QPointF &p : polygon )
  {
    result.append( QgsPointXY( p ) );
  }
  return result;
}

bool QgsGeometry::compare( const QgsPolylineXY &p1, const QgsPolylineXY &p2, double epsilon )
{
  if ( p1.count() != p2.count() )
    return false;

  for ( int i = 0; i < p1.count(); ++i )
  {
    if ( !p1.at( i ).compare( p2.at( i ), epsilon ) )
      return false;
  }
  return true;
}

bool QgsGeometry::compare( const QgsPolygonXY &p1, const QgsPolygonXY &p2, double epsilon )
{
  if ( p1.count() != p2.count() )
    return false;

  for ( int i = 0; i < p1.count(); ++i )
  {
    if ( !QgsGeometry::compare( p1.at( i ), p2.at( i ), epsilon ) )
      return false;
  }
  return true;
}


bool QgsGeometry::compare( const QgsMultiPolygonXY &p1, const QgsMultiPolygonXY &p2, double epsilon )
{
  if ( p1.count() != p2.count() )
    return false;

  for ( int i = 0; i < p1.count(); ++i )
  {
    if ( !QgsGeometry::compare( p1.at( i ), p2.at( i ), epsilon ) )
      return false;
  }
  return true;
}

QgsGeometry QgsGeometry::smooth( const unsigned int iterations, const double offset, double minimumDistance, double maxAngle ) const
{
  if ( !d->geometry || d->geometry->isEmpty() )
    return QgsGeometry();

  QgsGeometry geom = *this;
  if ( QgsWkbTypes::isCurvedType( wkbType() ) )
    geom = QgsGeometry( d->geometry->segmentize() );

  switch ( QgsWkbTypes::flatType( geom.wkbType() ) )
  {
    case QgsWkbTypes::Point:
    case QgsWkbTypes::MultiPoint:
      //can't smooth a point based geometry
      return geom;

    case QgsWkbTypes::LineString:
    {
      const QgsLineString *lineString = qgsgeometry_cast< const QgsLineString * >( geom.constGet() );
      return QgsGeometry( smoothLine( *lineString, iterations, offset, minimumDistance, maxAngle ) );
    }

    case QgsWkbTypes::MultiLineString:
    {
      const QgsMultiLineString *multiLine = qgsgeometry_cast< const QgsMultiLineString * >( geom.constGet() );

      std::unique_ptr< QgsMultiLineString > resultMultiline = std::make_unique< QgsMultiLineString> ();
      resultMultiline->reserve( multiLine->numGeometries() );
      for ( int i = 0; i < multiLine->numGeometries(); ++i )
      {
        resultMultiline->addGeometry( smoothLine( *( multiLine->lineStringN( i ) ), iterations, offset, minimumDistance, maxAngle ).release() );
      }
      return QgsGeometry( std::move( resultMultiline ) );
    }

    case QgsWkbTypes::Polygon:
    {
      const QgsPolygon *poly = qgsgeometry_cast< const QgsPolygon * >( geom.constGet() );
      return QgsGeometry( smoothPolygon( *poly, iterations, offset, minimumDistance, maxAngle ) );
    }

    case QgsWkbTypes::MultiPolygon:
    {
      const QgsMultiPolygon *multiPoly = qgsgeometry_cast< const QgsMultiPolygon * >( geom.constGet() );

      std::unique_ptr< QgsMultiPolygon > resultMultiPoly = std::make_unique< QgsMultiPolygon >();
      resultMultiPoly->reserve( multiPoly->numGeometries() );
      for ( int i = 0; i < multiPoly->numGeometries(); ++i )
      {
        resultMultiPoly->addGeometry( smoothPolygon( *( multiPoly->polygonN( i ) ), iterations, offset, minimumDistance, maxAngle ).release() );
      }
      return QgsGeometry( std::move( resultMultiPoly ) );
    }

    case QgsWkbTypes::Unknown:
    default:
      return QgsGeometry( *this );
  }
}

std::unique_ptr< QgsLineString > smoothCurve( const QgsLineString &line, const unsigned int iterations,
    const double offset, double squareDistThreshold, double maxAngleRads,
    bool isRing )
{
  std::unique_ptr< QgsLineString > result = std::make_unique< QgsLineString >( line );
  QgsPointSequence outputLine;
  for ( unsigned int iteration = 0; iteration < iterations; ++iteration )
  {
    outputLine.resize( 0 );
    outputLine.reserve( 2 * ( result->numPoints() - 1 ) );
    bool skipFirst = false;
    bool skipLast = false;
    if ( isRing )
    {
      QgsPoint p1 = result->pointN( result->numPoints() - 2 );
      QgsPoint p2 = result->pointN( 0 );
      QgsPoint p3 = result->pointN( 1 );
      double angle = QgsGeometryUtils::angleBetweenThreePoints( p1.x(), p1.y(), p2.x(), p2.y(),
                     p3.x(), p3.y() );
      angle = std::fabs( M_PI - angle );
      skipFirst = angle > maxAngleRads;
    }
    for ( int i = 0; i < result->numPoints() - 1; i++ )
    {
      QgsPoint p1 = result->pointN( i );
      QgsPoint p2 = result->pointN( i + 1 );

      double angle = M_PI;
      if ( i == 0 && isRing )
      {
        QgsPoint p3 = result->pointN( result->numPoints() - 2 );
        angle = QgsGeometryUtils::angleBetweenThreePoints( p1.x(), p1.y(), p2.x(), p2.y(),
                p3.x(), p3.y() );
      }
      else if ( i < result->numPoints() - 2 )
      {
        QgsPoint p3 = result->pointN( i + 2 );
        angle = QgsGeometryUtils::angleBetweenThreePoints( p1.x(), p1.y(), p2.x(), p2.y(),
                p3.x(), p3.y() );
      }
      else if ( i == result->numPoints() - 2 && isRing )
      {
        QgsPoint p3 = result->pointN( 1 );
        angle = QgsGeometryUtils::angleBetweenThreePoints( p1.x(), p1.y(), p2.x(), p2.y(),
                p3.x(), p3.y() );
      }

      skipLast = angle < M_PI - maxAngleRads || angle > M_PI + maxAngleRads;

      // don't apply distance threshold to first or last segment
      if ( i == 0 || i >= result->numPoints() - 2
           || QgsGeometryUtils::sqrDistance2D( p1, p2 ) > squareDistThreshold )
      {
        if ( !isRing )
        {
          if ( !skipFirst )
            outputLine << ( i == 0 ? result->pointN( i ) : QgsGeometryUtils::interpolatePointOnLine( p1, p2, offset ) );
          if ( !skipLast )
            outputLine << ( i == result->numPoints() - 2 ? result->pointN( i + 1 ) : QgsGeometryUtils::interpolatePointOnLine( p1, p2, 1.0 - offset ) );
          else
            outputLine << p2;
        }
        else
        {
          // ring
          if ( !skipFirst )
            outputLine << QgsGeometryUtils::interpolatePointOnLine( p1, p2, offset );
          else if ( i == 0 )
            outputLine << p1;
          if ( !skipLast )
            outputLine << QgsGeometryUtils::interpolatePointOnLine( p1, p2, 1.0 - offset );
          else
            outputLine << p2;
        }
      }
      skipFirst = skipLast;
    }

    if ( isRing && outputLine.at( 0 ) != outputLine.at( outputLine.count() - 1 ) )
      outputLine << outputLine.at( 0 );

    result->setPoints( outputLine );
  }
  return result;
}

std::unique_ptr<QgsLineString> QgsGeometry::smoothLine( const QgsLineString &line, const unsigned int iterations, const double offset, double minimumDistance, double maxAngle ) const
{
  double maxAngleRads = maxAngle * M_PI / 180.0;
  double squareDistThreshold = minimumDistance > 0 ? minimumDistance * minimumDistance : -1;
  return smoothCurve( line, iterations, offset, squareDistThreshold, maxAngleRads, false );
}

std::unique_ptr<QgsPolygon> QgsGeometry::smoothPolygon( const QgsPolygon &polygon, const unsigned int iterations, const double offset, double minimumDistance, double maxAngle ) const
{
  double maxAngleRads = maxAngle * M_PI / 180.0;
  double squareDistThreshold = minimumDistance > 0 ? minimumDistance * minimumDistance : -1;
  std::unique_ptr< QgsPolygon > resultPoly = std::make_unique< QgsPolygon >();

  resultPoly->setExteriorRing( smoothCurve( *( static_cast< const QgsLineString *>( polygon.exteriorRing() ) ), iterations, offset,
                               squareDistThreshold, maxAngleRads, true ).release() );

  for ( int i = 0; i < polygon.numInteriorRings(); ++i )
  {
    resultPoly->addInteriorRing( smoothCurve( *( static_cast< const QgsLineString *>( polygon.interiorRing( i ) ) ), iterations, offset,
                                 squareDistThreshold, maxAngleRads, true ).release() );
  }
  return resultPoly;
}

QgsGeometry QgsGeometry::convertToPoint( bool destMultipart ) const
{
  switch ( type() )
  {
    case QgsWkbTypes::PointGeometry:
    {
      bool srcIsMultipart = isMultipart();

      if ( ( destMultipart && srcIsMultipart ) ||
           ( !destMultipart && !srcIsMultipart ) )
      {
        // return a copy of the same geom
        return QgsGeometry( *this );
      }
      if ( destMultipart )
      {
        // layer is multipart => make a multipoint with a single point
        return fromMultiPointXY( QgsMultiPointXY() << asPoint() );
      }
      else
      {
        // destination is singlepart => make a single part if possible
        QgsMultiPointXY multiPoint = asMultiPoint();
        if ( multiPoint.count() == 1 )
        {
          return fromPointXY( multiPoint[0] );
        }
      }
      return QgsGeometry();
    }

    case QgsWkbTypes::LineGeometry:
    {
      // only possible if destination is multipart
      if ( !destMultipart )
        return QgsGeometry();

      // input geometry is multipart
      if ( isMultipart() )
      {
        const QgsMultiPolylineXY multiLine = asMultiPolyline();
        QgsMultiPointXY multiPoint;
        for ( const QgsPolylineXY &l : multiLine )
          for ( const QgsPointXY &p : l )
            multiPoint << p;
        return fromMultiPointXY( multiPoint );
      }
      // input geometry is not multipart: copy directly the line into a multipoint
      else
      {
        QgsPolylineXY line = asPolyline();
        if ( !line.isEmpty() )
          return fromMultiPointXY( line );
      }
      return QgsGeometry();
    }

    case QgsWkbTypes::PolygonGeometry:
    {
      // can only transform if destination is multipoint
      if ( !destMultipart )
        return QgsGeometry();

      // input geometry is multipart: make a multipoint from multipolygon
      if ( isMultipart() )
      {
        const QgsMultiPolygonXY multiPolygon = asMultiPolygon();
        QgsMultiPointXY multiPoint;
        for ( const QgsPolygonXY &poly : multiPolygon )
          for ( const QgsPolylineXY &line : poly )
            for ( const QgsPointXY &pt : line )
              multiPoint << pt;
        return fromMultiPointXY( multiPoint );
      }
      // input geometry is not multipart: make a multipoint from polygon
      else
      {
        const QgsPolygonXY polygon = asPolygon();
        QgsMultiPointXY multiPoint;
        for ( const QgsPolylineXY &line : polygon )
          for ( const QgsPointXY &pt : line )
            multiPoint << pt;
        return fromMultiPointXY( multiPoint );
      }
    }

    default:
      return QgsGeometry();
  }
}

QgsGeometry QgsGeometry::convertToLine( bool destMultipart ) const
{
  switch ( type() )
  {
    case QgsWkbTypes::PointGeometry:
    {
      if ( !isMultipart() )
        return QgsGeometry();

      QgsMultiPointXY multiPoint = asMultiPoint();
      if ( multiPoint.count() < 2 )
        return QgsGeometry();

      if ( destMultipart )
        return fromMultiPolylineXY( QgsMultiPolylineXY() << multiPoint );
      else
        return fromPolylineXY( multiPoint );
    }

    case QgsWkbTypes::LineGeometry:
    {
      bool srcIsMultipart = isMultipart();

      if ( ( destMultipart && srcIsMultipart ) ||
           ( !destMultipart && ! srcIsMultipart ) )
      {
        // return a copy of the same geom
        return QgsGeometry( *this );
      }
      if ( destMultipart )
      {
        // destination is multipart => makes a multipoint with a single line
        QgsPolylineXY line = asPolyline();
        if ( !line.isEmpty() )
          return fromMultiPolylineXY( QgsMultiPolylineXY() << line );
      }
      else
      {
        // destination is singlepart => make a single part if possible
        QgsMultiPolylineXY multiLine = asMultiPolyline();
        if ( multiLine.count() == 1 )
          return fromPolylineXY( multiLine[0] );
      }
      return QgsGeometry();
    }

    case QgsWkbTypes::PolygonGeometry:
    {
      // input geometry is multipolygon
      if ( isMultipart() )
      {
        const QgsMultiPolygonXY multiPolygon = asMultiPolygon();
        QgsMultiPolylineXY multiLine;
        for ( const QgsPolygonXY &poly : multiPolygon )
          for ( const QgsPolylineXY &line : poly )
            multiLine << line;

        if ( destMultipart )
        {
          // destination is multipart
          return fromMultiPolylineXY( multiLine );
        }
        else if ( multiLine.count() == 1 )
        {
          // destination is singlepart => make a single part if possible
          return fromPolylineXY( multiLine[0] );
        }
      }
      // input geometry is single polygon
      else
      {
        QgsPolygonXY polygon = asPolygon();
        // if polygon has rings
        if ( polygon.count() > 1 )
        {
          // cannot fit a polygon with rings in a single line layer
          // TODO: would it be better to remove rings?
          if ( destMultipart )
          {
            const QgsPolygonXY polygon = asPolygon();
            QgsMultiPolylineXY multiLine;
            multiLine.reserve( polygon.count() );
            for ( const QgsPolylineXY &line : polygon )
              multiLine << line;
            return fromMultiPolylineXY( multiLine );
          }
        }
        // no rings
        else if ( polygon.count() == 1 )
        {
          if ( destMultipart )
          {
            return fromMultiPolylineXY( polygon );
          }
          else
          {
            return fromPolylineXY( polygon[0] );
          }
        }
      }
      return QgsGeometry();
    }

    default:
      return QgsGeometry();
  }
}

QgsGeometry QgsGeometry::convertToPolygon( bool destMultipart ) const
{
  switch ( type() )
  {
    case QgsWkbTypes::PointGeometry:
    {
      if ( !isMultipart() )
        return QgsGeometry();

      QgsMultiPointXY multiPoint = asMultiPoint();
      if ( multiPoint.count() < 3 )
        return QgsGeometry();

      if ( multiPoint.last() != multiPoint.first() )
        multiPoint << multiPoint.first();

      QgsPolygonXY polygon = QgsPolygonXY() << multiPoint;
      if ( destMultipart )
        return fromMultiPolygonXY( QgsMultiPolygonXY() << polygon );
      else
        return fromPolygonXY( polygon );
    }

    case QgsWkbTypes::LineGeometry:
    {
      // input geometry is multiline
      if ( isMultipart() )
      {
        QgsMultiPolylineXY multiLine = asMultiPolyline();
        QgsMultiPolygonXY multiPolygon;
        for ( QgsMultiPolylineXY::iterator multiLineIt = multiLine.begin(); multiLineIt != multiLine.end(); ++multiLineIt )
        {
          // do not create polygon for a 1 segment line
          if ( ( *multiLineIt ).count() < 3 )
            return QgsGeometry();
          if ( ( *multiLineIt ).count() == 3 && ( *multiLineIt ).first() == ( *multiLineIt ).last() )
            return QgsGeometry();

          // add closing node
          if ( ( *multiLineIt ).first() != ( *multiLineIt ).last() )
            *multiLineIt << ( *multiLineIt ).first();
          multiPolygon << ( QgsPolygonXY() << *multiLineIt );
        }
        // check that polygons were inserted
        if ( !multiPolygon.isEmpty() )
        {
          if ( destMultipart )
          {
            return fromMultiPolygonXY( multiPolygon );
          }
          else if ( multiPolygon.count() == 1 )
          {
            // destination is singlepart => make a single part if possible
            return fromPolygonXY( multiPolygon[0] );
          }
        }
      }
      // input geometry is single line
      else
      {
        QgsPolylineXY line = asPolyline();

        // do not create polygon for a 1 segment line
        if ( line.count() < 3 )
          return QgsGeometry();
        if ( line.count() == 3 && line.first() == line.last() )
          return QgsGeometry();

        // add closing node
        if ( line.first() != line.last() )
          line << line.first();

        // destination is multipart
        if ( destMultipart )
        {
          return fromMultiPolygonXY( QgsMultiPolygonXY() << ( QgsPolygonXY() << line ) );
        }
        else
        {
          return fromPolygonXY( QgsPolygonXY() << line );
        }
      }
      return QgsGeometry();
    }

    case QgsWkbTypes::PolygonGeometry:
    {
      bool srcIsMultipart = isMultipart();

      if ( ( destMultipart && srcIsMultipart ) ||
           ( !destMultipart && ! srcIsMultipart ) )
      {
        // return a copy of the same geom
        return QgsGeometry( *this );
      }
      if ( destMultipart )
      {
        // destination is multipart => makes a multipoint with a single polygon
        QgsPolygonXY polygon = asPolygon();
        if ( !polygon.isEmpty() )
          return fromMultiPolygonXY( QgsMultiPolygonXY() << polygon );
      }
      else
      {
        QgsMultiPolygonXY multiPolygon = asMultiPolygon();
        if ( multiPolygon.count() == 1 )
        {
          // destination is singlepart => make a single part if possible
          return fromPolygonXY( multiPolygon[0] );
        }
      }
      return QgsGeometry();
    }

    default:
      return QgsGeometry();
  }
}

QgsGeometryEngine *QgsGeometry::createGeometryEngine( const QgsAbstractGeometry *geometry )
{
  return new QgsGeos( geometry );
}

QDataStream &operator<<( QDataStream &out, const QgsGeometry &geometry )
{
  out << geometry.asWkb();
  return out;
}

QDataStream &operator>>( QDataStream &in, QgsGeometry &geometry )
{
  QByteArray byteArray;
  in >> byteArray;
  if ( byteArray.isEmpty() )
  {
    geometry.set( nullptr );
    return in;
  }

  geometry.fromWkb( byteArray );
  return in;
}


QString QgsGeometry::Error::what() const
{
  return mMessage;
}

QgsPointXY QgsGeometry::Error::where() const
{
  return mLocation;
}

bool QgsGeometry::Error::hasWhere() const
{
  return mHasLocation;
}
