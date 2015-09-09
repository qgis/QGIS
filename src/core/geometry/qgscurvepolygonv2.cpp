/***************************************************************************
                         qgscurvepolygonv2.cpp
                         ---------------------
    begin                : September 2014
    copyright            : (C) 2014 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscurvepolygonv2.h"
#include "qgsapplication.h"
#include "qgscircularstringv2.h"
#include "qgscompoundcurvev2.h"
#include "qgsgeometryutils.h"
#include "qgslinestringv2.h"
#include "qgspolygonv2.h"
#include "qgswkbptr.h"
#include <QPainter>
#include <QPainterPath>

QgsCurvePolygonV2::QgsCurvePolygonV2(): QgsSurfaceV2(), mExteriorRing( 0 )
{

}

QgsCurvePolygonV2::~QgsCurvePolygonV2()
{
  clear();
}

QgsCurvePolygonV2::QgsCurvePolygonV2( const QgsCurvePolygonV2& p ) : QgsSurfaceV2( p ), mExteriorRing( 0 )
{
  if ( p.mExteriorRing )
  {
    mExteriorRing = static_cast<QgsCurveV2*>( p.mExteriorRing->clone() );
  }

  Q_FOREACH ( const QgsCurveV2* ring, p.mInteriorRings )
  {
    mInteriorRings.push_back( static_cast<QgsCurveV2*>( ring->clone() ) );
  }
}

QgsCurvePolygonV2& QgsCurvePolygonV2::operator=( const QgsCurvePolygonV2 & p )
{
  if ( &p != this )
  {
    QgsSurfaceV2::operator=( p );
    if ( p.mExteriorRing )
    {
      mExteriorRing = static_cast<QgsCurveV2*>( p.mExteriorRing->clone() );
    }

    Q_FOREACH ( const QgsCurveV2* ring, p.mInteriorRings )
    {
      mInteriorRings.push_back( static_cast<QgsCurveV2*>( ring->clone() ) );
    }
  }
  return *this;
}

QgsAbstractGeometryV2* QgsCurvePolygonV2::clone() const
{
  return new QgsCurvePolygonV2( *this );
}

void QgsCurvePolygonV2::clear()
{
  delete mExteriorRing;
  mExteriorRing = 0;
  qDeleteAll( mInteriorRings );
  mInteriorRings.clear();
  mWkbType = QgsWKBTypes::Unknown;
}


bool QgsCurvePolygonV2::fromWkb( const unsigned char* wkb )
{
  clear();
  if ( !wkb )
  {
    return false;
  }
  QgsConstWkbPtr wkbPtr( wkb );
  QgsWKBTypes::Type type = wkbPtr.readHeader();
  if ( QgsWKBTypes::flatType( type ) != QgsWKBTypes::CurvePolygon )
  {
    return false;
  }
  mWkbType = type;

  int nRings;
  wkbPtr >> nRings;
  QgsCurveV2* currentCurve = 0;
  int currentCurveSize = 0;
  for ( int i = 0; i < nRings; ++i )
  {
    wkbPtr += 1; //skip endian
    QgsWKBTypes::Type curveType;
    wkbPtr >> curveType;
    wkbPtr -= ( 1 + sizeof( int ) );
    if ( curveType == QgsWKBTypes::LineString || curveType == QgsWKBTypes::LineStringZ || curveType == QgsWKBTypes::LineStringM ||
         curveType == QgsWKBTypes::LineStringZM || curveType == QgsWKBTypes::LineString25D )
    {
      currentCurve = new QgsLineStringV2();
    }
    else if ( curveType == QgsWKBTypes::CircularString || curveType == QgsWKBTypes::CircularStringZ || curveType == QgsWKBTypes::CircularStringZM ||
              curveType == QgsWKBTypes::CircularStringM )
    {
      currentCurve = new QgsCircularStringV2();
    }
    else if ( curveType == QgsWKBTypes::CompoundCurve || curveType == QgsWKBTypes::CompoundCurveZ || curveType == QgsWKBTypes::CompoundCurveZM )
    {
      currentCurve = new QgsCompoundCurveV2();
    }
    else
    {
      return false;
    }
    currentCurve->fromWkb( wkbPtr );
    currentCurveSize = currentCurve->wkbSize();
    if ( i == 0 )
    {
      mExteriorRing = currentCurve;
    }
    else
    {
      mInteriorRings.append( currentCurve );
    }
    wkbPtr += currentCurveSize;
  }

  return true;
}

bool QgsCurvePolygonV2::fromWkt( const QString& wkt )
{
  clear();

  QPair<QgsWKBTypes::Type, QString> parts = QgsGeometryUtils::wktReadBlock( wkt );

  if ( QgsWKBTypes::flatType( parts.first ) != QgsWKBTypes::parseType( geometryType() ) )
    return false;
  mWkbType = parts.first;

  QString defaultChildWkbType = QString( "LineString%1%2" ).arg( is3D() ? "Z" : "" ).arg( isMeasure() ? "M" : "" );

  Q_FOREACH ( const QString& childWkt, QgsGeometryUtils::wktGetChildBlocks( parts.second, defaultChildWkbType ) )
  {
    QPair<QgsWKBTypes::Type, QString> childParts = QgsGeometryUtils::wktReadBlock( childWkt );

    if ( QgsWKBTypes::flatType( childParts.first ) == QgsWKBTypes::LineString )
      mInteriorRings.append( new QgsLineStringV2() );
    else if ( QgsWKBTypes::flatType( childParts.first ) == QgsWKBTypes::CircularString )
      mInteriorRings.append( new QgsCircularStringV2() );
    else
    {
      clear();
      return false;
    }
    if ( !mInteriorRings.back()->fromWkt( childWkt ) )
    {
      clear();
      return false;
    }
  }

  mExteriorRing = mInteriorRings.first();
  mInteriorRings.removeFirst();

  return true;
}

QgsRectangle QgsCurvePolygonV2::calculateBoundingBox() const
{
  if ( mExteriorRing )
  {
    return mExteriorRing->calculateBoundingBox();
  }
  return QgsRectangle();
}

int QgsCurvePolygonV2::wkbSize() const
{
  int size = sizeof( char ) + sizeof( quint32 ) + sizeof( quint32 );
  if ( mExteriorRing )
  {
    size += mExteriorRing->wkbSize();
  }
  Q_FOREACH ( const QgsCurveV2* curve, mInteriorRings )
  {
    size += curve->wkbSize();
  }
  return size;
}

unsigned char* QgsCurvePolygonV2::asWkb( int& binarySize ) const
{
  binarySize = wkbSize();
  unsigned char* geomPtr = new unsigned char[binarySize];
  QgsWkbPtr wkb( geomPtr );
  wkb << static_cast<char>( QgsApplication::endian() );
  wkb << static_cast<quint32>( wkbType() );
  wkb << static_cast<quint32>(( mExteriorRing != 0 ) + mInteriorRings.size() );
  if ( mExteriorRing )
  {
    int curveWkbLen = 0;
    unsigned char* ringWkb = mExteriorRing->asWkb( curveWkbLen );
    memcpy( wkb, ringWkb, curveWkbLen );
    wkb += curveWkbLen;
  }
  Q_FOREACH ( const QgsCurveV2* curve, mInteriorRings )
  {
    int curveWkbLen = 0;
    unsigned char* ringWkb = curve->asWkb( curveWkbLen );
    memcpy( wkb, ringWkb, curveWkbLen );
    wkb += curveWkbLen;
  }
  return geomPtr;
}

QString QgsCurvePolygonV2::asWkt( int precision ) const
{
  QString wkt = wktTypeStr() + " (";
  if ( mExteriorRing )
  {
    QString childWkt = mExteriorRing->asWkt( precision );
    if ( dynamic_cast<QgsLineStringV2*>( mExteriorRing ) )
    {
      // Type names of linear geometries are omitted
      childWkt = childWkt.mid( childWkt.indexOf( "(" ) );
    }
    wkt += childWkt + ",";
  }
  Q_FOREACH ( const QgsCurveV2* curve, mInteriorRings )
  {
    QString childWkt = curve->asWkt( precision );
    if ( dynamic_cast<const QgsLineStringV2*>( curve ) )
    {
      // Type names of linear geometries are omitted
      childWkt = childWkt.mid( childWkt.indexOf( "(" ) );
    }
    wkt += childWkt + ",";
  }
  if ( wkt.endsWith( "," ) )
  {
    wkt.chop( 1 ); // Remove last ","
  }
  wkt += ")";
  return wkt;
}

QDomElement QgsCurvePolygonV2::asGML2( QDomDocument& doc, int precision, const QString& ns ) const
{
  // GML2 does not support curves
  QDomElement elemPolygon = doc.createElementNS( ns, "Polygon" );
  QDomElement elemOuterBoundaryIs = doc.createElementNS( ns, "outerBoundaryIs" );
  QgsLineStringV2* exteriorLineString = exteriorRing()->curveToLine();
  elemOuterBoundaryIs.appendChild( exteriorLineString->asGML2( doc, precision, ns ) );
  delete exteriorLineString;
  elemPolygon.appendChild( elemOuterBoundaryIs );
  QDomElement elemInnerBoundaryIs = doc.createElementNS( ns, "innerBoundaryIs" );
  for ( int i = 0, n = numInteriorRings(); i < n; ++i )
  {
    QgsLineStringV2* interiorLineString = interiorRing( i )->curveToLine();
    elemInnerBoundaryIs.appendChild( interiorLineString->asGML2( doc, precision, ns ) );
    delete interiorLineString;
  }
  elemPolygon.appendChild( elemInnerBoundaryIs );
  return elemPolygon;
}

QDomElement QgsCurvePolygonV2::asGML3( QDomDocument& doc, int precision, const QString& ns ) const
{
  QDomElement elemCurvePolygon = doc.createElementNS( ns, "Polygon" );
  QDomElement elemExterior = doc.createElementNS( ns, "exterior" );
  elemExterior.appendChild( exteriorRing()->asGML2( doc, precision, ns ) );
  elemCurvePolygon.appendChild( elemExterior );
  QDomElement elemInterior = doc.createElementNS( ns, "interior" );
  for ( int i = 0, n = numInteriorRings(); i < n; ++i )
  {
    elemInterior.appendChild( interiorRing( i )->asGML2( doc, precision, ns ) );
  }
  elemCurvePolygon.appendChild( elemInterior );
  return elemCurvePolygon;
}

QString QgsCurvePolygonV2::asJSON( int precision ) const
{
  // GeoJSON does not support curves
  QString json = "{\"type\": \"Polygon\", \"coordinates\": [";

  QgsLineStringV2* exteriorLineString = exteriorRing()->curveToLine();
  QList<QgsPointV2> exteriorPts;
  exteriorLineString->points( exteriorPts );
  json += QgsGeometryUtils::pointsToJSON( exteriorPts, precision ) + ", ";
  delete exteriorLineString;

  for ( int i = 0, n = numInteriorRings(); i < n; ++i )
  {
    QgsLineStringV2* interiorLineString = interiorRing( i )->curveToLine();
    QList<QgsPointV2> interiorPts;
    interiorLineString->points( interiorPts );
    json += QgsGeometryUtils::pointsToJSON( interiorPts, precision ) + ", ";
    delete interiorLineString;
  }
  if ( json.endsWith( ", " ) )
  {
    json.chop( 2 ); // Remove last ", "
  }
  json += "] }";
  return json;
}

double QgsCurvePolygonV2::area() const
{
  if ( !mExteriorRing )
  {
    return 0.0;
  }

  double area = mExteriorRing->area();
  QList<QgsCurveV2*>::const_iterator ringIt = mInteriorRings.constBegin();
  for ( ; ringIt != mInteriorRings.constEnd(); ++ringIt )
  {
    area -= ( *ringIt )->area();
  }
  return area;
}

double QgsCurvePolygonV2::length() const
{
  //sum perimeter of rings
  double length = mExteriorRing->length();
  QList<QgsCurveV2*>::const_iterator ringIt = mInteriorRings.constBegin();
  for ( ; ringIt != mInteriorRings.constEnd(); ++ringIt )
  {
    length += ( *ringIt )->length();
  }
  return length;
}

QgsPointV2 QgsCurvePolygonV2::centroid() const
{
  return QgsPointV2( 0, 0 );
}

QgsPointV2 QgsCurvePolygonV2::pointOnSurface() const
{
  return QgsPointV2( 0, 0 );
}

QgsPolygonV2* QgsCurvePolygonV2::surfaceToPolygon() const
{
  QgsPolygonV2* polygon = new QgsPolygonV2();
  polygon->setExteriorRing( exteriorRing()->curveToLine() );
  QList<QgsCurveV2*> interiors;
  for ( int i = 0, n = numInteriorRings(); i < n; ++i )
  {
    interiors.append( interiorRing( i )->curveToLine() );
  }
  polygon->setInteriorRings( interiors );
  return polygon;
}

QgsPolygonV2* QgsCurvePolygonV2::toPolygon() const
{
  if ( !mExteriorRing )
  {
    return 0;
  }

  QgsPolygonV2* poly = new QgsPolygonV2();
  poly->setExteriorRing( mExteriorRing->curveToLine() );

  QList<QgsCurveV2*> rings;
  QList<QgsCurveV2*>::const_iterator it = mInteriorRings.constBegin();
  for ( ; it != mInteriorRings.constEnd(); ++it )
  {
    rings.push_back(( *it )->curveToLine() );
  }
  poly->setInteriorRings( rings );
  return poly;
}

int QgsCurvePolygonV2::numInteriorRings() const
{
  return mInteriorRings.size();
}

const QgsCurveV2* QgsCurvePolygonV2::exteriorRing() const
{
  return mExteriorRing;
}

const QgsCurveV2* QgsCurvePolygonV2::interiorRing( int i ) const
{
  if ( i >= mInteriorRings.size() )
  {
    return 0;
  }
  return mInteriorRings.at( i );
}

void QgsCurvePolygonV2::setExteriorRing( QgsCurveV2* ring )
{
  if ( !ring )
  {
    return;
  }
  delete mExteriorRing;
  mExteriorRing = ring;

  //set proper wkb type
  if ( geometryType() == "Polygon" )
  {
    setZMTypeFromSubGeometry( ring, QgsWKBTypes::Polygon );
  }
  else if ( geometryType() == "CurvePolygon" )
  {
    setZMTypeFromSubGeometry( ring, QgsWKBTypes::CurvePolygon );
  }
}

void QgsCurvePolygonV2::setInteriorRings( QList<QgsCurveV2*> rings )
{
  qDeleteAll( mInteriorRings );
  mInteriorRings = rings;
}

void QgsCurvePolygonV2::addInteriorRing( QgsCurveV2* ring )
{
  mInteriorRings.append( ring );
}

bool QgsCurvePolygonV2::removeInteriorRing( int nr )
{
  if ( nr >= mInteriorRings.size() )
  {
    return false;
  }
  mInteriorRings.removeAt( nr );
  return true;
}

void QgsCurvePolygonV2::draw( QPainter& p ) const
{
  if ( mInteriorRings.size() < 1 )
  {
    if ( mExteriorRing )
    {
      mExteriorRing->drawAsPolygon( p );
    }
  }
  else
  {
    QPainterPath path;
    mExteriorRing->addToPainterPath( path );

    QList<QgsCurveV2*>::const_iterator it = mInteriorRings.constBegin();
    for ( ; it != mInteriorRings.constEnd(); ++it )
    {
      ( *it )->addToPainterPath( path );
    }
    p.drawPath( path );
  }
}

void QgsCurvePolygonV2::transform( const QgsCoordinateTransform& ct, QgsCoordinateTransform::TransformDirection d )
{
  if ( mExteriorRing )
  {
    mExteriorRing->transform( ct, d );
  }

  QList<QgsCurveV2*>::iterator it = mInteriorRings.begin();
  for ( ; it != mInteriorRings.end(); ++it )
  {
    ( *it )->transform( ct, d );
  }
}

void QgsCurvePolygonV2::transform( const QTransform& t )
{
  if ( mExteriorRing )
  {
    mExteriorRing->transform( t );
  }

  QList<QgsCurveV2*>::iterator it = mInteriorRings.begin();
  for ( ; it != mInteriorRings.end(); ++it )
  {
    ( *it )->transform( t );
  }
}

void QgsCurvePolygonV2::coordinateSequence( QList< QList< QList< QgsPointV2 > > >& coord ) const
{
  coord.clear();

  QList< QList< QgsPointV2 > > coordinates;
  QList< QgsPointV2 > ringCoords;
  if ( mExteriorRing )
  {
    mExteriorRing->points( ringCoords );
    coordinates.append( ringCoords );
  }

  QList<QgsCurveV2*>::const_iterator it = mInteriorRings.constBegin();
  for ( ; it != mInteriorRings.constEnd(); ++it )
  {
    ( *it )->points( ringCoords );
    coordinates.append( ringCoords );
  }
  coord.append( coordinates );
}

double QgsCurvePolygonV2::closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const
{
  if ( !mExteriorRing )
  {
    return 0.0;
  }
  QList<QgsCurveV2*> segmentList;
  segmentList.append( mExteriorRing );
  segmentList.append( mInteriorRings );
  return QgsGeometryUtils::closestSegmentFromComponents( segmentList, QgsGeometryUtils::RING, pt, segmentPt,  vertexAfter, leftOf, epsilon );
}

bool QgsCurvePolygonV2::nextVertex( QgsVertexId& vId, QgsPointV2& vertex ) const
{
  if ( !mExteriorRing || vId.ring >= 1 + mInteriorRings.size() )
  {
    return false;
  }

  if ( vId.ring < 0 )
  {
    vId.ring = 0; vId.vertex = -1;
    if ( vId.part < 0 )
    {
      vId.part = 0;
    }
    return mExteriorRing->nextVertex( vId, vertex );
  }
  else
  {
    QgsCurveV2* ring = vId.ring == 0 ? mExteriorRing : mInteriorRings[vId.ring - 1];

    if ( ring->nextVertex( vId, vertex ) )
    {
      return true;
    }
    ++vId.ring;
    vId.vertex = -1;
    if ( vId.ring >= 1 + mInteriorRings.size() )
    {
      return false;
    }
    ring = mInteriorRings[ vId.ring - 1 ];
    return ring->nextVertex( vId, vertex );
  }
}

bool QgsCurvePolygonV2::insertVertex( const QgsVertexId& vId, const QgsPointV2& vertex )
{
  if ( !mExteriorRing || vId.ring < 0 || vId.ring >= 1 + mInteriorRings.size() )
  {
    return false;
  }

  QgsCurveV2* ring = vId.ring == 0 ? mExteriorRing : mInteriorRings[vId.ring - 1];
  int n = ring->numPoints();
  bool success = ring->insertVertex( QgsVertexId( 0, 0, vId.vertex ), vertex );
  if ( !success )
  {
    return false;
  }

  // If first or last vertex is inserted, re-sync the last/first vertex
  if ( vId.vertex == 0 )
    ring->moveVertex( QgsVertexId( 0, 0, n ), vertex );
  else if ( vId.vertex == n )
    ring->moveVertex( QgsVertexId( 0, 0, 0 ), vertex );

  mBoundingBox = QgsRectangle();
  return true;
}

bool QgsCurvePolygonV2::moveVertex( const QgsVertexId& vId, const QgsPointV2& newPos )
{
  if ( !mExteriorRing || vId.ring < 0 || vId.ring >= 1 + mInteriorRings.size() )
  {
    return false;
  }

  QgsCurveV2* ring = vId.ring == 0 ? mExteriorRing : mInteriorRings[vId.ring - 1];
  int n = ring->numPoints();
  bool success = ring->moveVertex( vId, newPos );
  if ( success )
  {
    // If first or last vertex is moved, also move the last/first vertex
    if ( vId.vertex == 0 )
      ring->moveVertex( QgsVertexId( vId.part, vId.ring, n - 1 ), newPos );
    else if ( vId.vertex == n - 1 )
      ring->moveVertex( QgsVertexId( vId.part, vId.ring, 0 ), newPos );
    mBoundingBox = QgsRectangle();
  }
  return success;
}

bool QgsCurvePolygonV2::deleteVertex( const QgsVertexId& vId )
{
  if ( !mExteriorRing || vId.ring < 0 || vId.ring >= 1 + mInteriorRings.size() )
  {
    return false;
  }

  QgsCurveV2* ring = vId.ring == 0 ? mExteriorRing : mInteriorRings[vId.ring - 1];
  int n = ring->numPoints();
  if ( n <= 4 )
  {
    return false;
  }
  bool success = ring->deleteVertex( vId );
  if ( success )
  {
    // If first or last vertex is removed, re-sync the last/first vertex
    if ( vId.vertex == 0 )
      ring->moveVertex( QgsVertexId( 0, 0, n - 2 ), ring->vertexAt( QgsVertexId( 0, 0, 0 ) ) );
    else if ( vId.vertex == n - 1 )
      ring->moveVertex( QgsVertexId( 0, 0, 0 ), ring->vertexAt( QgsVertexId( 0, 0, n - 2 ) ) );
    mBoundingBox = QgsRectangle();
  }
  return success;
}

bool QgsCurvePolygonV2::hasCurvedSegments() const
{
  if ( mExteriorRing && mExteriorRing->hasCurvedSegments() )
  {
    return true;
  }

  QList<QgsCurveV2*>::const_iterator it = mInteriorRings.constBegin();
  for ( ; it != mInteriorRings.constEnd(); ++it )
  {
    if (( *it )->hasCurvedSegments() )
    {
      return true;
    }
  }
  return false;
}

QgsAbstractGeometryV2* QgsCurvePolygonV2::segmentize() const
{
  return toPolygon();
}

double QgsCurvePolygonV2::vertexAngle( const QgsVertexId& vertex ) const
{
  if ( !mExteriorRing || vertex.ring < 0 || vertex.ring >= 1 + mInteriorRings.size() )
  {
    return false;
  }

  QgsCurveV2* ring = vertex.ring == 0 ? mExteriorRing : mInteriorRings[vertex.ring - 1];
  return ring->vertexAngle( vertex );
}
