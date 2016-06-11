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

#include "qgis.h"
#include "qgsgeometry.h"
#include "qgsgeometryeditutils.h"
#include "qgsgeometryfactory.h"
#include "qgsgeometryutils.h"
#include "qgsinternalgeometryengine.h"
#include "qgsgeos.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgspoint.h"
#include "qgsrectangle.h"

#include "qgsmaplayerregistry.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsgeometryvalidator.h"

#include "qgsmulticurvev2.h"
#include "qgsmultilinestringv2.h"
#include "qgsmultipointv2.h"
#include "qgsmultipolygonv2.h"
#include "qgsmultisurfacev2.h"
#include "qgspointv2.h"
#include "qgspolygonv2.h"
#include "qgslinestringv2.h"

#ifndef Q_WS_WIN
#include <netinet/in.h>
#else
#include <winsock.h>
#endif

struct QgsGeometryPrivate
{
  QgsGeometryPrivate(): ref( 1 ), geometry( nullptr ), mWkb( nullptr ), mWkbSize( 0 ), mGeos( nullptr ) {}
  ~QgsGeometryPrivate() { delete geometry; delete[] mWkb; GEOSGeom_destroy_r( QgsGeos::getGEOSHandler(), mGeos ); }
  QAtomicInt ref;
  QgsAbstractGeometryV2* geometry;
  mutable const unsigned char* mWkb; //store wkb pointer for backward compatibility
  mutable int mWkbSize;
  mutable GEOSGeometry* mGeos;
};

QgsGeometry::QgsGeometry(): d( new QgsGeometryPrivate() )
{
}

QgsGeometry::~QgsGeometry()
{
  if ( !d->ref.deref() )
    delete d;
}

QgsGeometry::QgsGeometry( QgsAbstractGeometryV2* geom ): d( new QgsGeometryPrivate() )
{
  d->geometry = geom;
  d->ref = QAtomicInt( 1 );
}

QgsGeometry::QgsGeometry( const QgsGeometry& other )
{
  d = other.d;
  d->ref.ref();
}

QgsGeometry& QgsGeometry::operator=( QgsGeometry const & other )
{
  if ( !d->ref.deref() )
  {
    delete d;
  }

  d = other.d;
  d->ref.ref();
  return *this;
}

void QgsGeometry::detach( bool cloneGeom )
{
  if ( d->ref > 1 )
  {
    ( void )d->ref.deref();
    QgsAbstractGeometryV2* cGeom = nullptr;

    if ( d->geometry && cloneGeom )
    {
      cGeom = d->geometry->clone();
    }

    d = new QgsGeometryPrivate();
    d->geometry = cGeom;
  }
}

void QgsGeometry::removeWkbGeos()
{
  delete[] d->mWkb;
  d->mWkb = nullptr;
  d->mWkbSize = 0;
  if ( d->mGeos )
  {
    GEOSGeom_destroy_r( QgsGeos::getGEOSHandler(), d->mGeos );
    d->mGeos = nullptr;
  }
}

QgsAbstractGeometryV2* QgsGeometry::geometry() const
{
  return d->geometry;
}

void QgsGeometry::setGeometry( QgsAbstractGeometryV2* geometry )
{
  if ( d->geometry == geometry )
  {
    return;
  }

  detach( false );
  if ( d->geometry )
  {
    delete d->geometry;
    d->geometry = nullptr;
  }
  removeWkbGeos();

  d->geometry = geometry;
}

bool QgsGeometry::isEmpty() const
{
  return !d->geometry;
}

QgsGeometry* QgsGeometry::fromWkt( const QString& wkt )
{
  QgsAbstractGeometryV2* geom = QgsGeometryFactory::geomFromWkt( wkt );
  if ( !geom )
  {
    return nullptr;
  }
  return new QgsGeometry( geom );
}

QgsGeometry* QgsGeometry::fromPoint( const QgsPoint& point )
{
  QgsAbstractGeometryV2* geom = QgsGeometryFactory::fromPoint( point );
  if ( geom )
  {
    return new QgsGeometry( geom );
  }
  return nullptr;
}

QgsGeometry* QgsGeometry::fromPolyline( const QgsPolyline& polyline )
{
  QgsAbstractGeometryV2* geom = QgsGeometryFactory::fromPolyline( polyline );
  if ( geom )
  {
    return new QgsGeometry( geom );
  }
  return nullptr;
}

QgsGeometry* QgsGeometry::fromPolygon( const QgsPolygon& polygon )
{
  QgsAbstractGeometryV2* geom = QgsGeometryFactory::fromPolygon( polygon );
  if ( geom )
  {
    return new QgsGeometry( geom );
  }
  return nullptr;
}

QgsGeometry* QgsGeometry::fromMultiPoint( const QgsMultiPoint& multipoint )
{
  QgsAbstractGeometryV2* geom = QgsGeometryFactory::fromMultiPoint( multipoint );
  if ( geom )
  {
    return new QgsGeometry( geom );
  }
  return nullptr;
}

QgsGeometry* QgsGeometry::fromMultiPolyline( const QgsMultiPolyline& multiline )
{
  QgsAbstractGeometryV2* geom = QgsGeometryFactory::fromMultiPolyline( multiline );
  if ( geom )
  {
    return new QgsGeometry( geom );
  }
  return nullptr;
}

QgsGeometry* QgsGeometry::fromMultiPolygon( const QgsMultiPolygon& multipoly )
{
  QgsAbstractGeometryV2* geom = QgsGeometryFactory::fromMultiPolygon( multipoly );
  if ( geom )
  {
    return new QgsGeometry( geom );
  }
  return nullptr;
}

QgsGeometry* QgsGeometry::fromRect( const QgsRectangle& rect )
{
  QgsPolyline ring;
  ring.append( QgsPoint( rect.xMinimum(), rect.yMinimum() ) );
  ring.append( QgsPoint( rect.xMaximum(), rect.yMinimum() ) );
  ring.append( QgsPoint( rect.xMaximum(), rect.yMaximum() ) );
  ring.append( QgsPoint( rect.xMinimum(), rect.yMaximum() ) );
  ring.append( QgsPoint( rect.xMinimum(), rect.yMinimum() ) );

  QgsPolygon polygon;
  polygon.append( ring );

  return fromPolygon( polygon );
}

void QgsGeometry::fromWkb( unsigned char *wkb, int length )
{
  detach( false );

  if ( d->geometry )
  {
    delete d->geometry;
    removeWkbGeos();
  }
  d->geometry = QgsGeometryFactory::geomFromWkb( QgsConstWkbPtr( wkb, length ) );
  if ( d->geometry )
  {
    d->mWkb = wkb;
    d->mWkbSize = length;
  }
  else
  {
    delete [] wkb;
    d->mWkb = nullptr;
    d->mWkbSize = 0;
  }
}

const unsigned char *QgsGeometry::asWkb() const
{
  if ( !d->geometry )
  {
    return nullptr;
  }

  if ( !d->mWkb )
  {
    d->mWkb = d->geometry->asWkb( d->mWkbSize );
  }
  return d->mWkb;
}

int QgsGeometry::wkbSize() const
{
  if ( !d->geometry )
  {
    return 0;
  }

  if ( !d->mWkb )
  {
    d->mWkb = d->geometry->asWkb( d->mWkbSize );
  }
  return d->mWkbSize;
}

const GEOSGeometry* QgsGeometry::asGeos( double precision ) const
{
  if ( !d->geometry )
  {
    return nullptr;
  }

  if ( !d->mGeos )
  {
    d->mGeos = QgsGeos::asGeos( d->geometry, precision );
  }
  return d->mGeos;
}


