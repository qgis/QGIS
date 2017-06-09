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


#include "qgspoint.h"
#include "qgsapplication.h"
#include "qgscoordinatetransform.h"
#include "qgsgeometryutils.h"
#include "qgsmaptopixel.h"
#include "qgswkbptr.h"
#include <QPainter>
#include <QRegularExpression>

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

QgsPoint::QgsPoint( double x, double y, double z, double m, QgsWkbTypes::Type wkbType )
  : QgsAbstractGeometry()
  , mX( x )
  , mY( y )
  , mZ( z )
  , mM( m )
{
  if ( wkbType != QgsWkbTypes::Unknown )
  {
    Q_ASSERT( QgsWkbTypes::flatType( wkbType ) == QgsWkbTypes::Point );
    mWkbType = wkbType;
  }
  else if ( qIsNaN( z ) )
  {
    if ( qIsNaN( m ) )
      mWkbType = QgsWkbTypes::Point;
    else
      mWkbType = QgsWkbTypes::PointM;
  }
  else if ( qIsNaN( m ) )
    mWkbType = QgsWkbTypes::PointZ;
  else
    mWkbType = QgsWkbTypes::PointZM;
}

QgsPoint::QgsPoint( const QgsPointXY &p )
  : QgsAbstractGeometry()
  , mX( p.x() )
  , mY( p.y() )
  , mZ( 0.0 )
  , mM( 0.0 )
{
  mWkbType = QgsWkbTypes::Point;
}

QgsPoint::QgsPoint( QPointF p )
  : QgsAbstractGeometry()
  , mX( p.x() )
  , mY( p.y() )
  , mZ( 0.0 )
  , mM( 0.0 )
{
  mWkbType = QgsWkbTypes::Point;
}

