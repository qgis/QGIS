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
#include "qgsgeometrytransformer.h"

#include <cmath>
#include <QPainter>
#include <QPainterPath>
#include <QRegularExpression>
#include <QJsonObject>
#include <QJsonArray>
#include <nlohmann/json.hpp>

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

QgsPoint::QgsPoint( double x, double y, double z, double m, QgsWkbTypes::Type wkbType )
  : mX( x )
  , mY( y )
  , mZ( z )
  , mM( m )
{
  if ( wkbType != QgsWkbTypes::Unknown )
  {
    Q_ASSERT( QgsWkbTypes::flatType( wkbType ) == QgsWkbTypes::Point );
    mWkbType = wkbType;
  }
  else if ( std::isnan( z ) )
  {
    if ( std::isnan( m ) )
      mWkbType = QgsWkbTypes::Point;
    else
      mWkbType = QgsWkbTypes::PointM;
  }
  else if ( std::isnan( m ) )
    mWkbType = QgsWkbTypes::PointZ;
  else
    mWkbType = QgsWkbTypes::PointZM;
}

QgsPoint::QgsPoint( const QgsPointXY &p )
  : mX( p.x() )
  , mY( p.y() )
  , mZ( std::numeric_limits<double>::quiet_NaN() )
  , mM( std::numeric_limits<double>::quiet_NaN() )
{
  mWkbType = QgsWkbTypes::Point;
  if ( p.isEmpty() )
  {
    mX = std::numeric_limits<double>::quiet_NaN();
    mY = std::numeric_limits<double>::quiet_NaN();
  }
}

QgsPoint::QgsPoint( QPointF p )
  : mX( p.x() )
  , mY( p.y() )
  , mZ( std::numeric_limits<double>::quiet_NaN() )
  , mM( std::numeric_limits<double>::quiet_NaN() )
{
  mWkbType = QgsWkbTypes::Point;
}

