/***************************************************************************
                         qgscurvepolygon.cpp
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

#include "qgscurvepolygon.h"
#include "qgsapplication.h"
#include "qgscircularstring.h"
#include "qgscompoundcurve.h"
#include "qgsgeometryutils.h"
#include "qgslinestring.h"
#include "qgspolygon.h"
#include "qgswkbptr.h"
#include "qgsmulticurve.h"
#include <QPainter>
#include <QPainterPath>

QgsCurvePolygon::QgsCurvePolygon(): QgsSurface(), mExteriorRing( nullptr )
{
  mWkbType = QgsWkbTypes::CurvePolygon;
}

QgsCurvePolygon::~QgsCurvePolygon()
{
  clear();
}

QgsCurvePolygon::QgsCurvePolygon( const QgsCurvePolygon &p )
  : QgsSurface( p )
  , mExteriorRing( nullptr )
{
  mWkbType = p.mWkbType;
  if ( p.mExteriorRing )
  {
    mExteriorRing = static_cast<QgsCurve *>( p.mExteriorRing->clone() );
  }

  Q_FOREACH ( const QgsCurve *ring, p.mInteriorRings )
  {
    mInteriorRings.push_back( static_cast<QgsCurve *>( ring->clone() ) );
  }
}

QgsCurvePolygon &QgsCurvePolygon::operator=( const QgsCurvePolygon &p )
{
  if ( &p != this )
  {
    clearCache();
    QgsSurface::operator=( p );
    if ( p.mExteriorRing )
    {
      mExteriorRing = static_cast<QgsCurve *>( p.mExteriorRing->clone() );
    }

    Q_FOREACH ( const QgsCurve *ring, p.mInteriorRings )
    {
      mInteriorRings.push_back( static_cast<QgsCurve *>( ring->clone() ) );
    }
  }
  return *this;
}

QgsCurvePolygon *QgsCurvePolygon::clone() const
{
  return new QgsCurvePolygon( *this );
}

void QgsCurvePolygon::clear()
{
  mWkbType = QgsWkbTypes::CurvePolygon;
  delete mExteriorRing;
  mExteriorRing = nullptr;
  qDeleteAll( mInteriorRings );
  mInteriorRings.clear();
  clearCache();
}


bool QgsCurvePolygon::fromWkb( QgsConstWkbPtr &wkbPtr )
{
  clear();
  if ( !wkbPtr )
  {
    return false;
  }

  QgsWkbTypes::Type type = wkbPtr.readHeader();
  if ( QgsWkbTypes::flatType( type ) != QgsWkbTypes::CurvePolygon )
  {
    return false;
  }
  mWkbType = type;

  int nRings;
  wkbPtr >> nRings;
  QgsCurve *currentCurve = nullptr;
  for ( int i = 0; i < nRings; ++i )
  {
    QgsWkbTypes::Type curveType = wkbPtr.readHeader();
    wkbPtr -= 1 + sizeof( int );
    QgsWkbTypes::Type flatCurveType = QgsWkbTypes::flatType( curveType );
    if ( flatCurveType == QgsWkbTypes::LineString )
    {
      currentCurve = new QgsLineString();
    }
    else if ( flatCurveType == QgsWkbTypes::CircularString )
    {
      currentCurve = new QgsCircularString();
    }
    else if ( flatCurveType == QgsWkbTypes::CompoundCurve )
    {
      currentCurve = new QgsCompoundCurve();
    }
    else
    {
      return false;
    }
    currentCurve->fromWkb( wkbPtr );  // also updates wkbPtr
    if ( i == 0 )
    {
      mExteriorRing = currentCurve;
    }
    else
    {
      mInteriorRings.append( currentCurve );
    }
  }

  return true;
}

bool QgsCurvePolygon::fromWkt( const QString &wkt )
{
  clear();

  QPair<QgsWkbTypes::Type, QString> parts = QgsGeometryUtils::wktReadBlock( wkt );

  if ( QgsWkbTypes::geometryType( parts.first ) != QgsWkbTypes::PolygonGeometry )
    return false;

  mWkbType = parts.first;

  QString defaultChildWkbType = QStringLiteral( "LineString%1%2" ).arg( is3D() ? "Z" : QString(), isMeasure() ? "M" : QString() );

  Q_FOREACH ( const QString &childWkt, QgsGeometryUtils::wktGetChildBlocks( parts.second, defaultChildWkbType ) )
  {
    QPair<QgsWkbTypes::Type, QString> childParts = QgsGeometryUtils::wktReadBlock( childWkt );

    QgsWkbTypes::Type flatCurveType = QgsWkbTypes::flatType( childParts.first );
    if ( flatCurveType == QgsWkbTypes::LineString )
      mInteriorRings.append( new QgsLineString() );
    else if ( flatCurveType == QgsWkbTypes::CircularString )
      mInteriorRings.append( new QgsCircularString() );
    else if ( flatCurveType == QgsWkbTypes::CompoundCurve )
      mInteriorRings.append( new QgsCompoundCurve() );
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
  Q_FOREACH ( const QgsCurve *curve, mInteriorRings )
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

QgsRectangle QgsCurvePolygon::calculateBoundingBox() const
{
  if ( mExteriorRing )
  {
    return mExteriorRing->boundingBox();
  }
  return QgsRectangle();
}

QByteArray QgsCurvePolygon::asWkb() const
{
  int binarySize = sizeof( char ) + sizeof( quint32 ) + sizeof( quint32 );
  QList<QByteArray> wkbForRings;
  if ( mExteriorRing )
  {
    QByteArray wkb( mExteriorRing->asWkb() );
    binarySize += wkb.length();
    wkbForRings << wkb;
  }
  Q_FOREACH ( const QgsCurve *curve, mInteriorRings )
  {
    QByteArray wkb( curve->asWkb() );
    binarySize += wkb.length();
    wkbForRings << wkb;
  }

  QByteArray wkbArray;
  wkbArray.resize( binarySize );
  QgsWkbPtr wkbPtr( wkbArray );
  wkbPtr << static_cast<char>( QgsApplication::endian() );
  wkbPtr << static_cast<quint32>( wkbType() );
  wkbPtr << static_cast<quint32>( wkbForRings.count() );
  Q_FOREACH ( const QByteArray &wkb, wkbForRings )
  {
    wkbPtr << wkb;
  }
  return wkbArray;
}

QString QgsCurvePolygon::asWkt( int precision ) const
{
  QString wkt = wktTypeStr() + " (";
  if ( mExteriorRing )
  {
    QString childWkt = mExteriorRing->asWkt( precision );
    if ( qgsgeometry_cast<QgsLineString *>( mExteriorRing ) )
    {
      // Type names of linear geometries are omitted
      childWkt = childWkt.mid( childWkt.indexOf( '(' ) );
    }
    wkt += childWkt + ',';
  }
  Q_FOREACH ( const QgsCurve *curve, mInteriorRings )
  {
    QString childWkt = curve->asWkt( precision );
    if ( qgsgeometry_cast<const QgsLineString *>( curve ) )
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

QDomElement QgsCurvePolygon::asGML2( QDomDocument &doc, int precision, const QString &ns ) const
{
  // GML2 does not support curves
  QDomElement elemPolygon = doc.createElementNS( ns, QStringLiteral( "Polygon" ) );
  QDomElement elemOuterBoundaryIs = doc.createElementNS( ns, QStringLiteral( "outerBoundaryIs" ) );
  QgsLineString *exteriorLineString = exteriorRing()->curveToLine();
  QDomElement outerRing = exteriorLineString->asGML2( doc, precision, ns );
  outerRing.toElement().setTagName( QStringLiteral( "LinearRing" ) );
  elemOuterBoundaryIs.appendChild( outerRing );
  delete exteriorLineString;
  elemPolygon.appendChild( elemOuterBoundaryIs );
  for ( int i = 0, n = numInteriorRings(); i < n; ++i )
  {
    QDomElement elemInnerBoundaryIs = doc.createElementNS( ns, QStringLiteral( "innerBoundaryIs" ) );
    QgsLineString *interiorLineString = interiorRing( i )->curveToLine();
    QDomElement innerRing = interiorLineString->asGML2( doc, precision, ns );
    innerRing.toElement().setTagName( QStringLiteral( "LinearRing" ) );
    elemInnerBoundaryIs.appendChild( innerRing );
    delete interiorLineString;
    elemPolygon.appendChild( elemInnerBoundaryIs );
  }
  return elemPolygon;
}

QDomElement QgsCurvePolygon::asGML3( QDomDocument &doc, int precision, const QString &ns ) const
{
  QDomElement elemCurvePolygon = doc.createElementNS( ns, QStringLiteral( "Polygon" ) );
  QDomElement elemExterior = doc.createElementNS( ns, QStringLiteral( "exterior" ) );
  QDomElement curveElem = exteriorRing()->asGML3( doc, precision, ns );
  if ( curveElem.tagName() == "LineString" )
  {
    curveElem.setTagName( QStringLiteral( "LinearRing" ) );
  }
  elemExterior.appendChild( curveElem );
  elemCurvePolygon.appendChild( elemExterior );

  for ( int i = 0, n = numInteriorRings(); i < n; ++i )
  {
    QDomElement elemInterior = doc.createElementNS( ns, QStringLiteral( "interior" ) );
    QDomElement innerRing = interiorRing( i )->asGML3( doc, precision, ns );
    if ( innerRing.tagName() == "LineString" )
    {
      innerRing.setTagName( QStringLiteral( "LinearRing" ) );
    }
    elemInterior.appendChild( innerRing );
    elemCurvePolygon.appendChild( elemInterior );
  }
  return elemCurvePolygon;
}

QString QgsCurvePolygon::asJSON( int precision ) const
{
  // GeoJSON does not support curves
  QString json = QStringLiteral( "{\"type\": \"Polygon\", \"coordinates\": [" );

  QgsLineString *exteriorLineString = exteriorRing()->curveToLine();
  QgsPointSequence exteriorPts;
  exteriorLineString->points( exteriorPts );
  json += QgsGeometryUtils::pointsToJSON( exteriorPts, precision ) + ", ";
  delete exteriorLineString;

  for ( int i = 0, n = numInteriorRings(); i < n; ++i )
  {
    QgsLineString *interiorLineString = interiorRing( i )->curveToLine();
    QgsPointSequence interiorPts;
    interiorLineString->points( interiorPts );
    json += QgsGeometryUtils::pointsToJSON( interiorPts, precision ) + ", ";
    delete interiorLineString;
  }
  if ( json.endsWith( QLatin1String( ", " ) ) )
  {
    json.chop( 2 ); // Remove last ", "
  }
  json += QLatin1String( "] }" );
  return json;
}

double QgsCurvePolygon::area() const
{
  if ( !mExteriorRing )
  {
    return 0.0;
  }

  double totalArea = 0.0;

  if ( mExteriorRing->isRing() )
  {
    double area = 0.0;
    mExteriorRing->sumUpArea( area );
    totalArea += std::fabs( area );
  }

  QList<QgsCurve *>::const_iterator ringIt = mInteriorRings.constBegin();
  for ( ; ringIt != mInteriorRings.constEnd(); ++ringIt )
  {
    double area = 0.0;
    if ( ( *ringIt )->isRing() )
    {
      ( *ringIt )->sumUpArea( area );
      totalArea -= std::fabs( area );
    }
  }
  return totalArea;
}

double QgsCurvePolygon::perimeter() const
{
  if ( !mExteriorRing )
    return 0.0;

  //sum perimeter of rings
  double perimeter = mExteriorRing->length();
  QList<QgsCurve *>::const_iterator ringIt = mInteriorRings.constBegin();
  for ( ; ringIt != mInteriorRings.constEnd(); ++ringIt )
  {
    perimeter += ( *ringIt )->length();
  }
  return perimeter;
}

QgsPolygonV2 *QgsCurvePolygon::surfaceToPolygon() const
{
  QgsPolygonV2 *polygon = new QgsPolygonV2();
  polygon->setExteriorRing( exteriorRing()->curveToLine() );
  QList<QgsCurve *> interiors;
  int n = numInteriorRings();
  interiors.reserve( n );
  for ( int i = 0; i < n; ++i )
  {
    interiors.append( interiorRing( i )->curveToLine() );
  }
  polygon->setInteriorRings( interiors );
  return polygon;
}

QgsAbstractGeometry *QgsCurvePolygon::boundary() const
{
  if ( mInteriorRings.isEmpty() )
  {
    return mExteriorRing->clone();
  }
  else
  {
    QgsMultiCurve *multiCurve = new QgsMultiCurve();
    multiCurve->addGeometry( mExteriorRing->clone() );
    int nInteriorRings = mInteriorRings.size();
    for ( int i = 0; i < nInteriorRings; ++i )
    {
      multiCurve->addGeometry( mInteriorRings.at( i )->clone() );
    }
    return multiCurve;
  }
}

QgsPolygonV2 *QgsCurvePolygon::toPolygon( double tolerance, SegmentationToleranceType toleranceType ) const
{
  if ( !mExteriorRing )
  {
    return nullptr;
  }

  QgsPolygonV2 *poly = new QgsPolygonV2();
  poly->setExteriorRing( mExteriorRing->curveToLine( tolerance, toleranceType ) );

  QList<QgsCurve *> rings;
  QList<QgsCurve *>::const_iterator it = mInteriorRings.constBegin();
  for ( ; it != mInteriorRings.constEnd(); ++it )
  {
    rings.push_back( ( *it )->curveToLine( tolerance, toleranceType ) );
  }
  poly->setInteriorRings( rings );
  return poly;
}

int QgsCurvePolygon::numInteriorRings() const
{
  return mInteriorRings.size();
}

const QgsCurve *QgsCurvePolygon::exteriorRing() const
{
  return mExteriorRing;
}

const QgsCurve *QgsCurvePolygon::interiorRing( int i ) const
{
  if ( i < 0 || i >= mInteriorRings.size() )
  {
    return nullptr;
  }
  return mInteriorRings.at( i );
}

void QgsCurvePolygon::setExteriorRing( QgsCurve *ring )
{
  if ( !ring )
  {
    return;
  }
  delete mExteriorRing;
  mExteriorRing = ring;

  //set proper wkb type
  if ( QgsWkbTypes::flatType( wkbType() ) == QgsWkbTypes::Polygon )
  {
    setZMTypeFromSubGeometry( ring, QgsWkbTypes::Polygon );
  }
  else if ( QgsWkbTypes::flatType( wkbType() ) == QgsWkbTypes::CurvePolygon )
  {
    setZMTypeFromSubGeometry( ring, QgsWkbTypes::CurvePolygon );
  }

  //match dimensionality for rings
  Q_FOREACH ( QgsCurve *ring, mInteriorRings )
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

void QgsCurvePolygon::setInteriorRings( const QList<QgsCurve *> &rings )
{
  qDeleteAll( mInteriorRings );
  mInteriorRings.clear();

  //add rings one-by-one, so that they can each be converted to the correct type for the CurvePolygon
  Q_FOREACH ( QgsCurve *ring, rings )
  {
    addInteriorRing( ring );
  }
  clearCache();
}

void QgsCurvePolygon::addInteriorRing( QgsCurve *ring )
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

bool QgsCurvePolygon::removeInteriorRing( int nr )
{
  if ( nr < 0 || nr >= mInteriorRings.size() )
  {
    return false;
  }
  delete mInteriorRings.takeAt( nr );
  clearCache();
  return true;
}

void QgsCurvePolygon::removeInteriorRings( double minimumAllowedArea )
{
  for ( int ringIndex = mInteriorRings.size() - 1; ringIndex >= 0; --ringIndex )
  {
    if ( minimumAllowedArea < 0 )
      delete mInteriorRings.takeAt( ringIndex );
    else
    {
      double area = 0.0;
      mInteriorRings.at( ringIndex )->sumUpArea( area );
      if ( area < minimumAllowedArea )
        delete mInteriorRings.takeAt( ringIndex );
    }
  }

  clearCache();
}

void QgsCurvePolygon::draw( QPainter &p ) const
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

    QList<QgsCurve *>::const_iterator it = mInteriorRings.constBegin();
    for ( ; it != mInteriorRings.constEnd(); ++it )
    {
      ( *it )->addToPainterPath( path );
    }
    p.drawPath( path );
  }
}

void QgsCurvePolygon::transform( const QgsCoordinateTransform &ct, QgsCoordinateTransform::TransformDirection d, bool transformZ )
{
  if ( mExteriorRing )
  {
    mExteriorRing->transform( ct, d, transformZ );
  }

  Q_FOREACH ( QgsCurve *curve, mInteriorRings )
  {
    curve->transform( ct, d, transformZ );
  }
  clearCache();
}

void QgsCurvePolygon::transform( const QTransform &t )
{
  if ( mExteriorRing )
  {
    mExteriorRing->transform( t );
  }

  Q_FOREACH ( QgsCurve *curve, mInteriorRings )
  {
    curve->transform( t );
  }
  clearCache();
}

QgsCoordinateSequence QgsCurvePolygon::coordinateSequence() const
{
  if ( !mCoordinateSequence.isEmpty() )
    return mCoordinateSequence;

  mCoordinateSequence.append( QgsRingSequence() );

  if ( mExteriorRing )
  {
    mCoordinateSequence.back().append( QgsPointSequence() );
    mExteriorRing->points( mCoordinateSequence.back().back() );
  }

  QList<QgsCurve *>::const_iterator it = mInteriorRings.constBegin();
  for ( ; it != mInteriorRings.constEnd(); ++it )
  {
    mCoordinateSequence.back().append( QgsPointSequence() );
    ( *it )->points( mCoordinateSequence.back().back() );
  }

  return mCoordinateSequence;
}

int QgsCurvePolygon::nCoordinates() const
{
  if ( !mCoordinateSequence.isEmpty() )
    return QgsAbstractGeometry::nCoordinates();

  int count = 0;

  if ( mExteriorRing )
  {
    count += mExteriorRing->nCoordinates();
  }

  QList<QgsCurve *>::const_iterator it = mInteriorRings.constBegin();
  for ( ; it != mInteriorRings.constEnd(); ++it )
  {
    count += ( *it )->nCoordinates();
  }

  return count;
}

bool QgsCurvePolygon::isEmpty() const
{
  if ( !mExteriorRing )
    return true;

  return mExteriorRing->isEmpty();
}

double QgsCurvePolygon::closestSegment( const QgsPoint &pt, QgsPoint &segmentPt, QgsVertexId &vertexAfter, bool *leftOf, double epsilon ) const
{
  if ( !mExteriorRing )
  {
    return -1;
  }
  QList<QgsCurve *> segmentList;
  segmentList.append( mExteriorRing );
  segmentList.append( mInteriorRings );
  return QgsGeometryUtils::closestSegmentFromComponents( segmentList, QgsGeometryUtils::Ring, pt, segmentPt,  vertexAfter, leftOf, epsilon );
}

bool QgsCurvePolygon::nextVertex( QgsVertexId &vId, QgsPoint &vertex ) const
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
    QgsCurve *ring = vId.ring == 0 ? mExteriorRing : mInteriorRings[vId.ring - 1];

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

bool QgsCurvePolygon::insertVertex( QgsVertexId vId, const QgsPoint &vertex )
{
  if ( !mExteriorRing || vId.ring < 0 || vId.ring >= 1 + mInteriorRings.size() )
  {
    return false;
  }

  QgsCurve *ring = vId.ring == 0 ? mExteriorRing : mInteriorRings.at( vId.ring - 1 );
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

bool QgsCurvePolygon::moveVertex( QgsVertexId vId, const QgsPoint &newPos )
{
  if ( !mExteriorRing || vId.ring < 0 || vId.ring >= 1 + mInteriorRings.size() )
  {
    return false;
  }

  QgsCurve *ring = vId.ring == 0 ? mExteriorRing : mInteriorRings.at( vId.ring - 1 );
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

bool QgsCurvePolygon::deleteVertex( QgsVertexId vId )
{
  if ( !mExteriorRing || vId.ring < 0 || vId.ring >= 1 + mInteriorRings.size() )
  {
    return false;
  }

  QgsCurve *ring = vId.ring == 0 ? mExteriorRing : mInteriorRings.at( vId.ring - 1 );
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

bool QgsCurvePolygon::hasCurvedSegments() const
{
  if ( mExteriorRing && mExteriorRing->hasCurvedSegments() )
  {
    return true;
  }

  QList<QgsCurve *>::const_iterator it = mInteriorRings.constBegin();
  for ( ; it != mInteriorRings.constEnd(); ++it )
  {
    if ( ( *it )->hasCurvedSegments() )
    {
      return true;
    }
  }
  return false;
}

QgsAbstractGeometry *QgsCurvePolygon::segmentize( double tolerance, SegmentationToleranceType toleranceType ) const
{
  return toPolygon( tolerance, toleranceType );
}

double QgsCurvePolygon::vertexAngle( QgsVertexId vertex ) const
{
  if ( !mExteriorRing || vertex.ring < 0 || vertex.ring >= 1 + mInteriorRings.size() )
  {
    //makes no sense - conversion of false to double!
    return false;
  }

  QgsCurve *ring = vertex.ring == 0 ? mExteriorRing : mInteriorRings[vertex.ring - 1];
  return ring->vertexAngle( vertex );
}

int QgsCurvePolygon::vertexCount( int /*part*/, int ring ) const
{
  return ring == 0 ? mExteriorRing->vertexCount() : mInteriorRings[ring - 1]->vertexCount();
}

