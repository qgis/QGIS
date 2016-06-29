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

QgsCurvePolygonV2::QgsCurvePolygonV2(): QgsSurfaceV2(), mExteriorRing( nullptr )
{
  mWkbType = QgsWKBTypes::CurvePolygon;
}

QgsCurvePolygonV2::~QgsCurvePolygonV2()
{
  clear();
}

QgsCurvePolygonV2::QgsCurvePolygonV2( const QgsCurvePolygonV2& p )
    : QgsSurfaceV2( p )
    , mExteriorRing( nullptr )
{
  mWkbType = p.mWkbType;
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
    clearCache();
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

QgsCurvePolygonV2* QgsCurvePolygonV2::clone() const
{
  return new QgsCurvePolygonV2( *this );
}

void QgsCurvePolygonV2::clear()
{
  mWkbType = QgsWKBTypes::CurvePolygon;
  delete mExteriorRing;
  mExteriorRing = nullptr;
  qDeleteAll( mInteriorRings );
  mInteriorRings.clear();
  clearCache();
}


bool QgsCurvePolygonV2::fromWkb( QgsConstWkbPtr wkbPtr )
{
  clear();
  if ( !wkbPtr )
  {
    return false;
  }

  QgsWKBTypes::Type type = wkbPtr.readHeader();
  if ( QgsWKBTypes::flatType( type ) != QgsWKBTypes::CurvePolygon )
  {
    return false;
  }
  mWkbType = type;

  int nRings;
  wkbPtr >> nRings;
  QgsCurveV2* currentCurve = nullptr;
  int currentCurveSize = 0;
  for ( int i = 0; i < nRings; ++i )
  {
    QgsWKBTypes::Type curveType = wkbPtr.readHeader();
    wkbPtr -= 1 + sizeof( int );
    QgsWKBTypes::Type flatCurveType = QgsWKBTypes::flatType( curveType );
    if ( flatCurveType == QgsWKBTypes::LineString )
    {
      currentCurve = new QgsLineStringV2();
    }
    else if ( flatCurveType == QgsWKBTypes::CircularString )
    {
      currentCurve = new QgsCircularStringV2();
    }
    else if ( flatCurveType == QgsWKBTypes::CompoundCurve )
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

  if ( QgsWKBTypes::geometryType( parts.first ) != QgsWKBTypes::PolygonGeometry )
    return false;

  mWkbType = parts.first;

  QString defaultChildWkbType = QString( "LineString%1%2" ).arg( is3D() ? "Z" : "", isMeasure() ? "M" : "" );

  Q_FOREACH ( const QString& childWkt, QgsGeometryUtils::wktGetChildBlocks( parts.second, defaultChildWkbType ) )
  {
    QPair<QgsWKBTypes::Type, QString> childParts = QgsGeometryUtils::wktReadBlock( childWkt );

    QgsWKBTypes::Type flatCurveType = QgsWKBTypes::flatType( childParts.first );
    if ( flatCurveType == QgsWKBTypes::LineString )
      mInteriorRings.append( new QgsLineStringV2() );
    else if ( flatCurveType == QgsWKBTypes::CircularString )
      mInteriorRings.append( new QgsCircularStringV2() );
    else if ( flatCurveType == QgsWKBTypes::CompoundCurve )
      mInteriorRings.append( new QgsCompoundCurveV2() );
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

  if ( mInteriorRings.isEmpty() )
  {
    clear();
    return false;
  }

  mExteriorRing = mInteriorRings.at( 0 );
  mInteriorRings.removeFirst();

  //scan through rings and check if dimensionality of rings is different to CurvePolygon.
  //if so, update the type dimensionality of the CurvePolygon to match
  bool hasZ = false;
  bool hasM = false;
  if ( mExteriorRing )
  {
    hasZ = hasZ || mExteriorRing->is3D();
    hasM = hasM || mExteriorRing->isMeasure();
  }
  Q_FOREACH ( const QgsCurveV2* curve, mInteriorRings )
  {
    hasZ = hasZ || curve->is3D();
    hasM = hasM || curve->isMeasure();
    if ( hasZ && hasM )
      break;
  }
  if ( hasZ )
    addZValue( 0 );
  if ( hasM )
    addMValue( 0 );

  return true;
}

QgsRectangle QgsCurvePolygonV2::calculateBoundingBox() const
{
  if ( mExteriorRing )
  {
    return mExteriorRing->boundingBox();
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
  QgsWkbPtr wkbPtr( geomPtr, binarySize );
  wkbPtr << static_cast<char>( QgsApplication::endian() );
  wkbPtr << static_cast<quint32>( wkbType() );
  wkbPtr << static_cast<quint32>(( nullptr != mExteriorRing ) + mInteriorRings.size() );
  if ( mExteriorRing )
  {
    int curveWkbLen = 0;
    unsigned char *ringWkb = mExteriorRing->asWkb( curveWkbLen );
    memcpy( wkbPtr, ringWkb, curveWkbLen );
    wkbPtr += curveWkbLen;
  }
  Q_FOREACH ( const QgsCurveV2* curve, mInteriorRings )
  {
    int curveWkbLen = 0;
    unsigned char *ringWkb = curve->asWkb( curveWkbLen );
    memcpy( wkbPtr, ringWkb, curveWkbLen );
    wkbPtr += curveWkbLen;
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
      childWkt = childWkt.mid( childWkt.indexOf( '(' ) );
    }
    wkt += childWkt + ',';
  }
  Q_FOREACH ( const QgsCurveV2* curve, mInteriorRings )
  {
    QString childWkt = curve->asWkt( precision );
    if ( dynamic_cast<const QgsLineStringV2*>( curve ) )
    {
      // Type names of linear geometries are omitted
      childWkt = childWkt.mid( childWkt.indexOf( '(' ) );
    }
    wkt += childWkt + ',';
  }
  if ( wkt.endsWith( ',' ) )
  {
    wkt.chop( 1 ); // Remove last ','
  }
  wkt += ')';
  return wkt;
}

QDomElement QgsCurvePolygonV2::asGML2( QDomDocument& doc, int precision, const QString& ns ) const
{
  // GML2 does not support curves
  QDomElement elemPolygon = doc.createElementNS( ns, "Polygon" );
  QDomElement elemOuterBoundaryIs = doc.createElementNS( ns, "outerBoundaryIs" );
  QgsLineStringV2* exteriorLineString = exteriorRing()->curveToLine();
  QDomElement outerRing = exteriorLineString->asGML2( doc, precision, ns );
  outerRing.toElement().setTagName( "LinearRing" );
  elemOuterBoundaryIs.appendChild( outerRing );
  delete exteriorLineString;
  elemPolygon.appendChild( elemOuterBoundaryIs );
  QDomElement elemInnerBoundaryIs = doc.createElementNS( ns, "innerBoundaryIs" );
  for ( int i = 0, n = numInteriorRings(); i < n; ++i )
  {
    QgsLineStringV2* interiorLineString = interiorRing( i )->curveToLine();
    QDomElement innerRing = interiorLineString->asGML2( doc, precision, ns );
    innerRing.toElement().setTagName( "LinearRing" );
    elemInnerBoundaryIs.appendChild( innerRing );
    delete interiorLineString;
  }
  elemPolygon.appendChild( elemInnerBoundaryIs );
  return elemPolygon;
}

QDomElement QgsCurvePolygonV2::asGML3( QDomDocument& doc, int precision, const QString& ns ) const
{
  QDomElement elemCurvePolygon = doc.createElementNS( ns, "Polygon" );
  QDomElement elemExterior = doc.createElementNS( ns, "exterior" );
  QDomElement outerRing = exteriorRing()->asGML2( doc, precision, ns );
  outerRing.toElement().setTagName( "LinearRing" );
  elemExterior.appendChild( outerRing );
  elemCurvePolygon.appendChild( elemExterior );
  QDomElement elemInterior = doc.createElementNS( ns, "interior" );
  for ( int i = 0, n = numInteriorRings(); i < n; ++i )
  {
    QDomElement innerRing = interiorRing( i )->asGML2( doc, precision, ns );
    innerRing.toElement().setTagName( "LinearRing" );
    elemInterior.appendChild( innerRing );
  }
  elemCurvePolygon.appendChild( elemInterior );
  return elemCurvePolygon;
}

QString QgsCurvePolygonV2::asJSON( int precision ) const
{
  // GeoJSON does not support curves
  QString json = "{\"type\": \"Polygon\", \"coordinates\": [";

  QgsLineStringV2* exteriorLineString = exteriorRing()->curveToLine();
  QgsPointSequenceV2 exteriorPts;
  exteriorLineString->points( exteriorPts );
  json += QgsGeometryUtils::pointsToJSON( exteriorPts, precision ) + ", ";
  delete exteriorLineString;

  for ( int i = 0, n = numInteriorRings(); i < n; ++i )
  {
    QgsLineStringV2* interiorLineString = interiorRing( i )->curveToLine();
    QgsPointSequenceV2 interiorPts;
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

  double totalArea = 0.0;

  if ( mExteriorRing->isClosed() )
  {
    double area = 0.0;
    mExteriorRing->sumUpArea( area );
    totalArea += qAbs( area );
  }

  QList<QgsCurveV2*>::const_iterator ringIt = mInteriorRings.constBegin();
  for ( ; ringIt != mInteriorRings.constEnd(); ++ringIt )
  {
    double area = 0.0;
    if (( *ringIt )->isClosed() )
    {
      ( *ringIt )->sumUpArea( area );
      totalArea -= qAbs( area );
    }
  }
  return totalArea;
}

double QgsCurvePolygonV2::perimeter() const
{
  if ( !mExteriorRing )
    return 0.0;

  //sum perimeter of rings
  double perimeter = mExteriorRing->length();
  QList<QgsCurveV2*>::const_iterator ringIt = mInteriorRings.constBegin();
  for ( ; ringIt != mInteriorRings.constEnd(); ++ringIt )
  {
    perimeter += ( *ringIt )->length();
  }
  return perimeter;
}

QgsPolygonV2* QgsCurvePolygonV2::surfaceToPolygon() const
{
  QgsPolygonV2* polygon = new QgsPolygonV2();
  polygon->setExteriorRing( exteriorRing()->curveToLine() );
  QList<QgsCurveV2*> interiors;
  int n = numInteriorRings();
  interiors.reserve( n );
  for ( int i = 0; i < n; ++i )
  {
    interiors.append( interiorRing( i )->curveToLine() );
  }
  polygon->setInteriorRings( interiors );
  return polygon;
}

QgsPolygonV2* QgsCurvePolygonV2::toPolygon( double tolerance, SegmentationToleranceType toleranceType ) const
{
  if ( !mExteriorRing )
  {
    return nullptr;
  }

  QgsPolygonV2* poly = new QgsPolygonV2();
  poly->setExteriorRing( mExteriorRing->curveToLine( tolerance, toleranceType ) );

  QList<QgsCurveV2*> rings;
  QList<QgsCurveV2*>::const_iterator it = mInteriorRings.constBegin();
  for ( ; it != mInteriorRings.constEnd(); ++it )
  {
    rings.push_back(( *it )->curveToLine( tolerance, toleranceType ) );
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
  if ( i < 0 || i >= mInteriorRings.size() )
  {
    return nullptr;
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
  if ( QgsWKBTypes::flatType( wkbType() ) == QgsWKBTypes::Polygon )
  {
    setZMTypeFromSubGeometry( ring, QgsWKBTypes::Polygon );
  }
  else if ( QgsWKBTypes::flatType( wkbType() ) == QgsWKBTypes::CurvePolygon )
  {
    setZMTypeFromSubGeometry( ring, QgsWKBTypes::CurvePolygon );
  }

  //match dimensionality for rings
  Q_FOREACH ( QgsCurveV2* ring, mInteriorRings )
  {
    if ( is3D() )
      ring->addZValue();
    else
      ring->dropZValue();

    if ( isMeasure() )
      ring->addMValue();
    else
      ring->dropMValue();
  }
  clearCache();
}

void QgsCurvePolygonV2::setInteriorRings( const QList<QgsCurveV2*>& rings )
{
  qDeleteAll( mInteriorRings );
  mInteriorRings.clear();

  //add rings one-by-one, so that they can each be converted to the correct type for the CurvePolygon
  Q_FOREACH ( QgsCurveV2* ring, rings )
  {
    addInteriorRing( ring );
  }
  clearCache();
}

void QgsCurvePolygonV2::addInteriorRing( QgsCurveV2* ring )
{
  if ( !ring )
    return;

  //ensure dimensionality of ring matches curve polygon
  if ( !is3D() )
    ring->dropZValue();
  else if ( !ring->is3D() )
    ring->addZValue();

  if ( !isMeasure() )
    ring->dropMValue();
  else if ( !ring->isMeasure() )
    ring->addMValue();

  mInteriorRings.append( ring );
  clearCache();
}

bool QgsCurvePolygonV2::removeInteriorRing( int nr )
{
  if ( nr < 0 || nr >= mInteriorRings.size() )
  {
    return false;
  }
  delete mInteriorRings.takeAt( nr );
  clearCache();
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

void QgsCurvePolygonV2::transform( const QgsCoordinateTransform& ct, QgsCoordinateTransform::TransformDirection d, bool transformZ )
{
  if ( mExteriorRing )
  {
    mExteriorRing->transform( ct, d, transformZ );
  }

  Q_FOREACH ( QgsCurveV2* curve, mInteriorRings )
  {
    curve->transform( ct, d, transformZ );
  }
  clearCache();
}

void QgsCurvePolygonV2::transform( const QTransform& t )
{
  if ( mExteriorRing )
  {
    mExteriorRing->transform( t );
  }

  Q_FOREACH ( QgsCurveV2* curve, mInteriorRings )
  {
    curve->transform( t );
  }
  clearCache();
}

QgsCoordinateSequenceV2 QgsCurvePolygonV2::coordinateSequence() const
{
  if ( !mCoordinateSequence.isEmpty() )
    return mCoordinateSequence;

  mCoordinateSequence.append( QgsRingSequenceV2() );

  if ( mExteriorRing )
  {
    mCoordinateSequence.back().append( QgsPointSequenceV2() );
    mExteriorRing->points( mCoordinateSequence.back().back() );
  }

  QList<QgsCurveV2*>::const_iterator it = mInteriorRings.constBegin();
  for ( ; it != mInteriorRings.constEnd(); ++it )
  {
    mCoordinateSequence.back().append( QgsPointSequenceV2() );
    ( *it )->points( mCoordinateSequence.back().back() );
  }

  return mCoordinateSequence;
}

double QgsCurvePolygonV2::closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt, QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const
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
    vId.ring = 0;
    vId.vertex = -1;
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

bool QgsCurvePolygonV2::insertVertex( QgsVertexId vId, const QgsPointV2& vertex )
{
  if ( !mExteriorRing || vId.ring < 0 || vId.ring >= 1 + mInteriorRings.size() )
  {
    return false;
  }

  QgsCurveV2* ring = vId.ring == 0 ? mExteriorRing : mInteriorRings.at( vId.ring - 1 );
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

  clearCache();

  return true;
}

bool QgsCurvePolygonV2::moveVertex( QgsVertexId vId, const QgsPointV2& newPos )
{
  if ( !mExteriorRing || vId.ring < 0 || vId.ring >= 1 + mInteriorRings.size() )
  {
    return false;
  }

  QgsCurveV2* ring = vId.ring == 0 ? mExteriorRing : mInteriorRings.at( vId.ring - 1 );
  int n = ring->numPoints();
  bool success = ring->moveVertex( vId, newPos );
  if ( success )
  {
    // If first or last vertex is moved, also move the last/first vertex
    if ( vId.vertex == 0 )
      ring->moveVertex( QgsVertexId( vId.part, vId.ring, n - 1 ), newPos );
    else if ( vId.vertex == n - 1 )
      ring->moveVertex( QgsVertexId( vId.part, vId.ring, 0 ), newPos );
    clearCache();
  }
  return success;
}

bool QgsCurvePolygonV2::deleteVertex( QgsVertexId vId )
{
  if ( !mExteriorRing || vId.ring < 0 || vId.ring >= 1 + mInteriorRings.size() )
  {
    return false;
  }

  QgsCurveV2* ring = vId.ring == 0 ? mExteriorRing : mInteriorRings.at( vId.ring - 1 );
  int n = ring->numPoints();
  if ( n <= 4 )
  {
    //no points will be left in ring, so remove whole ring
    if ( vId.ring == 0 )
    {
      delete mExteriorRing;
      mExteriorRing = nullptr;
      if ( !mInteriorRings.isEmpty() )
      {
        mExteriorRing = mInteriorRings.takeFirst();
      }
    }
    else
    {
      removeInteriorRing( vId.ring - 1 );
    }
    clearCache();
    return true;
  }

  bool success = ring->deleteVertex( vId );
  if ( success )
  {
    // If first or last vertex is removed, re-sync the last/first vertex
    // Do not use "n - 2", but "ring->numPoints() - 1" as more than one vertex
    // may have been deleted (e.g. with CircularString)
    if ( vId.vertex == 0 )
      ring->moveVertex( QgsVertexId( 0, 0, ring->numPoints() - 1 ), ring->vertexAt( QgsVertexId( 0, 0, 0 ) ) );
    else if ( vId.vertex == n - 1 )
      ring->moveVertex( QgsVertexId( 0, 0, 0 ), ring->vertexAt( QgsVertexId( 0, 0, ring->numPoints() - 1 ) ) );
    clearCache();
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

QgsAbstractGeometryV2* QgsCurvePolygonV2::segmentize( double tolerance, SegmentationToleranceType toleranceType ) const
{
  return toPolygon( tolerance, toleranceType );
}

double QgsCurvePolygonV2::vertexAngle( QgsVertexId vertex ) const
{
  if ( !mExteriorRing || vertex.ring < 0 || vertex.ring >= 1 + mInteriorRings.size() )
  {
    //makes no sense - conversion of false to double!
    return false;
  }

  QgsCurveV2* ring = vertex.ring == 0 ? mExteriorRing : mInteriorRings[vertex.ring - 1];
  return ring->vertexAngle( vertex );
}

int QgsCurvePolygonV2::vertexCount( int /*part*/, int ring ) const
{
  return ring == 0 ? mExteriorRing->vertexCount() : mInteriorRings[ring - 1]->vertexCount();
}

QgsPointV2 QgsCurvePolygonV2::vertexAt( QgsVertexId id ) const
{
  return id.ring == 0 ? mExteriorRing->vertexAt( id ) : mInteriorRings[id.ring - 1]->vertexAt( id );
}

bool QgsCurvePolygonV2::addZValue( double zValue )
{
  if ( QgsWKBTypes::hasZ( mWkbType ) )
    return false;

  mWkbType = QgsWKBTypes::addZ( mWkbType );

  if ( mExteriorRing )
    mExteriorRing->addZValue( zValue );
  Q_FOREACH ( QgsCurveV2* curve, mInteriorRings )
  {
    curve->addZValue( zValue );
  }
  clearCache();
  return true;
}

bool QgsCurvePolygonV2::addMValue( double mValue )
{
  if ( QgsWKBTypes::hasM( mWkbType ) )
    return false;

  mWkbType = QgsWKBTypes::addM( mWkbType );

  if ( mExteriorRing )
    mExteriorRing->addMValue( mValue );
  Q_FOREACH ( QgsCurveV2* curve, mInteriorRings )
  {
    curve->addMValue( mValue );
  }
  clearCache();
  return true;
}

bool QgsCurvePolygonV2::dropZValue()
{
  if ( !is3D() )
    return false;

  mWkbType = QgsWKBTypes::dropZ( mWkbType );
  if ( mExteriorRing )
    mExteriorRing->dropZValue();
  Q_FOREACH ( QgsCurveV2* curve, mInteriorRings )
  {
    curve->dropZValue();
  }
  clearCache();
  return true;
}

bool QgsCurvePolygonV2::dropMValue()
{
  if ( !isMeasure() )
    return false;

  mWkbType = QgsWKBTypes::dropM( mWkbType );
  if ( mExteriorRing )
    mExteriorRing->dropMValue();
  Q_FOREACH ( QgsCurveV2* curve, mInteriorRings )
  {
    curve->dropMValue();
  }
  clearCache();
  return true;
}
