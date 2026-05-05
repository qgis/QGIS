/***************************************************************************
  qgstessellator.cpp
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstessellator.h"

#include <algorithm>
#include <earcut.hpp>
#include <unordered_set>

#include "poly2tri.h"
#include "qgis.h"
#include "qgscurve.h"
#include "qgsgeometry.h"
#include "qgsgeometryutils_base.h"
#include "qgsmultipolygon.h"
#include "qgspoint.h"
#include "qgspolygon.h"
#include "qgstriangle.h"

#include <QMatrix4x4>
#include <QVector3D>
#include <QtDebug>
#include <QtMath>

void QgsTessellator::addExtrusionWallQuad( const QVector3D &pt1, const QVector3D &pt2, float height, float u1, float u2 )
{
  const float dx = pt2.x() - pt1.x();
  const float dy = pt2.y() - pt1.y();

  // perpendicular vector in plane to [x,y] is [-y,x]
  QVector3D vn = QVector3D( -dy, dx, 0 );
  vn.normalize();

  QVector4D vt;
  if ( mAddTangents )
  {
    // first make tangents following the walls horizontally
    QVector3D tangentDir( dx, dy, 0 );
    tangentDir.normalize();

    // if we flipped the direction of U along the wall, then we'll need to adjust the tangent accordingly
    // so that it's always pointing in the direction of increasing U
    if ( u2 < u1 )
    {
      tangentDir = -tangentDir;
    }

    vt = QVector4D( tangentDir.x(), tangentDir.y(), tangentDir.z(), 1.0f );
  }

  // here, we have to flip the z coordinate from an "increasing vertically" axis
  // to a "decreasing vertically" axis -- otherwise textures are rendered upside down.
  // we align textures so that the bottom of the texture is aligned to the bottom of
  // the wall (so eg doors in a facade texture are correctly placed at the bottom
  // of the wall)
  const float v0 = -height;
  const float v1 = -height;
  const float v2 = 0.0f;
  const float v3 = 0.0f;

  // triangle 1 vertex 1
  mIndexBuffer << uniqueVertexCount();
  mData << pt1.x() << pt1.y() << pt1.z() + height;
  if ( mAddNormals )
    mData << vn.x() << vn.y() << vn.z();
  if ( mAddTangents )
    mData << vt.x() << vt.y() << vt.z() << vt.w();
  if ( mAddTextureCoords )
    mData << u1 << v0;

  // triangle 1 vertex 2
  mIndexBuffer << uniqueVertexCount();
  mData << pt2.x() << pt2.y() << pt2.z() + height;
  if ( mAddNormals )
    mData << vn.x() << vn.y() << vn.z();
  if ( mAddTangents )
    mData << vt.x() << vt.y() << vt.z() << vt.w();
  if ( mAddTextureCoords )
    mData << u2 << v1;

  // triangle 1 vertex 3
  mIndexBuffer << uniqueVertexCount();
  mData << pt1.x() << pt1.y() << pt1.z();
  if ( mAddNormals )
    mData << vn.x() << vn.y() << vn.z();
  if ( mAddTangents )
    mData << vt.x() << vt.y() << vt.z() << vt.w();
  if ( mAddTextureCoords )
    mData << u1 << v2;

  // triangle 2 vertex 1
  mIndexBuffer << uniqueVertexCount() - 1;

  // triangle 2 vertex 2
  mIndexBuffer << uniqueVertexCount() - 2;

  // triangle 2 vertex 3
  mIndexBuffer << uniqueVertexCount();
  mData << pt2.x() << pt2.y() << pt2.z();
  if ( mAddNormals )
    mData << vn.x() << vn.y() << vn.z();
  if ( mAddTangents )
    mData << vt.x() << vt.y() << vt.z() << vt.w();
  if ( mAddTextureCoords )
    mData << u2 << v3;
}

QgsTessellator::QgsTessellator() = default;

QgsTessellator::QgsTessellator( double originX, double originY, bool addNormals, bool invertNormals, bool addBackFaces, bool noZ, bool addTextureCoords, int facade, float )
{
  setOrigin( QgsVector3D( originX, originY, 0 ) );
  setAddNormals( addNormals );
  setInvertNormals( invertNormals );
  setExtrusionFacesLegacy( facade );
  setBackFacesEnabled( addBackFaces );
  setAddTextureUVs( addTextureCoords );
  setInputZValueIgnored( noZ );
}

QgsTessellator::QgsTessellator( const QgsRectangle &bounds, bool addNormals, bool invertNormals, bool addBackFaces, bool noZ, bool addTextureCoords, int facade, float )
{
  setAddTextureUVs( addTextureCoords );
  setExtrusionFacesLegacy( facade );
  setBounds( bounds );
  setAddNormals( addNormals );
  setInvertNormals( invertNormals );
  setBackFacesEnabled( addBackFaces );
  setInputZValueIgnored( noZ );
}

void QgsTessellator::setOrigin( const QgsVector3D &origin )
{
  mOrigin = origin;
}

void QgsTessellator::setBounds( const QgsRectangle &bounds )
{
  mOrigin = QgsVector3D( bounds.xMinimum(), bounds.yMinimum(), 0 );
  mScale = bounds.isNull() ? 1.0 : std::max( 10000.0 / bounds.width(), 10000.0 / bounds.height() );
}

void QgsTessellator::setInputZValueIgnored( bool ignore )
{
  mInputZValueIgnored = ignore;
}

void QgsTessellator::setExtrusionFaces( Qgis::ExtrusionFaces faces )
{
  mExtrusionFaces = faces;
}

void QgsTessellator::setExtrusionFacesLegacy( int facade )
{
  switch ( facade )
  {
    case 0:
      mExtrusionFaces = Qgis::ExtrusionFace::NoFace;
      break;
    case 1:
      mExtrusionFaces = Qgis::ExtrusionFace::Walls;
      break;
    case 2:
      mExtrusionFaces = Qgis::ExtrusionFace::Roof;
      break;
    case 3:
      mExtrusionFaces = Qgis::ExtrusionFace::Walls | Qgis::ExtrusionFace::Roof;
      break;
    case 7:
      mExtrusionFaces = Qgis::ExtrusionFace::Walls | Qgis::ExtrusionFace::Roof | Qgis::ExtrusionFace::Floor;
      break;
    default:
      break;
  }
}

void QgsTessellator::setTextureRotation( float )
{}

void QgsTessellator::setBackFacesEnabled( bool addBackFaces )
{
  mAddBackFaces = addBackFaces;
}

void QgsTessellator::setInvertNormals( bool invertNormals )
{
  mInvertNormals = invertNormals;
}

void QgsTessellator::setAddNormals( bool addNormals )
{
  mAddNormals = addNormals;
  updateStride();
}

void QgsTessellator::setAddTangents( bool addTangents )
{
  mAddTangents = addTangents;
  updateStride();
}

void QgsTessellator::setAddTextureUVs( bool addTextureUVs )
{
  mAddTextureCoords = addTextureUVs;
  updateStride();
}

void QgsTessellator::setTriangulationAlgorithm( Qgis::TriangulationAlgorithm algorithm )
{
  mTriangulationAlgorithm = algorithm;
}

void QgsTessellator::setOutputZUp( bool )
{}

void QgsTessellator::updateStride()
{
  mStride = 3 * sizeof( float );
  if ( mAddNormals )
    mStride += 3 * sizeof( float );
  if ( mAddTangents )
    mStride += 4 * sizeof( float );
  if ( mAddTextureCoords )
    mStride += 2 * sizeof( float );
}

void QgsTessellator::makeWalls( const QgsLineString &ring, bool ccw, float extrusionHeight )
{
  // we need to find out orientation of the ring so that the triangles we generate
  // face the right direction
  // (for exterior we want clockwise order, for holes we want counter-clockwise order)
  const bool isCounterClockwise = ring.orientation() == Qgis::AngularDirection::CounterClockwise;

  QgsPoint pt;
  QgsPoint ptPrev = ring.pointN( isCounterClockwise == ccw ? 0 : ring.numPoints() - 1 );

  // accumulate texture U as we travel around the ring, so that texture seamless wraps
  // around the walls
  float accumulatedU = 0.0f;
  for ( int i = 1; i < ring.numPoints(); ++i )
  {
    pt = ring.pointN( isCounterClockwise == ccw ? i : ring.numPoints() - i - 1 );

    const QVector3D pt1( static_cast<float>( ptPrev.x() - mOrigin.x() ), static_cast<float>( ptPrev.y() - mOrigin.y() ), static_cast<float>( std::isnan( ptPrev.z() ) ? 0 : ptPrev.z() - mOrigin.z() ) );

    const QVector3D pt2( static_cast<float>( pt.x() - mOrigin.x() ), static_cast<float>( pt.y() - mOrigin.y() ), static_cast<float>( std::isnan( pt.z() ) ? 0 : pt.z() - mOrigin.z() ) );

    const float segmentLength = pt1.distanceToPoint( pt2 );

    // make a quad
    addExtrusionWallQuad( pt1, pt2, extrusionHeight, -accumulatedU, -( accumulatedU + segmentLength ) );

    if ( mAddBackFaces )
    {
      // texture start/end u are reversed so that texture isn't flipped on the backface
      addExtrusionWallQuad( pt2, pt1, extrusionHeight, accumulatedU + segmentLength, accumulatedU );
    }

    accumulatedU += segmentLength;
    ptPrev = pt;
  }
}

static QVector3D calculateNormal( const QgsLineString *curve, double originX, double originY, double originZ, bool invertNormal, float extrusionHeight )
{
  if ( !QgsWkbTypes::hasZ( curve->wkbType() ) )
  {
    // In case of extrusions, flat polygons always face up
    if ( extrusionHeight != 0 )
      return QVector3D( 0, 0, 1 );

    // For non-extrusions, decide based on polygon winding order and invertNormal flag
    float orientation = 1.f;
    if ( curve->orientation() == Qgis::AngularDirection::Clockwise )
      orientation = -orientation;
    if ( invertNormal )
      orientation = -orientation;
    return QVector3D( 0, 0, orientation );
  }

  // often we have 3D coordinates, but Z is the same for all vertices
  // if these flat polygons are extruded, we consider them up-facing regardless of winding order
  bool sameZ = true;
  QgsPoint pt1 = curve->pointN( 0 );
  QgsPoint pt2;
  for ( int i = 1; i < curve->numPoints(); i++ )
  {
    pt2 = curve->pointN( i );
    if ( pt1.z() != pt2.z() )
    {
      sameZ = false;
      break;
    }
  }
  if ( sameZ && extrusionHeight != 0 )
    return QVector3D( 0, 0, 1 );

  // Calculate the polygon's normal vector, based on Newell's method
  // https://www.khronos.org/opengl/wiki/Calculating_a_Surface_Normal
  //
  // Order of vertices is important here as it determines the front/back face of the polygon

  double nx = 0, ny = 0, nz = 0;

  // shift points by the tessellator's origin - this does not affect normal calculation and it may save us from losing some precision
  pt1.setX( pt1.x() - originX );
  pt1.setY( pt1.y() - originY );
  pt1.setZ( std::isnan( pt1.z() ) ? 0.0 : pt1.z() - originZ );
  for ( int i = 1; i < curve->numPoints(); i++ )
  {
    pt2 = curve->pointN( i );
    pt2.setX( pt2.x() - originX );
    pt2.setY( pt2.y() - originY );
    pt2.setZ( std::isnan( pt2.z() ) ? 0.0 : pt2.z() - originZ );

    nx += ( pt1.y() - pt2.y() ) * ( pt1.z() + pt2.z() );
    ny += ( pt1.z() - pt2.z() ) * ( pt1.x() + pt2.x() );
    nz += ( pt1.x() - pt2.x() ) * ( pt1.y() + pt2.y() );

    pt1 = pt2;
  }

  QVector3D normal( nx, ny, nz );
  if ( invertNormal )
    normal = -normal;
  normal.normalize();
  return normal;
}


static void normalVectorToXYVectors( const QVector3D &pNormal, QVector3D &pXVector, QVector3D &pYVector )
{
  // Here we define the two perpendicular vectors that define the local
  // 2D space on the plane. They will act as axis for which we will
  // calculate the projection coordinates of a 3D point to the plane.
  if ( pNormal.z() > 0.001 || pNormal.z() < -0.001 )
  {
    pXVector = QVector3D( 1, 0, -pNormal.x() / pNormal.z() );
  }
  else if ( pNormal.y() > 0.001 || pNormal.y() < -0.001 )
  {
    pXVector = QVector3D( 1, -pNormal.x() / pNormal.y(), 0 );
  }
  else
  {
    pXVector = QVector3D( -pNormal.y() / pNormal.x(), 1, 0 );
  }
  pXVector.normalize();
  pYVector = QVector3D::normal( pNormal, pXVector );
}

struct float_pair_hash
{
    std::size_t operator()( const std::pair<float, float> pair ) const
    {
      const std::size_t h1 = std::hash<float>()( pair.first );
      const std::size_t h2 = std::hash<float>()( pair.second );

      return h1 ^ h2;
    }
};

void ringToPoly2tri( const QgsLineString *ring, std::vector<p2t::Point *> &polyline, QHash<p2t::Point *, float> *zHash )
{
  const int pCount = ring->numPoints();

  polyline.reserve( pCount );

  const double *srcXData = ring->xData();
  const double *srcYData = ring->yData();
  const double *srcZData = ring->zData();
  std::unordered_set<std::pair<float, float>, float_pair_hash> foundPoints;

  for ( int i = 0; i < pCount - 1; ++i )
  {
    const float x = *srcXData++;
    const float y = *srcYData++;

    const auto res = foundPoints.insert( std::make_pair( x, y ) );
    if ( !res.second )
    {
      // already used this point, don't add a second time
      continue;
    }

    p2t::Point *pt2 = new p2t::Point( x, y );
    polyline.push_back( pt2 );
    if ( zHash )
    {
      ( *zHash )[pt2] = *srcZData++;
    }
  }
}


double roundCoord( double x )
{
  const double exp = 1e10; // round to 10 decimal digits
  return round( x * exp ) / exp;
}


static QgsCurve *transformRingToNewBase( const QgsLineString &curve, const QgsPoint &pt0, const QMatrix4x4 *toNewBase, const float scale )
{
  const int count = curve.numPoints();
  QVector<double> x;
  QVector<double> y;
  QVector<double> z;
  x.resize( count );
  y.resize( count );
  z.resize( count );
  double *xData = x.data();
  double *yData = y.data();
  double *zData = z.data();

  const double *srcXData = curve.xData();
  const double *srcYData = curve.yData();
  const double *srcZData = curve.is3D() ? curve.zData() : nullptr;

  for ( int i = 0; i < count; ++i )
  {
    QVector4D v( *srcXData++ - pt0.x(), *srcYData++ - pt0.y(), srcZData ? *srcZData++ - pt0.z() : 0, 0 );
    if ( toNewBase )
      v = toNewBase->map( v );

    // scale coordinates
    v.setX( v.x() * scale );
    v.setY( v.y() * scale );

    // we also round coordinates before passing them to poly2tri triangulation in order to fix possible numerical
    // stability issues. We had crashes with nearly collinear points where one of the points was off by a tiny bit (e.g. by 1e-20).
    // See TestQgsTessellator::testIssue17745().
    //
    // A hint for a similar issue: https://github.com/greenm01/poly2tri/issues/99
    //
    //    The collinear tests uses epsilon 1e-12. Seems rounding to 12 places you still
    //    can get problems with this test when points are pretty much on a straight line.
    //    I suggest you round to 10 decimals for stability and you can live with that
    //    precision.
    *xData++ = roundCoord( v.x() );
    *yData++ = roundCoord( v.y() );
    *zData++ = roundCoord( v.z() );
  }
  return new QgsLineString( x, y, z );
}


QgsPolygon *transformPolygonToNewBase( const QgsPolygon &polygon, const QgsPoint &pt0, const QMatrix4x4 *toNewBase, const float scale )
{
  QgsPolygon *p = new QgsPolygon;
  p->setExteriorRing( transformRingToNewBase( *qgsgeometry_cast< const QgsLineString * >( polygon.exteriorRing() ), pt0, toNewBase, scale ) );
  for ( int i = 0; i < polygon.numInteriorRings(); ++i )
    p->addInteriorRing( transformRingToNewBase( *qgsgeometry_cast< const QgsLineString * >( polygon.interiorRing( i ) ), pt0, toNewBase, scale ) );
  return p;
}


double minimumDistanceBetweenCoordinates( const QgsPolygon &polygon )
{
  double min_d = 1e20;

  std::vector< const QgsLineString * > rings;
  rings.reserve( 1 + polygon.numInteriorRings() );
  rings.emplace_back( qgsgeometry_cast< const QgsLineString * >( polygon.exteriorRing() ) );
  for ( int i = 0; i < polygon.numInteriorRings(); ++i )
    rings.emplace_back( qgsgeometry_cast< const QgsLineString * >( polygon.interiorRing( i ) ) );

  for ( const QgsLineString *ring : rings )
  {
    const int numPoints = ring->numPoints();
    if ( numPoints <= 1 )
      continue;

    const double *srcXData = ring->xData();
    const double *srcYData = ring->yData();
    double x0 = *srcXData++;
    double y0 = *srcYData++;
    for ( int i = 1; i < numPoints; ++i )
    {
      const double x1 = *srcXData++;
      const double y1 = *srcYData++;
      const double d = QgsGeometryUtilsBase::sqrDistance2D( x0, y0, x1, y1 );
      if ( d < min_d )
        min_d = d;
      x0 = x1;
      y0 = y1;
    }
  }

  return min_d != 1e20 ? std::sqrt( min_d ) : 1e20;
}

QByteArray QgsTessellator::vertexBuffer() const
{
  return QByteArray( reinterpret_cast<const char *>( mData.constData() ), sizeof( float ) * mData.size() );
}

QByteArray QgsTessellator::indexBuffer() const
{
  return QByteArray( reinterpret_cast<const char *>( mIndexBuffer.constData() ), sizeof( uint32_t ) * mIndexBuffer.size() );
}

void QgsTessellator::calculateBaseTransform( const QVector3D &pNormal, QMatrix4x4 *base ) const
{
  if ( !mInputZValueIgnored && pNormal != QVector3D( 0, 0, 1 ) )
  {
    // this is not a horizontal plane - need to reproject to a new base so that
    // we can do the triangulation in a plane
    QVector3D pXVector, pYVector;
    normalVectorToXYVectors( pNormal, pXVector, pYVector );

    // so now we have three orthogonal unit vectors defining new base
    // let's build transform matrix. We actually need just a 3x3 matrix,
    // but Qt does not have good support for it, so using 4x4 matrix instead.
    *base = QMatrix4x4( pXVector.x(), pXVector.y(), pXVector.z(), 0, pYVector.x(), pYVector.y(), pYVector.z(), 0, pNormal.x(), pNormal.y(), pNormal.z(), 0, 0, 0, 0, 0 );
  }
  else
  {
    base->setToIdentity();
  }
}

QVector3D QgsTessellator::applyTransformWithExtrusion( const QVector3D point, float extrusionHeight, QMatrix4x4 *transformMatrix, const QgsPoint *originOffset )
{
  const float z = mInputZValueIgnored ? 0.0f : point.z();
  QVector4D pt( point.x(), point.y(), z, 0 );

  pt = *transformMatrix * pt;

  const double fx = pt.x() - mOrigin.x() + originOffset->x();
  const double fy = pt.y() - mOrigin.y() + originOffset->y();
  const double baseHeight = mInputZValueIgnored ? 0 : pt.z() - mOrigin.z() + originOffset->z();
  const double fz = mInputZValueIgnored ? 0.0 : ( baseHeight + extrusionHeight );

  if ( baseHeight < mZMin )
    mZMin = static_cast<float>( baseHeight );
  if ( baseHeight > mZMax )
    mZMax = static_cast<float>( baseHeight );
  if ( fz > mZMax )
    mZMax = static_cast<float>( fz );

  return QVector3D( static_cast<float>( fx ), static_cast<float>( fy ), static_cast<float>( fz ) );
}


void ringToEarcutPoints( const QgsLineString *ring, std::vector<std::array<double, 2>> &polyline, std::vector<double> *zValues )
{
  const int pCount = ring->numPoints();

  polyline.reserve( pCount );

  const double *srcXData = ring->xData();
  const double *srcYData = ring->yData();
  const double *srcZData = ring->zData();

  // earcut handles duplicates, we do not need to remove them here
  for ( int i = 0; i < pCount - 1; ++i )
  {
    const double x = *srcXData++;
    const double y = *srcYData++;

    std::array<double, 2> pt = { x, y };
    polyline.push_back( pt );
  }

  if ( zValues && srcZData )
  {
    zValues->insert( zValues->end(), srcZData, srcZData + pCount - 1 );
  }
}

std::vector<QVector3D> QgsTessellator::generateConstrainedDelaunayTriangles( const QgsPolygon *polygonNew )
{
  QList<std::vector<p2t::Point *>> polylinesToDelete;
  QHash<p2t::Point *, float> z;

  // polygon exterior
  std::vector<p2t::Point *> polyline;
  ringToPoly2tri( qgsgeometry_cast< const QgsLineString * >( polygonNew->exteriorRing() ), polyline, mInputZValueIgnored ? nullptr : &z );
  polylinesToDelete << polyline;

  p2t::CDT cdt = p2t::CDT( polyline );

  // polygon holes
  for ( int i = 0; i < polygonNew->numInteriorRings(); ++i )
  {
    std::vector<p2t::Point *> holePolyline;
    const QgsLineString *hole = qgsgeometry_cast< const QgsLineString *>( polygonNew->interiorRing( i ) );

    ringToPoly2tri( hole, holePolyline, mInputZValueIgnored ? nullptr : &z );

    cdt.AddHole( holePolyline );
    polylinesToDelete << holePolyline;
  }

  cdt.Triangulate();
  std::vector<p2t::Triangle *> triangles = cdt.GetTriangles();

  std::vector<QVector3D> trianglePoints;
  trianglePoints.reserve( triangles.size() * 3 );

  for ( p2t::Triangle *t : triangles )
  {
    trianglePoints.emplace_back( t->GetPoint( 0 )->x / mScale, t->GetPoint( 0 )->y / mScale, z.value( t->GetPoint( 0 ) ) );
    trianglePoints.emplace_back( t->GetPoint( 1 )->x / mScale, t->GetPoint( 1 )->y / mScale, z.value( t->GetPoint( 1 ) ) );
    trianglePoints.emplace_back( t->GetPoint( 2 )->x / mScale, t->GetPoint( 2 )->y / mScale, z.value( t->GetPoint( 2 ) ) );
  }

  for ( int i = 0; i < polylinesToDelete.count(); ++i )
    qDeleteAll( polylinesToDelete[i] );

  return trianglePoints;
}

std::vector<QVector3D> QgsTessellator::generateEarcutTriangles( const QgsPolygon *polygonNew )
{
  const bool useZValues = !mInputZValueIgnored && polygonNew->is3D();
  const int allVerticesCount = polygonNew->nCoordinates();
  std::vector<double> zValues;

  if ( useZValues )
    zValues.reserve( allVerticesCount );

  std::vector<std::vector<std::array<double, 2>>> rings;
  rings.reserve( polygonNew->numInteriorRings() + 1 );

  {
    std::vector<std::array<double, 2>> polyline;
    ringToEarcutPoints( qgsgeometry_cast< const QgsLineString * >( polygonNew->exteriorRing() ), polyline, useZValues ? &zValues : nullptr );
    rings.push_back( std::move( polyline ) );
  }

  for ( int i = 0; i < polygonNew->numInteriorRings(); ++i )
  {
    std::vector<std::array<double, 2>> holePolyline;
    ringToEarcutPoints( qgsgeometry_cast<const QgsLineString *>( polygonNew->interiorRing( i ) ), holePolyline, useZValues ? &zValues : nullptr );
    rings.push_back( std::move( holePolyline ) );
  }

  std::vector<std::array<double, 2>> points;
  points.reserve( allVerticesCount );

  for ( const auto &ring : rings )
  {
    points.insert( points.end(), ring.begin(), ring.end() );
  }

  std::vector<uint32_t> indices = mapbox::earcut<uint32_t>( rings );
  std::vector<QVector3D> trianglePoints;
  trianglePoints.reserve( indices.size() );

  for ( size_t i = 0; i < indices.size(); i++ )
  {
    uint32_t vertexIndex = indices[i];

    const float x = static_cast<float>( points[vertexIndex][0] );
    const float y = static_cast<float>( points[vertexIndex][1] );
    const float z = static_cast<float>( useZValues ? zValues[vertexIndex] : 0.0 );

    trianglePoints.emplace_back( x / mScale, y / mScale, z );
  }

  return trianglePoints;
}

void QgsTessellator::addVertex(
  const QVector3D &point,
  const QVector3D &normal,
  const QVector4D &tangent,
  float extrusionHeight,
  QMatrix4x4 *transformMatrix,
  const QgsPoint *originOffset,
  QHash<VertexPoint, unsigned int> *vertexBuffer,
  const size_t &vertexBufferOffset,
  bool isFloor
)
{
  const QVector3D pt = applyTransformWithExtrusion( point, extrusionHeight, transformMatrix, originOffset );
  const VertexPoint vertex( pt, normal, tangent );
  if ( vertexBuffer->contains( vertex ) )
  {
    unsigned int index = vertexBuffer->value( vertex );
    mIndexBuffer << vertexBufferOffset + index;
  }
  else
  {
    unsigned int index = vertexBuffer->size();
    vertexBuffer->insert( vertex, index );
    mIndexBuffer << vertexBufferOffset + index;

    mData << pt.x() << pt.y() << pt.z();
    if ( mAddNormals )
    {
      mData << normal.x() << normal.y() << normal.z();
    }
    if ( mAddTangents )
    {
      mData << tangent.x() << tangent.y() << tangent.z() << tangent.w();
    }
    if ( mAddTextureCoords )
    {
      // flip y coordinate -- source texture images will have increasing y from top-to-bottom,
      // but 3d textures need to increase from bottom-to-top
      float u = pt.x();
      float v = -pt.y();

      if ( isFloor )
      {
        u = -u;
      }

      mData << u << v;
    }
  }
}

void QgsTessellator::addVertex( const QVector3D &point, const QVector3D &normal, const QVector4D &tangent, float extrusionHeight, QMatrix4x4 *transformMatrix, const QgsPoint *originOffset, bool isFloor )
{
  const QVector3D pt = applyTransformWithExtrusion( point, extrusionHeight, transformMatrix, originOffset );
  mIndexBuffer << uniqueVertexCount();
  mData << pt.x() << pt.y() << pt.z();
  if ( mAddNormals )
  {
    mData << normal.x() << normal.y() << normal.z();
  }
  if ( mAddTangents )
  {
    mData << tangent.x() << tangent.y() << tangent.z() << tangent.w();
  }
  if ( mAddTextureCoords )
  {
    // flip y coordinate -- source texture images will have increasing y from top-to-bottom,
    // but 3d textures need to increase from bottom-to-top
    float u = pt.x();
    float v = -pt.y();

    if ( isFloor )
    {
      u = -u;
    }

    mData << u << v;
  }
}

void QgsTessellator::addPolygon( const QgsPolygon &polygon, float extrusionHeight )
{
  const QgsLineString *exterior = qgsgeometry_cast< const QgsLineString * >( polygon.exteriorRing() );
  if ( !exterior )
    return;

  const QVector3D pNormal = !mInputZValueIgnored ? calculateNormal( exterior, mOrigin.x(), mOrigin.y(), mOrigin.z(), mInvertNormals, extrusionHeight ) : QVector3D();
  // calculate the tangent for the flat polygon (roof and floor)
  const QVector4D frontTangent( 1.0f, 0.0f, 0.0f, 1.0f );
  const QVector4D floorFrontTangent( -1.0f, 0.0f, 0.0f, 1.0f );
  // back face tangent
  const QVector4D backTangent( frontTangent.x(), frontTangent.y(), frontTangent.z(), -1.0f );
  const QVector4D floorBackTangent( floorFrontTangent.x(), floorFrontTangent.y(), floorFrontTangent.z(), -1.0f );

  const int pCount = exterior->numPoints();
  if ( pCount == 0 )
    return;

  QMatrix4x4 base; // identity matrix by default
  const QgsPoint ptStart( exterior->startPoint() );
  const QgsPoint extrusionOrigin( Qgis::WkbType::PointZ, ptStart.x(), ptStart.y(), std::isnan( ptStart.z() ) ? 0 : ptStart.z() );
  std::unique_ptr<QgsPolygon> polygonNew;

  if ( !mInputZValueIgnored && !qgsDoubleNear( pNormal.length(), 1, 0.001 ) )
    return; // this should not happen - pNormal should be normalized to unit length

  const bool buildWalls = mExtrusionFaces.testFlag( Qgis::ExtrusionFace::Walls );
  const bool buildFloor = mExtrusionFaces.testFlag( Qgis::ExtrusionFace::Floor );
  const bool buildRoof = mExtrusionFaces.testFlag( Qgis::ExtrusionFace::Roof );

  if ( buildFloor || buildRoof )
  {
    calculateBaseTransform( pNormal, &base );
    polygonNew.reset( transformPolygonToNewBase( polygon, extrusionOrigin, &base, mScale ) );

    QVector3D normal = pNormal;
    // our 3x3 matrix is orthogonal, so for inverse we only need to transpose it
    base = base.transposed();

    if ( pCount == 4 && polygon.numInteriorRings() == 0 )
    {
      Q_ASSERT( polygonNew->exteriorRing()->numPoints() >= 3 );

      const QgsLineString *triangle = qgsgeometry_cast< const QgsLineString * >( polygonNew->exteriorRing() );
      const QVector3D p1( static_cast<float>( triangle->xAt( 0 ) ), static_cast<float>( triangle->yAt( 0 ) ), static_cast<float>( triangle->zAt( 0 ) ) );
      const QVector3D p2( static_cast<float>( triangle->xAt( 1 ) ), static_cast<float>( triangle->yAt( 1 ) ), static_cast<float>( triangle->zAt( 1 ) ) );
      const QVector3D p3( static_cast<float>( triangle->xAt( 2 ) ), static_cast<float>( triangle->yAt( 2 ) ), static_cast<float>( triangle->zAt( 2 ) ) );
      std::array<QVector3D, 3> points = { p1, p2, p3 };

      for ( const QVector3D &point : points )
      {
        addVertex( point, normal, frontTangent, extrusionHeight, &base, &extrusionOrigin );
      }

      if ( mAddBackFaces )
      {
        for ( size_t i = points.size(); i-- > 0; )
        {
          const QVector3D &point = points[i];
          addVertex( point, -normal, backTangent, extrusionHeight, &base, &extrusionOrigin );
        }
      }

      if ( extrusionHeight != 0 && buildFloor )
      {
        for ( const QVector3D &point : points )
        {
          addVertex( point, normal, floorFrontTangent, 0.0, &base, &extrusionOrigin, true );
        }

        if ( mAddBackFaces )
        {
          for ( size_t i = points.size(); i-- > 0; )
          {
            const QVector3D &point = points[i];
            addVertex( point, -normal, floorBackTangent, 0.0, &base, &extrusionOrigin, true );
          }
        }
      }
    }
    else // we need to triangulate the polygon
    {
      if ( minimumDistanceBetweenCoordinates( *polygonNew ) < 0.001 )
      {
        // when the distances between coordinates of input points are very small,
        // the triangulation likes to crash on numerical errors - when the distances are ~ 1e-5
        // Assuming that the coordinates should be in a projected CRS, we should be able
        // to simplify geometries that may cause problems and avoid possible crashes
        const QgsGeometry polygonSimplified = QgsGeometry( polygonNew->clone() ).simplify( 0.001 );
        if ( polygonSimplified.isNull() )
        {
          mError = QObject::tr( "geometry simplification failed - skipping" );
          return;
        }
        const QgsPolygon *polygonSimplifiedData = qgsgeometry_cast<const QgsPolygon *>( polygonSimplified.constGet() );
        if ( !polygonSimplifiedData || minimumDistanceBetweenCoordinates( *polygonSimplifiedData ) < 0.001 )
        {
          // Failed to fix that. It could be a really tiny geometry... or maybe they gave us
          // geometry in unprojected lat/lon coordinates
          mError = QObject::tr( "geometry's coordinates are too close to each other and simplification failed - skipping" );
          return;
        }
        else
        {
          polygonNew.reset( polygonSimplifiedData->clone() );
        }
      }

      // run triangulation and write vertices to the output data array
      try
      {
        std::vector<QVector3D> trianglePoints;
        switch ( mTriangulationAlgorithm )
        {
          case Qgis::TriangulationAlgorithm::ConstrainedDelaunay:
            trianglePoints = generateConstrainedDelaunayTriangles( polygonNew.get() );
            break;
          case Qgis::TriangulationAlgorithm::Earcut:
            trianglePoints = generateEarcutTriangles( polygonNew.get() );
            break;
        }

        if ( trianglePoints.empty() )
        {
          mError = QObject::tr( "Failed to triangulate polygon." );
          return;
        }

        Q_ASSERT( trianglePoints.size() % 3 == 0 );

        mData.reserve( mData.size() + trianglePoints.size() * 3 * ( stride() / sizeof( float ) ) );

        const size_t vertexBufferSize = uniqueVertexCount();
        QHash<VertexPoint, unsigned int> vertexBuffer;
        for ( size_t i = 0; i < trianglePoints.size(); i += 3 )
        {
          const std::array<QVector3D, 3> points = { trianglePoints[i + 0], trianglePoints[i + 1], trianglePoints[i + 2] };

          // roof
          for ( const QVector3D &point : points )
          {
            addVertex( point, normal, frontTangent, extrusionHeight, &base, &extrusionOrigin, &vertexBuffer, vertexBufferSize );
          }

          if ( mAddBackFaces )
          {
            for ( size_t i = points.size(); i-- > 0; )
            {
              const QVector3D &point = points[i];
              addVertex( point, -normal, backTangent, extrusionHeight, &base, &extrusionOrigin, &vertexBuffer, vertexBufferSize );
            }
          }

          if ( extrusionHeight != 0 && buildFloor )
          {
            for ( const QVector3D &point : points )
            {
              addVertex( point, normal, floorFrontTangent, 0.0, &base, &extrusionOrigin, &vertexBuffer, vertexBufferSize, true );
            }

            if ( mAddBackFaces )
            {
              for ( size_t i = points.size(); i-- > 0; )
              {
                const QVector3D &point = points[i];
                addVertex( point, -normal, floorBackTangent, 0.0, &base, &extrusionOrigin, &vertexBuffer, vertexBufferSize, true );
              }
            }
          }
        }
      }
      catch ( std::runtime_error &err )
      {
        mError = err.what();
      }
      catch ( ... )
      {
        mError = QObject::tr( "An unknown error occurred" );
      }
    }
  }

  // add walls if extrusion is enabled
  if ( extrusionHeight != 0 && buildWalls )
  {
    makeWalls( *exterior, false, extrusionHeight );
    for ( int i = 0; i < polygon.numInteriorRings(); ++i )
      makeWalls( *qgsgeometry_cast< const QgsLineString * >( polygon.interiorRing( i ) ), true, extrusionHeight );
  }
}

int QgsTessellator::dataVerticesCount() const
{
  return mIndexBuffer.size();
}

std::unique_ptr<QgsMultiPolygon> QgsTessellator::asMultiPolygon() const
{
  auto mp = std::make_unique< QgsMultiPolygon >();
  const size_t nVals = mIndexBuffer.size();

  Q_ASSERT( nVals % 3 == 0 );

  mp->reserve( nVals / 3 );
  const size_t noOfElements = stride() / sizeof( float );

  for ( size_t i = 0; i + 2 < nVals; i += 3 )
  {
    const uint32_t index1 = mIndexBuffer[i] * noOfElements;
    const uint32_t index2 = mIndexBuffer[i + 1] * noOfElements;
    const uint32_t index3 = mIndexBuffer[i + 2] * noOfElements;

    const QgsPoint p1( mData[index1], mData[index1 + 1], mData[index1 + 2] );
    const QgsPoint p2( mData[index2], mData[index2 + 1], mData[index2 + 2] );
    const QgsPoint p3( mData[index3], mData[index3 + 1], mData[index3 + 2] );
    mp->addGeometry( new QgsTriangle( p1, p2, p3 ) );
  }

  return mp;
}

QVector<float> QgsTessellator::data() const
{
  const size_t n = mIndexBuffer.size();
  if ( n == 0 )
    return QVector<float>();

  QVector<float> tData;
  size_t noOfElements = stride() / sizeof( float );
  tData.reserve( n * noOfElements );

  for ( auto &index : mIndexBuffer )
  {
    for ( size_t i = 0; i < noOfElements; i++ )
    {
      tData << mData[index * noOfElements + i];
    }
  }

  return tData;
}

int QgsTessellator::uniqueVertexCount() const
{
  if ( mData.size() == 0 )
    return 0;

  return mData.size() / ( stride() / sizeof( float ) );
}
