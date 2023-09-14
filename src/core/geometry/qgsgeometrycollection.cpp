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
#include "qgsgeos.h"
#include "qgsfeedback.h"

#include <nlohmann/json.hpp>
#include <memory>

QgsGeometryCollection::QgsGeometryCollection()
{
  mWkbType = QgsWkbTypes::GeometryCollection;
}

QgsGeometryCollection::QgsGeometryCollection( const QgsGeometryCollection &c ):
  QgsAbstractGeometry( c ),
  mBoundingBox( c.mBoundingBox ),
  mHasCachedValidity( c.mHasCachedValidity ),
  mValidityFailureReason( c.mValidityFailureReason )
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
    QgsAbstractGeometry *g1 = mGeometries.at( i );
    QgsAbstractGeometry *g2 = otherCollection->mGeometries.at( i );

    // Quick check if the geometries are exactly the same
    if ( g1 != g2 )
    {
      if ( !g1 || !g2 )
        return false;

      // Slower check, compare the contents of the geometries
      if ( *g1 != *g2 )
        return false;
    }
  }

  return true;
}

bool QgsGeometryCollection::operator!=( const QgsAbstractGeometry &other ) const
{
  return !operator==( other );
}

QgsGeometryCollection *QgsGeometryCollection::createEmptyWithSameType() const
{
  auto result = std::make_unique< QgsGeometryCollection >();
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
  for ( QgsAbstractGeometry *geom : std::as_const( mGeometries ) )
  {
    if ( geom->removeDuplicateNodes( epsilon, useZValues ) ) result = true;
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

bool QgsGeometryCollection::boundingBoxIntersects( const QgsRectangle &rectangle ) const
{
  if ( mGeometries.empty() )
    return false;

  // if we already have the bounding box calculated, then this check is trivial!
  if ( !mBoundingBox.isNull() )
  {
    return mBoundingBox.intersects( rectangle );
  }

  // otherwise loop through each member geometry and test the bounding box intersection.
  // This gives us a chance to use optimisations which may be present on the individual
  // geometry subclasses, and at worst it will cause a calculation of the bounding box
  // of each individual member geometry which we would have to do anyway... (and these
  // bounding boxes are cached, so would be reused without additional expense)
  for ( const QgsAbstractGeometry *geometry : mGeometries )
  {
    if ( geometry->boundingBoxIntersects( rectangle ) )
      return true;
  }

  // even if we don't intersect the bounding box of any member geometries, we may still intersect the
  // bounding box of the overall collection.
  // so here we fall back to the non-optimised base class check which has to first calculate
  // the overall bounding box of the collection..
  return QgsAbstractGeometry::boundingBoxIntersects( rectangle );
}

void QgsGeometryCollection::reserve( int size )
{
  mGeometries.reserve( size );
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

  index = std::min( static_cast<int>( mGeometries.count() ), index );

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

void QgsGeometryCollection::normalize()
{
  for ( QgsAbstractGeometry *geometry : std::as_const( mGeometries ) )
  {
    geometry->normalize();
  }
  std::sort( mGeometries.begin(), mGeometries.end(), []( const QgsAbstractGeometry * a, const QgsAbstractGeometry * b )
  {
    return a->compareTo( b ) > 0;
  } );
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

void QgsGeometryCollection::transform( const QgsCoordinateTransform &ct, Qgis::TransformDirection d, bool transformZ )
{
  for ( QgsAbstractGeometry *g : std::as_const( mGeometries ) )
  {
    g->transform( ct, d, transformZ );
  }
  clearCache(); //set bounding box invalid
}

void QgsGeometryCollection::transform( const QTransform &t, double zTranslate, double zScale, double mTranslate, double mScale )
{
  for ( QgsAbstractGeometry *g : std::as_const( mGeometries ) )
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

QPainterPath QgsGeometryCollection::asQPainterPath() const
{
  QPainterPath p;
  for ( const QgsAbstractGeometry *geom : mGeometries )
  {
    QPainterPath partPath = geom->asQPainterPath();
    if ( !partPath.isEmpty() )
      p.addPath( partPath );
  }
  return p;
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
  mGeometries.reserve( nGeometries );
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

int QgsGeometryCollection::wkbSize( QgsAbstractGeometry::WkbFlags flags ) const
{
  int binarySize = sizeof( char ) + sizeof( quint32 ) + sizeof( quint32 );
  for ( const QgsAbstractGeometry *geom : mGeometries )
  {
    if ( geom )
    {
      binarySize += geom->wkbSize( flags );
    }
  }

  return binarySize;
}

QByteArray QgsGeometryCollection::asWkb( WkbFlags flags ) const
{
  int countNonNull = 0;
  for ( const QgsAbstractGeometry *geom : mGeometries )
  {
    if ( geom )
    {
      countNonNull ++;
    }
  }

  QByteArray wkbArray;
  wkbArray.resize( QgsGeometryCollection::wkbSize( flags ) );
  QgsWkbPtr wkb( wkbArray );
  wkb << static_cast<char>( QgsApplication::endian() );
  wkb << static_cast<quint32>( wkbType() );
  wkb << static_cast<quint32>( countNonNull );
  for ( const QgsAbstractGeometry *geom : mGeometries )
  {
    if ( geom )
    {
      wkb << geom->asWkb( flags );
    }
  }
  return wkbArray;
}

QString QgsGeometryCollection::asWkt( int precision ) const
{
  QString wkt = wktTypeStr();

  if ( isEmpty() )
    wkt += QLatin1String( " EMPTY" );
  else
  {
    wkt += QLatin1String( " (" );
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
  }
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

json QgsGeometryCollection::asJsonObject( int precision ) const
{
  json coordinates( json::array( ) );
  for ( const QgsAbstractGeometry *geom : std::as_const( mGeometries ) )
  {
    coordinates.push_back( geom->asJsonObject( precision ) );
  }
  return
  {
    { "type",  "GeometryCollection" },
    { "geometries", coordinates }
  };
}

QString QgsGeometryCollection::asKml( int precision ) const
{
  QString kml;
  kml.append( QLatin1String( "<MultiGeometry>" ) );
  const QVector< QgsAbstractGeometry * > &geometries = mGeometries;
  for ( const QgsAbstractGeometry *geometry : geometries )
  {
    kml.append( geometry->asKml( precision ) );
  }
  kml.append( QLatin1String( "</MultiGeometry>" ) );
  return kml;
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
    if ( mGeometries.at( i )->isEmpty() )
      continue;

    QgsRectangle geomBox = mGeometries.at( i )->boundingBox();
    if ( bbox.isNull() )
    {
      // workaround treatment of a QgsRectangle(0,0,0,0) as a "null"/invalid rectangle
      // if bbox is null, then the first geometry must have returned a bounding box of (0,0,0,0)
      // so just manually include that as a point... ew.
      geomBox.combineExtentWith( QPointF( 0, 0 ) );
      bbox = geomBox;
    }
    else if ( geomBox.isNull() )
    {
      // ...as above... this part must have a bounding box of (0,0,0,0).
      // if we try to combine the extent with this "null" box it will just be ignored.
      bbox.combineExtentWith( QPointF( 0, 0 ) );
    }
    else
    {
      bbox.combineExtentWith( geomBox );
    }
  }
  return bbox;
}

void QgsGeometryCollection::clearCache() const
{
  mBoundingBox = QgsRectangle();
  mHasCachedValidity = false;
  mValidityFailureReason.clear();
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

  QString secondWithoutParentheses = parts.second;
  secondWithoutParentheses = secondWithoutParentheses.remove( '(' ).remove( ')' ).simplified().remove( ' ' );
  if ( ( parts.second.compare( QLatin1String( "EMPTY" ), Qt::CaseInsensitive ) == 0 ) ||
       secondWithoutParentheses.isEmpty() )
  {
    qDeleteAll( subtypes );
    return true;
  }

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
  for ( QgsAbstractGeometry *geom : std::as_const( mGeometries ) )
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
  std::unique_ptr< QgsAbstractGeometry > geom( QgsGeometryFactory::geomFromWkbType( QgsWkbTypes::linearType( mWkbType ) ) );
  QgsGeometryCollection *geomCollection = qgsgeometry_cast<QgsGeometryCollection *>( geom.get() );
  if ( !geomCollection )
  {
    return clone();
  }

  geomCollection->reserve( mGeometries.size() );
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
  if ( id.part < 0 || id.part >= mGeometries.size() )
  {
    return QgsPoint();
  }

  const QgsAbstractGeometry *geom = mGeometries[id.part];
  if ( !geom )
  {
    return QgsPoint();
  }

  return geom->vertexAt( id );
}

bool QgsGeometryCollection::isValid( QString &error, Qgis::GeometryValidityFlags flags ) const
{
  if ( flags == 0 && mHasCachedValidity )
  {
    // use cached validity results
    error = mValidityFailureReason;
    return error.isEmpty();
  }

  QgsGeos geos( this );
  bool res = geos.isValid( &error, flags & Qgis::GeometryValidityFlag::AllowSelfTouchingHoles, nullptr );
  if ( flags == 0 )
  {
    mValidityFailureReason = !res ? error : QString();
    mHasCachedValidity = true;
  }
  return res;
}

bool QgsGeometryCollection::addZValue( double zValue )
{
  if ( QgsWkbTypes::hasZ( mWkbType ) )
    return false;

  mWkbType = QgsWkbTypes::addZ( mWkbType );

  for ( QgsAbstractGeometry *geom : std::as_const( mGeometries ) )
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

  for ( QgsAbstractGeometry *geom : std::as_const( mGeometries ) )
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
  for ( QgsAbstractGeometry *geom : std::as_const( mGeometries ) )
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
  for ( QgsAbstractGeometry *geom : std::as_const( mGeometries ) )
  {
    geom->dropMValue();
  }
  clearCache();
  return true;
}

void QgsGeometryCollection::filterVertices( const std::function<bool ( const QgsPoint & )> &filter )
{
  for ( QgsAbstractGeometry *geom : std::as_const( mGeometries ) )
  {
    if ( geom )
      geom->filterVertices( filter );
  }
  clearCache();
}

void QgsGeometryCollection::transformVertices( const std::function<QgsPoint( const QgsPoint & )> &transform )
{
  for ( QgsAbstractGeometry *geom : std::as_const( mGeometries ) )
  {
    if ( geom )
      geom->transformVertices( transform );
  }
  clearCache();
}

void QgsGeometryCollection::swapXy()
{
  for ( QgsAbstractGeometry *geom : std::as_const( mGeometries ) )
  {
    if ( geom )
      geom->swapXy();
  }
  clearCache();
}

QgsGeometryCollection *QgsGeometryCollection::toCurveType() const
{
  std::unique_ptr< QgsGeometryCollection > newCollection( new QgsGeometryCollection() );
  newCollection->reserve( mGeometries.size() );
  for ( QgsAbstractGeometry *geom : mGeometries )
  {
    newCollection->addGeometry( geom->toCurveType() );
  }
  return newCollection.release();
}

const QgsAbstractGeometry *QgsGeometryCollection::simplifiedTypeRef() const
{
  if ( mGeometries.size() == 1 )
    return mGeometries.at( 0 )->simplifiedTypeRef();
  else
    return this;
}

bool QgsGeometryCollection::transform( QgsAbstractGeometryTransformer *transformer, QgsFeedback *feedback )
{
  if ( !transformer )
    return false;

  bool res = true;
  for ( QgsAbstractGeometry *geom : std::as_const( mGeometries ) )
  {
    if ( geom )
      res = geom->transform( transformer, feedback );

    if ( feedback && feedback->isCanceled() )
      res = false;

    if ( !res )
      break;
  }
  clearCache();
  return res;
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

int QgsGeometryCollection::compareToSameClass( const QgsAbstractGeometry *other ) const
{
  const QgsGeometryCollection *otherCollection = qgsgeometry_cast<const QgsGeometryCollection *>( other );
  if ( !otherCollection )
    return -1;

  int i = 0;
  int j = 0;
  while ( i < mGeometries.size() && j < otherCollection->mGeometries.size() )
  {
    const QgsAbstractGeometry *aGeom = mGeometries[i];
    const QgsAbstractGeometry *bGeom = otherCollection->mGeometries[j];
    const int comparison = aGeom->compareTo( bGeom );
    if ( comparison != 0 )
    {
      return comparison;
    }
    i++;
    j++;
  }
  if ( i < mGeometries.size() )
  {
    return 1;
  }
  if ( j < otherCollection->mGeometries.size() )
  {
    return -1;
  }
  return 0;
}