QGis::WkbType QgsGeometry::wkbType() const
{
  if ( !d->geometry )
  {
    return QGis::WKBUnknown;
  }
  else
  {
    return QGis::fromNewWkbType( d->geometry->wkbType() );
  }
}


QGis::GeometryType QgsGeometry::type() const
{
  if ( !d->geometry )
  {
    return QGis::UnknownGeometry;
  }
  return static_cast< QGis::GeometryType >( QgsWKBTypes::geometryType( d->geometry->wkbType() ) );
}

bool QgsGeometry::isMultipart() const
{
  if ( !d->geometry )
  {
    return false;
  }
  return QgsWKBTypes::isMultiType( d->geometry->wkbType() );
}

void QgsGeometry::fromGeos( GEOSGeometry *geos )
{
  detach( false );
  delete d->geometry;
  d->geometry = QgsGeos::fromGeos( geos );
  d->mGeos = geos;
}

QgsPoint QgsGeometry::closestVertex( const QgsPoint& point, int& atVertex, int& beforeVertex, int& afterVertex, double& sqrDist ) const
{
  if ( !d->geometry )
  {
    return QgsPoint( 0, 0 );
  }

  QgsPointV2 pt( point.x(), point.y() );
  QgsVertexId id;

  QgsPointV2 vp = QgsGeometryUtils::closestVertex( *( d->geometry ), pt, id );
  if ( !id.isValid() )
  {
    sqrDist = -1;
    return QgsPoint( 0, 0 );
  }
  sqrDist = QgsGeometryUtils::sqrDistance2D( pt, vp );

  atVertex = vertexNrFromVertexId( id );
  adjacentVertices( atVertex, beforeVertex, afterVertex );
  return QgsPoint( vp.x(), vp.y() );
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

void QgsGeometry::adjacentVertices( int atVertex, int& beforeVertex, int& afterVertex ) const
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
  QgsGeometryUtils::adjacentVertices( *( d->geometry ), id, beforeVertexId, afterVertexId );
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

  detach( true );

  removeWkbGeos();
  return d->geometry->moveVertex( id, QgsPointV2( x, y ) );
}

bool QgsGeometry::moveVertex( const QgsPointV2& p, int atVertex )
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

  detach( true );

  removeWkbGeos();
  return d->geometry->moveVertex( id, p );
}

bool QgsGeometry::deleteVertex( int atVertex )
{
  if ( !d->geometry )
  {
    return false;
  }

  //maintain compatibility with < 2.10 API
  if ( QgsWKBTypes::flatType( d->geometry->wkbType() ) == QgsWKBTypes::MultiPoint )
  {
    detach( true );
    removeWkbGeos();
    //delete geometry instead of point
    return static_cast< QgsGeometryCollectionV2* >( d->geometry )->removeGeometry( atVertex );
  }

  //if it is a point, set the geometry to nullptr
  if ( QgsWKBTypes::flatType( d->geometry->wkbType() ) == QgsWKBTypes::Point )
  {
    detach( false );
    delete d->geometry;
    removeWkbGeos();
    d->geometry = nullptr;
    return true;
  }

  QgsVertexId id;
  if ( !vertexIdFromVertexNr( atVertex, id ) )
  {
    return false;
  }

  detach( true );

  removeWkbGeos();
  return d->geometry->deleteVertex( id );
}

bool QgsGeometry::insertVertex( double x, double y, int beforeVertex )
{
  if ( !d->geometry )
  {
    return false;
  }

  //maintain compatibility with < 2.10 API
  if ( QgsWKBTypes::flatType( d->geometry->wkbType() ) == QgsWKBTypes::MultiPoint )
  {
    detach( true );
    removeWkbGeos();
    //insert geometry instead of point
    return static_cast< QgsGeometryCollectionV2* >( d->geometry )->insertGeometry( new QgsPointV2( x, y ), beforeVertex );
  }

  QgsVertexId id;
  if ( !vertexIdFromVertexNr( beforeVertex, id ) )
  {
    return false;
  }

  detach( true );

  removeWkbGeos();

  return d->geometry->insertVertex( id, QgsPointV2( x, y ) );
}

QgsPoint QgsGeometry::vertexAt( int atVertex ) const
{
  if ( !d->geometry )
  {
    return QgsPoint( 0, 0 );
  }

  QgsVertexId vId;
  ( void )vertexIdFromVertexNr( atVertex, vId );
  if ( vId.vertex < 0 )
  {
    return QgsPoint( 0, 0 );
  }
  QgsPointV2 pt = d->geometry->vertexAt( vId );
  return QgsPoint( pt.x(), pt.y() );
}

double QgsGeometry::sqrDistToVertexAt( QgsPoint& point, int atVertex ) const
{
  QgsPoint vertexPoint = vertexAt( atVertex );
  return QgsGeometryUtils::sqrDistance2D( QgsPointV2( vertexPoint.x(), vertexPoint.y() ), QgsPointV2( point.x(), point.y() ) );
}

QgsGeometry QgsGeometry::nearestPoint( const QgsGeometry& other ) const
{
  QgsGeos geos( d->geometry );
  return geos.closestPoint( other );
}

QgsGeometry QgsGeometry::shortestLine( const QgsGeometry& other ) const
{
  QgsGeos geos( d->geometry );
  return geos.shortestLine( other );
}

double QgsGeometry::closestVertexWithContext( const QgsPoint& point, int& atVertex ) const
{
  if ( !d->geometry )
  {
    return 0.0;
  }

  QgsVertexId vId;
  QgsPointV2 pt( point.x(), point.y() );
  QgsPointV2 closestPoint = QgsGeometryUtils::closestVertex( *( d->geometry ), pt, vId );
  atVertex = vertexNrFromVertexId( vId );
  return QgsGeometryUtils::sqrDistance2D( closestPoint, pt );
}

double QgsGeometry::closestSegmentWithContext(
  const QgsPoint& point,
  QgsPoint& minDistPoint,
  int& afterVertex,
  double *leftOf,
  double epsilon ) const
{
  if ( !d->geometry )
  {
    return 0;
  }

  QgsPointV2 segmentPt;
  QgsVertexId vertexAfter;
  bool leftOfBool;

  double sqrDist = d->geometry->closestSegment( QgsPointV2( point.x(), point.y() ), segmentPt,  vertexAfter, &leftOfBool, epsilon );

  minDistPoint.setX( segmentPt.x() );
  minDistPoint.setY( segmentPt.y() );
  afterVertex = vertexNrFromVertexId( vertexAfter );
  if ( leftOf )
  {
    *leftOf = leftOfBool ? 1.0 : -1.0;
  }
  return sqrDist;
}

int QgsGeometry::addRing( const QList<QgsPoint> &ring )
{
  detach( true );

  removeWkbGeos();
  QgsLineStringV2* ringLine = new QgsLineStringV2();
  QgsPointSequenceV2 ringPoints;
  convertPointList( ring, ringPoints );
  ringLine->setPoints( ringPoints );
  return addRing( ringLine );
}

int QgsGeometry::addRing( QgsCurveV2* ring )
{
  if ( !d->geometry )
  {
    delete ring;
    return 1;
  }

  detach( true );

  removeWkbGeos();
  return QgsGeometryEditUtils::addRing( d->geometry, ring );
}

int QgsGeometry::addPart( const QList<QgsPoint> &points, QGis::GeometryType geomType )
{
  QgsPointSequenceV2 l;
  convertPointList( points, l );
  return addPart( l, geomType );
}

int QgsGeometry::addPart( const QgsPointSequenceV2 &points, QGis::GeometryType geomType )
{
  QgsAbstractGeometryV2* partGeom = nullptr;
  if ( points.size() == 1 )
  {
    partGeom = new QgsPointV2( points[0] );
  }
  else if ( points.size() > 1 )
  {
    QgsLineStringV2* ringLine = new QgsLineStringV2();
    ringLine->setPoints( points );
    partGeom = ringLine;
  }
  return addPart( partGeom, geomType );
}

