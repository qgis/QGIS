/***************************************************************************
                         qgspointv2.cpp
                         --------------
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


#include "qgspointv2.h"
#include "qgsapplication.h"
#include "qgscoordinatetransform.h"
#include "qgsgeometryutils.h"
#include "qgsmaptopixel.h"
#include "qgswkbptr.h"
#include <QPainter>

QgsPointV2::QgsPointV2( double x, double y ): QgsAbstractGeometryV2(), mX( x ), mY( y ), mZ( 0.0 ), mM( 0.0 )
{
  mWkbType = QgsWKBTypes::Point;
}

QgsPointV2::QgsPointV2( QgsWKBTypes::Type type, double x, double y, double z, double m ): mX( x ), mY( y ), mZ( z ), mM( m )
{
  mWkbType = type;
}

bool QgsPointV2::operator==( const QgsPointV2& pt ) const
{
  return ( pt.wkbType() == wkbType() &&
           qgsDoubleNear( pt.x(), mX, 1E-8 ) &&
           qgsDoubleNear( pt.y(), mY, 1E-8 ) &&
           qgsDoubleNear( pt.z(), mZ, 1E-8 ) &&
           qgsDoubleNear( pt.m(), mM, 1E-8 ) );
}

bool QgsPointV2::operator!=( const QgsPointV2& pt ) const
{
  return !operator==( pt );
}

QgsAbstractGeometryV2* QgsPointV2::clone() const
{
  return new QgsPointV2( *this );
}

bool QgsPointV2::fromWkb( const unsigned char* wkb )
{
  QgsConstWkbPtr wkbPtr( wkb );
  QgsWKBTypes::Type type = wkbPtr.readHeader();
  if ( QgsWKBTypes::flatType( type ) != QgsWKBTypes::Point )
  {
    return false;
  }
  mWkbType = type;

  wkbPtr >> mX;
  wkbPtr >> mY;
  if ( is3D() )
    wkbPtr >> mZ;
  if ( isMeasure() )
    wkbPtr >> mM;

  return true;
}

bool QgsPointV2::fromWkt( const QString& wkt )
{
  clear();

  QPair<QgsWKBTypes::Type, QString> parts = QgsGeometryUtils::wktReadBlock( wkt );

  if ( QgsWKBTypes::flatType( parts.first ) != QgsWKBTypes::parseType( geometryType() ) )
    return false;
  mWkbType = parts.first;

  QStringList coordinates = parts.second.split( " ", QString::SkipEmptyParts );
  if ( coordinates.size() < 2 + is3D() + isMeasure() )
  {
    clear();
    return false;
  }

  int idx = 0;
  mX = coordinates[idx++].toDouble();
  mY = coordinates[idx++].toDouble();
  if ( is3D() )
    mZ = coordinates[idx++].toDouble();
  if ( isMeasure() )
    mM = coordinates[idx++].toDouble();

  return true;
}

int QgsPointV2::wkbSize() const
{
  int size = sizeof( char ) + sizeof( quint32 );
  size += ( 2 + is3D() + isMeasure() ) * sizeof( double );
  return size;
}

unsigned char* QgsPointV2::asWkb( int& binarySize ) const
{
  binarySize = wkbSize();
  unsigned char* geomPtr = new unsigned char[binarySize];
  QgsWkbPtr wkb( geomPtr );
  wkb << static_cast<char>( QgsApplication::endian() );
  wkb << static_cast<quint32>( wkbType() );
  wkb << mX << mY;
  if ( is3D() )
  {
    wkb << mZ;
  }
  if ( isMeasure() )
  {
    wkb << mM;
  }
  return geomPtr;
}

QString QgsPointV2::asWkt( int precision ) const
{
  QString wkt = wktTypeStr() + " (";
  wkt += qgsDoubleToString( mX, precision ) + " " + qgsDoubleToString( mY, precision );
  if ( is3D() )
    wkt += " " + qgsDoubleToString( mZ, precision );
  if ( isMeasure() )
    wkt += " " + qgsDoubleToString( mM, precision );
  wkt += ")";
  return wkt;
}

QDomElement QgsPointV2::asGML2( QDomDocument& doc, int precision, const QString& ns ) const
{
  QDomElement elemPoint = doc.createElementNS( ns, "Point" );
  QDomElement elemCoordinates = doc.createElementNS( ns, "coordinates" );
  QString strCoordinates = qgsDoubleToString( mX, precision ) + "," + qgsDoubleToString( mY, precision );
  elemCoordinates.appendChild( doc.createTextNode( strCoordinates ) );
  elemPoint.appendChild( elemCoordinates );
  return elemPoint;
}

QDomElement QgsPointV2::asGML3( QDomDocument& doc, int precision, const QString& ns ) const
{
  QDomElement elemPoint = doc.createElementNS( ns, "Point" );
  QDomElement elemPosList = doc.createElementNS( ns, "posList" );
  elemPosList.setAttribute( "srsDimension", is3D() ? 3 : 2 );
  QString strCoordinates = qgsDoubleToString( mX, precision ) + " " + qgsDoubleToString( mY, precision );
  if ( is3D() )
    strCoordinates += " " + qgsDoubleToString( mZ, precision );

  elemPosList.appendChild( doc.createTextNode( strCoordinates ) );
  elemPoint.appendChild( elemPosList );
  return elemPoint;
}

QString QgsPointV2::asJSON( int precision ) const
{
  return "{\"type\": \"Point\", \"coordinates\": ["
         + qgsDoubleToString( mX, precision ) + ", " + qgsDoubleToString( mY, precision )
         + "]}";
}

void QgsPointV2::draw( QPainter& p ) const
{
  p.drawRect( mX - 2, mY - 2, 4, 4 );
}

void QgsPointV2::clear()
{
  mWkbType = QgsWKBTypes::Unknown;
  mX = mY = mZ = mM = 0.;
}

void QgsPointV2::transform( const QgsCoordinateTransform& ct )
{
  ct.transformInPlace( mX, mY, mZ );
}

void QgsPointV2::coordinateSequence( QList< QList< QList< QgsPointV2 > > >& coord ) const
{
  coord.clear();
  QList< QList< QgsPointV2 > > featureCoord;
  featureCoord.append( QList< QgsPointV2 >() << QgsPointV2( *this ) );
  coord.append( featureCoord );
}

bool QgsPointV2::moveVertex( const QgsVertexId& position, const QgsPointV2& newPos )
{
  Q_UNUSED( position );
  mX = newPos.mX;
  mY = newPos.mY;
  if ( is3D() && newPos.is3D() )
  {
    mZ = newPos.mZ;
  }
  if ( isMeasure() && newPos.isMeasure() )
  {
    mM = newPos.mM;
  }
  mBoundingBox = QgsRectangle(); //set bounding box invalid
  return true;
}

double QgsPointV2::closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const
{
  Q_UNUSED( segmentPt ); Q_UNUSED( vertexAfter ); Q_UNUSED( leftOf ); Q_UNUSED( leftOf ); Q_UNUSED( epsilon );
  return QgsGeometryUtils::sqrDistance2D( *this, pt );
}

bool QgsPointV2::nextVertex( QgsVertexId& id, QgsPointV2& vertex ) const
{
  if ( id.vertex < 0 )
  {
    id.vertex = 0;
    if ( id.part < 0 )
    {
      id.part = 0;
    }
    if ( id.ring < 0 )
    {
      id.ring = 0;
    }
    vertex = *this;
    return true;
  }
  else
  {
    return false;
  }
}

void QgsPointV2::transform( const QTransform& t )
{
  qreal x, y;
  t.map( mX, mY, &x, &y );
  mX = x; mY = y;
}
