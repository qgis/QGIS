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

bool QgsAbstractGeometry::is3D() const
{
  return QgsWkbTypes::hasZ( mWkbType );
}

bool QgsAbstractGeometry::isMeasure() const
{
  return QgsWkbTypes::hasM( mWkbType );
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

  bool hasZ = subgeom->is3D();
  bool hasM = subgeom->isMeasure();

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

QgsPoint QgsAbstractGeometry::centroid() const
{
  // http://en.wikipedia.org/wiki/Centroid#Centroid_of_polygon
  // Pick the first ring of first part for the moment

  int n = vertexCount( 0, 0 );
  if ( n == 1 )
  {
    return vertexAt( QgsVertexId( 0, 0, 0 ) );
  }

  double A = 0.;
  double Cx = 0.;
  double Cy = 0.;
  QgsPoint v0 = vertexAt( QgsVertexId( 0, 0, 0 ) );
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
    double d = vi.x() * vj.y() - vj.x() * vi.y();
    A += d;
    Cx += ( vi.x() + vj.x() ) * d;
    Cy += ( vi.y() + vj.y() ) * d;
  }

  if ( A < 1E-12 )
  {
    Cx = Cy = 0.;
    for ( int i = 0; i < n - 1; ++i )
    {
      QgsPoint vi = vertexAt( QgsVertexId( 0, 0, i ) );
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

  bool needZ = QgsWkbTypes::hasZ( type );
  bool needM = QgsWkbTypes::hasM( type );
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

QgsVertexIterator QgsAbstractGeometry::vertices() const
{
  return QgsVertexIterator( this );
}

bool QgsAbstractGeometry::hasChildGeometries() const
{
  return QgsWkbTypes::isMultiType( wkbType() ) || dimension() == 2;
}

QgsPoint QgsAbstractGeometry::childPoint( int index ) const
{
  Q_UNUSED( index );
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

QgsAbstractGeometry *QgsAbstractGeometry::segmentize( double tolerance, SegmentationToleranceType toleranceType ) const
{
  Q_UNUSED( tolerance );
  Q_UNUSED( toleranceType );
  return clone();
}


QgsAbstractGeometry::vertex_iterator::vertex_iterator( const QgsAbstractGeometry *g, int index )
  : depth( 0 )
{
  ::memset( levels, 0, sizeof( Level ) * 3 );  // make sure we clean up also the padding areas (for memcmp test in operator==)
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
  QgsVertexId::VertexType vertexType = QgsVertexId::SegmentVertex;
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
  int res = ::memcmp( levels, other.levels, sizeof( Level ) * ( depth + 1 ) );
  return res == 0;
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
