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
    : QgsPolygonV2()
{
  mWkbType = QgsWkbTypes::Triangle;
}

QgsTriangle::QgsTriangle( const QgsPointV2 &p1, const QgsPointV2 &p2, const QgsPointV2 &p3 )
{
  mWkbType = QgsWkbTypes::Triangle;

  if ((( p1 == p2 ) || ( p1 == p3 ) || ( p2 == p3 ) ) || qgsDoubleNear( QgsGeometryUtils::leftOfLine( p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y() ), 0.0 ) )
  {
    return;
  }
  QgsLineString *ext = new QgsLineString();
  ext->setPoints( QgsPointSequence() << p1 << p2 << p3 << p1 );
  setExteriorRing( ext );

}

void QgsTriangle::clear()
{
  QgsCurvePolygon::clear();
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

  QgsLineString* line = new QgsLineString();
  line->fromWkbPoints( ringType, wkbPtr );
  if ( !mExteriorRing )
  {
    mExteriorRing = line;
  }

  return true;
}

bool QgsTriangle::fromWkt( const QString &wkt )
{

  clear();

  QPair<QgsWkbTypes::Type, QString> parts = QgsGeometryUtils::wktReadBlock( wkt );

  if ( QgsWkbTypes::geometryType( parts.first ) != QgsWkbTypes::PolygonGeometry )
    return false;

  mWkbType = parts.first;

  QString defaultChildWkbType = QStringLiteral( "LineString%1%2" ).arg( is3D() ? "Z" : QString(), isMeasure() ? "M" : QString() );

  Q_FOREACH ( const QString& childWkt, QgsGeometryUtils::wktGetChildBlocks( parts.second, defaultChildWkbType ) )
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
  if ( hasZ )
    addZValue( 0 );
  if ( hasM )
    addMValue( 0 );

  return true;
}

QgsAbstractGeometry *QgsTriangle::toCurveType() const
{
  std::unique_ptr<QgsCurvePolygon> curvePolygon( new QgsCurvePolygon() );
  curvePolygon->setExteriorRing( mExteriorRing->clone() );

  return curvePolygon.release();
}

void QgsTriangle::addInteriorRing( QgsCurve *ring )
{
  Q_UNUSED( ring );
  return;
}

bool QgsTriangle::deleteVertex( QgsVertexId position )
{
  Q_UNUSED( position );
  return false;
}

bool QgsTriangle::insertVertex( QgsVertexId position, const QgsPointV2 &vertex )
{
  Q_UNUSED( position );
  Q_UNUSED( vertex );
  return false;
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
    QgsCurve* line = ring->segmentize();
    delete ring;
    ring = line;
  }

  if ( ring->numPoints() == 4 )
  {
    if ( !ring->isClosed() )
    {
      return;
    }
  }
  else if ( ring->numPoints() == 3 )
  {
    if ( ring->isClosed() )
    {
      return;
    }
    QgsLineString* lineString = dynamic_cast< QgsLineString*>( ring );
    if ( lineString && !lineString->isClosed() )
    {
      lineString->close();
    }
    ring = lineString;
  }

  delete mExteriorRing;

  mExteriorRing = ring;

  //set proper wkb type
  setZMTypeFromSubGeometry( ring, QgsWkbTypes::Triangle );

  clearCache();
}

QgsAbstractGeometry *QgsTriangle::boundary() const
{
  if ( !mExteriorRing )
    return nullptr;

  return mExteriorRing->clone();
}

QgsPointV2 QgsTriangle::vertexAt( int atVertex ) const
{
  QgsVertexId id( 0, 0, atVertex );
  return mExteriorRing->vertexAt( id );
}

QVector<double> QgsTriangle::lengths() const
{
  QVector<double> lengths;
  lengths.append( vertexAt( 0 ).distance( vertexAt( 1 ) ) );
  lengths.append( vertexAt( 1 ).distance( vertexAt( 2 ) ) );
  lengths.append( vertexAt( 2 ).distance( vertexAt( 0 ) ) );

  return lengths;
}

QVector<double> QgsTriangle::angles() const
{
  QVector<double> angles;
  double ax, ay, bx, by, cx, cy;

  ax = vertexAt( 0 ).x();
  ay = vertexAt( 0 ).y();
  bx = vertexAt( 1 ).x();
  by = vertexAt( 1 ).y();
  cx = vertexAt( 2 ).x();
  cy = vertexAt( 2 ).y();

  double a1 = fmod( QgsGeometryUtils::angleBetweenThreePoints( cx, cy, ax, ay, bx, by ), M_PI );
  double a2 = fmod( QgsGeometryUtils::angleBetweenThreePoints( ax, ay, bx, by, cx, cy ), M_PI );
  double a3 = fmod( QgsGeometryUtils::angleBetweenThreePoints( bx, by, cx, cy, ax, ay ), M_PI );

  angles.append(( a1 > M_PI / 2 ? a1 - M_PI / 2 : a1 ) );
  angles.append(( a2 > M_PI / 2 ? a2 - M_PI / 2 : a2 ) );
  angles.append(( a3 > M_PI / 2 ? a3 - M_PI / 2 : a3 ) );

  return angles;
}

bool QgsTriangle::isIsocele( double lengthTolerance ) const
{
  QVector<double> sides = lengths();
  bool ab_bc = qgsDoubleNear( sides.at( 0 ), sides.at( 1 ), lengthTolerance );
  bool bc_ca = qgsDoubleNear( sides.at( 1 ), sides.at( 2 ), lengthTolerance );
  bool ca_ab = qgsDoubleNear( sides.at( 2 ), sides.at( 0 ), lengthTolerance );

  return ( ab_bc || bc_ca || ca_ab );
}

