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
#include "qgsgeometryimport.h"
#include "qgsgeometryutils.h"
#include "qgsgeos.h"
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgspoint.h"
#include "qgsrectangle.h"

#include "qgsmaplayerregistry.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsmessagelog.h"
#include "qgsgeometryvalidator.h"

#include "qgsmulticurvev2.h"
#include "qgsmultipointv2.h"
#include "qgsmultisurfacev2.h"
#include "qgspointv2.h"
#include "qgspolygonv2.h"
#include "qgslinestringv2.h"

#ifndef Q_WS_WIN
#include <netinet/in.h>
#else
#include <winsock.h>
#endif

struct QgsGeometryData
{
  QAtomicInt ref;
  QgsAbstractGeometryV2* geometry;
};

QgsGeometry::QgsGeometry(): d( new QgsGeometryData() ), mWkb( 0 ), mWkbSize( 0 ), mGeos( 0 )
{
  d->geometry = 0;
  d->ref = QAtomicInt( 1 );
}

QgsGeometry::~QgsGeometry()
{
  if ( d )
  {
    if ( !d->ref.deref() )
    {
      delete d->geometry;
      delete d;
    }
  }
  removeWkbGeos();
}

QgsGeometry::QgsGeometry( QgsAbstractGeometryV2* geom ): d( new QgsGeometryData() ), mWkb( 0 ), mWkbSize( 0 ), mGeos( 0 )
{
  d->geometry = geom;
  d->ref = QAtomicInt( 1 );
}

QgsGeometry::QgsGeometry( const QgsGeometry& other ): mWkb( 0 ), mWkbSize( 0 ), mGeos( 0 )
{
  d = other.d;
  d->ref.ref();
}

QgsGeometry& QgsGeometry::operator=( QgsGeometry const & other )
{
  if ( !d->ref.deref() )
  {
    delete d->geometry;
    delete d;
  }

  removeWkbGeos();

  d = other.d;
  d->ref.ref();
  return *this;
}

void QgsGeometry::detach( bool cloneGeom )
{
  if ( !d )
  {
    return;
  }

  removeWkbGeos();

  if ( d->ref > 1 )
  {
    d->ref.deref();
    QgsAbstractGeometryV2* cGeom = 0;
    if ( d->geometry && cloneGeom )
    {
      cGeom = d->geometry->clone();
    }
    d = new QgsGeometryData();
    d->geometry = cGeom;
    d->ref = QAtomicInt( 1 );
  }
}

void QgsGeometry::removeWkbGeos()
{
  delete[] mWkb;
  mWkb = 0;
  mWkbSize = 0;
  GEOSGeom_destroy( mGeos );
  mGeos = 0;
}

const QgsAbstractGeometryV2* QgsGeometry::geometry() const
{
  if ( !d )
  {
    return 0;
  }
  return d->geometry;
}

QgsGeometry* QgsGeometry::fromWkt( QString wkt )
{
  QgsAbstractGeometryV2* geom = QgsGeometryImport::geomFromWkt( wkt );
  return new QgsGeometry( geom );
}

QgsGeometry* QgsGeometry::fromPoint( const QgsPoint& point )
{
  QgsAbstractGeometryV2* geom = QgsGeometryImport::fromPoint( point );
  if ( geom )
  {
    return new QgsGeometry( geom );
  }
  return 0;
}

QgsGeometry* QgsGeometry::fromPolyline( const QgsPolyline& polyline )
{
  QgsAbstractGeometryV2* geom = QgsGeometryImport::fromPolyline( polyline );
  if ( geom )
  {
    return new QgsGeometry( geom );
  }
  return 0;
}

QgsGeometry* QgsGeometry::fromPolygon( const QgsPolygon& polygon )
{
  QgsAbstractGeometryV2* geom = QgsGeometryImport::fromPolygon( polygon );
  if ( geom )
  {
    return new QgsGeometry( geom );
  }
  return 0;
}

QgsGeometry* QgsGeometry::fromMultiPoint( const QgsMultiPoint& multipoint )
{
  QgsAbstractGeometryV2* geom = QgsGeometryImport::fromMultiPoint( multipoint );
  if ( geom )
  {
    return new QgsGeometry( geom );
  }
  return 0;
}

QgsGeometry* QgsGeometry::fromMultiPolyline( const QgsMultiPolyline& multiline )
{
  QgsAbstractGeometryV2* geom = QgsGeometryImport::fromMultiPolyline( multiline );
  if ( geom )
  {
    return new QgsGeometry( geom );
  }
  return 0;
}