QgsPoint::QgsPoint( QgsWkbTypes::Type wkbType, double x, double y, double z, double m )
  : mX( x )
  , mY( y )
  , mZ( QgsWkbTypes::hasZ( wkbType ) ? z : std::numeric_limits<double>::quiet_NaN() )
  , mM( QgsWkbTypes::hasM( wkbType ) ? m : std::numeric_limits<double>::quiet_NaN() )
{
  Q_ASSERT( QgsWkbTypes::flatType( wkbType ) == QgsWkbTypes::Point );
  mWkbType = wkbType;
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

QgsPoint *QgsPoint::clone() const
{
  return new QgsPoint( *this );
}

QgsPoint *QgsPoint::snappedToGrid( double hSpacing, double vSpacing, double dSpacing, double mSpacing ) const
{
  // helper function
  auto gridifyValue = []( double value, double spacing, bool extraCondition = true ) -> double
  {
    if ( spacing > 0 && extraCondition )
      return  std::round( value / spacing ) * spacing;
    else
      return value;
  };

  // Get the new values
  const auto x = gridifyValue( mX, hSpacing );
  const auto y = gridifyValue( mY, vSpacing );
  const auto z = gridifyValue( mZ, dSpacing, QgsWkbTypes::hasZ( mWkbType ) );
  const auto m = gridifyValue( mM, mSpacing, QgsWkbTypes::hasM( mWkbType ) );

  // return the new object
  return new QgsPoint( mWkbType, x, y, z, m );
}

bool QgsPoint::removeDuplicateNodes( double, bool )
{
  return false;
}

bool QgsPoint::fromWkb( QgsConstWkbPtr &wkbPtr )
{
  const QgsWkbTypes::Type type = wkbPtr.readHeader();
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

  QString secondWithoutParentheses = parts.second;
  secondWithoutParentheses = secondWithoutParentheses.remove( '(' ).remove( ')' ).simplified().remove( ' ' );
  parts.second = parts.second.remove( '(' ).remove( ')' );
  if ( ( parts.second.compare( QLatin1String( "EMPTY" ), Qt::CaseInsensitive ) == 0 ) ||
       secondWithoutParentheses.isEmpty() )
    return true;

  const thread_local QRegularExpression rx( QStringLiteral( "\\s" ) );
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
  QStringList coordinates = parts.second.split( rx, QString::SkipEmptyParts );
#else
  QStringList coordinates = parts.second.split( rx, Qt::SkipEmptyParts );
#endif

  // So far the parser hasn't looked at the coordinates. We'll avoid having anything but numbers and return NULL instead of 0 as a coordinate.
  // Without this check, "POINT (a, b)" or "POINT (( 4, 3 ))" returned "POINT (0 ,0)"
  // And some strange conversion...
  // .. python:
  // p = QgsPoint()
  // p.fromWkt("POINT (-3.12, -4.2")
  // False
  // p.fromWkt( "POINT (-5.1234, -1.4321)" )
  // True
  // p.asWkt()
  // 'Point (0 -1.43209999999999993)'
  const thread_local QRegularExpression rxIsNumber( QStringLiteral( "^[+-]?(\\d\\.?\\d*[Ee][+\\-]?\\d+|(\\d+\\.\\d*|\\d*\\.\\d+)|\\d+)$" ) );
  if ( coordinates.filter( rxIsNumber ).size() != coordinates.size() )
    return false;

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

int QgsPoint::wkbSize( WkbFlags ) const
{
  int binarySize = sizeof( char ) + sizeof( quint32 );
  binarySize += ( 2 + is3D() + isMeasure() ) * sizeof( double );
  return binarySize;
}

QByteArray QgsPoint::asWkb( WkbFlags flags ) const
{
  QByteArray wkbArray;
  wkbArray.resize( QgsPoint::wkbSize( flags ) );
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
  QString wkt = wktTypeStr();

  if ( isEmpty() )
    wkt += QLatin1String( " EMPTY" );
  else
  {
    wkt += QLatin1String( " (" );
    wkt += qgsDoubleToString( mX, precision ) + ' ' + qgsDoubleToString( mY, precision );
    if ( is3D() )
      wkt += ' ' + qgsDoubleToString( mZ, precision );
    if ( isMeasure() )
      wkt += ' ' + qgsDoubleToString( mM, precision );
    wkt += ')';
  }
  return wkt;
}

QDomElement QgsPoint::asGml2( QDomDocument &doc, int precision, const QString &ns, const QgsAbstractGeometry::AxisOrder axisOrder ) const
{
  QDomElement elemPoint = doc.createElementNS( ns, QStringLiteral( "Point" ) );
  QDomElement elemCoordinates = doc.createElementNS( ns, QStringLiteral( "coordinates" ) );

  // coordinate separator
  const QString cs = QStringLiteral( "," );
  // tuple separator
  const QString ts = QStringLiteral( " " );

  elemCoordinates.setAttribute( QStringLiteral( "cs" ), cs );
  elemCoordinates.setAttribute( QStringLiteral( "ts" ), ts );

  QString strCoordinates;
  if ( axisOrder == QgsAbstractGeometry::AxisOrder::XY )
    strCoordinates = qgsDoubleToString( mX, precision ) + cs + qgsDoubleToString( mY, precision );
  else
    strCoordinates = qgsDoubleToString( mY, precision ) + cs + qgsDoubleToString( mX, precision );
  elemCoordinates.appendChild( doc.createTextNode( strCoordinates ) );
  elemPoint.appendChild( elemCoordinates );
  return elemPoint;
}

QDomElement QgsPoint::asGml3( QDomDocument &doc, int precision, const QString &ns, const QgsAbstractGeometry::AxisOrder axisOrder ) const
{
  QDomElement elemPoint = doc.createElementNS( ns, QStringLiteral( "Point" ) );
  QDomElement elemPosList = doc.createElementNS( ns, QStringLiteral( "pos" ) );
  elemPosList.setAttribute( QStringLiteral( "srsDimension" ), is3D() ? 3 : 2 );
  QString strCoordinates;
  if ( axisOrder == QgsAbstractGeometry::AxisOrder::XY )
    strCoordinates = qgsDoubleToString( mX, precision ) + ' ' + qgsDoubleToString( mY, precision );
  else
    strCoordinates = qgsDoubleToString( mY, precision ) + ' ' + qgsDoubleToString( mX, precision );
  if ( is3D() )
    strCoordinates += ' ' + qgsDoubleToString( mZ, precision );

  elemPosList.appendChild( doc.createTextNode( strCoordinates ) );
  elemPoint.appendChild( elemPosList );
  return elemPoint;
}


json QgsPoint::asJsonObject( int precision ) const
{
  json j
  {
    { "type", "Point" },
    { "coordinates", json::array() },
  };
  if ( ! isEmpty() )
  {
    j["coordinates"].push_back( qgsRound( mX, precision ) );
    j["coordinates"].push_back( qgsRound( mY, precision ) );
    if ( is3D() )
    {
      j["coordinates"].push_back( qgsRound( mZ, precision ) );
    }
  }
  return j;
}

QString QgsPoint::asKml( int precision ) const
{
  return QStringLiteral( "<Point><coordinates>%1,%2</coordinates></Point>" ).arg( qgsDoubleToString( mX, precision ), qgsDoubleToString( mY, precision ) );
}

void QgsPoint::draw( QPainter &p ) const
{
  p.drawRect( QRectF( mX - 2, mY - 2, 4, 4 ) );
}

QPainterPath QgsPoint::asQPainterPath() const
{
  return QPainterPath();
}

void QgsPoint::clear()
{
  mX = mY = std::numeric_limits<double>::quiet_NaN();
  if ( is3D() )
    mZ = 0.;
  else
    mZ = std::numeric_limits<double>::quiet_NaN();

  if ( isMeasure() )
    mM = 0.;
  else
    mM = std::numeric_limits<double>::quiet_NaN();

  clearCache();
}


/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

void QgsPoint::transform( const QgsCoordinateTransform &ct, Qgis::TransformDirection d, bool transformZ )
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

int QgsPoint::nCoordinates() const
{
  return 1;
}

int QgsPoint::vertexNumberFromVertexId( QgsVertexId id ) const
{
  if ( id.vertex != 0 )
    return -1;
  else
    return 0;
}

QgsAbstractGeometry *QgsPoint::boundary() const
{
  return nullptr;
}

bool QgsPoint::isValid( QString &, Qgis::GeometryValidityFlags ) const
{
  return true;
}

bool QgsPoint::insertVertex( QgsVertexId position, const QgsPoint &vertex )
{
  Q_UNUSED( position )
  Q_UNUSED( vertex )
  return false;
}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

bool QgsPoint::moveVertex( QgsVertexId position, const QgsPoint &newPos )
{
  Q_UNUSED( position )
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

bool QgsPoint::deleteVertex( QgsVertexId position )
{
  Q_UNUSED( position )
  return false;
}

double QgsPoint::closestSegment( const QgsPoint &pt, QgsPoint &segmentPt,  QgsVertexId &vertexAfter, int *leftOf, double epsilon ) const
{
  Q_UNUSED( pt )
  Q_UNUSED( segmentPt )
  Q_UNUSED( vertexAfter )
  if ( leftOf )
    *leftOf = 0;
  Q_UNUSED( epsilon )
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

void QgsPoint::adjacentVertices( QgsVertexId, QgsVertexId &previousVertex, QgsVertexId &nextVertex ) const
{
  previousVertex = QgsVertexId();
  nextVertex = QgsVertexId();
}

double QgsPoint::vertexAngle( QgsVertexId vertex ) const
{
  Q_UNUSED( vertex )
  return 0.0;
}

int QgsPoint::vertexCount( int, int ) const
{
  return 1;
}

int QgsPoint::ringCount( int ) const
{
  return 1;
}

int QgsPoint::partCount() const
{
  return 1;
}

QgsPoint QgsPoint::vertexAt( QgsVertexId ) const
{
  return *this;
}

QgsPoint *QgsPoint::toCurveType() const
{
  return clone();
}

double QgsPoint::segmentLength( QgsVertexId ) const
{
  return 0.0;
}

bool QgsPoint::boundingBoxIntersects( const QgsRectangle &rectangle ) const
{
  return rectangle.contains( mX, mY );
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

void QgsPoint::transform( const QTransform &t, double zTranslate, double zScale, double mTranslate, double mScale )
{
  clearCache();
  qreal x, y;
  t.map( mX, mY, &x, &y );
  mX = x;
  mY = y;

  if ( is3D() )
  {
    mZ = mZ * zScale + zTranslate;
  }
  if ( isMeasure() )
  {
    mM = mM * mScale + mTranslate;
  }
}


bool QgsPoint::dropZValue()
{
  if ( !is3D() )
    return false;

  mWkbType = QgsWkbTypes::dropZ( mWkbType );
  mZ = std::numeric_limits<double>::quiet_NaN();
  clearCache();
  return true;
}

bool QgsPoint::dropMValue()
{
  if ( !isMeasure() )
    return false;

  mWkbType = QgsWkbTypes::dropM( mWkbType );
  mM = std::numeric_limits<double>::quiet_NaN();
  clearCache();
  return true;
}

void QgsPoint::swapXy()
{
  std::swap( mX, mY );
  clearCache();
}

bool QgsPoint::convertTo( QgsWkbTypes::Type type )
{
  if ( type == mWkbType )
    return true;

  clearCache();

  switch ( type )
  {
    case QgsWkbTypes::Point:
      mZ = std::numeric_limits<double>::quiet_NaN();
      mM = std::numeric_limits<double>::quiet_NaN();
      mWkbType = type;
      return true;
    case QgsWkbTypes::PointZ:
    case QgsWkbTypes::Point25D:
      mM = std::numeric_limits<double>::quiet_NaN();
      mWkbType = type;
      return true;
    case QgsWkbTypes::PointM:
      mZ = std::numeric_limits<double>::quiet_NaN();
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

bool QgsPoint::transform( QgsAbstractGeometryTransformer *transformer, QgsFeedback * )
{
  if ( !transformer )
    return false;

  const bool res = transformer->transformPoint( mX, mY, mZ, mM );
  clearCache();
  return res;
}

void QgsPoint::filterVertices( const std::function<bool ( const QgsPoint & )> & )
{
  // no meaning for points
}

void QgsPoint::transformVertices( const std::function<QgsPoint( const QgsPoint & )> &transform )
{
  const QgsPoint res = transform( *this );
  mX = res.x();
  mY = res.y();
  if ( is3D() )
    mZ = res.z();
  if ( isMeasure() )
    mM = res.m();
  clearCache();
}

double QgsPoint::distance3D( double x, double y, double z ) const
{
  double zDistSquared = 0.0;
  if ( is3D() || !std::isnan( z ) )
    zDistSquared = ( mZ - z ) * ( mZ - z );

  return std::sqrt( ( mX - x ) * ( mX - x ) + ( mY - y ) * ( mY - y ) + zDistSquared );
}

double QgsPoint::distance3D( const QgsPoint &other ) const
{
  double zDistSquared = 0.0;
  if ( is3D() || other.is3D() )
    zDistSquared = ( mZ - other.z() ) * ( mZ - other.z() );

  return std::sqrt( ( mX - other.x() ) * ( mX - other.x() ) + ( mY - other.y() ) * ( mY - other.y() ) + zDistSquared );
}

double QgsPoint::distanceSquared3D( double x, double y, double z ) const
{
  double zDistSquared = 0.0;
  if ( is3D() || !std::isnan( z ) )
    zDistSquared = ( mZ - z ) * ( mZ - z );

  return ( mX - x ) * ( mX - x ) + ( mY - y ) * ( mY - y ) + zDistSquared;
}

double QgsPoint::distanceSquared3D( const QgsPoint &other ) const
{
  double zDistSquared = 0.0;
  if ( is3D() || other.is3D() )
    zDistSquared = ( mZ - other.z() ) * ( mZ - other.z() );

  return ( mX - other.x() ) * ( mX - other.x() ) + ( mY - other.y() ) * ( mY - other.y() ) + zDistSquared;
}

double QgsPoint::azimuth( const QgsPoint &other ) const
{
  const double dx = other.x() - mX;
  const double dy = other.y() - mY;
  return ( std::atan2( dx, dy ) * 180.0 / M_PI );
}

double QgsPoint::inclination( const QgsPoint &other ) const
{
  const double distance = distance3D( other );
  if ( qgsDoubleNear( distance, 0.0 ) )
  {
    return 90.0;
  }
  const double dz = other.z() - mZ;

  return ( std::acos( dz / distance ) * 180.0 / M_PI );
}

QgsPoint QgsPoint::project( double distance, double azimuth, double inclination ) const
{
  QgsWkbTypes::Type pType = mWkbType;
  const double radsXy = azimuth * M_PI / 180.0;
  double dx = 0.0, dy = 0.0, dz = 0.0;

  inclination = std::fmod( inclination, 360.0 );

  if ( !qgsDoubleNear( inclination, 90.0 ) )
    pType = QgsWkbTypes::addZ( pType );

  if ( !is3D() && qgsDoubleNear( inclination, 90.0 ) )
  {
    dx = distance * std::sin( radsXy );
    dy = distance * std::cos( radsXy );
  }
  else
  {
    const double radsZ = inclination * M_PI / 180.0;
    dx = distance * std::sin( radsZ ) * std::sin( radsXy );
    dy = distance * std::sin( radsZ ) * std::cos( radsXy );
    dz = distance * std::cos( radsZ );
  }

  return QgsPoint( mX + dx, mY + dy, mZ + dz, mM, pType );
}

void QgsPoint::normalize()
{
  // nothing to do
}

bool QgsPoint::isEmpty() const
{
  return std::isnan( mX ) || std::isnan( mY );
}

QgsRectangle QgsPoint::boundingBox() const
{
  return QgsRectangle( mX, mY, mX, mY );
}

QString QgsPoint::geometryType() const
{
  return QStringLiteral( "Point" );
}

int QgsPoint::dimension() const
{
  return 0;
}

int QgsPoint::childCount() const
{
  return 1;
}

QgsPoint QgsPoint::childPoint( int index ) const
{
  Q_ASSERT( index == 0 );
  return *this;
}

QgsPoint *QgsPoint::createEmptyWithSameType() const
{
  const double nan = std::numeric_limits<double>::quiet_NaN();
  return new QgsPoint( nan, nan, nan, nan, mWkbType );
}

int QgsPoint::compareToSameClass( const QgsAbstractGeometry *other ) const
{
  const QgsPoint *otherPoint = qgsgeometry_cast< const QgsPoint * >( other );
  if ( !otherPoint )
    return -1;

  if ( mX < otherPoint->mX )
  {
    return -1;
  }
  else if ( mX > otherPoint->mX )
  {
    return 1;
  }

  if ( mY < otherPoint->mY )
  {
    return -1;
  }
  else if ( mY > otherPoint->mY )
  {
    return 1;
  }

  if ( is3D() && !otherPoint->is3D() )
    return 1;
  else if ( !is3D() && otherPoint->is3D() )
    return -1;
  else if ( is3D() && otherPoint->is3D() )
  {
    if ( mZ < otherPoint->mZ )
    {
      return -1;
    }
    else if ( mZ > otherPoint->mZ )
    {
      return 1;
    }
  }

  if ( isMeasure() && !otherPoint->isMeasure() )
    return 1;
  else if ( !isMeasure() && otherPoint->isMeasure() )
    return -1;
  else if ( isMeasure() && otherPoint->isMeasure() )
  {
    if ( mM < otherPoint->mM )
    {
      return -1;
    }
    else if ( mM > otherPoint->mM )
    {
      return 1;
    }
  }

  return 0;
}