int QgsGeometry::addPart( QgsAbstractGeometryV2* part, QGis::GeometryType geomType )
{
  if ( !d->geometry )
  {
    detach( false );
    switch ( geomType )
    {
      case QGis::Point:
        d->geometry = new QgsMultiPointV2();
        break;
      case QGis::Line:
        d->geometry = new QgsMultiLineStringV2();
        break;
      case QGis::Polygon:
        d->geometry = new QgsMultiPolygonV2();
        break;
      default:
        return 1;
    }
  }
  else
  {
    detach( true );
    removeWkbGeos();
  }

  convertToMultiType();
  return QgsGeometryEditUtils::addPart( d->geometry, part );
}

int QgsGeometry::addPart( const QgsGeometry *newPart )
{
  if ( !d->geometry || !newPart || !newPart->d || !newPart->d->geometry )
  {
    return 1;
  }

  return addPart( newPart->d->geometry->clone() );
}

int QgsGeometry::addPart( GEOSGeometry *newPart )
{
  if ( !d->geometry || !newPart )
  {
    return 1;
  }

  detach( true );

  QgsAbstractGeometryV2* geom = QgsGeos::fromGeos( newPart );
  removeWkbGeos();
  return QgsGeometryEditUtils::addPart( d->geometry, geom );
}

int QgsGeometry::translate( double dx, double dy )
{
  if ( !d->geometry )
  {
    return 1;
  }

  detach( true );

  d->geometry->transform( QTransform::fromTranslate( dx, dy ) );
  removeWkbGeos();
  return 0;
}

int QgsGeometry::rotate( double rotation, const QgsPoint& center )
{
  if ( !d->geometry )
  {
    return 1;
  }

  detach( true );

  QTransform t = QTransform::fromTranslate( center.x(), center.y() );
  t.rotate( -rotation );
  t.translate( -center.x(), -center.y() );
  d->geometry->transform( t );
  removeWkbGeos();
  return 0;
}

int QgsGeometry::splitGeometry( const QList<QgsPoint>& splitLine, QList<QgsGeometry*>& newGeometries, bool topological, QList<QgsPoint> &topologyTestPoints )
{
  if ( !d->geometry )
  {
    return 0;
  }

  QList<QgsAbstractGeometryV2*> newGeoms;
  QgsLineStringV2 splitLineString;
  QgsPointSequenceV2 splitLinePointsV2;
  convertPointList( splitLine, splitLinePointsV2 );
  splitLineString.setPoints( splitLinePointsV2 );
  QgsPointSequenceV2 tp;

  QgsGeos geos( d->geometry );
  int result = geos.splitGeometry( splitLineString, newGeoms, topological, tp );

  if ( result == 0 )
  {
    detach( false );
    d->geometry = newGeoms.at( 0 );

    newGeometries.clear();
    for ( int i = 1; i < newGeoms.size(); ++i )
    {
      newGeometries.push_back( new QgsGeometry( newGeoms.at( i ) ) );
    }
  }

  convertPointList( tp, topologyTestPoints );
  removeWkbGeos();
  return result;
}

/** Replaces a part of this geometry with another line*/
int QgsGeometry::reshapeGeometry( const QList<QgsPoint>& reshapeWithLine )
{
  if ( !d->geometry )
  {
    return 0;
  }

  QgsPointSequenceV2 reshapeLine;
  convertPointList( reshapeWithLine, reshapeLine );
  QgsLineStringV2 reshapeLineString;
  reshapeLineString.setPoints( reshapeLine );

  QgsGeos geos( d->geometry );
  int errorCode = 0;
  QgsAbstractGeometryV2* geom = geos.reshapeGeometry( reshapeLineString, &errorCode );
  if ( errorCode == 0 && geom )
  {
    detach( false );
    delete d->geometry;
    d->geometry = geom;
    removeWkbGeos();
    return 0;
  }
  return errorCode;
}

int QgsGeometry::makeDifference( const QgsGeometry* other )
{
  if ( !d->geometry || !other->d->geometry )
  {
    return 0;
  }

  QgsGeos geos( d->geometry );

  QgsAbstractGeometryV2* diffGeom = geos.intersection( *( other->geometry() ) );
  if ( !diffGeom )
  {
    return 1;
  }

  detach( false );

  delete d->geometry;
  d->geometry = diffGeom;
  removeWkbGeos();
  return 0;
}

QgsRectangle QgsGeometry::boundingBox() const
{
  if ( d->geometry )
  {
    return d->geometry->boundingBox();
  }
  return QgsRectangle();
}

bool QgsGeometry::intersects( const QgsRectangle& r ) const
{
  QgsGeometry* g = fromRect( r );
  bool res = intersects( g );
  delete g;
  return res;
}

bool QgsGeometry::intersects( const QgsGeometry* geometry ) const
{
  if ( !d->geometry || !geometry || !geometry->d->geometry )
  {
    return false;
  }

  QgsGeos geos( d->geometry );
  return geos.intersects( *( geometry->d->geometry ) );
}

bool QgsGeometry::contains( const QgsPoint* p ) const
{
  if ( !d->geometry || !p )
  {
    return false;
  }

  QgsPointV2 pt( p->x(), p->y() );
  QgsGeos geos( d->geometry );
  return geos.contains( pt );
}

bool QgsGeometry::contains( const QgsGeometry* geometry ) const
{
  if ( !d->geometry || !geometry || !geometry->d->geometry )
  {
    return false;
  }

  QgsGeos geos( d->geometry );
  return geos.contains( *( geometry->d->geometry ) );
}

bool QgsGeometry::disjoint( const QgsGeometry* geometry ) const
{
  if ( !d->geometry || !geometry || !geometry->d->geometry )
  {
    return false;
  }

  QgsGeos geos( d->geometry );
  return geos.disjoint( *( geometry->d->geometry ) );
}

bool QgsGeometry::equals( const QgsGeometry* geometry ) const
{
  if ( !d->geometry || !geometry || !geometry->d->geometry )
  {
    return false;
  }

  QgsGeos geos( d->geometry );
  return geos.isEqual( *( geometry->d->geometry ) );
}

bool QgsGeometry::touches( const QgsGeometry* geometry ) const
{
  if ( !d->geometry || !geometry || !geometry->d->geometry )
  {
    return false;
  }

  QgsGeos geos( d->geometry );
  return geos.touches( *( geometry->d->geometry ) );
}

bool QgsGeometry::overlaps( const QgsGeometry* geometry ) const
{
  if ( !d->geometry || !geometry || !geometry->d->geometry )
  {
    return false;
  }

  QgsGeos geos( d->geometry );
  return geos.overlaps( *( geometry->d->geometry ) );
}

bool QgsGeometry::within( const QgsGeometry* geometry ) const
{
  if ( !d->geometry || !geometry || !geometry->d->geometry )
  {
    return false;
  }

  QgsGeos geos( d->geometry );
  return geos.within( *( geometry->d->geometry ) );
}

bool QgsGeometry::crosses( const QgsGeometry* geometry ) const
{
  if ( !d->geometry || !geometry || !geometry->d->geometry )
  {
    return false;
  }

  QgsGeos geos( d->geometry );
  return geos.crosses( *( geometry->d->geometry ) );
}

QString QgsGeometry::exportToWkt( int precision ) const
{
  if ( !d->geometry )
  {
    return QString();
  }
  return d->geometry->asWkt( precision );
}