bool QgsTriangle::isEquilateral( double lengthTolerance ) const
{
  QVector<double> sides = lengths();
  bool ab_bc = qgsDoubleNear( sides.at( 0 ), sides.at( 1 ), lengthTolerance );
  bool bc_ca = qgsDoubleNear( sides.at( 1 ), sides.at( 2 ), lengthTolerance );
  bool ca_ab = qgsDoubleNear( sides.at( 2 ), sides.at( 0 ), lengthTolerance );

  return ( ab_bc && bc_ca && ca_ab );
}

bool QgsTriangle::isRight( double angleTolerance ) const
{
  QVector<double> a = angles();
  QVector<double>::iterator ita = a.begin();
  while ( ita != a.end() )
  {
    if ( qgsDoubleNear( *ita, M_PI / 2.0, angleTolerance ) )
      return true;
    ita++;
  }
  return false;
}

bool QgsTriangle::isScalene( double lengthTolerance ) const
{
  return !isIsocele( lengthTolerance );
}

QVector<QgsLineString> QgsTriangle::altitudes() const
{
  QVector<QgsLineString> alt;
  alt.append( *QgsGeometryUtils::perpendicularSegment( vertexAt( 0 ), vertexAt( 2 ), vertexAt( 1 ) ) );
  alt.append( *QgsGeometryUtils::perpendicularSegment( vertexAt( 1 ), vertexAt( 0 ), vertexAt( 2 ) ) );
  alt.append( *QgsGeometryUtils::perpendicularSegment( vertexAt( 2 ), vertexAt( 0 ), vertexAt( 1 ) ) );

  return alt;
}

QVector<QgsLineString> QgsTriangle::medians() const
{
  QVector<QgsLineString> med;

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
  QgsLineString bis1;
  QgsLineString bis2;
  QgsLineString bis3;
  QgsPointV2 incenter = inscribedCenter();
  QgsPointV2 out;

  QgsGeometryUtils::segmentIntersection( vertexAt( 0 ), incenter, vertexAt( 1 ), vertexAt( 2 ), out, lengthTolerance );
  bis1.setPoints( QgsPointSequence() <<  vertexAt( 0 ) << out );

  QgsGeometryUtils::segmentIntersection( vertexAt( 1 ), incenter, vertexAt( 0 ), vertexAt( 2 ), out, lengthTolerance );
  bis2.setPoints( QgsPointSequence() <<  vertexAt( 1 ) << out );

  QgsGeometryUtils::segmentIntersection( vertexAt( 2 ), incenter, vertexAt( 0 ), vertexAt( 1 ), out, lengthTolerance );
  bis3.setPoints( QgsPointSequence() <<  vertexAt( 2 ) << out );

  bis.append( bis1 );
  bis.append( bis2 );
  bis.append( bis3 );

  return bis;
}

QgsTriangle QgsTriangle::medial() const
{
  QgsPointV2 p1, p2, p3;
  p1 = QgsGeometryUtils::midpoint( vertexAt( 0 ), vertexAt( 1 ) );
  p2 = QgsGeometryUtils::midpoint( vertexAt( 1 ), vertexAt( 2 ) );
  p3 = QgsGeometryUtils::midpoint( vertexAt( 2 ), vertexAt( 0 ) );
  return QgsTriangle( p1, p2, p3 );
}

QgsPointV2 QgsTriangle::orthocenter( double lengthTolerance ) const
{
  QVector<QgsLineString> alt = altitudes();
  QgsPointV2 ortho;
  QgsGeometryUtils::segmentIntersection( alt.at( 0 ).pointN( 0 ), alt.at( 0 ).pointN( 1 ), alt.at( 1 ).pointN( 0 ), alt.at( 1 ).pointN( 1 ), ortho, lengthTolerance );

  return ortho;
}

QgsPointV2 QgsTriangle::circumscribedCenter() const
{
  double r, x, y;
  QgsGeometryUtils::circleCenterRadius( vertexAt( 0 ), vertexAt( 1 ), vertexAt( 2 ), r, x, y );
  return QgsPointV2( x, y );
}

double QgsTriangle::circumscribedRadius() const
{
  double r, x, y;
  QgsGeometryUtils::circleCenterRadius( vertexAt( 0 ), vertexAt( 1 ), vertexAt( 2 ), r, x, y );
  return r;
}

/*
QgsCircle QgsTriangle::circumscribedCircle() const
{

}*/

QgsPointV2 QgsTriangle::inscribedCenter() const
{

  QVector<double> l = lengths();
  double x = ( l.at( 0 ) * vertexAt( 2 ).x() +
               l.at( 1 ) * vertexAt( 0 ).x() +
               l.at( 2 ) * vertexAt( 1 ).x() ) / perimeter();
  double y = ( l.at( 0 ) * vertexAt( 2 ).y() +
               l.at( 1 ) * vertexAt( 0 ).y() +
               l.at( 2 ) * vertexAt( 1 ).y() ) / perimeter();

  return QgsPointV2( x, y );
}

double QgsTriangle::inscribedRadius() const
{
  return ( 2.0 * area() / perimeter() );
}

/*
QgsCircle QgsTriangle::inscribedCircle() const
{

}*/



