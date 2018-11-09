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
#include "qgspoint.h"
#include "qgsmultipoint.h"
#include "qgspolygon.h"
#include "qgsmultipolygon.h"
#include "qgswkbptr.h"
#include <memory>

QgsGeometryCollection::QgsGeometryCollection()
{
  mWkbType = QgsWkbTypes::GeometryCollection;
}

QgsGeometryCollection::QgsGeometryCollection( const QgsGeometryCollection &c ): QgsAbstractGeometry( c )
{
  int nGeoms = c.mGeometries.size();
  mGeometries.resize( nGeoms );
  for ( int i = 0; i < nGeoms; ++i )
  {
    mGeometries[i] = c.mGeometries.at( i )->clone();
  }
}

QgsGeometryCollection &QgsGeometryCollection::operator=( const QgsGeometryCollection &c )
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

bool QgsGeometryCollection::operator==( const QgsAbstractGeometry &other ) const
{
  const QgsGeometryCollection *otherCollection = qgsgeometry_cast< const QgsGeometryCollection * >( &other );
  if ( !otherCollection )
    return false;

  if ( mWkbType != otherCollection->mWkbType )
    return false;

  if ( mGeometries.count() != otherCollection->mGeometries.count() )
    return false;

  for ( int i = 0; i < mGeometries.count(); ++i )
  {
    if ( mGeometries.at( i ) != otherCollection->mGeometries.at( i ) )
      return false;
  }

  return true;
}

bool QgsGeometryCollection::operator!=( const QgsAbstractGeometry &other ) const
{
  return !operator==( other );
}