QString QgsGeometry::exportToGeoJSON( int precision ) const
{
  if ( !d->geometry )
  {
    return QString( "null" );
  }
  return d->geometry->asJSON( precision );
}

QgsGeometry* QgsGeometry::convertToType( QGis::GeometryType destType, bool destMultipart ) const
{
  switch ( destType )
  {
    case QGis::Point:
      return convertToPoint( destMultipart );

    case QGis::Line:
      return convertToLine( destMultipart );

    case QGis::Polygon:
      return convertToPolygon( destMultipart );

    default:
      return nullptr;
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

  QgsGeometryCollectionV2* multiGeom = dynamic_cast<QgsGeometryCollectionV2*>
                                       ( QgsGeometryFactory::geomFromWkbType( QgsWKBTypes::multiType( d->geometry->wkbType() ) ) );
  if ( !multiGeom )
  {
    return false;
  }

  detach( true );
  multiGeom->addGeometry( d->geometry );
  d->geometry = multiGeom;
  removeWkbGeos();
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

  QgsGeometryCollectionV2* multiGeom = dynamic_cast<QgsGeometryCollectionV2*>( d->geometry );
  if ( !multiGeom || multiGeom->partCount() < 1 )
    return false;

  QgsAbstractGeometryV2* firstPart = multiGeom->geometryN( 0 )->clone();
  detach( false );

  d->geometry = firstPart;
  removeWkbGeos();
  return true;
}

QgsPoint QgsGeometry::asPoint() const
{
  if ( !d->geometry || QgsWKBTypes::flatType( d->geometry->wkbType() ) != QgsWKBTypes::Point )
  {
    return QgsPoint();
  }
  QgsPointV2* pt = dynamic_cast<QgsPointV2*>( d->geometry );
  if ( !pt )
  {
    return QgsPoint();
  }

  return QgsPoint( pt->x(), pt->y() );
}

QgsPolyline QgsGeometry::asPolyline() const
{
  QgsPolyline polyLine;
  if ( !d->geometry )
  {
    return polyLine;
  }

  bool doSegmentation = ( QgsWKBTypes::flatType( d->geometry->wkbType() ) == QgsWKBTypes::CompoundCurve
                          || QgsWKBTypes::flatType( d->geometry->wkbType() ) == QgsWKBTypes::CircularString );
  QgsLineStringV2* line = nullptr;
  if ( doSegmentation )
  {
    QgsCurveV2* curve = dynamic_cast<QgsCurveV2*>( d->geometry );
    if ( !curve )
    {
      return polyLine;
    }
    line = curve->curveToLine();
  }
  else
  {
    line = dynamic_cast<QgsLineStringV2*>( d->geometry );
    if ( !line )
    {
      return polyLine;
    }
  }

  int nVertices = line->numPoints();
  polyLine.resize( nVertices );
  for ( int i = 0; i < nVertices; ++i )
  {
    QgsPointV2 pt = line->pointN( i );
    polyLine[i].setX( pt.x() );
    polyLine[i].setY( pt.y() );
  }

  if ( doSegmentation )
  {
    delete line;
  }

  return polyLine;
}

QgsPolygon QgsGeometry::asPolygon() const
{
  if ( !d->geometry )
    return QgsPolygon();

  bool doSegmentation = ( QgsWKBTypes::flatType( d->geometry->wkbType() ) == QgsWKBTypes::CurvePolygon );

  QgsPolygonV2* p = nullptr;
  if ( doSegmentation )
  {
    QgsCurvePolygonV2* curvePoly = dynamic_cast<QgsCurvePolygonV2*>( d->geometry );
    if ( !curvePoly )
    {
      return QgsPolygon();
    }
    p = curvePoly->toPolygon();
  }
  else
  {
    p = dynamic_cast<QgsPolygonV2*>( d->geometry );
  }

  if ( !p )
  {
    return QgsPolygon();
  }

  QgsPolygon polygon;
  convertPolygon( *p, polygon );

  if ( doSegmentation )
  {
    delete p;
  }
  return polygon;
}

QgsMultiPoint QgsGeometry::asMultiPoint() const
{
  if ( !d->geometry || QgsWKBTypes::flatType( d->geometry->wkbType() ) != QgsWKBTypes::MultiPoint )
  {
    return QgsMultiPoint();
  }

  const QgsMultiPointV2* mp = dynamic_cast<QgsMultiPointV2*>( d->geometry );
  if ( !mp )
  {
    return QgsMultiPoint();
  }

  int nPoints = mp->numGeometries();
  QgsMultiPoint multiPoint( nPoints );
  for ( int i = 0; i < nPoints; ++i )
  {
    const QgsPointV2* pt = static_cast<const QgsPointV2*>( mp->geometryN( i ) );
    multiPoint[i].setX( pt->x() );
    multiPoint[i].setY( pt->y() );
  }
  return multiPoint;
}

QgsMultiPolyline QgsGeometry::asMultiPolyline() const
{
  if ( !d->geometry )
  {
    return QgsMultiPolyline();
  }

  QgsGeometryCollectionV2* geomCollection = dynamic_cast<QgsGeometryCollectionV2*>( d->geometry );
  if ( !geomCollection )
  {
    return QgsMultiPolyline();
  }

  int nLines = geomCollection->numGeometries();
  if ( nLines < 1 )
  {
    return QgsMultiPolyline();
  }

  QgsMultiPolyline mpl;
  for ( int i = 0; i < nLines; ++i )
  {
    bool deleteLine = false;
    const QgsLineStringV2* line = dynamic_cast<const QgsLineStringV2*>( geomCollection->geometryN( i ) );
    if ( !line )
    {
      const QgsCurveV2* curve = dynamic_cast<const QgsCurveV2*>( geomCollection->geometryN( i ) );
      if ( !curve )
      {
        continue;
      }
      deleteLine = true;
      line = curve->curveToLine();
    }

    QgsPointSequenceV2 lineCoords;
    line->points( lineCoords );
    QgsPolyline polyLine;
    convertToPolyline( lineCoords, polyLine );
    mpl.append( polyLine );

    if ( deleteLine )
    {
      delete line;
    }
  }
  return mpl;
}

QgsMultiPolygon QgsGeometry::asMultiPolygon() const
{
  if ( !d->geometry )
  {
    return QgsMultiPolygon();
  }

  QgsGeometryCollectionV2* geomCollection = dynamic_cast<QgsGeometryCollectionV2*>( d->geometry );
  if ( !geomCollection )
  {
    return QgsMultiPolygon();
  }

  int nPolygons = geomCollection->numGeometries();
  if ( nPolygons < 1 )
  {
    return QgsMultiPolygon();
  }

  QgsMultiPolygon mp;
  for ( int i = 0; i < nPolygons; ++i )
  {
    const QgsPolygonV2* polygon = dynamic_cast<const QgsPolygonV2*>( geomCollection->geometryN( i ) );
    if ( !polygon )
    {
      const QgsCurvePolygonV2* cPolygon = dynamic_cast<const QgsCurvePolygonV2*>( geomCollection->geometryN( i ) );
      if ( cPolygon )
      {
        polygon = cPolygon->toPolygon();
      }
      else
      {
        continue;
      }
    }

    QgsPolygon poly;
    convertPolygon( *polygon, poly );
    mp.append( poly );
  }
  return mp;
}

double QgsGeometry::area() const
{
  if ( !d->geometry )
  {
    return -1.0;
  }
  QgsGeos g( d->geometry );

#if 0
  //debug: compare geos area with calculation in QGIS
  double geosArea = g.area();
  double qgisArea = 0;
  QgsSurfaceV2* surface = dynamic_cast<QgsSurfaceV2*>( d->geometry );
  if ( surface )
  {
    qgisArea = surface->area();
  }
#endif

  return g.area();
}

double QgsGeometry::length() const
{
  if ( !d->geometry )
  {
    return -1.0;
  }
  QgsGeos g( d->geometry );
  return g.length();
}

double QgsGeometry::distance( const QgsGeometry& geom ) const
{
  if ( !d->geometry || !geom.d->geometry )
  {
    return -1.0;
  }

  QgsGeos g( d->geometry );
  return g.distance( *( geom.d->geometry ) );
}

QgsGeometry* QgsGeometry::buffer( double distance, int segments ) const
{
  if ( !d->geometry )
  {
    return nullptr;
  }

  QgsGeos g( d->geometry );
  QgsAbstractGeometryV2* geom = g.buffer( distance, segments );
  if ( !geom )
  {
    return nullptr;
  }
  return new QgsGeometry( geom );
}

QgsGeometry* QgsGeometry::buffer( double distance, int segments, int endCapStyle, int joinStyle, double mitreLimit ) const
{
  if ( !d->geometry )
  {
    return nullptr;
  }

  QgsGeos g( d->geometry );
  QgsAbstractGeometryV2* geom = g.buffer( distance, segments, endCapStyle, joinStyle, mitreLimit );
  if ( !geom )
  {
    return nullptr;
  }
  return new QgsGeometry( geom );
}

QgsGeometry* QgsGeometry::offsetCurve( double distance, int segments, int joinStyle, double mitreLimit ) const
{
  if ( !d->geometry )
  {
    return nullptr;
  }

  QgsGeos geos( d->geometry );
  QgsAbstractGeometryV2* offsetGeom = geos.offsetCurve( distance, segments, joinStyle, mitreLimit );
  if ( !offsetGeom )
  {
    return nullptr;
  }
  return new QgsGeometry( offsetGeom );
}

QgsGeometry* QgsGeometry::simplify( double tolerance ) const
{
  if ( !d->geometry )
  {
    return nullptr;
  }

  QgsGeos geos( d->geometry );
  QgsAbstractGeometryV2* simplifiedGeom = geos.simplify( tolerance );
  if ( !simplifiedGeom )
  {
    return nullptr;
  }
  return new QgsGeometry( simplifiedGeom );
}

QgsGeometry* QgsGeometry::centroid() const
{
  if ( !d->geometry )
  {
    return nullptr;
  }

  QgsGeos geos( d->geometry );
  QgsPointV2 centroid;
  bool ok = geos.centroid( centroid );
  if ( !ok )
  {
    return nullptr;
  }
  return new QgsGeometry( centroid.clone() );
}

QgsGeometry* QgsGeometry::pointOnSurface() const
{
  if ( !d->geometry )
  {
    return nullptr;
  }

  QgsGeos geos( d->geometry );
  QgsPointV2 pt;
  bool ok = geos.pointOnSurface( pt );
  if ( !ok )
  {
    return nullptr;
  }
  return new QgsGeometry( pt.clone() );
}

QgsGeometry* QgsGeometry::convexHull() const
{
  if ( !d->geometry )
  {
    return nullptr;
  }
  QgsGeos geos( d->geometry );
  QgsAbstractGeometryV2* cHull = geos.convexHull();
  if ( !cHull )
  {
    return nullptr;
  }
  return new QgsGeometry( cHull );
}

QgsGeometry* QgsGeometry::interpolate( double distance ) const
{
  if ( !d->geometry )
  {
    return nullptr;
  }
  QgsGeos geos( d->geometry );
  QgsAbstractGeometryV2* result = geos.interpolate( distance );
  if ( !result )
  {
    return nullptr;
  }
  return new QgsGeometry( result );
}

QgsGeometry* QgsGeometry::intersection( const QgsGeometry* geometry ) const
{
  if ( !d->geometry || !geometry->d->geometry )
  {
    return nullptr;
  }

  QgsGeos geos( d->geometry );

  QgsAbstractGeometryV2* resultGeom = geos.intersection( *( geometry->d->geometry ) );
  return new QgsGeometry( resultGeom );
}

QgsGeometry* QgsGeometry::combine( const QgsGeometry* geometry ) const
{
  if ( !d->geometry || !geometry->d->geometry )
  {
    return nullptr;
  }

  QgsGeos geos( d->geometry );

  QgsAbstractGeometryV2* resultGeom = geos.combine( *( geometry->d->geometry ) );
  if ( !resultGeom )
  {
    return nullptr;
  }
  return new QgsGeometry( resultGeom );
}

QgsGeometry* QgsGeometry::difference( const QgsGeometry* geometry ) const
{
  if ( !d->geometry || !geometry->d->geometry )
  {
    return nullptr;
  }

  QgsGeos geos( d->geometry );

  QgsAbstractGeometryV2* resultGeom = geos.difference( *( geometry->d->geometry ) );
  if ( !resultGeom )
  {
    return nullptr;
  }
  return new QgsGeometry( resultGeom );
}

QgsGeometry* QgsGeometry::symDifference( const QgsGeometry* geometry ) const
{
  if ( !d->geometry || !geometry->d->geometry )
  {
    return nullptr;
  }

  QgsGeos geos( d->geometry );

  QgsAbstractGeometryV2* resultGeom = geos.symDifference( *( geometry->d->geometry ) );
  if ( !resultGeom )
  {
    return nullptr;
  }
  return new QgsGeometry( resultGeom );
}

QgsGeometry QgsGeometry::extrude( double x, double y )
{
  QgsInternalGeometryEngine engine( *this );

  return engine.extrude( x, y );
}

QList<QgsGeometry*> QgsGeometry::asGeometryCollection() const
{
  QList<QgsGeometry*> geometryList;
  if ( !d->geometry )
  {
    return geometryList;
  }

  QgsGeometryCollectionV2* gc = dynamic_cast<QgsGeometryCollectionV2*>( d->geometry );
  if ( gc )
  {
    int numGeom = gc->numGeometries();
    geometryList.reserve( numGeom );
    for ( int i = 0; i < numGeom; ++i )
    {
      geometryList.append( new QgsGeometry( gc->geometryN( i )->clone() ) );
    }
  }
  else //a singlepart geometry
  {
    geometryList.append( new QgsGeometry( d->geometry->clone() ) );
  }

  return geometryList;
}

QPointF QgsGeometry::asQPointF() const
{
  QgsPoint point = asPoint();
  return point.toQPointF();
}

QPolygonF QgsGeometry::asQPolygonF() const
{
  QPolygonF result;
  QgsPolyline polyline;
  QGis::WkbType type = wkbType();
  if ( type == QGis::WKBLineString || type == QGis::WKBLineString25D )
  {
    polyline = asPolyline();
  }
  else if ( type == QGis::WKBPolygon || type == QGis::WKBPolygon25D )
  {
    QgsPolygon polygon = asPolygon();
    if ( polygon.size() < 1 )
      return result;
    polyline = polygon.at( 0 );
  }
  else
  {
    return result;
  }

  QgsPolyline::const_iterator lineIt = polyline.constBegin();
  for ( ; lineIt != polyline.constEnd(); ++lineIt )
  {
    result << lineIt->toQPointF();
  }
  return result;
}

bool QgsGeometry::deleteRing( int ringNum, int partNum )
{
  if ( !d->geometry )
  {
    return false;
  }

  detach( true );
  bool ok = QgsGeometryEditUtils::deleteRing( d->geometry, ringNum, partNum );
  removeWkbGeos();
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
    setGeometry( nullptr );
    return true;
  }

  detach( true );
  bool ok = QgsGeometryEditUtils::deletePart( d->geometry, partNum );
  removeWkbGeos();
  return ok;
}

