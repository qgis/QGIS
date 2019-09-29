/***************************************************************************
                         qgstriangle.cpp
                         -------------------
    begin                : January 2017
    copyright            : (C) 2017 by Loïc Bartoletti
    email                : lbartoletti at tuxfamily dot org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstriangle.h"
#include "qgsgeometryutils.h"
#include "qgslinestring.h"
#include "qgswkbptr.h"

#include <memory>

QgsTriangle::QgsTriangle()
{
  mWkbType = QgsWkbTypes::Triangle;
}

QgsTriangle::QgsTriangle( const QgsPoint &p1, const QgsPoint &p2, const QgsPoint &p3 )
{
  mWkbType = QgsWkbTypes::Triangle;

  QVector< double > x { p1.x(), p2.x(), p3.x(), p1.x() };
  QVector< double > y { p1.y(), p2.y(), p3.y(), p1.y() };
  QVector< double > z;
  if ( p1.is3D() )
  {
    z = { p1.z(), p2.z(), p3.z(), p1.z() };
  }
  QVector< double > m;
  if ( p1.isMeasure() )
  {
    m = {p1.m(), p2.m(), p3.m(), p1.m() };
  }
  setExteriorRing( new QgsLineString( x, y, z, m ) );
}

QgsTriangle::QgsTriangle( const QgsPointXY &p1, const QgsPointXY &p2, const QgsPointXY &p3 )
{
  mWkbType = QgsWkbTypes::Triangle;

  QVector< double > x { p1.x(), p2.x(), p3.x(), p1.x() };
  QVector< double > y {p1.y(), p2.y(), p3.y(), p1.y() };
  QgsLineString *ext = new QgsLineString( x, y );
  setExteriorRing( ext );
}

QgsTriangle::QgsTriangle( const QPointF p1, const QPointF p2, const QPointF p3 )
{
  mWkbType = QgsWkbTypes::Triangle;

  QVector< double > x{ p1.x(), p2.x(), p3.x(), p1.x() };
  QVector< double > y{ p1.y(), p2.y(), p3.y(), p1.y() };
  QgsLineString *ext = new QgsLineString( x, y );
  setExteriorRing( ext );
}

bool QgsTriangle::operator==( const QgsTriangle &other ) const
{
  if ( isEmpty() && other.isEmpty() )
  {
    return true;
  }
  else if ( isEmpty() || other.isEmpty() )
  {
    return false;
  }

  return ( ( vertexAt( 0 ) == other.vertexAt( 0 ) ) &&
           ( vertexAt( 1 ) == other.vertexAt( 1 ) ) &&
           ( vertexAt( 2 ) == other.vertexAt( 2 ) )
         );
}

bool QgsTriangle::operator!=( const QgsTriangle &other ) const
{
  return !operator==( other );
}

QString QgsTriangle::geometryType() const
{
  return QStringLiteral( "Triangle" );
}

QgsTriangle *QgsTriangle::createEmptyWithSameType() const
{
  auto result = qgis::make_unique< QgsTriangle >();
  result->mWkbType = mWkbType;
  return result.release();
}

void QgsTriangle::clear()
{
  QgsPolygon::clear();
  mWkbType = QgsWkbTypes::Triangle;
}

QgsTriangle *QgsTriangle::clone() const
{
  return new QgsTriangle( *this );
}

bool QgsTriangle::fromWkb( QgsConstWkbPtr &wkbPtr )
{
  clear();
  if ( !wkbPtr )
  {
    return false;
  }

  QgsWkbTypes::Type type = wkbPtr.readHeader();
  if ( QgsWkbTypes::flatType( type ) != QgsWkbTypes::Triangle )
  {
    return false;
  }
  mWkbType = type;

  QgsWkbTypes::Type ringType;
  switch ( mWkbType )
  {
    case QgsWkbTypes::TriangleZ:
      ringType = QgsWkbTypes::LineStringZ;
      break;
    case QgsWkbTypes::TriangleM:
      ringType = QgsWkbTypes::LineStringM;
      break;
    case QgsWkbTypes::TriangleZM:
      ringType = QgsWkbTypes::LineStringZM;
      break;
    default:
      ringType = QgsWkbTypes::LineString;
      break;
  }

  int nRings;
  wkbPtr >> nRings;
  if ( nRings > 1 )
  {
    return false;
  }

  QgsLineString *line = new QgsLineString();
  line->fromWkbPoints( ringType, wkbPtr );
  mExteriorRing.reset( line );

  return true;
}

bool QgsTriangle::fromWkt( const QString &wkt )
{

  clear();

  QPair<QgsWkbTypes::Type, QString> parts = QgsGeometryUtils::wktReadBlock( wkt );

  if ( QgsWkbTypes::geometryType( parts.first ) != QgsWkbTypes::PolygonGeometry )
    return false;

  mWkbType = parts.first;

  if ( parts.second.compare( QLatin1String( "EMPTY" ), Qt::CaseInsensitive ) == 0 )
    return true;

  QString defaultChildWkbType = QStringLiteral( "LineString%1%2" ).arg( is3D() ? QStringLiteral( "Z" ) : QString(), isMeasure() ? QStringLiteral( "M" ) : QString() );

  const QStringList blocks = QgsGeometryUtils::wktGetChildBlocks( parts.second, defaultChildWkbType );
  for ( const QString &childWkt : blocks )
  {
    QPair<QgsWkbTypes::Type, QString> childParts = QgsGeometryUtils::wktReadBlock( childWkt );

    QgsWkbTypes::Type flatCurveType = QgsWkbTypes::flatType( childParts.first );
    if ( flatCurveType == QgsWkbTypes::LineString )
      mInteriorRings.append( new QgsLineString() );
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
  mExteriorRing.reset( mInteriorRings.takeFirst() );

  //scan through rings and check if dimensionality of rings is different to CurvePolygon.
  //if so, update the type dimensionality of the CurvePolygon to match
  bool hasZ = false;
  bool hasM = false;
  if ( mExteriorRing )
  {
    hasZ = hasZ || mExteriorRing->is3D();
    hasM = hasM || mExteriorRing->isMeasure();
  }
  if ( hasZ )
    addZValue( 0 );
  if ( hasM )
    addMValue( 0 );

  return true;
}

QDomElement QgsTriangle::asGml3( QDomDocument &doc, int precision, const QString &ns, const AxisOrder axisOrder ) const
{

  QDomElement elemTriangle = doc.createElementNS( ns, QStringLiteral( "Triangle" ) );

  if ( isEmpty() )
    return elemTriangle;

  QDomElement elemExterior = doc.createElementNS( ns, QStringLiteral( "exterior" ) );
  QDomElement curveElem = exteriorRing()->asGml3( doc, precision, ns, axisOrder );
  if ( curveElem.tagName() == QLatin1String( "LineString" ) )
  {
    curveElem.setTagName( QStringLiteral( "LinearRing" ) );
  }
  elemExterior.appendChild( curveElem );
  elemTriangle.appendChild( elemExterior );

  return elemTriangle;
}

QgsPolygon *QgsTriangle::surfaceToPolygon() const
{
  return toPolygon();
}

QgsCurvePolygon *QgsTriangle::toCurveType() const
{
  std::unique_ptr<QgsCurvePolygon> curvePolygon( new QgsCurvePolygon() );
  curvePolygon->setExteriorRing( mExteriorRing->clone() );

  return curvePolygon.release();
}

void QgsTriangle::addInteriorRing( QgsCurve *ring )
{
  delete ring;
}

bool QgsTriangle::deleteVertex( QgsVertexId position )
{
  Q_UNUSED( position )
  return false;
}

bool QgsTriangle::insertVertex( QgsVertexId position, const QgsPoint &vertex )
{
  Q_UNUSED( position )
  Q_UNUSED( vertex )
  return false;
}

bool QgsTriangle::moveVertex( QgsVertexId vId, const QgsPoint &newPos )
{
  if ( isEmpty() )
    return false;

  if ( !mExteriorRing || vId.part != 0 || vId.ring != 0 || vId.vertex < 0 || vId.vertex > 4 )
  {
    return false;
  }

  if ( vId.vertex == 4 )
  {
    vId.vertex = 0;
  }

  int n = mExteriorRing->numPoints();
  bool success = mExteriorRing->moveVertex( vId, newPos );
  if ( success )
  {
    // If first or last vertex is moved, also move the last/first vertex
    if ( vId.vertex == 0 )
      mExteriorRing->moveVertex( QgsVertexId( vId.part, vId.ring, n - 1 ), newPos );
    clearCache();
  }
  return success;
}

void QgsTriangle::setExteriorRing( QgsCurve *ring )
{
  if ( !ring )
  {
    return;
  }

  if ( ring->hasCurvedSegments() )
  {
    //need to segmentize ring as polygon does not support curves
    QgsCurve *line = ring->segmentize();
    delete ring;
    ring = line;
  }

  if ( ( ring->numPoints() > 4 ) || ( ring->numPoints() < 3 ) )
  {
    delete ring;
    return;
  }
  else if ( ring->numPoints() == 4 )
  {
    if ( !ring->isClosed() )
    {
      delete ring;
      return;
    }
  }
  else if ( ring->numPoints() == 3 )
  {
    if ( ring->isClosed() )
    {
      delete ring;
      return;
    }
    QgsLineString *lineString = static_cast< QgsLineString *>( ring );
    if ( !lineString->isClosed() )
    {
      lineString->close();
    }
    ring = lineString;
  }

  mExteriorRing.reset( ring );

  //set proper wkb type
  setZMTypeFromSubGeometry( ring, QgsWkbTypes::Triangle );

  clearCache();
}

QgsCurve *QgsTriangle::boundary() const
{
  if ( !mExteriorRing )
    return nullptr;

  return mExteriorRing->clone();
}

QgsPoint QgsTriangle::vertexAt( int atVertex ) const
{
  if ( isEmpty() )
    return QgsPoint();

  QgsVertexId id( 0, 0, atVertex );
  return mExteriorRing->vertexAt( id );
}

QVector<double> QgsTriangle::lengths() const
{
  QVector<double> lengths;
  if ( isEmpty() )
    return lengths;

  lengths.append( vertexAt( 0 ).distance( vertexAt( 1 ) ) );
  lengths.append( vertexAt( 1 ).distance( vertexAt( 2 ) ) );
  lengths.append( vertexAt( 2 ).distance( vertexAt( 0 ) ) );

  return lengths;
}

QVector<double> QgsTriangle::angles() const
{
  QVector<double> angles;
  if ( isEmpty() )
    return angles;
  double ax, ay, bx, by, cx, cy;

  ax = vertexAt( 0 ).x();
  ay = vertexAt( 0 ).y();
  bx = vertexAt( 1 ).x();
  by = vertexAt( 1 ).y();
  cx = vertexAt( 2 ).x();
  cy = vertexAt( 2 ).y();

  double a1 = std::fmod( QgsGeometryUtils::angleBetweenThreePoints( cx, cy, ax, ay, bx, by ), M_PI );
  double a2 = std::fmod( QgsGeometryUtils::angleBetweenThreePoints( ax, ay, bx, by, cx, cy ), M_PI );
  double a3 = std::fmod( QgsGeometryUtils::angleBetweenThreePoints( bx, by, cx, cy, ax, ay ), M_PI );

  angles.append( ( a1 > M_PI_2 ? a1 - M_PI_2 : a1 ) );
  angles.append( ( a2 > M_PI_2 ? a2 - M_PI_2 : a2 ) );
  angles.append( ( a3 > M_PI_2 ? a3 - M_PI_2 : a3 ) );

  return angles;
}

bool QgsTriangle::isDegenerate()
{
  if ( isEmpty() )
    return true;

  QgsPoint p1( vertexAt( 0 ) );
  QgsPoint p2( vertexAt( 1 ) );
  QgsPoint p3( vertexAt( 2 ) );
  return ( ( ( p1 == p2 ) || ( p1 == p3 ) || ( p2 == p3 ) ) || QgsGeometryUtils::leftOfLine( p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y() ) == 0 );
}

bool QgsTriangle::isIsocele( double lengthTolerance ) const
{
  if ( isEmpty() )
    return false;
  QVector<double> sides = lengths();
  bool ab_bc = qgsDoubleNear( sides.at( 0 ), sides.at( 1 ), lengthTolerance );
  bool bc_ca = qgsDoubleNear( sides.at( 1 ), sides.at( 2 ), lengthTolerance );
  bool ca_ab = qgsDoubleNear( sides.at( 2 ), sides.at( 0 ), lengthTolerance );

  return ( ab_bc || bc_ca || ca_ab );
}

bool QgsTriangle::isEquilateral( double lengthTolerance ) const
{
  if ( isEmpty() )
    return false;
  QVector<double> sides = lengths();
  bool ab_bc = qgsDoubleNear( sides.at( 0 ), sides.at( 1 ), lengthTolerance );
  bool bc_ca = qgsDoubleNear( sides.at( 1 ), sides.at( 2 ), lengthTolerance );
  bool ca_ab = qgsDoubleNear( sides.at( 2 ), sides.at( 0 ), lengthTolerance );

  return ( ab_bc && bc_ca && ca_ab );
}

bool QgsTriangle::isRight( double angleTolerance ) const
{
  if ( isEmpty() )
    return false;
  QVector<double> a = angles();
  QVector<double>::iterator ita = a.begin();
  while ( ita != a.end() )
  {
    if ( qgsDoubleNear( *ita, M_PI_2, angleTolerance ) )
      return true;
    ita++;
  }
  return false;
}

bool QgsTriangle::isScalene( double lengthTolerance ) const
{
  if ( isEmpty() )
    return false;
  return !isIsocele( lengthTolerance );
}

QVector<QgsLineString> QgsTriangle::altitudes() const
{
  QVector<QgsLineString> alt;
  if ( isEmpty() )
    return alt;

  alt.append( QgsGeometryUtils::perpendicularSegment( vertexAt( 0 ), vertexAt( 2 ), vertexAt( 1 ) ) );
  alt.append( QgsGeometryUtils::perpendicularSegment( vertexAt( 1 ), vertexAt( 0 ), vertexAt( 2 ) ) );
  alt.append( QgsGeometryUtils::perpendicularSegment( vertexAt( 2 ), vertexAt( 0 ), vertexAt( 1 ) ) );

  return alt;
}

QVector<QgsLineString> QgsTriangle::medians() const
{
  QVector<QgsLineString> med;
  if ( isEmpty() )
    return med;

  QgsLineString med1;
  QgsLineString med2;
  QgsLineString med3;
  med1.setPoints( QgsPointSequence() << vertexAt( 0 ) << QgsGeometryUtils::midpoint( vertexAt( 1 ), vertexAt( 2 ) ) );
  med2.setPoints( QgsPointSequence() << vertexAt( 1 ) << QgsGeometryUtils::midpoint( vertexAt( 0 ), vertexAt( 2 ) ) );
  med3.setPoints( QgsPointSequence() << vertexAt( 2 ) << QgsGeometryUtils::midpoint( vertexAt( 0 ), vertexAt( 1 ) ) );
  med.append( med1 );
  med.append( med2 );
  med.append( med3 );

  return med;
}

QVector<QgsLineString> QgsTriangle::bisectors( double lengthTolerance ) const
{
  QVector<QgsLineString> bis;
  if ( isEmpty() )
    return bis;

  QgsLineString bis1;
  QgsLineString bis2;
  QgsLineString bis3;
  QgsPoint incenter = inscribedCenter();
  QgsPoint out;
  bool intersection = false;

  QgsGeometryUtils::segmentIntersection( vertexAt( 0 ), incenter, vertexAt( 1 ), vertexAt( 2 ), out, intersection, lengthTolerance );
  bis1.setPoints( QgsPointSequence() <<  vertexAt( 0 ) << out );

  QgsGeometryUtils::segmentIntersection( vertexAt( 1 ), incenter, vertexAt( 0 ), vertexAt( 2 ), out, intersection, lengthTolerance );
  bis2.setPoints( QgsPointSequence() <<  vertexAt( 1 ) << out );

  QgsGeometryUtils::segmentIntersection( vertexAt( 2 ), incenter, vertexAt( 0 ), vertexAt( 1 ), out, intersection, lengthTolerance );
  bis3.setPoints( QgsPointSequence() <<  vertexAt( 2 ) << out );

  bis.append( bis1 );
  bis.append( bis2 );
  bis.append( bis3 );

  return bis;
}

QgsTriangle QgsTriangle::medial() const
{
  if ( isEmpty() )
    return QgsTriangle();
  QgsPoint p1, p2, p3;
  p1 = QgsGeometryUtils::midpoint( vertexAt( 0 ), vertexAt( 1 ) );
  p2 = QgsGeometryUtils::midpoint( vertexAt( 1 ), vertexAt( 2 ) );
  p3 = QgsGeometryUtils::midpoint( vertexAt( 2 ), vertexAt( 0 ) );
  return QgsTriangle( p1, p2, p3 );
}

QgsPoint QgsTriangle::orthocenter( double lengthTolerance ) const
{
  if ( isEmpty() )
    return QgsPoint();
  QVector<QgsLineString> alt = altitudes();
  QgsPoint ortho;
  bool intersection;
  QgsGeometryUtils::segmentIntersection( alt.at( 0 ).pointN( 0 ), alt.at( 0 ).pointN( 1 ), alt.at( 1 ).pointN( 0 ), alt.at( 1 ).pointN( 1 ), ortho, intersection, lengthTolerance );

  return ortho;
}

QgsPoint QgsTriangle::circumscribedCenter() const
{
  if ( isEmpty() )
    return QgsPoint();
  double r, x, y;
  QgsGeometryUtils::circleCenterRadius( vertexAt( 0 ), vertexAt( 1 ), vertexAt( 2 ), r, x, y );
  return QgsPoint( x, y );
}

double QgsTriangle::circumscribedRadius() const
{
  if ( isEmpty() )
    return 0.0;
  double r, x, y;
  QgsGeometryUtils::circleCenterRadius( vertexAt( 0 ), vertexAt( 1 ), vertexAt( 2 ), r, x, y );
  return r;
}

QgsCircle QgsTriangle::circumscribedCircle() const
{
  if ( isEmpty() )
    return QgsCircle();
  return QgsCircle( circumscribedCenter(), circumscribedRadius() );
}

QgsPoint QgsTriangle::inscribedCenter() const
{
  if ( isEmpty() )
    return QgsPoint();

  QVector<double> l = lengths();
  double x = ( l.at( 0 ) * vertexAt( 2 ).x() +
               l.at( 1 ) * vertexAt( 0 ).x() +
               l.at( 2 ) * vertexAt( 1 ).x() ) / perimeter();
  double y = ( l.at( 0 ) * vertexAt( 2 ).y() +
               l.at( 1 ) * vertexAt( 0 ).y() +
               l.at( 2 ) * vertexAt( 1 ).y() ) / perimeter();

  QgsPoint center( x, y );

  QgsPointSequence points;
  points << vertexAt( 0 ) << vertexAt( 1 ) << vertexAt( 2 );
  QgsGeometryUtils::setZValueFromPoints( points, center );

  return center;
}

double QgsTriangle::inscribedRadius() const
{
  if ( isEmpty() )
    return 0.0;
  return ( 2.0 * area() / perimeter() );
}

QgsCircle QgsTriangle::inscribedCircle() const
{
  if ( isEmpty() )
    return QgsCircle();
  return QgsCircle( inscribedCenter(), inscribedRadius() );
}