QgsPoint QgsCurvePolygon::vertexAt( QgsVertexId id ) const
{
  return id.ring == 0 ? mExteriorRing->vertexAt( id ) : mInteriorRings[id.ring - 1]->vertexAt( id );
}

bool QgsCurvePolygon::addZValue( double zValue )
{
  if ( QgsWkbTypes::hasZ( mWkbType ) )
    return false;

  mWkbType = QgsWkbTypes::addZ( mWkbType );

  if ( mExteriorRing )
    mExteriorRing->addZValue( zValue );
  Q_FOREACH ( QgsCurve *curve, mInteriorRings )
  {
    curve->addZValue( zValue );
  }
  clearCache();
  return true;
}

bool QgsCurvePolygon::addMValue( double mValue )
{
  if ( QgsWkbTypes::hasM( mWkbType ) )
    return false;

  mWkbType = QgsWkbTypes::addM( mWkbType );

  if ( mExteriorRing )
    mExteriorRing->addMValue( mValue );
  Q_FOREACH ( QgsCurve *curve, mInteriorRings )
  {
    curve->addMValue( mValue );
  }
  clearCache();
  return true;
}

bool QgsCurvePolygon::dropZValue()
{
  if ( !is3D() )
    return false;

  mWkbType = QgsWkbTypes::dropZ( mWkbType );
  if ( mExteriorRing )
    mExteriorRing->dropZValue();
  Q_FOREACH ( QgsCurve *curve, mInteriorRings )
  {
    curve->dropZValue();
  }
  clearCache();
  return true;
}

bool QgsCurvePolygon::dropMValue()
{
  if ( !isMeasure() )
    return false;

  mWkbType = QgsWkbTypes::dropM( mWkbType );
  if ( mExteriorRing )
    mExteriorRing->dropMValue();
  Q_FOREACH ( QgsCurve *curve, mInteriorRings )
  {
    curve->dropMValue();
  }
  clearCache();
  return true;
}