int QgsGeometry::avoidIntersections( const QMap<QgsVectorLayer*, QSet< QgsFeatureId > >& ignoreFeatures )
{
  if ( !d->geometry )
  {
    return 1;
  }

  QgsAbstractGeometryV2* diffGeom = QgsGeometryEditUtils::avoidIntersections( *( d->geometry ), ignoreFeatures );
  if ( diffGeom )
  {
    detach( false );
    d->geometry = diffGeom;
    removeWkbGeos();
  }
  return 0;
}

void QgsGeometry::validateGeometry( QList<Error> &errors )
{
  QgsGeometryValidator::validateGeometry( this, errors );
}

bool QgsGeometry::isGeosValid() const
{
  if ( !d->geometry )
  {
    return false;
  }

  QgsGeos geos( d->geometry );
  return geos.isValid();
}

bool QgsGeometry::isGeosEqual( const QgsGeometry& g ) const
{
  if ( !d->geometry || !g.d->geometry )
  {
    return false;
  }

  QgsGeos geos( d->geometry );
  return geos.isEqual( *( g.d->geometry ) );
}

bool QgsGeometry::isGeosEmpty() const
{
  if ( !d->geometry )
  {
    return false;
  }

  QgsGeos geos( d->geometry );
  return geos.isEmpty();
}