QgsPoint::QgsPoint( QgsWkbTypes::Type wkbType, double x, double y, double z, double m )
  : QgsAbstractGeometry()
  , mX( x )
  , mY( y )
  , mZ( z )
  , mM( m )
{
  Q_ASSERT( QgsWkbTypes::flatType( wkbType ) == QgsWkbTypes::Point );
  mWkbType = wkbType;
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

bool QgsPoint::operator==( const QgsPoint &pt ) const
{
  const QgsWkbTypes::Type type = wkbType();

  bool equal = pt.wkbType() == type;
  equal &= qgsDoubleNear( pt.x(), mX, 1E-8 );
  equal &= qgsDoubleNear( pt.y(), mY, 1E-8 );
  if ( QgsWkbTypes::hasZ( type ) )
    equal &= qgsDoubleNear( pt.z(), mZ, 1E-8 );
  if ( QgsWkbTypes::hasM( type ) )
    equal &= qgsDoubleNear( pt.m(), mM, 1E-8 );

  return equal;
}

bool QgsPoint::operator!=( const QgsPoint &pt ) const
{
  return !operator==( pt );
}

QgsPoint *QgsPoint::clone() const
{
  return new QgsPoint( *this );
}

bool QgsPoint::fromWkb( QgsConstWkbPtr &wkbPtr )
{
  QgsWkbTypes::Type type = wkbPtr.readHeader();
  if ( QgsWkbTypes::flatType( type ) != QgsWkbTypes::Point )
  {
    clear();
    return false;
  }
  mWkbType = type;

  wkbPtr >> mX;
  wkbPtr >> mY;
  if ( is3D() )
    wkbPtr >> mZ;
  if ( isMeasure() )
    wkbPtr >> mM;

  clearCache();

  return true;
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

bool QgsPoint::fromWkt( const QString &wkt )
{
  clear();

  QPair<QgsWkbTypes::Type, QString> parts = QgsGeometryUtils::wktReadBlock( wkt );

  if ( QgsWkbTypes::flatType( parts.first ) != QgsWkbTypes::Point )
    return false;
  mWkbType = parts.first;

  QRegularExpression rx( "\\s" );
  QStringList coordinates = parts.second.split( rx, QString::SkipEmptyParts );
  if ( coordinates.size() < 2 )
  {
    clear();
    return false;
  }
  else if ( coordinates.size() == 3 && !is3D() && !isMeasure() )
  {
    // 3 dimensional coordinates, but not specifically marked as such. We allow this
    // anyway and upgrade geometry to have Z dimension
    mWkbType = QgsWkbTypes::addZ( mWkbType );
  }
  else if ( coordinates.size() >= 4 && ( !is3D() || !isMeasure() ) )
  {
    // 4 (or more) dimensional coordinates, but not specifically marked as such. We allow this
    // anyway and upgrade geometry to have Z&M dimensions
    mWkbType = QgsWkbTypes::addZ( mWkbType );
    mWkbType = QgsWkbTypes::addM( mWkbType );
  }

  int idx = 0;
  mX = coordinates[idx++].toDouble();
  mY = coordinates[idx++].toDouble();
  if ( is3D() && coordinates.length() > 2 )
    mZ = coordinates[idx++].toDouble();
  if ( isMeasure() && coordinates.length() > 2 + is3D() )
    mM = coordinates[idx++].toDouble();

  return true;
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

QByteArray QgsPoint::asWkb() const
{
  int binarySize = sizeof( char ) + sizeof( quint32 );
  binarySize += ( 2 + is3D() + isMeasure() ) * sizeof( double );

  QByteArray wkbArray;
  wkbArray.resize( binarySize );
  QgsWkbPtr wkb( wkbArray );
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
  return wkbArray;
}

QString QgsPoint::asWkt( int precision ) const
{
  QString wkt = wktTypeStr() + " (";
  wkt += qgsDoubleToString( mX, precision ) + ' ' + qgsDoubleToString( mY, precision );
  if ( is3D() )
    wkt += ' ' + qgsDoubleToString( mZ, precision );
  if ( isMeasure() )
    wkt += ' ' + qgsDoubleToString( mM, precision );
  wkt += ')';
  return wkt;
}

QDomElement QgsPoint::asGML2( QDomDocument &doc, int precision, const QString &ns ) const
{
  QDomElement elemPoint = doc.createElementNS( ns, QStringLiteral( "Point" ) );
  QDomElement elemCoordinates = doc.createElementNS( ns, QStringLiteral( "coordinates" ) );
  QString strCoordinates = qgsDoubleToString( mX, precision ) + ',' + qgsDoubleToString( mY, precision );
  elemCoordinates.appendChild( doc.createTextNode( strCoordinates ) );
  elemPoint.appendChild( elemCoordinates );
  return elemPoint;
}

QDomElement QgsPoint::asGML3( QDomDocument &doc, int precision, const QString &ns ) const
{
  QDomElement elemPoint = doc.createElementNS( ns, QStringLiteral( "Point" ) );
  QDomElement elemPosList = doc.createElementNS( ns, QStringLiteral( "pos" ) );
  elemPosList.setAttribute( QStringLiteral( "srsDimension" ), is3D() ? 3 : 2 );
  QString strCoordinates = qgsDoubleToString( mX, precision ) + ' ' + qgsDoubleToString( mY, precision );
  if ( is3D() )
    strCoordinates += ' ' + qgsDoubleToString( mZ, precision );

  elemPosList.appendChild( doc.createTextNode( strCoordinates ) );
  elemPoint.appendChild( elemPosList );
  return elemPoint;
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

QString QgsPoint::asJSON( int precision ) const
{
  return "{\"type\": \"Point\", \"coordinates\": ["
         + qgsDoubleToString( mX, precision ) + ", " + qgsDoubleToString( mY, precision )
         + "]}";
}

void QgsPoint::draw( QPainter &p ) const
{
  p.drawRect( mX - 2, mY - 2, 4, 4 );
}

void QgsPoint::clear()
{
  mX = mY = mZ = mM = 0.;
  clearCache();
}

void QgsPoint::transform( const QgsCoordinateTransform &ct, QgsCoordinateTransform::TransformDirection d, bool transformZ )
{
  clearCache();
  if ( transformZ )
  {
    ct.transformInPlace( mX, mY, mZ, d );
  }
  else
  {
    double z = 0.0;
    ct.transformInPlace( mX, mY, z, d );
  }
}

QgsCoordinateSequence QgsPoint::coordinateSequence() const
{
  QgsCoordinateSequence cs;

  cs.append( QgsRingSequence() );
  cs.back().append( QgsPointSequence() << QgsPoint( *this ) );

  return cs;
}

QgsAbstractGeometry *QgsPoint::boundary() const
{
  return nullptr;
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

bool QgsPoint::moveVertex( QgsVertexId position, const QgsPoint &newPos )
{
  Q_UNUSED( position );
  clearCache();
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
  return true;
}

double QgsPoint::closestSegment( const QgsPoint &pt, QgsPoint &segmentPt,  QgsVertexId &vertexAfter, bool *leftOf, double epsilon ) const
{
  Q_UNUSED( pt );
  Q_UNUSED( segmentPt );
  Q_UNUSED( vertexAfter );
  Q_UNUSED( leftOf );
  Q_UNUSED( epsilon );
  return -1;  // no segments - return error
}

bool QgsPoint::nextVertex( QgsVertexId &id, QgsPoint &vertex ) const
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

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

bool QgsPoint::addZValue( double zValue )
{
  if ( QgsWkbTypes::hasZ( mWkbType ) )
    return false;

  mWkbType = QgsWkbTypes::addZ( mWkbType );
  mZ = zValue;
  clearCache();
  return true;
}

bool QgsPoint::addMValue( double mValue )
{
  if ( QgsWkbTypes::hasM( mWkbType ) )
    return false;

  mWkbType = QgsWkbTypes::addM( mWkbType );
  mM = mValue;
  clearCache();
  return true;
}

void QgsPoint::transform( const QTransform &t )
{
  clearCache();
  qreal x, y;
  t.map( mX, mY, &x, &y );
  mX = x;
  mY = y;
}


bool QgsPoint::dropZValue()
{
  if ( !is3D() )
    return false;

  mWkbType = QgsWkbTypes::dropZ( mWkbType );
  mZ = 0.0;
  clearCache();
  return true;
}

bool QgsPoint::dropMValue()
{
  if ( !isMeasure() )
    return false;

  mWkbType = QgsWkbTypes::dropM( mWkbType );
  mM = 0.0;
  clearCache();
  return true;
}

bool QgsPoint::convertTo( QgsWkbTypes::Type type )
{
  if ( type == mWkbType )
    return true;

  clearCache();

  switch ( type )
  {
    case QgsWkbTypes::Point:
      mZ = 0.0;
      mM = 0.0;
      mWkbType = type;
      return true;
    case QgsWkbTypes::PointZ:
    case QgsWkbTypes::Point25D:
      mM = 0.0;
      mWkbType = type;
      return true;
    case QgsWkbTypes::PointM:
      mZ = 0.0;
      mWkbType = type;
      return true;
    case QgsWkbTypes::PointZM:
      mWkbType = type;
      return true;
    default:
      break;
  }

  return false;
}


QPointF QgsPoint::toQPointF() const
{
  return QPointF( mX, mY );
}

double QgsPoint::distance( double x, double y ) const
{
  return sqrt( ( mX - x ) * ( mX - x ) + ( mY - y ) * ( mY - y ) );
}

double QgsPoint::distance( const QgsPoint &other ) const
{
  return sqrt( ( mX - other.x() ) * ( mX - other.x() ) + ( mY - other.y() ) * ( mY - other.y() ) );
}

double QgsPoint::distanceSquared( double x, double y ) const
{
  return ( mX - x ) * ( mX - x ) + ( mY - y ) * ( mY - y );
}

double QgsPoint::distanceSquared( const QgsPoint &other ) const
{
  return ( mX - other.x() ) * ( mX - other.x() ) + ( mY - other.y() ) * ( mY - other.y() ) ;
}

double QgsPoint::distance3D( double x, double y, double z ) const
{
  return sqrt( ( mX - x ) * ( mX - x ) + ( mY - y ) * ( mY - y ) + ( mZ - z ) * ( mZ - z ) );
}

double QgsPoint::distance3D( const QgsPoint &other ) const
{
  return sqrt( ( mX - other.x() ) * ( mX - other.x() ) + ( mY - other.y() ) * ( mY - other.y() ) + ( mZ - other.z() ) * ( mZ - other.z() ) );
}

double QgsPoint::distanceSquared3D( double x, double y, double z ) const
{
  return ( mX - x ) * ( mX - x ) + ( mY - y ) * ( mY - y ) + ( mZ - z ) * ( mZ - z );
}

double QgsPoint::distanceSquared3D( const QgsPoint &other ) const
{
  return ( mX - other.x() ) * ( mX - other.x() ) + ( mY - other.y() ) * ( mY - other.y() ) + ( mZ - other.z() ) * ( mZ - other.z() );
}

double QgsPoint::azimuth( const QgsPoint &other ) const
{
  double dx = other.x() - mX;
  double dy = other.y() - mY;
  return ( atan2( dx, dy ) * 180.0 / M_PI );
}

double QgsPoint::inclination( const QgsPoint &other ) const
{
  double distance = distance3D( other );
  if ( qgsDoubleNear( distance, 0.0 ) )
  {
    return 90.0;
  }
  double dz = other.z() - mZ;

  return ( acos( dz / distance ) * 180.0 / M_PI );
}

QgsPoint QgsPoint::project( double distance, double azimuth, double inclination ) const
{
  QgsWkbTypes::Type pType( QgsWkbTypes::Point );

  double radsXy = azimuth * M_PI / 180.0;
  double dx = 0.0, dy = 0.0, dz = 0.0;

  inclination = fmod( inclination, 360.0 );

  if ( !is3D() && qgsDoubleNear( inclination, 90.0 ) )
  {
    dx = distance * sin( radsXy );
    dy = distance * cos( radsXy );
  }
  else
  {
    pType = QgsWkbTypes::addZ( pType );
    double radsZ = inclination * M_PI / 180.0;
    dx = distance * sin( radsZ ) * sin( radsXy );
    dy = distance * sin( radsZ ) * cos( radsXy );
    dz = distance * cos( radsZ );
  }

  if ( isMeasure() )
  {
    pType = QgsWkbTypes::addM( pType );
  }

  return QgsPoint( mX + dx, mY + dy, mZ + dz, mM, pType );
}
