/***************************************************************************
                        qgsgeometrycollectionv2.cpp
  -------------------------------------------------------------------
Date                 : 28 Oct 2014
Copyright            : (C) 2014 by Marco Hugentobler
email                : marco.hugentobler at sourcepole dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeometrycollectionv2.h"
#include "qgsapplication.h"
#include "qgsgeometryfactory.h"
#include "qgsgeometryutils.h"
#include "qgscircularstringv2.h"
#include "qgscompoundcurvev2.h"
#include "qgslinestringv2.h"
#include "qgsmultilinestringv2.h"
#include "qgspointv2.h"
#include "qgsmultipointv2.h"
#include "qgspolygonv2.h"
#include "qgsmultipolygonv2.h"
#include "qgswkbptr.h"

QgsGeometryCollectionV2::QgsGeometryCollectionV2(): QgsAbstractGeometryV2()
{
  mWkbType = QgsWKBTypes::GeometryCollection;
}

QgsGeometryCollectionV2::QgsGeometryCollectionV2( const QgsGeometryCollectionV2& c ): QgsAbstractGeometryV2( c )
{
  int nGeoms = c.mGeometries.size();
  mGeometries.resize( nGeoms );
  for ( int i = 0; i < nGeoms; ++i )
  {
    mGeometries[i] = c.mGeometries.at( i )->clone();
  }
}

QgsGeometryCollectionV2& QgsGeometryCollectionV2::operator=( const QgsGeometryCollectionV2 & c )
{
  if ( &c != this )
  {
    clearCache();
    QgsAbstractGeometryV2::operator=( c );
    int nGeoms = c.mGeometries.size();
    mGeometries.resize( nGeoms );
    for ( int i = 0; i < nGeoms; ++i )
    {
      mGeometries[i] = c.mGeometries.at( i )->clone();
    }
  }
  return *this;
}

QgsGeometryCollectionV2::~QgsGeometryCollectionV2()
{
  clear();
}

QgsGeometryCollectionV2 *QgsGeometryCollectionV2::clone() const
{
  return new QgsGeometryCollectionV2( *this );
}

void QgsGeometryCollectionV2::clear()
{
  qDeleteAll( mGeometries );
  mGeometries.clear();
  clearCache(); //set bounding box invalid
}

int QgsGeometryCollectionV2::numGeometries() const
{
  return mGeometries.size();
}

const QgsAbstractGeometryV2* QgsGeometryCollectionV2::geometryN( int n ) const
{
  return mGeometries.value( n );
}

QgsAbstractGeometryV2* QgsGeometryCollectionV2::geometryN( int n )
{
  clearCache();
  return mGeometries.value( n );
}

bool QgsGeometryCollectionV2::addGeometry( QgsAbstractGeometryV2* g )
{
  if ( !g )
  {
    return false;
  }

  mGeometries.append( g );
  clearCache(); //set bounding box invalid
  return true;
}

bool QgsGeometryCollectionV2::insertGeometry( QgsAbstractGeometryV2 *g, int index )
{
  if ( !g )
  {
    return false;
  }

  mGeometries.insert( index, g );
  clearCache(); //set bounding box invalid
  return true;
}

bool QgsGeometryCollectionV2::removeGeometry( int nr )
{
  if ( nr >= mGeometries.size() || nr < 0 )
  {
    return false;
  }
  delete mGeometries.at( nr );
  mGeometries.remove( nr );
  clearCache(); //set bounding box invalid
  return true;
}

int QgsGeometryCollectionV2::dimension() const
{
  int maxDim = 0;
  QVector< QgsAbstractGeometryV2* >::const_iterator it = mGeometries.constBegin();
  for ( ; it != mGeometries.constEnd(); ++it )
  {
    int dim = ( *it )->dimension();
    if ( dim > maxDim )
    {
      maxDim = dim;
    }
  }
  return maxDim;
}

void QgsGeometryCollectionV2::transform( const QgsCoordinateTransform& ct, QgsCoordinateTransform::TransformDirection d, bool transformZ )
{
  Q_FOREACH ( QgsAbstractGeometryV2* g, mGeometries )
  {
    g->transform( ct, d, transformZ );
  }
  clearCache(); //set bounding box invalid
}

void QgsGeometryCollectionV2::transform( const QTransform& t )
{
  Q_FOREACH ( QgsAbstractGeometryV2* g, mGeometries )
  {
    g->transform( t );
  }
  clearCache(); //set bounding box invalid
}

#if 0
void QgsGeometryCollectionV2::clip( const QgsRectangle& rect )
{
  QVector< QgsAbstractGeometryV2* >::iterator it = mGeometries.begin();
  for ( ; it != mGeometries.end(); ++it )
  {
    ( *it )->clip( rect );
  }
}
#endif

void QgsGeometryCollectionV2::draw( QPainter& p ) const
{
  QVector< QgsAbstractGeometryV2* >::const_iterator it = mGeometries.constBegin();
  for ( ; it != mGeometries.constEnd(); ++it )
  {
    ( *it )->draw( p );
  }
}

bool QgsGeometryCollectionV2::fromWkb( QgsConstWkbPtr wkbPtr )
{
  if ( !wkbPtr )
  {
    return false;
  }

  mWkbType = wkbPtr.readHeader();

  int nGeometries = 0;
  wkbPtr >> nGeometries;

  QVector<QgsAbstractGeometryV2*> geometryListBackup = mGeometries;
  mGeometries.clear();
  for ( int i = 0; i < nGeometries; ++i )
  {
    QgsAbstractGeometryV2* geom = QgsGeometryFactory::geomFromWkb( wkbPtr );
    if ( geom )
    {
      if ( !addGeometry( geom ) )
      {
        qDeleteAll( mGeometries );
        mGeometries = geometryListBackup;
        return false;
      }
      wkbPtr += geom->wkbSize();
    }
  }
  qDeleteAll( geometryListBackup );

  clearCache(); //set bounding box invalid

  return true;
}

bool QgsGeometryCollectionV2::fromWkt( const QString& wkt )
{
  return fromCollectionWkt( wkt, QList<QgsAbstractGeometryV2*>() << new QgsPointV2 << new QgsLineStringV2 << new QgsPolygonV2
                            << new QgsCircularStringV2 << new QgsCompoundCurveV2
                            << new QgsCurvePolygonV2
                            << new QgsMultiPointV2 << new QgsMultiLineStringV2
                            << new QgsMultiPolygonV2 << new QgsGeometryCollectionV2
                            << new QgsMultiCurveV2 << new QgsMultiSurfaceV2, "GeometryCollection" );
}

int QgsGeometryCollectionV2::wkbSize() const
{
  int size = sizeof( char ) + sizeof( quint32 ) + sizeof( quint32 );
  Q_FOREACH ( const QgsAbstractGeometryV2 *geom, mGeometries )
  {
    if ( geom )
    {
      size += geom->wkbSize();
    }
  }
  return size;
}

unsigned char* QgsGeometryCollectionV2::asWkb( int& binarySize ) const
{
  binarySize = wkbSize();
  unsigned char* geomPtr = new unsigned char[binarySize];
  QgsWkbPtr wkb( geomPtr, binarySize );
  wkb << static_cast<char>( QgsApplication::endian() );
  wkb << static_cast<quint32>( wkbType() );
  wkb << static_cast<quint32>( mGeometries.size() );
  Q_FOREACH ( const QgsAbstractGeometryV2 *geom, mGeometries )
  {
    int geomWkbLen = 0;
    if ( geom )
    {
      unsigned char* geomWkb = geom->asWkb( geomWkbLen );
      memcpy( wkb, geomWkb, geomWkbLen );
      wkb += geomWkbLen;
      delete[] geomWkb;
    }
  }
  return geomPtr;
}

QString QgsGeometryCollectionV2::asWkt( int precision ) const
{
  QString wkt = wktTypeStr() + " (";
  Q_FOREACH ( const QgsAbstractGeometryV2 *geom, mGeometries )
  {
    QString childWkt = geom->asWkt( precision );
    if ( wktOmitChildType() )
    {
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

QDomElement QgsGeometryCollectionV2::asGML2( QDomDocument& doc, int precision, const QString& ns ) const
{
  QDomElement elemMultiGeometry = doc.createElementNS( ns, "MultiGeometry" );
  Q_FOREACH ( const QgsAbstractGeometryV2 *geom, mGeometries )
  {
    QDomElement elemGeometryMember = doc.createElementNS( ns, "geometryMember" );
    elemGeometryMember.appendChild( geom->asGML2( doc, precision, ns ) );
    elemMultiGeometry.appendChild( elemGeometryMember );
  }
  return elemMultiGeometry;
}

QDomElement QgsGeometryCollectionV2::asGML3( QDomDocument& doc, int precision, const QString& ns ) const
{
  QDomElement elemMultiGeometry = doc.createElementNS( ns, "MultiGeometry" );
  Q_FOREACH ( const QgsAbstractGeometryV2 *geom, mGeometries )
  {
    QDomElement elemGeometryMember = doc.createElementNS( ns, "geometryMember" );
    elemGeometryMember.appendChild( geom->asGML3( doc, precision, ns ) );
    elemMultiGeometry.appendChild( elemGeometryMember );
  }
  return elemMultiGeometry;
}

QString QgsGeometryCollectionV2::asJSON( int precision ) const
{
  QString json = "{\"type\": \"GeometryCollection\", \"geometries\": [";
  Q_FOREACH ( const QgsAbstractGeometryV2 *geom, mGeometries )
  {
    json += geom->asJSON( precision ) + ", ";
  }
  if ( json.endsWith( ", " ) )
  {
    json.chop( 2 ); // Remove last ", "
  }
  json += "] }";
  return json;
}

QgsRectangle QgsGeometryCollectionV2::boundingBox() const
{
  if ( mBoundingBox.isNull() )
  {
    mBoundingBox = calculateBoundingBox();
  }
  return mBoundingBox;
}

QgsRectangle QgsGeometryCollectionV2::calculateBoundingBox() const
{
  if ( mGeometries.size() < 1 )
  {
    return QgsRectangle();
  }

  QgsRectangle bbox = mGeometries.at( 0 )->boundingBox();
  for ( int i = 1; i < mGeometries.size(); ++i )
  {
    QgsRectangle geomBox = mGeometries.at( i )->boundingBox();
    bbox.combineExtentWith( geomBox );
  }
  return bbox;
}

QgsCoordinateSequenceV2 QgsGeometryCollectionV2::coordinateSequence() const
{
  if ( !mCoordinateSequence.isEmpty() )
    return mCoordinateSequence;

  QVector< QgsAbstractGeometryV2* >::const_iterator geomIt = mGeometries.constBegin();
  for ( ; geomIt != mGeometries.constEnd(); ++geomIt )
  {
    QgsCoordinateSequenceV2 geomCoords = ( *geomIt )->coordinateSequence();

    QgsCoordinateSequenceV2::const_iterator cIt = geomCoords.constBegin();
    for ( ; cIt != geomCoords.constEnd(); ++cIt )
    {
      mCoordinateSequence.push_back( *cIt );
    }
  }

  return mCoordinateSequence;
}

double QgsGeometryCollectionV2::closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const
{
  return QgsGeometryUtils::closestSegmentFromComponents( mGeometries, QgsGeometryUtils::PART, pt, segmentPt, vertexAfter, leftOf, epsilon );
}

bool QgsGeometryCollectionV2::nextVertex( QgsVertexId& id, QgsPointV2& vertex ) const
{
  if ( id.part < 0 )
  {
    id.part = 0;
    id.ring = -1;
    id.vertex = -1;
  }
  if ( mGeometries.isEmpty() )
  {
    return false;
  }

  QgsAbstractGeometryV2* geom = mGeometries.at( id.part );
  if ( geom->nextVertex( id, vertex ) )
  {
    return true;
  }
  if (( id.part + 1 ) >= numGeometries() )
  {
    return false;
  }
  ++id.part;
  id.ring = -1;
  id.vertex = -1;
  return mGeometries.at( id.part )->nextVertex( id, vertex );
}

bool QgsGeometryCollectionV2::insertVertex( QgsVertexId position, const QgsPointV2& vertex )
{
  if ( position.part >= mGeometries.size() )
  {
    return false;
  }

  bool success = mGeometries.at( position.part )->insertVertex( position, vertex );
  if ( success )
  {
    clearCache(); //set bounding box invalid
  }
  return success;
}

bool QgsGeometryCollectionV2::moveVertex( QgsVertexId position, const QgsPointV2& newPos )
{
  if ( position.part >= mGeometries.size() )
  {
    return false;
  }

  bool success = mGeometries.at( position.part )->moveVertex( position, newPos );
  if ( success )
  {
    clearCache(); //set bounding box invalid
  }
  return success;
}

bool QgsGeometryCollectionV2::deleteVertex( QgsVertexId position )
{
  if ( position.part >= mGeometries.size() )
  {
    return false;
  }

  QgsAbstractGeometryV2* geom = mGeometries.at( position.part );
  if ( !geom )
  {
    return false;
  }

  bool success = geom->deleteVertex( position );

  //remove geometry if no vertices left
  if ( geom->isEmpty() )
  {
    removeGeometry( position.part );
  }

  if ( success )
  {
    clearCache(); //set bounding box invalid
  }
  return success;
}

double QgsGeometryCollectionV2::length() const
{
  double length = 0.0;
  QVector< QgsAbstractGeometryV2* >::const_iterator geomIt = mGeometries.constBegin();
  for ( ; geomIt != mGeometries.constEnd(); ++geomIt )
  {
    length += ( *geomIt )->length();
  }
  return length;
}

double QgsGeometryCollectionV2::area() const
{
  double area = 0.0;
  QVector< QgsAbstractGeometryV2* >::const_iterator geomIt = mGeometries.constBegin();
  for ( ; geomIt != mGeometries.constEnd(); ++geomIt )
  {
    area += ( *geomIt )->area();
  }
  return area;
}

double QgsGeometryCollectionV2::perimeter() const
{
  double perimeter = 0.0;
  QVector< QgsAbstractGeometryV2* >::const_iterator geomIt = mGeometries.constBegin();
  for ( ; geomIt != mGeometries.constEnd(); ++geomIt )
  {
    perimeter += ( *geomIt )->perimeter();
  }
  return perimeter;
}

bool QgsGeometryCollectionV2::fromCollectionWkt( const QString &wkt, const QList<QgsAbstractGeometryV2*>& subtypes, const QString& defaultChildWkbType )
{
  clear();

  QPair<QgsWKBTypes::Type, QString> parts = QgsGeometryUtils::wktReadBlock( wkt );

  if ( QgsWKBTypes::flatType( parts.first ) != QgsWKBTypes::flatType( wkbType() ) )
    return false;
  mWkbType = parts.first;

  QString defChildWkbType = QString( "%1%2%3 " ).arg( defaultChildWkbType, is3D() ? "Z" : "", isMeasure() ? "M" : "" );

  Q_FOREACH ( const QString& childWkt, QgsGeometryUtils::wktGetChildBlocks( parts.second, defChildWkbType ) )
  {
    QPair<QgsWKBTypes::Type, QString> childParts = QgsGeometryUtils::wktReadBlock( childWkt );

    bool success = false;
    Q_FOREACH ( const QgsAbstractGeometryV2* geom, subtypes )
    {
      if ( QgsWKBTypes::flatType( childParts.first ) == QgsWKBTypes::flatType( geom->wkbType() ) )
      {
        mGeometries.append( geom->clone() );
        if ( mGeometries.back()->fromWkt( childWkt ) )
        {
          success = true;
          break;
        }
      }
    }
    if ( !success )
    {
      clear();
      qDeleteAll( subtypes );
      return false;
    }
  }
  qDeleteAll( subtypes );

  //scan through geometries and check if dimensionality of geometries is different to collection.
  //if so, update the type dimensionality of the collection to match
  bool hasZ = false;
  bool hasM = false;
  Q_FOREACH ( QgsAbstractGeometryV2* geom, mGeometries )
  {
    hasZ = hasZ || geom->is3D();
    hasM = hasM || geom->isMeasure();
    if ( hasZ && hasM )
      break;
  }
  if ( hasZ )
    addZValue( 0 );
  if ( hasM )
    addMValue( 0 );

  return true;
}

bool QgsGeometryCollectionV2::hasCurvedSegments() const
{
  QVector< QgsAbstractGeometryV2* >::const_iterator it = mGeometries.constBegin();
  for ( ; it != mGeometries.constEnd(); ++it )
  {
    if (( *it )->hasCurvedSegments() )
    {
      return true;
    }
  }
  return false;
}

QgsAbstractGeometryV2* QgsGeometryCollectionV2::segmentize( double tolerance, SegmentationToleranceType toleranceType ) const
{
  QgsAbstractGeometryV2* geom = QgsGeometryFactory::geomFromWkbType( mWkbType );
  QgsGeometryCollectionV2* geomCollection = dynamic_cast<QgsGeometryCollectionV2*>( geom );
  if ( !geomCollection )
  {
    delete geom;
    return clone();
  }

  QVector< QgsAbstractGeometryV2* >::const_iterator geomIt = mGeometries.constBegin();
  for ( ; geomIt != mGeometries.constEnd(); ++geomIt )
  {
    geomCollection->addGeometry(( *geomIt )->segmentize( tolerance, toleranceType ) );
  }
  return geomCollection;
}

double QgsGeometryCollectionV2::vertexAngle( QgsVertexId vertex ) const
{
  if ( vertex.part >= mGeometries.size() )
  {
    return 0.0;
  }

  QgsAbstractGeometryV2* geom = mGeometries[vertex.part];
  if ( !geom )
  {
    return 0.0;
  }

  return geom->vertexAngle( vertex );
}

bool QgsGeometryCollectionV2::addZValue( double zValue )
{
  if ( QgsWKBTypes::hasZ( mWkbType ) )
    return false;

  mWkbType = QgsWKBTypes::addZ( mWkbType );

  Q_FOREACH ( QgsAbstractGeometryV2* geom, mGeometries )
  {
    geom->addZValue( zValue );
  }
  clearCache();
  return true;
}

bool QgsGeometryCollectionV2::addMValue( double mValue )
{
  if ( QgsWKBTypes::hasM( mWkbType ) )
    return false;

  mWkbType = QgsWKBTypes::addM( mWkbType );

  Q_FOREACH ( QgsAbstractGeometryV2* geom, mGeometries )
  {
    geom->addMValue( mValue );
  }
  clearCache();
  return true;
}


bool QgsGeometryCollectionV2::dropZValue()
{
  if ( !is3D() )
    return false;

  mWkbType = QgsWKBTypes::dropZ( mWkbType );
  Q_FOREACH ( QgsAbstractGeometryV2* geom, mGeometries )
  {
    geom->dropZValue();
  }
  clearCache();
  return true;
}

bool QgsGeometryCollectionV2::dropMValue()
{
  if ( !isMeasure() )
    return false;

  mWkbType = QgsWKBTypes::dropM( mWkbType );
  Q_FOREACH ( QgsAbstractGeometryV2* geom, mGeometries )
  {
    geom->dropMValue();
  }
  clearCache();
  return true;
}