QgsGeometry *QgsGeometry::unaryUnion( const QList<QgsGeometry*> &geometryList )
{
  QgsGeos geos( nullptr );

  QList<QgsAbstractGeometryV2*> geomV2List;
  QList<QgsGeometry*>::const_iterator it = geometryList.constBegin();
  for ( ; it != geometryList.constEnd(); ++it )
  {
    if ( *it )
    {
      geomV2List.append(( *it )->geometry() );
    }
  }

  QgsAbstractGeometryV2* geom = geos.combine( geomV2List );
  return new QgsGeometry( geom );
}

void QgsGeometry::convertToStraightSegment()
{
  if ( !d->geometry || !requiresConversionToStraightSegments() )
  {
    return;
  }

  QgsAbstractGeometryV2* straightGeom = d->geometry->segmentize();
  detach( false );

  d->geometry = straightGeom;
  removeWkbGeos();
}

bool QgsGeometry::requiresConversionToStraightSegments() const
{
  if ( !d->geometry )
  {
    return false;
  }

  return d->geometry->hasCurvedSegments();
}

int QgsGeometry::transform( const QgsCoordinateTransform& ct )
{
  if ( !d->geometry )
  {
    return 1;
  }

  detach();
  d->geometry->transform( ct );
  removeWkbGeos();
  return 0;
}

int QgsGeometry::transform( const QTransform& ct )
{
  if ( !d->geometry )
  {
    return 1;
  }

  detach();
  d->geometry->transform( ct );
  removeWkbGeos();
  return 0;
}

void QgsGeometry::mapToPixel( const QgsMapToPixel& mtp )
{
  if ( d->geometry )
  {
    detach();
    d->geometry->transform( mtp.transform() );
    removeWkbGeos();
  }
}

#if 0
void QgsGeometry::clip( const QgsRectangle& rect )
{
  if ( d->geometry )
  {
    detach();
    d->geometry->clip( rect );
    removeWkbGeos();
  }
}
#endif

void QgsGeometry::draw( QPainter& p ) const
{
  if ( d->geometry )
  {
    d->geometry->draw( p );
  }
}

bool QgsGeometry::vertexIdFromVertexNr( int nr, QgsVertexId& id ) const
{
  if ( !d->geometry )
  {
    return false;
  }

  QgsCoordinateSequenceV2 coords = d->geometry->coordinateSequence();

  int vertexCount = 0;
  for ( int part = 0; part < coords.size(); ++part )
  {
    const QgsRingSequenceV2 &featureCoords = coords.at( part );
    for ( int ring = 0; ring < featureCoords.size(); ++ring )
    {
      const QgsPointSequenceV2 &ringCoords = featureCoords.at( ring );
      for ( int vertex = 0; vertex < ringCoords.size(); ++vertex )
      {
        if ( vertexCount == nr )
        {
          id.part = part;
          id.ring = ring;
          id.vertex = vertex;
          return true;
        }
        ++vertexCount;
      }
    }
  }
  return false;
}

int QgsGeometry::vertexNrFromVertexId( QgsVertexId id ) const
{
  if ( !d->geometry )
  {
    return -1;
  }

  QgsCoordinateSequenceV2 coords = d->geometry->coordinateSequence();

  int vertexCount = 0;
  for ( int part = 0; part < coords.size(); ++part )
  {
    const QgsRingSequenceV2 &featureCoords = coords.at( part );
    for ( int ring = 0; ring < featureCoords.size(); ++ring )
    {
      const QgsPointSequenceV2 &ringCoords = featureCoords.at( ring );
      for ( int vertex = 0; vertex < ringCoords.size(); ++vertex )
      {
        if ( vertex == id.vertex && ring == id.ring && part == id.part )
        {
          return vertexCount;
        }
        ++vertexCount;
      }
    }
  }
  return -1;
}

void QgsGeometry::convertPointList( const QList<QgsPoint> &input, QgsPointSequenceV2 &output )
{
  output.clear();
  QList<QgsPoint>::const_iterator it = input.constBegin();
  for ( ; it != input.constEnd(); ++it )
  {
    output.append( QgsPointV2( it->x(), it->y() ) );
  }
}

void QgsGeometry::convertPointList( const QgsPointSequenceV2 &input, QList<QgsPoint> &output )
{
  output.clear();
  QgsPointSequenceV2::const_iterator it = input.constBegin();
  for ( ; it != input.constEnd(); ++it )
  {
    output.append( QgsPoint( it->x(), it->y() ) );
  }
}

void QgsGeometry::convertToPolyline( const QgsPointSequenceV2 &input, QgsPolyline& output )
{
  output.clear();
  output.resize( input.size() );

  for ( int i = 0; i < input.size(); ++i )
  {
    const QgsPointV2& pt = input.at( i );
    output[i].setX( pt.x() );
    output[i].setY( pt.y() );
  }
}

void QgsGeometry::convertPolygon( const QgsPolygonV2& input, QgsPolygon& output )
{
  output.clear();
  QgsCoordinateSequenceV2 coords = input.coordinateSequence();
  if ( coords.size() < 1 )
  {
    return;
  }
  const QgsRingSequenceV2 &rings = coords[0];
  output.resize( rings.size() );
  for ( int i = 0; i < rings.size(); ++i )
  {
    convertToPolyline( rings[i], output[i] );
  }
}

GEOSContextHandle_t QgsGeometry::getGEOSHandler()
{
  return QgsGeos::getGEOSHandler();
}

QgsGeometry *QgsGeometry::fromQPointF( QPointF point )
{
  return new QgsGeometry( new QgsPointV2( point.x(), point.y() ) );
}

QgsGeometry *QgsGeometry::fromQPolygonF( const QPolygonF &polygon )
{
  if ( polygon.isClosed() )
  {
    return QgsGeometry::fromPolygon( createPolygonFromQPolygonF( polygon ) );
  }
  else
  {
    return QgsGeometry::fromPolyline( createPolylineFromQPolygonF( polygon ) );
  }
}

QgsPolygon QgsGeometry::createPolygonFromQPolygonF( const QPolygonF &polygon )
{
  QgsPolygon result;
  result << createPolylineFromQPolygonF( polygon );
  return result;
}

QgsPolyline QgsGeometry::createPolylineFromQPolygonF( const QPolygonF &polygon )
{
  QgsPolyline result;
  QPolygonF::const_iterator it = polygon.constBegin();
  for ( ; it != polygon.constEnd(); ++it )
  {
    result.append( QgsPoint( *it ) );
  }
  return result;
}

