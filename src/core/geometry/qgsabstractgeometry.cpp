/***************************************************************************
                        qgsabstractgeometry.cpp
  -------------------------------------------------------------------
Date                 : 04 Sept 2014
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

#include "qgsapplication.h"
#include "qgsabstractgeometry.h"
#include "qgswkbptr.h"
#include "qgsgeos.h"
#include "qgsmaptopixel.h"
#include "qgspoint.h"
#include "qgsgeometrycollection.h"
#include "qgsvertexid.h"

#include <nlohmann/json.hpp>
#include <limits>
#include <QTransform>

QgsAbstractGeometry::QgsAbstractGeometry( const QgsAbstractGeometry &geom )
{
  mWkbType = geom.mWkbType;
}

QgsAbstractGeometry &QgsAbstractGeometry::operator=( const QgsAbstractGeometry &geom )
{
  if ( &geom != this )
  {
    clear();
    mWkbType = geom.mWkbType;
  }
  return *this;
}

int QgsAbstractGeometry::compareTo( const QgsAbstractGeometry *other ) const
{
  // compare to self
  if ( this == other )
  {
    return 0;
  }

  if ( sortIndex() != other->sortIndex() )
  {
    //different geometry types
    const int diff = sortIndex() - other->sortIndex();
    return ( diff > 0 ) - ( diff < 0 );
  }

  // same types
  if ( isEmpty() && other->isEmpty() )
  {
    return 0;
  }

  if ( isEmpty() )
  {
    return -1;
  }
  if ( other->isEmpty() )
  {
    return 1;
  }

  return compareToSameClass( other );
}

void QgsAbstractGeometry::setZMTypeFromSubGeometry( const QgsAbstractGeometry *subgeom, QgsWkbTypes::Type baseGeomType )
{
  if ( !subgeom )
  {
    return;
  }

  //special handling for 25d types:
  if ( baseGeomType == QgsWkbTypes::LineString &&
       ( subgeom->wkbType() == QgsWkbTypes::Point25D || subgeom->wkbType() == QgsWkbTypes::LineString25D ) )
  {
    mWkbType = QgsWkbTypes::LineString25D;
    return;
  }
  else if ( baseGeomType == QgsWkbTypes::Polygon &&
            ( subgeom->wkbType() == QgsWkbTypes::Point25D || subgeom->wkbType() == QgsWkbTypes::LineString25D ) )
  {
    mWkbType = QgsWkbTypes::Polygon25D;
    return;
  }

  const bool hasZ = subgeom->is3D();
  const bool hasM = subgeom->isMeasure();

  if ( hasZ && hasM )
  {
    mWkbType = QgsWkbTypes::addM( QgsWkbTypes::addZ( baseGeomType ) );
  }
  else if ( hasZ )
  {
    mWkbType = QgsWkbTypes::addZ( baseGeomType );
  }
  else if ( hasM )
  {
    mWkbType = QgsWkbTypes::addM( baseGeomType );
  }
  else
  {
    mWkbType = baseGeomType;
  }
}

QgsRectangle QgsAbstractGeometry::calculateBoundingBox() const
{
  double xmin = std::numeric_limits<double>::max();
  double ymin = std::numeric_limits<double>::max();
  double xmax = -std::numeric_limits<double>::max();
  double ymax = -std::numeric_limits<double>::max();

  QgsVertexId id;
  QgsPoint vertex;
  double x, y;
  while ( nextVertex( id, vertex ) )
  {
    x = vertex.x();
    y = vertex.y();
    if ( x < xmin )
      xmin = x;
    if ( x > xmax )
      xmax = x;
    if ( y < ymin )
      ymin = y;
    if ( y > ymax )
      ymax = y;
  }

  return QgsRectangle( xmin, ymin, xmax, ymax );
}

void QgsAbstractGeometry::clearCache() const
{
}

int QgsAbstractGeometry::nCoordinates() const
{
  int nCoords = 0;

  const QgsCoordinateSequence seq = coordinateSequence();
  for ( const QgsRingSequence &r : seq )
  {
    for ( const QgsPointSequence &p : r )
    {
      nCoords += p.size();
    }
  }

  return nCoords;
}

double QgsAbstractGeometry::length() const
{
  return 0.0;
}

double QgsAbstractGeometry::perimeter() const
{
  return 0.0;
}

double QgsAbstractGeometry::area() const
{
  return 0.0;
}

QString QgsAbstractGeometry::wktTypeStr() const
{
  QString wkt = geometryType();
  if ( is3D() )
    wkt += 'Z';
  if ( isMeasure() )
    wkt += 'M';
  return wkt;
}

QString QgsAbstractGeometry::asJson( int precision )
{
  return QString::fromStdString( asJsonObject( precision ).dump() );
}

json QgsAbstractGeometry::asJsonObject( int precision ) const
{
  Q_UNUSED( precision ) return nullptr;
}

QgsPoint QgsAbstractGeometry::centroid() const
{
  if ( isEmpty() )
    return QgsPoint();

  // http://en.wikipedia.org/wiki/Centroid#Centroid_of_polygon
  // Pick the first ring of first part for the moment

  const int n = vertexCount( 0, 0 );
  if ( n == 1 )
  {
    return vertexAt( QgsVertexId( 0, 0, 0 ) );
  }

  double A = 0.;
  double Cx = 0.;
  double Cy = 0.;
  const QgsPoint v0 = vertexAt( QgsVertexId( 0, 0, 0 ) );
  int i = 0, j = 1;
  if ( vertexAt( QgsVertexId( 0, 0, 0 ) ) != vertexAt( QgsVertexId( 0, 0, n - 1 ) ) )
  {
    i = n - 1;
    j = 0;
  }
  for ( ; j < n; i = j++ )
  {
    QgsPoint vi = vertexAt( QgsVertexId( 0, 0, i ) );
    QgsPoint vj = vertexAt( QgsVertexId( 0, 0, j ) );
    vi.rx() -= v0.x();
    vi.ry() -= v0.y();
    vj.rx() -= v0.x();
    vj.ry() -= v0.y();
    const double d = vi.x() * vj.y() - vj.x() * vi.y();
    A += d;
    Cx += ( vi.x() + vj.x() ) * d;
    Cy += ( vi.y() + vj.y() ) * d;
  }

  if ( A < 1E-12 )
  {
    Cx = Cy = 0.;
    for ( int i = 0; i < n - 1; ++i )
    {
      const QgsPoint vi = vertexAt( QgsVertexId( 0, 0, i ) );
      Cx += vi.x();
      Cy += vi.y();
    }
    return QgsPoint( Cx / ( n - 1 ), Cy / ( n - 1 ) );
  }
  else
  {
    return QgsPoint( v0.x() + Cx / ( 3. * A ), v0.y() + Cy / ( 3. * A ) );
  }
}

bool QgsAbstractGeometry::convertTo( QgsWkbTypes::Type type )
{
  if ( type == mWkbType )
    return true;

  if ( QgsWkbTypes::flatType( type ) != QgsWkbTypes::flatType( mWkbType ) )
    return false;

  const bool needZ = QgsWkbTypes::hasZ( type );
  const bool needM = QgsWkbTypes::hasM( type );
  if ( !needZ )
  {
    dropZValue();
  }
  else if ( !is3D() )
  {
    addZValue( std::numeric_limits<double>::quiet_NaN() );
  }

  if ( !needM )
  {
    dropMValue();
  }
  else if ( !isMeasure() )
  {
    addMValue( std::numeric_limits<double>::quiet_NaN() );
  }

  return true;
}

const QgsAbstractGeometry *QgsAbstractGeometry::simplifiedTypeRef() const
{
  return this;
}

void QgsAbstractGeometry::filterVertices( const std::function<bool ( const QgsPoint & )> & )
{
  // Ideally this would be pure virtual, but SIP has issues with that
}

void QgsAbstractGeometry::transformVertices( const std::function<QgsPoint( const QgsPoint & )> & )
{
  // Ideally this would be pure virtual, but SIP has issues with that
}

QgsAbstractGeometry::part_iterator QgsAbstractGeometry::parts_end()
{
  const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( this );
  return part_iterator( this, collection ? collection->partCount() : 1 );
}

QgsGeometryPartIterator QgsAbstractGeometry::parts()
{
  return QgsGeometryPartIterator( this );
}

QgsGeometryConstPartIterator QgsAbstractGeometry::parts() const
{
  return QgsGeometryConstPartIterator( this );
}

QgsAbstractGeometry::const_part_iterator QgsAbstractGeometry::const_parts_end() const
{
  const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( this );
  return const_part_iterator( this, collection ? collection->partCount() : 1 );
}

QgsVertexIterator QgsAbstractGeometry::vertices() const
{
  return QgsVertexIterator( this );
}

int QgsAbstractGeometry::sortIndex() const
{
  switch ( QgsWkbTypes::flatType( mWkbType ) )
  {
    case QgsWkbTypes::Point:
      return 0;
    case QgsWkbTypes::MultiPoint:
      return 1;
    case QgsWkbTypes::LineString:
      return 2;
    case QgsWkbTypes::CircularString:
      return 3;
    case QgsWkbTypes::CompoundCurve:
      return 4;
    case QgsWkbTypes::MultiLineString:
      return 5;
    case QgsWkbTypes::MultiCurve:
      return 6;
    case QgsWkbTypes::Polygon:
    case QgsWkbTypes::Triangle:
      return 7;
    case QgsWkbTypes::CurvePolygon:
      return 8;
    case QgsWkbTypes::MultiPolygon:
      return 9;
    case QgsWkbTypes::MultiSurface:
      return 10;
    case QgsWkbTypes::GeometryCollection:
      return 11;
    case QgsWkbTypes::Unknown:
      return 12;
    case QgsWkbTypes::NoGeometry:
    default:
      break;
  }
  return 13;
}

bool QgsAbstractGeometry::hasChildGeometries() const
{
  return QgsWkbTypes::isMultiType( wkbType() ) || dimension() == 2;
}

QgsPoint QgsAbstractGeometry::childPoint( int index ) const
{
  Q_UNUSED( index )
  return QgsPoint();
}

bool QgsAbstractGeometry::isEmpty() const
{
  QgsVertexId vId;
  QgsPoint vertex;
  return !nextVertex( vId, vertex );
}

bool QgsAbstractGeometry::hasCurvedSegments() const
{
  return false;
}

bool QgsAbstractGeometry::boundingBoxIntersects( const QgsRectangle &rectangle ) const
{
  return boundingBox().intersects( rectangle );
}

QgsAbstractGeometry *QgsAbstractGeometry::segmentize( double tolerance, SegmentationToleranceType toleranceType ) const
{
  Q_UNUSED( tolerance )
  Q_UNUSED( toleranceType )
  return clone();
}


QgsAbstractGeometry::vertex_iterator::vertex_iterator( const QgsAbstractGeometry *g, int index )
  : depth( 0 )
{
  levels.fill( Level() );
  levels[0].g = g;
  levels[0].index = index;

  digDown();  // go to the leaf level of the first vertex
}

QgsAbstractGeometry::vertex_iterator &QgsAbstractGeometry::vertex_iterator::operator++()
{
  if ( depth == 0 && levels[0].index >= levels[0].g->childCount() )
    return *this;  // end of geometry - nowhere else to go

  Q_ASSERT( !levels[depth].g->hasChildGeometries() );  // we should be at a leaf level

  ++levels[depth].index;

  // traverse up if we are at the end in the current level
  while ( depth > 0 && levels[depth].index >= levels[depth].g->childCount() )
  {
    --depth;
    ++levels[depth].index;
  }

  digDown();  // go to the leaf level again

  return *this;
}

QgsAbstractGeometry::vertex_iterator QgsAbstractGeometry::vertex_iterator::operator++( int )
{
  vertex_iterator it( *this );
  ++*this;
  return it;
}

QgsPoint QgsAbstractGeometry::vertex_iterator::operator*() const
{
  Q_ASSERT( !levels[depth].g->hasChildGeometries() );
  return levels[depth].g->childPoint( levels[depth].index );
}

QgsVertexId QgsAbstractGeometry::vertex_iterator::vertexId() const
{
  int part = 0, ring = 0, vertex = levels[depth].index;
  if ( depth == 0 )
  {
    // nothing else to do
  }
  else if ( depth == 1 )
  {
    if ( QgsWkbTypes::isMultiType( levels[0].g->wkbType() ) )
      part = levels[0].index;
    else
      ring = levels[0].index;
  }
  else if ( depth == 2 )
  {
    part = levels[0].index;
    ring = levels[1].index;
  }
  else
  {
    Q_ASSERT( false );
    return QgsVertexId();
  }

  // get the vertex type: find out from the leaf geometry
  Qgis::VertexType vertexType = Qgis::VertexType::Segment;
  if ( const QgsCurve *curve = dynamic_cast<const QgsCurve *>( levels[depth].g ) )
  {
    QgsPoint p;
    curve->pointAt( vertex, p, vertexType );
  }

  return QgsVertexId( part, ring, vertex, vertexType );
}

bool QgsAbstractGeometry::vertex_iterator::operator==( const QgsAbstractGeometry::vertex_iterator &other ) const
{
  if ( depth != other.depth )
    return false;
  return std::equal( std::begin( levels ), std::begin( levels ) + depth + 1, std::begin( other.levels ) );
}

void QgsAbstractGeometry::vertex_iterator::digDown()
{
  if ( levels[depth].g->hasChildGeometries() && levels[depth].index >= levels[depth].g->childCount() )
    return;  // first check we are not already at the end

  // while not "final" depth for the geom: go one level down.
  while ( levels[depth].g->hasChildGeometries() )
  {
    ++depth;
    Q_ASSERT( depth < 3 );  // that's capacity of the levels array
    levels[depth].index = 0;
    levels[depth].g = levels[depth - 1].g->childGeometry( levels[depth - 1].index );
  }
}

QgsPoint QgsVertexIterator::next()
{
  n = i++;
  return *n;
}

QgsAbstractGeometry::part_iterator::part_iterator( QgsAbstractGeometry *g, int index )
  : mIndex( index )
  , mGeometry( g )
{
}

QgsAbstractGeometry::part_iterator &QgsAbstractGeometry::part_iterator::operator++()
{
  const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( mGeometry );
  if ( !collection )
  {
    mIndex = 1;
    return *this; // end of geometry -- nowhere else to go
  }

  if ( mIndex >= collection->partCount() )
    return *this; // end of geometry - nowhere else to go

  mIndex++;
  return *this;
}

QgsAbstractGeometry::part_iterator QgsAbstractGeometry::part_iterator::operator++( int )
{
  part_iterator it( *this );
  ++*this;
  return it;
}

QgsAbstractGeometry *QgsAbstractGeometry::part_iterator::operator*() const
{
  QgsGeometryCollection *collection = qgsgeometry_cast< QgsGeometryCollection * >( mGeometry );
  if ( !collection )
  {
    return mGeometry;
  }

  return collection->geometryN( mIndex );
}

int QgsAbstractGeometry::part_iterator::partNumber() const
{
  return mIndex;
}

bool QgsAbstractGeometry::part_iterator::operator==( QgsAbstractGeometry::part_iterator other ) const
{
  return mGeometry == other.mGeometry && mIndex == other.mIndex;
}

QgsAbstractGeometry *QgsGeometryPartIterator::next()
{
  n = i++;
  return *n;
}



QgsAbstractGeometry::const_part_iterator::const_part_iterator( const QgsAbstractGeometry *g, int index )
  : mIndex( index )
  , mGeometry( g )
{
}

QgsAbstractGeometry::const_part_iterator &QgsAbstractGeometry::const_part_iterator::operator++()
{
  const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( mGeometry );
  if ( !collection )
  {
    mIndex = 1;
    return *this; // end of geometry -- nowhere else to go
  }

  if ( mIndex >= collection->partCount() )
    return *this; // end of geometry - nowhere else to go

  mIndex++;
  return *this;
}

QgsAbstractGeometry::const_part_iterator QgsAbstractGeometry::const_part_iterator::operator++( int )
{
  const_part_iterator it( *this );
  ++*this;
  return it;
}

const QgsAbstractGeometry *QgsAbstractGeometry::const_part_iterator::operator*() const
{
  const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( mGeometry );
  if ( !collection )
  {
    return mGeometry;
  }

  return collection->geometryN( mIndex );
}

int QgsAbstractGeometry::const_part_iterator::partNumber() const
{
  return mIndex;
}

bool QgsAbstractGeometry::const_part_iterator::operator==( QgsAbstractGeometry::const_part_iterator other ) const
{
  return mGeometry == other.mGeometry && mIndex == other.mIndex;
}

const QgsAbstractGeometry *QgsGeometryConstPartIterator::next()
{
  n = i++;
  return *n;
}

bool QgsAbstractGeometry::vertex_iterator::Level::operator==( const QgsAbstractGeometry::vertex_iterator::Level &other ) const
{
  return g == other.g && index == other.index;
}
