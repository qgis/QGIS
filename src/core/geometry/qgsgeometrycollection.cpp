/***************************************************************************
                        qgsgeometrycollection.cpp
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

#include "qgsgeometrycollection.h"
#include "qgsapplication.h"
#include "qgsgeometryfactory.h"
#include "qgsgeometryutils.h"
#include "qgscircularstring.h"
#include "qgscompoundcurve.h"
#include "qgslinestring.h"
#include "qgsmultilinestring.h"
#include "qgspointv2.h"
#include "qgsmultipoint.h"
#include "qgspolygon.h"
#include "qgsmultipolygon.h"
#include "qgswkbptr.h"

QgsGeometryCollection::QgsGeometryCollection(): QgsAbstractGeometry()
{
  mWkbType = QgsWkbTypes::GeometryCollection;
}

QgsGeometryCollection::QgsGeometryCollection( const QgsGeometryCollection& c ): QgsAbstractGeometry( c )
{
  int nGeoms = c.mGeometries.size();
  mGeometries.resize( nGeoms );
  for ( int i = 0; i < nGeoms; ++i )
  {
    mGeometries[i] = c.mGeometries.at( i )->clone();
  }
}

QgsGeometryCollection& QgsGeometryCollection::operator=( const QgsGeometryCollection & c )
{
  if ( &c != this )
  {
    clearCache();
    QgsAbstractGeometry::operator=( c );
    int nGeoms = c.mGeometries.size();
    mGeometries.resize( nGeoms );
    for ( int i = 0; i < nGeoms; ++i )
    {
      mGeometries[i] = c.mGeometries.at( i )->clone();
    }
  }
  return *this;
}

QgsGeometryCollection::~QgsGeometryCollection()
{
  clear();
}

QgsGeometryCollection *QgsGeometryCollection::clone() const
{
  return new QgsGeometryCollection( *this );
}

void QgsGeometryCollection::clear()
{
  qDeleteAll( mGeometries );
  mGeometries.clear();
  clearCache(); //set bounding box invalid
}

QgsAbstractGeometry*QgsGeometryCollection::boundary() const
{
  return nullptr;
}

int QgsGeometryCollection::numGeometries() const
{
  return mGeometries.size();
}

const QgsAbstractGeometry* QgsGeometryCollection::geometryN( int n ) const
{
  return mGeometries.value( n );
}

QgsAbstractGeometry* QgsGeometryCollection::geometryN( int n )
{
  clearCache();
  return mGeometries.value( n );
}

bool QgsGeometryCollection::addGeometry( QgsAbstractGeometry* g )
{
  if ( !g )
  {
    return false;
  }

  mGeometries.append( g );
  clearCache(); //set bounding box invalid
  return true;
}

bool QgsGeometryCollection::insertGeometry( QgsAbstractGeometry *g, int index )
{
  if ( !g )
  {
    return false;
  }

  mGeometries.insert( index, g );
  clearCache(); //set bounding box invalid
  return true;
}

bool QgsGeometryCollection::removeGeometry( int nr )
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

int QgsGeometryCollection::dimension() const
{
  int maxDim = 0;
  QVector< QgsAbstractGeometry* >::const_iterator it = mGeometries.constBegin();
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

void QgsGeometryCollection::transform( const QgsCoordinateTransform& ct, QgsCoordinateTransform::TransformDirection d, bool transformZ )
{
  Q_FOREACH ( QgsAbstractGeometry* g, mGeometries )
  {
    g->transform( ct, d, transformZ );
  }
  clearCache(); //set bounding box invalid
}

void QgsGeometryCollection::transform( const QTransform& t )
{
  Q_FOREACH ( QgsAbstractGeometry* g, mGeometries )
  {
    g->transform( t );
  }
  clearCache(); //set bounding box invalid
}

#if 0
void QgsGeometryCollection::clip( const QgsRectangle& rect )
{
  QVector< QgsAbstractGeometry* >::iterator it = mGeometries.begin();
  for ( ; it != mGeometries.end(); ++it )
  {
    ( *it )->clip( rect );
  }
}
#endif

void QgsGeometryCollection::draw( QPainter& p ) const
{
  QVector< QgsAbstractGeometry* >::const_iterator it = mGeometries.constBegin();
  for ( ; it != mGeometries.constEnd(); ++it )
  {
    ( *it )->draw( p );
  }
}

bool QgsGeometryCollection::fromWkb( QgsConstWkbPtr wkbPtr )
{
  if ( !wkbPtr )
  {
    return false;
  }

  mWkbType = wkbPtr.readHeader();

  int nGeometries = 0;
  wkbPtr >> nGeometries;

  QVector<QgsAbstractGeometry*> geometryListBackup = mGeometries;
  mGeometries.clear();
  for ( int i = 0; i < nGeometries; ++i )
  {
    QgsAbstractGeometry* geom = QgsGeometryFactory::geomFromWkb( wkbPtr );
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

bool QgsGeometryCollection::fromWkt( const QString& wkt )
{
  return fromCollectionWkt( wkt, QList<QgsAbstractGeometry*>() << new QgsPointV2 << new QgsLineString << new QgsPolygonV2
                            << new QgsCircularString << new QgsCompoundCurve
                            << new QgsCurvePolygon
                            << new QgsMultiPointV2 << new QgsMultiLineString
                            << new QgsMultiPolygonV2 << new QgsGeometryCollection
                            << new QgsMultiCurve << new QgsMultiSurface, "GeometryCollection" );
}

int QgsGeometryCollection::wkbSize() const
{
  int size = sizeof( char ) + sizeof( quint32 ) + sizeof( quint32 );
  Q_FOREACH ( const QgsAbstractGeometry *geom, mGeometries )
  {
    if ( geom )
    {
      size += geom->wkbSize();
    }
  }
  return size;
}

unsigned char* QgsGeometryCollection::asWkb( int& binarySize ) const
{
  binarySize = wkbSize();
  unsigned char* geomPtr = new unsigned char[binarySize];
  QgsWkbPtr wkb( geomPtr, binarySize );
  wkb << static_cast<char>( QgsApplication::endian() );
  wkb << static_cast<quint32>( wkbType() );
  wkb << static_cast<quint32>( mGeometries.size() );
  Q_FOREACH ( const QgsAbstractGeometry *geom, mGeometries )
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

QString QgsGeometryCollection::asWkt( int precision ) const
{
  QString wkt = wktTypeStr() + " (";
  Q_FOREACH ( const QgsAbstractGeometry *geom, mGeometries )
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

QDomElement QgsGeometryCollection::asGML2( QDomDocument& doc, int precision, const QString& ns ) const
{
  QDomElement elemMultiGeometry = doc.createElementNS( ns, "MultiGeometry" );
  Q_FOREACH ( const QgsAbstractGeometry *geom, mGeometries )
  {
    QDomElement elemGeometryMember = doc.createElementNS( ns, "geometryMember" );
    elemGeometryMember.appendChild( geom->asGML2( doc, precision, ns ) );
    elemMultiGeometry.appendChild( elemGeometryMember );
  }
  return elemMultiGeometry;
}

QDomElement QgsGeometryCollection::asGML3( QDomDocument& doc, int precision, const QString& ns ) const
{
  QDomElement elemMultiGeometry = doc.createElementNS( ns, "MultiGeometry" );
  Q_FOREACH ( const QgsAbstractGeometry *geom, mGeometries )
  {
    QDomElement elemGeometryMember = doc.createElementNS( ns, "geometryMember" );
    elemGeometryMember.appendChild( geom->asGML3( doc, precision, ns ) );
    elemMultiGeometry.appendChild( elemGeometryMember );
  }
  return elemMultiGeometry;
}

QString QgsGeometryCollection::asJSON( int precision ) const
{
  QString json = "{\"type\": \"GeometryCollection\", \"geometries\": [";
  Q_FOREACH ( const QgsAbstractGeometry *geom, mGeometries )
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

QgsRectangle QgsGeometryCollection::boundingBox() const
{
  if ( mBoundingBox.isNull() )
  {
    mBoundingBox = calculateBoundingBox();
  }
  return mBoundingBox;
}

QgsRectangle QgsGeometryCollection::calculateBoundingBox() const
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

QgsCoordinateSequence QgsGeometryCollection::coordinateSequence() const
{
  if ( !mCoordinateSequence.isEmpty() )
    return mCoordinateSequence;

  QVector< QgsAbstractGeometry* >::const_iterator geomIt = mGeometries.constBegin();
  for ( ; geomIt != mGeometries.constEnd(); ++geomIt )
  {
    QgsCoordinateSequence geomCoords = ( *geomIt )->coordinateSequence();

    QgsCoordinateSequence::const_iterator cIt = geomCoords.constBegin();
    for ( ; cIt != geomCoords.constEnd(); ++cIt )
    {
      mCoordinateSequence.push_back( *cIt );
    }
  }

  return mCoordinateSequence;
}

double QgsGeometryCollection::closestSegment( const QgsPointV2& pt, QgsPointV2& segmentPt,  QgsVertexId& vertexAfter, bool* leftOf, double epsilon ) const
{
  return QgsGeometryUtils::closestSegmentFromComponents( mGeometries, QgsGeometryUtils::PART, pt, segmentPt, vertexAfter, leftOf, epsilon );
}

bool QgsGeometryCollection::nextVertex( QgsVertexId& id, QgsPointV2& vertex ) const
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

  QgsAbstractGeometry* geom = mGeometries.at( id.part );
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

bool QgsGeometryCollection::insertVertex( QgsVertexId position, const QgsPointV2& vertex )
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

bool QgsGeometryCollection::moveVertex( QgsVertexId position, const QgsPointV2& newPos )
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

bool QgsGeometryCollection::deleteVertex( QgsVertexId position )
{
  if ( position.part >= mGeometries.size() )
  {
    return false;
  }

  QgsAbstractGeometry* geom = mGeometries.at( position.part );
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

double QgsGeometryCollection::length() const
{
  double length = 0.0;
  QVector< QgsAbstractGeometry* >::const_iterator geomIt = mGeometries.constBegin();
  for ( ; geomIt != mGeometries.constEnd(); ++geomIt )
  {
    length += ( *geomIt )->length();
  }
  return length;
}

double QgsGeometryCollection::area() const
{
  double area = 0.0;
  QVector< QgsAbstractGeometry* >::const_iterator geomIt = mGeometries.constBegin();
  for ( ; geomIt != mGeometries.constEnd(); ++geomIt )
  {
    area += ( *geomIt )->area();
  }
  return area;
}

double QgsGeometryCollection::perimeter() const
{
  double perimeter = 0.0;
  QVector< QgsAbstractGeometry* >::const_iterator geomIt = mGeometries.constBegin();
  for ( ; geomIt != mGeometries.constEnd(); ++geomIt )
  {
    perimeter += ( *geomIt )->perimeter();
  }
  return perimeter;
}

bool QgsGeometryCollection::fromCollectionWkt( const QString &wkt, const QList<QgsAbstractGeometry*>& subtypes, const QString& defaultChildWkbType )
{
  clear();

  QPair<QgsWkbTypes::Type, QString> parts = QgsGeometryUtils::wktReadBlock( wkt );

  if ( QgsWkbTypes::flatType( parts.first ) != QgsWkbTypes::flatType( wkbType() ) )
    return false;
  mWkbType = parts.first;

  QString defChildWkbType = QString( "%1%2%3 " ).arg( defaultChildWkbType, is3D() ? "Z" : "", isMeasure() ? "M" : "" );

  Q_FOREACH ( const QString& childWkt, QgsGeometryUtils::wktGetChildBlocks( parts.second, defChildWkbType ) )
  {
    QPair<QgsWkbTypes::Type, QString> childParts = QgsGeometryUtils::wktReadBlock( childWkt );

    bool success = false;
    Q_FOREACH ( const QgsAbstractGeometry* geom, subtypes )
    {
      if ( QgsWkbTypes::flatType( childParts.first ) == QgsWkbTypes::flatType( geom->wkbType() ) )
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
  Q_FOREACH ( QgsAbstractGeometry* geom, mGeometries )
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

bool QgsGeometryCollection::hasCurvedSegments() const
{
  QVector< QgsAbstractGeometry* >::const_iterator it = mGeometries.constBegin();
  for ( ; it != mGeometries.constEnd(); ++it )
  {
    if (( *it )->hasCurvedSegments() )
    {
      return true;
    }
  }
  return false;
}

QgsAbstractGeometry* QgsGeometryCollection::segmentize( double tolerance, SegmentationToleranceType toleranceType ) const
{
  QgsAbstractGeometry* geom = QgsGeometryFactory::geomFromWkbType( mWkbType );
  QgsGeometryCollection* geomCollection = dynamic_cast<QgsGeometryCollection*>( geom );
  if ( !geomCollection )
  {
    delete geom;
    return clone();
  }

  QVector< QgsAbstractGeometry* >::const_iterator geomIt = mGeometries.constBegin();
  for ( ; geomIt != mGeometries.constEnd(); ++geomIt )
  {
    geomCollection->addGeometry(( *geomIt )->segmentize( tolerance, toleranceType ) );
  }
  return geomCollection;
}

double QgsGeometryCollection::vertexAngle( QgsVertexId vertex ) const
{
  if ( vertex.part >= mGeometries.size() )
  {
    return 0.0;
  }

  QgsAbstractGeometry* geom = mGeometries[vertex.part];
  if ( !geom )
  {
    return 0.0;
  }

  return geom->vertexAngle( vertex );
}

bool QgsGeometryCollection::addZValue( double zValue )
{
  if ( QgsWkbTypes::hasZ( mWkbType ) )
    return false;

  mWkbType = QgsWkbTypes::addZ( mWkbType );

  Q_FOREACH ( QgsAbstractGeometry* geom, mGeometries )
  {
    geom->addZValue( zValue );
  }
  clearCache();
  return true;
}

bool QgsGeometryCollection::addMValue( double mValue )
{
  if ( QgsWkbTypes::hasM( mWkbType ) )
    return false;

  mWkbType = QgsWkbTypes::addM( mWkbType );

  Q_FOREACH ( QgsAbstractGeometry* geom, mGeometries )
  {
    geom->addMValue( mValue );
  }
  clearCache();
  return true;
}


bool QgsGeometryCollection::dropZValue()
{
  if ( !is3D() )
    return false;

  mWkbType = QgsWkbTypes::dropZ( mWkbType );
  Q_FOREACH ( QgsAbstractGeometry* geom, mGeometries )
  {
    geom->dropZValue();
  }
  clearCache();
  return true;
}

bool QgsGeometryCollection::dropMValue()
{
  if ( !isMeasure() )
    return false;

  mWkbType = QgsWkbTypes::dropM( mWkbType );
  Q_FOREACH ( QgsAbstractGeometry* geom, mGeometries )
  {
    geom->dropMValue();
  }
  clearCache();
  return true;
}