bool QgsGeometry::compare( const QgsPolyline &p1, const QgsPolyline &p2, double epsilon )
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

bool QgsGeometry::compare( const QgsPolygon &p1, const QgsPolygon &p2, double epsilon )
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


bool QgsGeometry::compare( const QgsMultiPolygon &p1, const QgsMultiPolygon &p2, double epsilon )
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

QgsGeometry* QgsGeometry::smooth( const unsigned int iterations, const double offset ) const
{
  switch ( wkbType() )
  {
    case QGis::WKBPoint:
    case QGis::WKBPoint25D:
    case QGis::WKBMultiPoint:
    case QGis::WKBMultiPoint25D:
      //can't smooth a point based geometry
      return new QgsGeometry( *this );

    case QGis::WKBLineString:
    case QGis::WKBLineString25D:
    {
      QgsPolyline line = asPolyline();
      return QgsGeometry::fromPolyline( smoothLine( line, iterations, offset ) );
    }

    case QGis::WKBMultiLineString:
    case QGis::WKBMultiLineString25D:
    {
      QgsMultiPolyline multiline = asMultiPolyline();
      QgsMultiPolyline resultMultiline;
      QgsMultiPolyline::const_iterator lineIt = multiline.constBegin();
      for ( ; lineIt != multiline.constEnd(); ++lineIt )
      {
        resultMultiline << smoothLine( *lineIt, iterations, offset );
      }
      return QgsGeometry::fromMultiPolyline( resultMultiline );
    }

    case QGis::WKBPolygon:
    case QGis::WKBPolygon25D:
    {
      QgsPolygon poly = asPolygon();
      return QgsGeometry::fromPolygon( smoothPolygon( poly, iterations, offset ) );
    }

    case QGis::WKBMultiPolygon:
    case QGis::WKBMultiPolygon25D:
    {
      QgsMultiPolygon multipoly = asMultiPolygon();
      QgsMultiPolygon resultMultipoly;
      QgsMultiPolygon::const_iterator polyIt = multipoly.constBegin();
      for ( ; polyIt != multipoly.constEnd(); ++polyIt )
      {
        resultMultipoly << smoothPolygon( *polyIt, iterations, offset );
      }
      return QgsGeometry::fromMultiPolygon( resultMultipoly );
    }

    case QGis::WKBUnknown:
    default:
      return new QgsGeometry( *this );
  }
}

inline QgsPoint interpolatePointOnLine( const QgsPoint& p1, const QgsPoint& p2, const double offset )
{
  double deltaX = p2.x() - p1.x();
  double deltaY = p2.y() - p1.y();
  return QgsPoint( p1.x() + deltaX * offset, p1.y() + deltaY * offset );
}

QgsPolyline QgsGeometry::smoothLine( const QgsPolyline& polyline, const unsigned int iterations, const double offset ) const
{
  QgsPolyline result = polyline;
  for ( unsigned int iteration = 0; iteration < iterations; ++iteration )
  {
    QgsPolyline outputLine = QgsPolyline();
    outputLine.reserve( 2 * ( result.count() - 1 ) );
    for ( int i = 0; i < result.count() - 1; i++ )
    {
      const QgsPoint& p1 = result.at( i );
      const QgsPoint& p2 = result.at( i + 1 );
      outputLine << ( i == 0 ? result.at( i ) : interpolatePointOnLine( p1, p2, offset ) );
      outputLine << ( i == result.count() - 2 ? result.at( i + 1 ) : interpolatePointOnLine( p1, p2, 1.0 - offset ) );
    }
    result = outputLine;
  }
  return result;
}

QgsPolygon QgsGeometry::smoothPolygon( const QgsPolygon& polygon, const unsigned int iterations, const double offset ) const
{
  QgsPolygon resultPoly;
  QgsPolygon::const_iterator ringIt = polygon.constBegin();
  for ( ; ringIt != polygon.constEnd(); ++ringIt )
  {
    QgsPolyline resultRing = *ringIt;
    for ( unsigned int iteration = 0; iteration < iterations; ++iteration )
    {
      QgsPolyline outputRing = QgsPolyline();
      outputRing.reserve( 2 * ( resultRing.count() - 1 ) + 1 );
      for ( int i = 0; i < resultRing.count() - 1; ++i )
      {
        const QgsPoint& p1 = resultRing.at( i );
        const QgsPoint& p2 = resultRing.at( i + 1 );
        outputRing << interpolatePointOnLine( p1, p2, offset );
        outputRing << interpolatePointOnLine( p1, p2, 1.0 - offset );
      }
      //close polygon
      outputRing << outputRing.at( 0 );

      resultRing = outputRing;
    }
    resultPoly << resultRing;
  }
  return resultPoly;
}

QgsGeometry* QgsGeometry::convertToPoint( bool destMultipart ) const
{
  switch ( type() )
  {
    case QGis::Point:
    {
      bool srcIsMultipart = isMultipart();

      if (( destMultipart && srcIsMultipart ) ||
          ( !destMultipart && !srcIsMultipart ) )
      {
        // return a copy of the same geom
        return new QgsGeometry( *this );
      }
      if ( destMultipart )
      {
        // layer is multipart => make a multipoint with a single point
        return fromMultiPoint( QgsMultiPoint() << asPoint() );
      }
      else
      {
        // destination is singlepart => make a single part if possible
        QgsMultiPoint multiPoint = asMultiPoint();
        if ( multiPoint.count() == 1 )
        {
          return fromPoint( multiPoint[0] );
        }
      }
      return nullptr;
    }

    case QGis::Line:
    {
      // only possible if destination is multipart
      if ( !destMultipart )
        return nullptr;

      // input geometry is multipart
      if ( isMultipart() )
      {
        QgsMultiPolyline multiLine = asMultiPolyline();
        QgsMultiPoint multiPoint;
        for ( QgsMultiPolyline::const_iterator multiLineIt = multiLine.constBegin(); multiLineIt != multiLine.constEnd(); ++multiLineIt )
          for ( QgsPolyline::const_iterator lineIt = ( *multiLineIt ).constBegin(); lineIt != ( *multiLineIt ).constEnd(); ++lineIt )
            multiPoint << *lineIt;
        return fromMultiPoint( multiPoint );
      }
      // input geometry is not multipart: copy directly the line into a multipoint
      else
      {
        QgsPolyline line = asPolyline();
        if ( !line.isEmpty() )
          return fromMultiPoint( line );
      }
      return nullptr;
    }

    case QGis::Polygon:
    {
      // can only transform if destination is multipoint
      if ( !destMultipart )
        return nullptr;

      // input geometry is multipart: make a multipoint from multipolygon
      if ( isMultipart() )
      {
        QgsMultiPolygon multiPolygon = asMultiPolygon();
        QgsMultiPoint multiPoint;
        for ( QgsMultiPolygon::const_iterator polygonIt = multiPolygon.constBegin(); polygonIt != multiPolygon.constEnd(); ++polygonIt )
          for ( QgsMultiPolyline::const_iterator multiLineIt = ( *polygonIt ).constBegin(); multiLineIt != ( *polygonIt ).constEnd(); ++multiLineIt )
            for ( QgsPolyline::const_iterator lineIt = ( *multiLineIt ).constBegin(); lineIt != ( *multiLineIt ).constEnd(); ++lineIt )
              multiPoint << *lineIt;
        return fromMultiPoint( multiPoint );
      }
      // input geometry is not multipart: make a multipoint from polygon
      else
      {
        QgsPolygon polygon = asPolygon();
        QgsMultiPoint multiPoint;
        for ( QgsMultiPolyline::const_iterator multiLineIt = polygon.constBegin(); multiLineIt != polygon.constEnd(); ++multiLineIt )
          for ( QgsPolyline::const_iterator lineIt = ( *multiLineIt ).constBegin(); lineIt != ( *multiLineIt ).constEnd(); ++lineIt )
            multiPoint << *lineIt;
        return fromMultiPoint( multiPoint );
      }
    }

    default:
      return nullptr;
  }
}