QgsGeometryCollection *QgsGeometryCollection::createEmptyWithSameType() const
{
  auto result = qgis::make_unique< QgsGeometryCollection >();
  result->mWkbType = mWkbType;
  return result.release();
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

QgsGeometryCollection *QgsGeometryCollection::snappedToGrid( double hSpacing, double vSpacing, double dSpacing, double mSpacing ) const
{
  std::unique_ptr<QgsGeometryCollection> result;

  for ( auto geom : mGeometries )
  {
    std::unique_ptr<QgsAbstractGeometry> gridified { geom->snappedToGrid( hSpacing, vSpacing, dSpacing, mSpacing ) };
    if ( gridified )
    {
      if ( !result )
        result = std::unique_ptr<QgsGeometryCollection> { createEmptyWithSameType() };

      result->mGeometries.append( gridified.release() );
    }
  }

  return result.release();
}

bool QgsGeometryCollection::removeDuplicateNodes( double epsilon, bool useZValues )
{
  bool result = false;
  for ( QgsAbstractGeometry *geom : qgis::as_const( mGeometries ) )
  {
    result = result || geom->removeDuplicateNodes( epsilon, useZValues );
  }
  return result;
}

QgsAbstractGeometry *QgsGeometryCollection::boundary() const
{
  return nullptr;
}

void QgsGeometryCollection::adjacentVertices( QgsVertexId vertex, QgsVertexId &previousVertex, QgsVertexId &nextVertex ) const
{
  if ( vertex.part < 0 || vertex.part >= mGeometries.count() )
  {
    previousVertex = QgsVertexId();
    nextVertex = QgsVertexId();
    return;
  }

  mGeometries.at( vertex.part )->adjacentVertices( vertex, previousVertex, nextVertex );
}

int QgsGeometryCollection::vertexNumberFromVertexId( QgsVertexId id ) const
{
  if ( id.part < 0 || id.part >= mGeometries.count() )
    return -1;

  int number = 0;
  int part = 0;
  for ( QgsAbstractGeometry *geometry : mGeometries )
  {
    if ( part == id.part )
    {
      int partNumber =  geometry->vertexNumberFromVertexId( QgsVertexId( 0, id.ring, id.vertex ) );
      if ( partNumber == -1 )
        return -1;
      return number + partNumber;
    }
    else
    {
      number += geometry->nCoordinates();
    }

    part++;
  }
  return -1; // should not happen
}

QgsAbstractGeometry *QgsGeometryCollection::geometryN( int n )
{
  clearCache();
  return mGeometries.value( n );
}

bool QgsGeometryCollection::isEmpty() const
{
  if ( mGeometries.isEmpty() )
    return true;

  for ( QgsAbstractGeometry *geometry : mGeometries )
  {
    if ( !geometry->isEmpty() )
      return false;
  }
  return true;
}

bool QgsGeometryCollection::addGeometry( QgsAbstractGeometry *g )
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

  index = std::min( mGeometries.count(), index );

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
  QVector< QgsAbstractGeometry * >::const_iterator it = mGeometries.constBegin();
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

QString QgsGeometryCollection::geometryType() const
{
  return QStringLiteral( "GeometryCollection" );
}

void QgsGeometryCollection::transform( const QgsCoordinateTransform &ct, QgsCoordinateTransform::TransformDirection d, bool transformZ )
{
  for ( QgsAbstractGeometry *g : qgis::as_const( mGeometries ) )
  {
    g->transform( ct, d, transformZ );
  }
  clearCache(); //set bounding box invalid
}

void QgsGeometryCollection::transform( const QTransform &t, double zTranslate, double zScale, double mTranslate, double mScale )
{
  for ( QgsAbstractGeometry *g : qgis::as_const( mGeometries ) )
  {
    g->transform( t, zTranslate, zScale, mTranslate, mScale );
  }
  clearCache(); //set bounding box invalid
}

void QgsGeometryCollection::draw( QPainter &p ) const
{
  QVector< QgsAbstractGeometry * >::const_iterator it = mGeometries.constBegin();
  for ( ; it != mGeometries.constEnd(); ++it )
  {
    ( *it )->draw( p );
  }
}

bool QgsGeometryCollection::fromWkb( QgsConstWkbPtr &wkbPtr )
{
  if ( !wkbPtr )
  {
    return false;
  }

  QgsWkbTypes::Type wkbType = wkbPtr.readHeader();
  if ( QgsWkbTypes::flatType( wkbType ) != QgsWkbTypes::flatType( mWkbType ) )
    return false;

  mWkbType = wkbType;

  int nGeometries = 0;
  wkbPtr >> nGeometries;

  QVector<QgsAbstractGeometry *> geometryListBackup = mGeometries;
  mGeometries.clear();
  for ( int i = 0; i < nGeometries; ++i )
  {
    std::unique_ptr< QgsAbstractGeometry > geom( QgsGeometryFactory::geomFromWkb( wkbPtr ) );  // also updates wkbPtr
    if ( geom )
    {
      if ( !addGeometry( geom.release() ) )
      {
        qDeleteAll( mGeometries );
        mGeometries = geometryListBackup;
        return false;
      }
    }
  }
  qDeleteAll( geometryListBackup );

  clearCache(); //set bounding box invalid

  return true;
}

bool QgsGeometryCollection::fromWkt( const QString &wkt )
{
  return fromCollectionWkt( wkt, QVector<QgsAbstractGeometry *>() << new QgsPoint << new QgsLineString << new QgsPolygon
                            << new QgsCircularString << new QgsCompoundCurve
                            << new QgsCurvePolygon
                            << new QgsMultiPoint << new QgsMultiLineString
                            << new QgsMultiPolygon << new QgsGeometryCollection
                            << new QgsMultiCurve << new QgsMultiSurface, QStringLiteral( "GeometryCollection" ) );
}

QByteArray QgsGeometryCollection::asWkb() const
{
  int binarySize = sizeof( char ) + sizeof( quint32 ) + sizeof( quint32 );
  QVector<QByteArray> wkbForGeometries;
  for ( const QgsAbstractGeometry *geom : mGeometries )
  {
    if ( geom )
    {
      QByteArray wkb( geom->asWkb() );
      binarySize += wkb.length();
      wkbForGeometries << wkb;
    }
  }

  QByteArray wkbArray;
  wkbArray.resize( binarySize );
  QgsWkbPtr wkb( wkbArray );
  wkb << static_cast<char>( QgsApplication::endian() );
  wkb << static_cast<quint32>( wkbType() );
  wkb << static_cast<quint32>( wkbForGeometries.count() );
  for ( const QByteArray &wkbForGeometry : qgis::as_const( wkbForGeometries ) )
  {
    wkb << wkbForGeometry;
  }
  return wkbArray;
}

QString QgsGeometryCollection::asWkt( int precision ) const
{
  QString wkt = wktTypeStr() + " (";
  for ( const QgsAbstractGeometry *geom : mGeometries )
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

QDomElement QgsGeometryCollection::asGml2( QDomDocument &doc, int precision, const QString &ns, const QgsAbstractGeometry::AxisOrder axisOrder ) const
{
  QDomElement elemMultiGeometry = doc.createElementNS( ns, QStringLiteral( "MultiGeometry" ) );
  for ( const QgsAbstractGeometry *geom : mGeometries )
  {
    QDomElement elemGeometryMember = doc.createElementNS( ns, QStringLiteral( "geometryMember" ) );
    elemGeometryMember.appendChild( geom->asGml2( doc, precision, ns, axisOrder ) );
    elemMultiGeometry.appendChild( elemGeometryMember );
  }
  return elemMultiGeometry;
}

QDomElement QgsGeometryCollection::asGml3( QDomDocument &doc, int precision, const QString &ns, const QgsAbstractGeometry::AxisOrder axisOrder ) const
{
  QDomElement elemMultiGeometry = doc.createElementNS( ns, QStringLiteral( "MultiGeometry" ) );
  for ( const QgsAbstractGeometry *geom : mGeometries )
  {
    QDomElement elemGeometryMember = doc.createElementNS( ns, QStringLiteral( "geometryMember" ) );
    elemGeometryMember.appendChild( geom->asGml3( doc, precision, ns, axisOrder ) );
    elemMultiGeometry.appendChild( elemGeometryMember );
  }
  return elemMultiGeometry;
}

QString QgsGeometryCollection::asJson( int precision ) const
{
  QString json = QStringLiteral( "{\"type\": \"GeometryCollection\", \"geometries\": [" );
  for ( const QgsAbstractGeometry *geom : mGeometries )
  {
    json += geom->asJson( precision ) + ", ";
  }
  if ( json.endsWith( QLatin1String( ", " ) ) )
  {
    json.chop( 2 ); // Remove last ", "
  }
  json += QLatin1String( "] }" );
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
  if ( mGeometries.empty() )
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

void QgsGeometryCollection::clearCache() const
{
  mBoundingBox = QgsRectangle();
  QgsAbstractGeometry::clearCache();
}

QgsCoordinateSequence QgsGeometryCollection::coordinateSequence() const
{
  QgsCoordinateSequence sequence;
  QVector< QgsAbstractGeometry * >::const_iterator geomIt = mGeometries.constBegin();
  for ( ; geomIt != mGeometries.constEnd(); ++geomIt )
  {
    QgsCoordinateSequence geomCoords = ( *geomIt )->coordinateSequence();

    QgsCoordinateSequence::const_iterator cIt = geomCoords.constBegin();
    for ( ; cIt != geomCoords.constEnd(); ++cIt )
    {
      sequence.push_back( *cIt );
    }
  }

  return sequence;
}

int QgsGeometryCollection::nCoordinates() const
{
  int count = 0;

  QVector< QgsAbstractGeometry * >::const_iterator geomIt = mGeometries.constBegin();
  for ( ; geomIt != mGeometries.constEnd(); ++geomIt )
  {
    count += ( *geomIt )->nCoordinates();
  }

  return count;
}

double QgsGeometryCollection::closestSegment( const QgsPoint &pt, QgsPoint &segmentPt,  QgsVertexId &vertexAfter, int *leftOf, double epsilon ) const
{
  return QgsGeometryUtils::closestSegmentFromComponents( mGeometries, QgsGeometryUtils::Part, pt, segmentPt, vertexAfter, leftOf, epsilon );
}

bool QgsGeometryCollection::nextVertex( QgsVertexId &id, QgsPoint &vertex ) const
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

  if ( id.part >= mGeometries.count() )
    return false;

  QgsAbstractGeometry *geom = mGeometries.at( id.part );
  if ( geom->nextVertex( id, vertex ) )
  {
    return true;
  }
  if ( ( id.part + 1 ) >= numGeometries() )
  {
    return false;
  }
  ++id.part;
  id.ring = -1;
  id.vertex = -1;
  return mGeometries.at( id.part )->nextVertex( id, vertex );
}

bool QgsGeometryCollection::insertVertex( QgsVertexId position, const QgsPoint &vertex )
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

bool QgsGeometryCollection::moveVertex( QgsVertexId position, const QgsPoint &newPos )
{
  if ( position.part < 0 || position.part >= mGeometries.size() )
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
  if ( position.part < 0 || position.part >= mGeometries.size() )
  {
    return false;
  }

  QgsAbstractGeometry *geom = mGeometries.at( position.part );
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
  QVector< QgsAbstractGeometry * >::const_iterator geomIt = mGeometries.constBegin();
  for ( ; geomIt != mGeometries.constEnd(); ++geomIt )
  {
    length += ( *geomIt )->length();
  }
  return length;
}

double QgsGeometryCollection::area() const
{
  double area = 0.0;
  QVector< QgsAbstractGeometry * >::const_iterator geomIt = mGeometries.constBegin();
  for ( ; geomIt != mGeometries.constEnd(); ++geomIt )
  {
    area += ( *geomIt )->area();
  }
  return area;
}

double QgsGeometryCollection::perimeter() const
{
  double perimeter = 0.0;
  QVector< QgsAbstractGeometry * >::const_iterator geomIt = mGeometries.constBegin();
  for ( ; geomIt != mGeometries.constEnd(); ++geomIt )
  {
    perimeter += ( *geomIt )->perimeter();
  }
  return perimeter;
}

bool QgsGeometryCollection::fromCollectionWkt( const QString &wkt, const QVector<QgsAbstractGeometry *> &subtypes, const QString &defaultChildWkbType )
{
  clear();

  QPair<QgsWkbTypes::Type, QString> parts = QgsGeometryUtils::wktReadBlock( wkt );

  if ( QgsWkbTypes::flatType( parts.first ) != QgsWkbTypes::flatType( wkbType() ) )
  {
    qDeleteAll( subtypes );
    return false;
  }
  mWkbType = parts.first;

  QString defChildWkbType = QStringLiteral( "%1%2%3 " ).arg( defaultChildWkbType, is3D() ? QStringLiteral( "Z" ) : QString(), isMeasure() ? QStringLiteral( "M" ) : QString() );

  const QStringList blocks = QgsGeometryUtils::wktGetChildBlocks( parts.second, defChildWkbType );
  for ( const QString &childWkt : blocks )
  {
    QPair<QgsWkbTypes::Type, QString> childParts = QgsGeometryUtils::wktReadBlock( childWkt );

    bool success = false;
    for ( const QgsAbstractGeometry *geom : subtypes )
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
  for ( QgsAbstractGeometry *geom : qgis::as_const( mGeometries ) )
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
  QVector< QgsAbstractGeometry * >::const_iterator it = mGeometries.constBegin();
  for ( ; it != mGeometries.constEnd(); ++it )
  {
    if ( ( *it )->hasCurvedSegments() )
    {
      return true;
    }
  }
  return false;
}

QgsAbstractGeometry *QgsGeometryCollection::segmentize( double tolerance, SegmentationToleranceType toleranceType ) const
{
  std::unique_ptr< QgsAbstractGeometry > geom( QgsGeometryFactory::geomFromWkbType( mWkbType ) );
  QgsGeometryCollection *geomCollection = qgsgeometry_cast<QgsGeometryCollection *>( geom.get() );
  if ( !geomCollection )
  {
    return clone();
  }

  QVector< QgsAbstractGeometry * >::const_iterator geomIt = mGeometries.constBegin();
  for ( ; geomIt != mGeometries.constEnd(); ++geomIt )
  {
    geomCollection->addGeometry( ( *geomIt )->segmentize( tolerance, toleranceType ) );
  }
  return geom.release();
}

double QgsGeometryCollection::vertexAngle( QgsVertexId vertex ) const
{
  if ( vertex.part < 0 || vertex.part >= mGeometries.size() )
  {
    return 0.0;
  }

  QgsAbstractGeometry *geom = mGeometries[vertex.part];
  if ( !geom )
  {
    return 0.0;
  }

  return geom->vertexAngle( vertex );
}

double QgsGeometryCollection::segmentLength( QgsVertexId startVertex ) const
{
  if ( startVertex.part < 0 || startVertex.part >= mGeometries.size() )
  {
    return 0.0;
  }

  const QgsAbstractGeometry *geom = mGeometries[startVertex.part];
  if ( !geom )
  {
    return 0.0;
  }

  return geom->segmentLength( startVertex );
}

int QgsGeometryCollection::vertexCount( int part, int ring ) const
{
  if ( part < 0 || part >= mGeometries.size() )
  {
    return 0;
  }

  return mGeometries[part]->vertexCount( 0, ring );
}

int QgsGeometryCollection::ringCount( int part ) const
{
  if ( part < 0 || part >= mGeometries.size() )
  {
    return 0;
  }

  return mGeometries[part]->ringCount();
}

int QgsGeometryCollection::partCount() const
{
  return mGeometries.size();
}

QgsPoint QgsGeometryCollection::vertexAt( QgsVertexId id ) const
{
  return mGeometries[id.part]->vertexAt( id );
}

bool QgsGeometryCollection::addZValue( double zValue )
{
  if ( QgsWkbTypes::hasZ( mWkbType ) )
    return false;

  mWkbType = QgsWkbTypes::addZ( mWkbType );

  for ( QgsAbstractGeometry *geom : qgis::as_const( mGeometries ) )
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

  for ( QgsAbstractGeometry *geom : qgis::as_const( mGeometries ) )
  {
    geom->addMValue( mValue );
  }
  clearCache();
  return true;
}


bool QgsGeometryCollection::dropZValue()
{
  if ( mWkbType != QgsWkbTypes::GeometryCollection && !is3D() )
    return false;

  mWkbType = QgsWkbTypes::dropZ( mWkbType );
  for ( QgsAbstractGeometry *geom : qgis::as_const( mGeometries ) )
  {
    geom->dropZValue();
  }
  clearCache();
  return true;
}

bool QgsGeometryCollection::dropMValue()
{
  if ( mWkbType != QgsWkbTypes::GeometryCollection && !isMeasure() )
    return false;

  mWkbType = QgsWkbTypes::dropM( mWkbType );
  for ( QgsAbstractGeometry *geom : qgis::as_const( mGeometries ) )
  {
    geom->dropMValue();
  }
  clearCache();
  return true;
}

void QgsGeometryCollection::filterVertices( const std::function<bool ( const QgsPoint & )> &filter )
{
  for ( QgsAbstractGeometry *geom : qgis::as_const( mGeometries ) )
  {
    if ( geom )
      geom->filterVertices( filter );
  }
  clearCache();
}

void QgsGeometryCollection::transformVertices( const std::function<QgsPoint( const QgsPoint & )> &transform )
{
  for ( QgsAbstractGeometry *geom : qgis::as_const( mGeometries ) )
  {
    if ( geom )
      geom->transformVertices( transform );
  }
  clearCache();
}

void QgsGeometryCollection::swapXy()
{
  for ( QgsAbstractGeometry *geom : qgis::as_const( mGeometries ) )
  {
    if ( geom )
      geom->swapXy();
  }
  clearCache();
}

QgsGeometryCollection *QgsGeometryCollection::toCurveType() const
{
  std::unique_ptr< QgsGeometryCollection > newCollection( new QgsGeometryCollection() );
  for ( QgsAbstractGeometry *geom : mGeometries )
  {
    newCollection->addGeometry( geom->toCurveType() );
  }
  return newCollection.release();
}

bool QgsGeometryCollection::wktOmitChildType() const
{
  return false;
}

int QgsGeometryCollection::childCount() const
{
  return mGeometries.count();
}

QgsAbstractGeometry *QgsGeometryCollection::childGeometry( int index ) const
{
  if ( index < 0 || index > mGeometries.count() )
    return nullptr;
  return mGeometries.at( index );
}