QgsGeometry* QgsGeometry::fromMultiPolygon( const QgsMultiPolygon& multipoly )
{
  QgsAbstractGeometryV2* geom = QgsGeometryImport::fromMultiPolygon( multipoly );
  if ( geom )
  {
    return new QgsGeometry( geom );
  }
  return 0;
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

void QgsGeometry::fromWkb( unsigned char *wkb, size_t length )
{
  Q_UNUSED( length );
  if ( !d )
  {
    return;
  }

  detach( false );

  if ( d->geometry )
  {
    delete d->geometry;
    removeWkbGeos();
  }
  d->geometry = QgsGeometryImport::geomFromWkb( wkb );
  delete[] wkb;
}

const unsigned char *QgsGeometry::asWkb() const
{
  if ( !d || !d->geometry )
  {
    return 0;
  }

  if ( !mWkb )
  {
    mWkb = d->geometry->asWkb( mWkbSize );
  }
  return mWkb;
}

size_t QgsGeometry::wkbSize() const
{
  if ( !d || !d->geometry )
  {
    return 0;
  }

  if ( !mWkb )
  {
    mWkb = d->geometry->asWkb( mWkbSize );
  }
  return mWkbSize;
}

const GEOSGeometry* QgsGeometry::asGeos() const
{
  if ( !d || !d->geometry )
  {
    return 0;
  }

  if ( !mGeos )
  {
    mGeos = QgsGeos::asGeos( d->geometry );
  }
  return mGeos;
}


QGis::WkbType QgsGeometry::wkbType() const
{
  if ( !d || !d->geometry )
  {
    return QGis::WKBUnknown;
  }
  else
  {
    return ( QGis::WkbType )d->geometry->wkbType();
  }
}


QGis::GeometryType QgsGeometry::type() const
{
  if ( !d || !d->geometry )
  {
    return QGis::UnknownGeometry;
  }
  return ( QGis::GeometryType )( QgsWKBTypes::geometryType( d->geometry->wkbType() ) );
}

bool QgsGeometry::isMultipart() const
{
  return QGis::isMultiType( wkbType() );
}

void QgsGeometry::fromGeos( GEOSGeometry *geos )
{
  if ( d )
  {
    detach( false );
    delete d->geometry;
    d->geometry = QgsGeos::fromGeos( geos );
  }
}

QgsPoint QgsGeometry::closestVertex( const QgsPoint& point, int& atVertex, int& beforeVertex, int& afterVertex, double& sqrDist ) const
{
  if ( !d || !d->geometry )
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

void QgsGeometry::adjacentVertices( int atVertex, int& beforeVertex, int& afterVertex ) const
{
  if ( !d || !d->geometry )
  {
    return;
  }

  QgsVertexId id;
  if ( !vertexIdFromVertexNr( atVertex, id ) )
  {
    return;
  }

  QgsVertexId beforeVertexId, afterVertexId;
  QgsGeometryUtils::adjacentVertices( *( d->geometry ), id, beforeVertexId, afterVertexId );
  beforeVertex = vertexNrFromVertexId( beforeVertexId );
  afterVertex = vertexNrFromVertexId( afterVertexId );
}

bool QgsGeometry::moveVertex( double x, double y, int atVertex )
{
  if ( !d || !d->geometry )
  {
    return false;
  }

  QgsVertexId id;
  if ( !vertexIdFromVertexNr( atVertex, id ) )
  {
    return false;
  }

  return d->geometry->moveVertex( id, QgsPointV2( x, y ) );
}

bool QgsGeometry::moveVertex( const QgsPointV2& p, int atVertex )
{
  if ( !d || !d->geometry )
  {
    return false;
  }

  QgsVertexId id;
  if ( !vertexIdFromVertexNr( atVertex, id ) )
  {
    return false;
  }

  return d->geometry->moveVertex( id, p );
}

bool QgsGeometry::deleteVertex( int atVertex )
{
  if ( !d || !d->geometry )
  {
    return false;
  }

  QgsVertexId id;
  if ( !vertexIdFromVertexNr( atVertex, id ) )
  {
    return false;
  }

  return d->geometry->deleteVertex( id );
}

bool QgsGeometry::insertVertex( double x, double y, int beforeVertex )
{
  if ( !d || !d->geometry )
  {
    return false;
  }

  QgsVertexId id;
  if ( !vertexIdFromVertexNr( beforeVertex, id ) )
  {
    return false;
  }

  return d->geometry->insertVertex( id, QgsPointV2( x, y ) );
}

QgsPoint QgsGeometry::vertexAt( int atVertex ) const
{
  if ( !d || !d->geometry )
  {
    return QgsPoint( 0, 0 );
  }

  QgsVertexId vId;
  vertexIdFromVertexNr( atVertex, vId );
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

double QgsGeometry::closestVertexWithContext( const QgsPoint& point, int& atVertex ) const
{
  if ( !d || !d->geometry )
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
  if ( !d || !d->geometry )
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

int QgsGeometry::addRing( const QList<QgsPoint>& ring )
{
  QgsLineStringV2* ringLine = new QgsLineStringV2();
  QList< QgsPointV2 > ringPoints;
  convertPointList( ring, ringPoints );
  ringLine->setPoints( ringPoints );
  return addRing( ringLine );
}

int QgsGeometry::addRing( QgsCurveV2* ring )
{
  if ( !d || !d->geometry )
  {
    return 1;
  }

  return QgsGeometryEditUtils::addRing( d->geometry, ring );
}

int QgsGeometry::addPart( const QList<QgsPoint> &points, QGis::GeometryType geomType )
{
  if ( !d || !d->geometry )
  {
    return 2;
  }

  QgsAbstractGeometryV2* partGeom = 0;
  if ( points.size() == 1 )
  {
    partGeom = new QgsPointV2( points[0].x(), points[0].y() );
  }
  else if ( points.size() > 1 )
  {
    QgsLineStringV2* ringLine = new QgsLineStringV2();
    QList< QgsPointV2 > partPoints;
    convertPointList( points, partPoints );
    ringLine->setPoints( partPoints );
    partGeom = ringLine;
  }
  return addPart( dynamic_cast<QgsCurveV2*>( partGeom ) );
}

int QgsGeometry::addPart( QgsCurveV2* part )
{
  return QgsGeometryEditUtils::addPart( d->geometry, part );
}

int QgsGeometry::addPart( const QgsGeometry *newPart )
{
  if ( !d || !d->geometry || !newPart || !newPart->d || !newPart->d->geometry )
  {
    return 1;
  }

  QgsAbstractGeometryV2* g = d->geometry->clone();
  QgsCurveV2* curve = dynamic_cast<QgsCurveV2*>( g );
  if ( !curve )
  {
    delete g; return 1;
  }
  return addPart( curve );
}

int QgsGeometry::addPart( GEOSGeometry *newPart )
{
  if ( !d || !d->geometry || !newPart )
  {
    return 1;
  }

  QgsAbstractGeometryV2* geom = QgsGeos::fromGeos( newPart );
  return QgsGeometryEditUtils::addPart( d->geometry, geom );
}

int QgsGeometry::translate( double dx, double dy )
{
  if ( !d || !d->geometry )
  {
    return 1;
  }

  d->geometry->transform( QTransform::fromTranslate( dx, dy ) );
  return 0;
}

int QgsGeometry::rotate( double rotation, const QgsPoint& center )
{
  if ( !d || !d->geometry )
  {
    return 1;
  }

  QTransform t = QTransform::fromTranslate( center.x(), center.y() );
  t.rotate( -rotation );
  t.translate( -center.x(), -center.y() );
  d->geometry->transform( t );
  return 0;
}

int QgsGeometry::splitGeometry( const QList<QgsPoint>& splitLine, QList<QgsGeometry*>& newGeometries, bool topological, QList<QgsPoint> &topologyTestPoints )
{
  if ( !d || !d->geometry )
  {
    return 0;
  }

  QList<QgsAbstractGeometryV2*> newGeoms;
  QgsLineStringV2 splitLineString;
  QList<QgsPointV2> splitLinePointsV2;
  convertPointList( splitLine, splitLinePointsV2 );
  splitLineString.setPoints( splitLinePointsV2 );
  QList<QgsPointV2> tp;

  QgsGeos geos( d->geometry );
  int result = geos.splitGeometry( splitLineString, newGeoms, topological, tp );

  newGeometries.clear();
  for ( int i = 0; i < newGeoms.size(); ++i )
  {
    newGeometries.push_back( new QgsGeometry( newGeoms.at( i ) ) );
  }

  convertPointList( tp, topologyTestPoints );
  return result;
}

/**Replaces a part of this geometry with another line*/
int QgsGeometry::reshapeGeometry( const QList<QgsPoint>& reshapeWithLine )
{
  if ( !d || !d->geometry )
  {
    return 0;
  }

  QList<QgsPointV2> reshapeLine;
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
    return 0;
  }
  return errorCode;
}

int QgsGeometry::makeDifference( const QgsGeometry* other )
{
  if ( !d || !d->geometry || !other->d || !other->d->geometry )
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
  return 0;
}

QgsRectangle QgsGeometry::boundingBox() const
{
  if ( d && d->geometry )
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
  if ( !d || !d->geometry || !geometry || !geometry->d || !geometry->d->geometry )
  {
    return false;
  }

  QgsGeos geos( d->geometry );
  return geos.intersects( *( geometry->d->geometry ) );
}

bool QgsGeometry::contains( const QgsPoint* p ) const
{
  if ( !d || !d->geometry || !p )
  {
    return false;
  }

  QgsPointV2 pt( p->x(), p->y() );
  QgsGeos geos( d->geometry );
  return geos.contains( pt );
}

bool QgsGeometry::contains( const QgsGeometry* geometry ) const
{
  if ( !d || !d->geometry || !geometry || !geometry->d || !geometry->d->geometry )
  {
    return false;
  }

  QgsGeos geos( d->geometry );
  return geos.contains( *( geometry->d->geometry ) );
}

bool QgsGeometry::disjoint( const QgsGeometry* geometry ) const
{
  if ( !d || !d->geometry || !geometry || !geometry->d || !geometry->d->geometry )
  {
    return false;
  }

  QgsGeos geos( d->geometry );
  return geos.disjoint( *( geometry->d->geometry ) );
}

bool QgsGeometry::equals( const QgsGeometry* geometry ) const
{
  if ( !d || !d->geometry || !geometry || !geometry->d || !geometry->d->geometry )
  {
    return false;
  }

  QgsGeos geos( d->geometry );
  return geos.isEqual( *( geometry->d->geometry ) );
}

bool QgsGeometry::touches( const QgsGeometry* geometry ) const
{
  if ( !d || !d->geometry || !geometry || !geometry->d || !geometry->d->geometry )
  {
    return false;
  }

  QgsGeos geos( d->geometry );
  return geos.touches( *( geometry->d->geometry ) );
}

bool QgsGeometry::overlaps( const QgsGeometry* geometry ) const
{
  if ( !d || !d->geometry || !geometry || !geometry->d || !geometry->d->geometry )
  {
    return false;
  }

  QgsGeos geos( d->geometry );
  return geos.overlaps( *( geometry->d->geometry ) );
}

bool QgsGeometry::within( const QgsGeometry* geometry ) const
{
  if ( !d || !d->geometry || !geometry || !geometry->d || !geometry->d->geometry )
  {
    return false;
  }

  QgsGeos geos( d->geometry );
  return geos.within( *( geometry->d->geometry ) );
}

bool QgsGeometry::crosses( const QgsGeometry* geometry ) const
{
  if ( !d || !d->geometry || !geometry || !geometry->d || !geometry->d->geometry )
  {
    return false;
  }

  QgsGeos geos( d->geometry );
  return geos.crosses( *( geometry->d->geometry ) );
}

QString QgsGeometry::exportToWkt( const int &precision ) const
{
  if ( !d || !d->geometry )
  {
    return QString();
  }
  return d->geometry->asWkt( precision );
}

QString QgsGeometry::exportToGeoJSON( const int &precision ) const
{
  if ( !d || !d->geometry )
  {
    return QString();
  }
  return d->geometry->asJSON( precision );
}

QgsGeometry* QgsGeometry::convertToType( QGis::GeometryType destType, bool destMultipart ) const
{
  return 0; //todo...
#if 0
  switch ( destType )
  {
    case QGis::Point:
      return convertToPoint( destMultipart );

    case QGis::Line:
      return convertToLine( destMultipart );

    case QGis::Polygon:
      return convertToPolygon( destMultipart );

    default:
      return 0;
  }
#endif //0
}

bool QgsGeometry::convertToMultiType()
{
  return false; //todo...

#if 0
  // TODO: implement with GEOS
  if ( mDirtyWkb )
  {
    exportGeosToWkb();
  }

  if ( !mGeometry )
  {
    return false;
  }

  QGis::WkbType geomType = wkbType();

  if ( geomType == QGis::WKBMultiPoint || geomType == QGis::WKBMultiPoint25D ||
       geomType == QGis::WKBMultiLineString || geomType == QGis::WKBMultiLineString25D ||
       geomType == QGis::WKBMultiPolygon || geomType == QGis::WKBMultiPolygon25D || geomType == QGis::WKBUnknown )
  {
    return false; //no need to convert
  }

  size_t newGeomSize = mGeometrySize + 1 + 2 * sizeof( int ); //endian: 1, multitype: sizeof(int), number of geometries: sizeof(int)
  unsigned char* newGeometry = new unsigned char[newGeomSize];

  //copy endian
  char byteOrder = QgsApplication::endian();

  QgsWkbPtr wkbPtr( newGeometry );
  wkbPtr << byteOrder;

  //copy wkbtype
  //todo
  QGis::WkbType newMultiType;
  switch ( geomType )
  {
    case QGis::WKBPoint:
      newMultiType = QGis::WKBMultiPoint;
      break;
    case QGis::WKBPoint25D:
      newMultiType = QGis::WKBMultiPoint25D;
      break;
    case QGis::WKBLineString:
      newMultiType = QGis::WKBMultiLineString;
      break;
    case QGis::WKBLineString25D:
      newMultiType = QGis::WKBMultiLineString25D;
      break;
    case QGis::WKBPolygon:
      newMultiType = QGis::WKBMultiPolygon;
      break;
    case QGis::WKBPolygon25D:
      newMultiType = QGis::WKBMultiPolygon25D;
      break;
    default:
      delete [] newGeometry;
      return false;
  }

  wkbPtr << newMultiType << 1;

  //copy the existing single geometry
  memcpy( wkbPtr, mGeometry, mGeometrySize );

  delete [] mGeometry;
  mGeometry = newGeometry;
  mGeometrySize = newGeomSize;
  mDirtyGeos = true;
  return true;
#endif //0
}

QgsPoint QgsGeometry::asPoint() const
{
  if ( !d || !d->geometry || d->geometry->geometryType() != "Point" )
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
  if ( !d || !d->geometry )
  {
    return polyLine;
  }

  bool doSegmentation = ( d->geometry->geometryType() == "CompoundCurve" || d->geometry->geometryType() == "CircularString" );
  QgsLineStringV2* line = 0;
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
  bool doSegmentation = ( d->geometry->geometryType() == "CurvePolygon" );

  QgsPolygonV2* p = 0;
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
  if ( !d || !d->geometry || d->geometry->geometryType() != "MultiPoint" )
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
    const QgsPointV2* pt = dynamic_cast<const QgsPointV2*>( mp->geometryN( i ) );
    multiPoint[i].setX( pt->x() );
    multiPoint[i].setY( pt->y() );
  }
  return multiPoint;
}

QgsMultiPolyline QgsGeometry::asMultiPolyline() const
{
  if ( !d || !d->geometry || d->geometry->geometryType() != "MultiCurve" )
  {
    return QgsMultiPolyline();
  }

  QgsMultiCurveV2* multiCurve = dynamic_cast<QgsMultiCurveV2*>( d->geometry );
  if ( !multiCurve )
  {
    return QgsMultiPolyline();
  }

  int nLines = multiCurve->numGeometries();
  if ( nLines < 1 )
  {
    return QgsMultiPolyline();
  }

  QgsMultiPolyline mpl;
  for ( int i = 0; i < nLines; ++i )
  {
    QgsCurveV2* curve = dynamic_cast<QgsCurveV2*>( multiCurve->geometryN( i ) );
    if ( !curve )
    {
      continue;
    }
    QgsLineStringV2* linestring = curve->curveToLine();
    if ( linestring )
    {
      QList< QgsPointV2 > lineCoords;
      linestring->points( lineCoords );
      delete linestring;
      QgsPolyline polyLine;
      convertToPolyline( lineCoords, polyLine );
      mpl.append( polyLine );
    }
  }
  return mpl;
}

QgsMultiPolygon QgsGeometry::asMultiPolygon() const
{
  if ( !d || !d->geometry || d->geometry->geometryType() != "MultiSurface" )
  {
    return QgsMultiPolygon();
  }

  QgsMultiSurfaceV2* multiSurface = dynamic_cast<QgsMultiSurfaceV2*>( d->geometry );
  if ( !multiSurface )
  {
    return QgsMultiPolygon();
  }

  int nPolygons = multiSurface->numGeometries();
  if ( nPolygons < 1 )
  {
    return QgsMultiPolygon();
  }

  QgsMultiPolygon mp;
  for ( int i = 0; i < nPolygons; ++i )
  {
    QgsCurvePolygonV2* curvePolygon = dynamic_cast<QgsCurvePolygonV2*>( multiSurface->geometryN( i ) );
    if ( !curvePolygon )
    {
      continue;
    }
    QgsPolygonV2* polygon = curvePolygon->toPolygon();
    QgsPolygon poly;
    convertPolygon( *polygon, poly );
    delete polygon;
    mp.append( poly );
  }
  return mp;
}

double QgsGeometry::area() const
{
  if ( !d || !d->geometry )
  {
    return -1.0;
  }
  QgsGeos g( d->geometry );

  //debug: compare geos area with calculation in QGIS
  double geosArea = g.area();
  double qgisArea = 0;
  QgsSurfaceV2* surface = dynamic_cast<QgsSurfaceV2*>( d->geometry );
  if ( surface )
  {
    qgisArea = surface->area();
  }

  return g.area();
}

double QgsGeometry::length() const
{
  if ( !d || !d->geometry )
  {
    return -1.0;
  }
  QgsGeos g( d->geometry );
  return g.length();
}

double QgsGeometry::distance( const QgsGeometry& geom ) const
{
  if ( !d || !d->geometry || !geom.d || !geom.d->geometry )
  {
    return -1.0;
  }

  QgsGeos g( d->geometry );
  return g.distance( *( geom.d->geometry ) );
}

QgsGeometry* QgsGeometry::buffer( double distance, int segments ) const
{
  if ( !d || !d->geometry )
  {
    return 0;
  }

  QgsGeos g( d->geometry );
  QgsAbstractGeometryV2* geom = g.buffer( distance, segments );
  if ( !geom )
  {
    return 0;
  }
  return new QgsGeometry( geom );
}

QgsGeometry* QgsGeometry::buffer( double distance, int segments, int endCapStyle, int joinStyle, double mitreLimit ) const
{
  return 0; //todo...

#if 0
#if defined(GEOS_VERSION_MAJOR) && defined(GEOS_VERSION_MINOR) && \
 ((GEOS_VERSION_MAJOR>3) || ((GEOS_VERSION_MAJOR==3) && (GEOS_VERSION_MINOR>=3)))
  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( !mGeos )
    return 0;

  try
  {
    return fromGeosGeom( GEOSBufferWithStyle( mGeos, distance, segments, endCapStyle, joinStyle, mitreLimit ) );
  }
  CATCH_GEOS( 0 )
#else
  return 0;
#endif
#endif //0
}

QgsGeometry* QgsGeometry::offsetCurve( double distance, int segments, int joinStyle, double mitreLimit ) const
{
  if ( !d || !d->geometry )
  {
    return 0;
  }

  QgsGeos geos( d->geometry );
  QgsAbstractGeometryV2* offsetGeom = geos.offsetCurve( distance, segments, joinStyle, mitreLimit );
  if ( !offsetGeom )
  {
    return 0;
  }
  return new QgsGeometry( offsetGeom );
}

QgsGeometry* QgsGeometry::simplify( double tolerance ) const
{
  if ( !d || !d->geometry )
  {
    return 0;
  }

  QgsGeos geos( d->geometry );
  QgsAbstractGeometryV2* simplifiedGeom = geos.simplify( tolerance );
  if ( !simplifiedGeom )
  {
    return 0;
  }
  return new QgsGeometry( simplifiedGeom );
}

QgsGeometry* QgsGeometry::centroid() const
{
  if ( !d || !d->geometry )
  {
    return 0;
  }

  QgsGeos geos( d->geometry );
  QgsPointV2 centroid;
  bool ok = geos.centroid( centroid );
  if ( !ok )
  {
    return 0;
  }
  return new QgsGeometry( centroid.clone() );
}

QgsGeometry* QgsGeometry::pointOnSurface() const
{
  if ( !d || !d->geometry )
  {
    return 0;
  }

  QgsGeos geos( d->geometry );
  QgsPointV2 pt;
  bool ok = geos.pointOnSurface( pt );
  if ( !ok )
  {
    return 0;
  }
  return new QgsGeometry( pt.clone() );
}

QgsGeometry* QgsGeometry::convexHull() const
{
  if ( !d || !d->geometry )
  {
    return 0;
  }
  QgsGeos geos( d->geometry );
  QgsAbstractGeometryV2* cHull = geos.convexHull();
  if ( !cHull )
  {
    return 0;
  }
  return new QgsGeometry( cHull );
}

QgsGeometry* QgsGeometry::interpolate( double distance )
{
  if ( !d || !d->geometry )
  {
    return 0;
  }
  QgsGeos geos( d->geometry );
  QgsAbstractGeometryV2* result = geos.interpolate( distance );
  if ( !result )
  {
    return 0;
  }
  return new QgsGeometry( result );
}

QgsGeometry* QgsGeometry::intersection( const QgsGeometry* geometry ) const
{
  if ( !d || !d->geometry || !geometry->d || !geometry->d->geometry )
  {
    return 0;
  }

  QgsGeos geos( d->geometry );

  QgsAbstractGeometryV2* resultGeom = geos.intersection( *( geometry->d->geometry ) );
  return new QgsGeometry( resultGeom );
}

QgsGeometry* QgsGeometry::combine( const QgsGeometry* geometry ) const
{
  if ( !d || !d->geometry || !geometry->d || !geometry->d->geometry )
  {
    return 0;
  }

  QgsGeos geos( d->geometry );

  QgsAbstractGeometryV2* resultGeom = geos.combine( *( geometry->d->geometry ) );
  if ( !resultGeom )
  {
    return 0;
  }
  return new QgsGeometry( resultGeom );
}

QgsGeometry* QgsGeometry::difference( const QgsGeometry* geometry ) const
{
  if ( !d || !d->geometry || !geometry->d || !geometry->d->geometry )
  {
    return 0;
  }

  QgsGeos geos( d->geometry );

  QgsAbstractGeometryV2* resultGeom = geos.difference( *( geometry->d->geometry ) );
  if ( !resultGeom )
  {
    return 0;
  }
  return new QgsGeometry( resultGeom );
}

QgsGeometry* QgsGeometry::symDifference( const QgsGeometry* geometry ) const
{
  if ( !d || !d->geometry || !geometry->d || !geometry->d->geometry )
  {
    return 0;
  }

  QgsGeos geos( d->geometry );

  QgsAbstractGeometryV2* resultGeom = geos.symDifference( *( geometry->d->geometry ) );
  if ( !resultGeom )
  {
    return 0;
  }
  return new QgsGeometry( resultGeom );
}

QList<QgsGeometry*> QgsGeometry::asGeometryCollection() const
{
  return QList<QgsGeometry*>(); //todo...

#if 0
  if ( mDirtyGeos )
    exportWkbToGeos();

  if ( !mGeos )
    return QList<QgsGeometry*>();

  int type = GEOSGeomTypeId( mGeos );
  QgsDebugMsg( "geom type: " + QString::number( type ) );

  QList<QgsGeometry*> geomCollection;

  if ( type != GEOS_MULTIPOINT &&
       type != GEOS_MULTILINESTRING &&
       type != GEOS_MULTIPOLYGON &&
       type != GEOS_GEOMETRYCOLLECTION )
  {
    // we have a single-part geometry - put there a copy of this one
    geomCollection.append( new QgsGeometry( *this ) );
    return geomCollection;
  }

  int count = GEOSGetNumGeometries( mGeos );
  QgsDebugMsg( "geom count: " + QString::number( count ) );

  for ( int i = 0; i < count; ++i )
  {
    const GEOSGeometry * geometry = GEOSGetGeometryN( mGeos, i );
    geomCollection.append( fromGeosGeom( GEOSGeom_clone( geometry ) ) );
  }

  return geomCollection;
#endif //0
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
  if ( !d || !d->geometry )
  {
    return false;
  }

  return QgsGeometryEditUtils::deleteRing( d->geometry, ringNum, partNum );
}

bool QgsGeometry::deletePart( int partNum )
{
  if ( !d || !d->geometry )
  {
    return false;
  }

  return QgsGeometryEditUtils::deletePart( d->geometry, partNum );
}

int QgsGeometry::avoidIntersections( QMap<QgsVectorLayer*, QSet< QgsFeatureId > > ignoreFeatures )
{
  if ( !d || !d->geometry )
  {
    return 1;
  }

  QgsAbstractGeometryV2* diffGeom = QgsGeometryEditUtils::avoidIntersections( *( d->geometry ), ignoreFeatures );
  if ( diffGeom )
  {
    delete d->geometry;
    d->geometry = diffGeom;
  }
  return 0;
}

void QgsGeometry::validateGeometry( QList<Error> &errors )
{
//todo // QgsGeometryValidator::validateGeometry( this, errors );
}

bool QgsGeometry::isGeosValid() const
{
  if ( !d || !d->geometry )
  {
    return false;
  }

  QgsGeos geos( d->geometry );
  return geos.isValid();
}

bool QgsGeometry::isGeosEqual( const QgsGeometry& g ) const
{
  if ( !d || !d->geometry || !g.d || !g.d->geometry )
  {
    return false;
  }

  QgsGeos geos( d->geometry );
  return geos.isEqual( *( g.d->geometry ) );
}

bool QgsGeometry::isGeosEmpty() const
{
  if ( !d || !d->geometry )
  {
    return false;
  }

  QgsGeos geos( d->geometry );
  return geos.isEmpty();
}

QgsGeometry *QgsGeometry::unaryUnion( const QList<QgsGeometry*> &geometryList )
{
  QgsGeos geos( 0 );

  QList<const QgsAbstractGeometryV2*> geomV2List;
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
  if ( !d || !d->geometry )
  {
    return;
  }

  detach();
  QgsWKBTypes::Type flatGeomType = QgsWKBTypes::flatType( d->geometry->wkbType() );
  if ( flatGeomType == QgsWKBTypes::CompoundCurve || flatGeomType == QgsWKBTypes::CircularString )
  {
    QgsCurveV2* curve = dynamic_cast<QgsCurveV2*>( d->geometry );
    if ( !curve )
    {
      return ;
    }
    d->geometry = curve->curveToLine();
    delete curve;
  }
  else if ( flatGeomType == QgsWKBTypes::CurvePolygon )
  {
    QgsCurvePolygonV2* curvePolygon = dynamic_cast<QgsCurvePolygonV2*>( d->geometry );
    if ( !curvePolygon )
    {
      return;
    }
    d->geometry = curvePolygon->toPolygon();
    delete curvePolygon;
  }
  else //no segmentation needed
  {
    return;
  }

  //compoundcurve / circularstring /multicurve ?

  //curve polygon / multisurface?
  delete[] mWkb;
  mWkb = 0;
  mWkbSize = 0;
  GEOSGeom_destroy( mGeos );
  mGeos = 0;
}

int QgsGeometry::transform( const QgsCoordinateTransform& ct )
{
  if ( !d || !d->geometry )
  {
    return 1;
  }

  detach();
  d->geometry->transform( ct );
  return 0;
}

void QgsGeometry::mapToPixel( const QgsMapToPixel& mtp )
{
  if ( d && d->geometry )
  {
    detach();
    d->geometry->transform( mtp.transform() );
  }
}

void QgsGeometry::clip( const QgsRectangle& rect )
{
  if ( d && d->geometry )
  {
    detach();
    d->geometry->clip( rect );
  }
}

void QgsGeometry::draw( QPainter& p ) const
{
  if ( d && d->geometry )
  {
    d->geometry->draw( p );
  }
}

bool QgsGeometry::vertexIdFromVertexNr( int nr, QgsVertexId& id ) const
{
  if ( !d || !d->geometry )
  {
    return false;
  }

  QList< QList< QList< QgsPointV2 > > > coords;
  d->geometry->coordinateSequence( coords );

  int vertexCount = 0;
  for ( int part = 0; part < coords.size(); ++part )
  {
    const QList< QList< QgsPointV2 > >& featureCoords = coords.at( part );
    for ( int ring = 0; ring < featureCoords.size(); ++ring )
    {
      const QList< QgsPointV2 >& ringCoords = featureCoords.at( ring );
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

int QgsGeometry::vertexNrFromVertexId( const QgsVertexId& id ) const
{
  if ( !d || !d->geometry )
  {
    return false;
  }

  QList< QList< QList< QgsPointV2 > > > coords;
  d->geometry->coordinateSequence( coords );

  int vertexCount = 0;
  for ( int part = 0; part < coords.size(); ++part )
  {
    const QList< QList< QgsPointV2 > >& featureCoords = coords.at( part );
    for ( int ring = 0; ring < featureCoords.size(); ++ring )
    {
      const QList< QgsPointV2 >& ringCoords = featureCoords.at( ring );
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

void QgsGeometry::convertPointList( const QList<QgsPoint>& input, QList<QgsPointV2>& output )
{
  output.clear();
  QList<QgsPoint>::const_iterator it = input.constBegin();
  for ( ; it != input.constEnd(); ++it )
  {
    output.append( QgsPointV2( it->x(), it->y() ) );
  }
}

void QgsGeometry::convertPointList( const QList<QgsPointV2>& input, QList<QgsPoint>& output )
{
  output.clear();
  QList<QgsPointV2>::const_iterator it = input.constBegin();
  for ( ; it != input.constEnd(); ++it )
  {
    output.append( QgsPoint( it->x(), it->y() ) );
  }
}

void QgsGeometry::convertToPolyline( const QList<QgsPointV2>& input, QgsPolyline& output )
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
  QList< QList< QList< QgsPointV2 > > > coord;
  input.coordinateSequence( coord );
  if ( coord.size() < 1 )
  {
    return;
  }

  const QList< QList< QgsPointV2 > >& rings = coord[0];
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

QgsGeometry *QgsGeometry::fromQPointF( const QPointF &point )
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
    break;

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