QgsGeometry* QgsGeometry::convertToLine( bool destMultipart ) const
{
  switch ( type() )
  {
    case QGis::Point:
    {
      if ( !isMultipart() )
        return nullptr;

      QgsMultiPoint multiPoint = asMultiPoint();
      if ( multiPoint.count() < 2 )
        return nullptr;

      if ( destMultipart )
        return fromMultiPolyline( QgsMultiPolyline() << multiPoint );
      else
        return fromPolyline( multiPoint );
    }

    case QGis::Line:
    {
      bool srcIsMultipart = isMultipart();

      if (( destMultipart && srcIsMultipart ) ||
          ( !destMultipart && ! srcIsMultipart ) )
      {
        // return a copy of the same geom
        return new QgsGeometry( *this );
      }
      if ( destMultipart )
      {
        // destination is multipart => makes a multipoint with a single line
        QgsPolyline line = asPolyline();
        if ( !line.isEmpty() )
          return fromMultiPolyline( QgsMultiPolyline() << line );
      }
      else
      {
        // destination is singlepart => make a single part if possible
        QgsMultiPolyline multiLine = asMultiPolyline();
        if ( multiLine.count() == 1 )
          return fromPolyline( multiLine[0] );
      }
      return nullptr;
    }

    case QGis::Polygon:
    {
      // input geometry is multipolygon
      if ( isMultipart() )
      {
        QgsMultiPolygon multiPolygon = asMultiPolygon();
        QgsMultiPolyline multiLine;
        for ( QgsMultiPolygon::const_iterator polygonIt = multiPolygon.constBegin(); polygonIt != multiPolygon.constEnd(); ++polygonIt )
          for ( QgsMultiPolyline::const_iterator multiLineIt = ( *polygonIt ).constBegin(); multiLineIt != ( *polygonIt ).constEnd(); ++multiLineIt )
            multiLine << *multiLineIt;

        if ( destMultipart )
        {
          // destination is multipart
          return fromMultiPolyline( multiLine );
        }
        else if ( multiLine.count() == 1 )
        {
          // destination is singlepart => make a single part if possible
          return fromPolyline( multiLine[0] );
        }
      }
      // input geometry is single polygon
      else
      {
        QgsPolygon polygon = asPolygon();
        // if polygon has rings
        if ( polygon.count() > 1 )
        {
          // cannot fit a polygon with rings in a single line layer
          // TODO: would it be better to remove rings?
          if ( destMultipart )
          {
            QgsPolygon polygon = asPolygon();
            QgsMultiPolyline multiLine;
            for ( QgsMultiPolyline::const_iterator multiLineIt = polygon.constBegin(); multiLineIt != polygon.constEnd(); ++multiLineIt )
              multiLine << *multiLineIt;
            return fromMultiPolyline( multiLine );
          }
        }
        // no rings
        else if ( polygon.count() == 1 )
        {
          if ( destMultipart )
          {
            return fromMultiPolyline( polygon );
          }
          else
          {
            return fromPolyline( polygon[0] );
          }
        }
      }
      return nullptr;
    }

    default:
      return nullptr;
  }
}

QgsGeometry* QgsGeometry::convertToPolygon( bool destMultipart ) const
{
  switch ( type() )
  {
    case QGis::Point:
    {
      if ( !isMultipart() )
        return nullptr;

      QgsMultiPoint multiPoint = asMultiPoint();
      if ( multiPoint.count() < 3 )
        return nullptr;

      if ( multiPoint.last() != multiPoint.first() )
        multiPoint << multiPoint.first();

      QgsPolygon polygon = QgsPolygon() << multiPoint;
      if ( destMultipart )
        return fromMultiPolygon( QgsMultiPolygon() << polygon );
      else
        return fromPolygon( polygon );
    }

    case QGis::Line:
    {
      // input geometry is multiline
      if ( isMultipart() )
      {
        QgsMultiPolyline multiLine = asMultiPolyline();
        QgsMultiPolygon multiPolygon;
        for ( QgsMultiPolyline::iterator multiLineIt = multiLine.begin(); multiLineIt != multiLine.end(); ++multiLineIt )
        {
          // do not create polygon for a 1 segment line
          if (( *multiLineIt ).count() < 3 )
            return nullptr;
          if (( *multiLineIt ).count() == 3 && ( *multiLineIt ).first() == ( *multiLineIt ).last() )
            return nullptr;

          // add closing node
          if (( *multiLineIt ).first() != ( *multiLineIt ).last() )
            *multiLineIt << ( *multiLineIt ).first();
          multiPolygon << ( QgsPolygon() << *multiLineIt );
        }
        // check that polygons were inserted
        if ( !multiPolygon.isEmpty() )
        {
          if ( destMultipart )
          {
            return fromMultiPolygon( multiPolygon );
          }
          else if ( multiPolygon.count() == 1 )
          {
            // destination is singlepart => make a single part if possible
            return fromPolygon( multiPolygon[0] );
          }
        }
      }
      // input geometry is single line
      else
      {
        QgsPolyline line = asPolyline();

        // do not create polygon for a 1 segment line
        if ( line.count() < 3 )
          return nullptr;
        if ( line.count() == 3 && line.first() == line.last() )
          return nullptr;

        // add closing node
        if ( line.first() != line.last() )
          line << line.first();

        // destination is multipart
        if ( destMultipart )
        {
          return fromMultiPolygon( QgsMultiPolygon() << ( QgsPolygon() << line ) );
        }
        else
        {
          return fromPolygon( QgsPolygon() << line );
        }
      }
      return nullptr;
    }

    case QGis::Polygon:
    {
      bool srcIsMultipart = isMultipart();

      if (( destMultipart && srcIsMultipart ) ||
          ( !destMultipart && ! srcIsMultipart ) )
      {
        // return a copy of the same geom
        return new QgsGeometry( *this );
      }
      if ( destMultipart )
      {
        // destination is multipart => makes a multipoint with a single polygon
        QgsPolygon polygon = asPolygon();
        if ( !polygon.isEmpty() )
          return fromMultiPolygon( QgsMultiPolygon() << polygon );
      }
      else
      {
        QgsMultiPolygon multiPolygon = asMultiPolygon();
        if ( multiPolygon.count() == 1 )
        {
          // destination is singlepart => make a single part if possible
          return fromPolygon( multiPolygon[0] );
        }
      }
      return nullptr;
    }

    default:
      return nullptr;
  }
}

QgsGeometryEngine* QgsGeometry::createGeometryEngine( const QgsAbstractGeometryV2* geometry )
{
  return new QgsGeos( geometry );
}

QDataStream& operator<<( QDataStream& out, const QgsGeometry& geometry )
{
  QByteArray byteArray = QByteArray::fromRawData( reinterpret_cast< const char * >( geometry.asWkb() ), geometry.wkbSize() ); // does not copy data and does not take ownership
  out << byteArray;
  return out;
}

QDataStream& operator>>( QDataStream& in, QgsGeometry& geometry )
{
  QByteArray byteArray;
  in >> byteArray;
  if ( byteArray.isEmpty() )
  {
    geometry.setGeometry( nullptr );
    return in;
  }

  char *data = new char[byteArray.size()];
  memcpy( data, byteArray.data(), byteArray.size() );
  geometry.fromWkb( reinterpret_cast< unsigned char* >( data ), byteArray.size() );
  return in;
}
