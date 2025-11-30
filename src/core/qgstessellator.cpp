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

static std::pair<float, float> rotateCoords( float x, float y, float origin_x, float origin_y, float r )
{
  r = qDegreesToRadians( r );
  float x0 = x - origin_x, y0 = y - origin_y;
  // p0 = x0 + i * y0
  // rot = cos(r) + i * sin(r)
  // p0 * rot = x0 * cos(r) - y0 * sin(r) + i * [ x0 * sin(r) + y0 * cos(r) ]
  const float x1 = origin_x + x0 * qCos( r ) - y0 * qSin( r );
  const float y1 = origin_y + x0 * qSin( r ) + y0 * qCos( r );
  return std::make_pair( x1, y1 );
}

void QgsTessellator::addExtrusionWallQuad( const QVector3D &pt1, const QVector3D &pt2, float height )
{
  const float dx = pt2.x() - pt1.x();
  const float dy = pt2.y() - pt1.y();

  // perpendicular vector in plane to [x,y] is [-y,x]
  QVector3D vn = mOutputZUp ? QVector3D( -dy, dx, 0 ) : QVector3D( -dy, 0, -dx );
  vn.normalize();

  float u0, v0;
  float u1, v1;
  float u2, v2;
  float u3, v3;

  QVector<double> textureCoordinates;
  textureCoordinates.reserve( 12 );
  // select which side of the coordinates to use (x, z or y, z) depending on which side is smaller
  if ( fabsf( dy ) <= fabsf( dx ) )
  {
    // consider x and z as the texture coordinates
    u0 = pt1.x();
    v0 = pt1.z() + height;

    u1 = pt2.x();
    v1 = pt2.z() + height;

    u2 = pt1.x();
    v2 = pt1.z();

    u3 = pt2.x();
    v3 = pt2.z();
  }
  else
  {
    // consider y and z as the texture coowallsTextureRotationrdinates
    u0 = -pt1.y();
    v0 = pt1.z() + height;

    u1 = -pt2.y();
    v1 = pt2.z() + height;

    u2 = -pt1.y();
    v2 = pt1.z();

    u3 = -pt2.y();
    v3 = pt2.z();
  }

  textureCoordinates.push_back( u0 );
  textureCoordinates.push_back( v0 );

  textureCoordinates.push_back( u1 );
  textureCoordinates.push_back( v1 );

  textureCoordinates.push_back( u2 );
  textureCoordinates.push_back( v2 );

  textureCoordinates.push_back( u2 );
  textureCoordinates.push_back( v2 );

  textureCoordinates.push_back( u1 );
  textureCoordinates.push_back( v1 );

  textureCoordinates.push_back( u3 );
  textureCoordinates.push_back( v3 );

  for ( int i = 0; i < textureCoordinates.size(); i += 2 )
  {
    const std::pair<float, float> rotated = rotateCoords( textureCoordinates[i], textureCoordinates[i + 1], 0, 0, mTextureRotation );
    textureCoordinates[i] = rotated.first;
    textureCoordinates[i + 1] = rotated.second;
  }

  VertexPoint vertexPoint;

  // triangle 1
  // vertice 1
  if ( mOutputZUp )
    vertexPoint.position = QVector3D( pt1.x(), pt1.y(), pt1.z() + height );
  else
    vertexPoint.position = QVector3D( pt1.x(), pt1.z() + height, -pt1.y() );
  if ( mAddNormals )
    vertexPoint.normal = vn;
  if ( mAddTextureCoords )
  {
    vertexPoint.u = textureCoordinates[0];
    vertexPoint.v = textureCoordinates[1];
  }

  addVertexPoint( vertexPoint );

  // vertice 2
  if ( mOutputZUp )
    vertexPoint.position = QVector3D( pt2.x(), pt2.y(), pt2.z() + height );
  else
    vertexPoint.position = QVector3D( pt2.x(), pt2.z() + height, -pt2.y() );
  if ( mAddNormals )
    vertexPoint.normal = vn;
  if ( mAddTextureCoords )
  {
    vertexPoint.u = textureCoordinates[2];
    vertexPoint.v = textureCoordinates[3];
  }

  addVertexPoint( vertexPoint );

  // vertice 3
  if ( mOutputZUp )
    vertexPoint.position = pt1;
  else
    vertexPoint.position = QVector3D( pt1.x(), pt1.z(), -pt1.y() );
  if ( mAddNormals )
    vertexPoint.normal = vn;
  if ( mAddTextureCoords )
  {
    vertexPoint.u = textureCoordinates[4];
    vertexPoint.v = textureCoordinates[5];
  }

  addVertexPoint( vertexPoint );

  // triangle 2
  // vertice 1
  if ( mOutputZUp )
    vertexPoint.position = pt1;
  else
    vertexPoint.position = QVector3D( pt1.x(), pt1.z(), -pt1.y() );
  if ( mAddNormals )
    vertexPoint.normal = vn;
  if ( mAddTextureCoords )
  {
    vertexPoint.u = textureCoordinates[6];
    vertexPoint.v = textureCoordinates[7];
  }

  addVertexPoint( vertexPoint );

  // vertice 2
  if ( mOutputZUp )
    vertexPoint.position = QVector3D( pt2.x(), pt2.y(), pt2.z() + height );
  else
    vertexPoint.position = QVector3D( pt2.x(), pt2.z() + height, -pt2.y() );
  if ( mAddNormals )
    vertexPoint.normal = vn;
  if ( mAddTextureCoords )
  {
    vertexPoint.u = textureCoordinates[8];
    vertexPoint.v = textureCoordinates[9];
  }

  addVertexPoint( vertexPoint );

  // vertice 3
  if ( mOutputZUp )
    vertexPoint.position = pt2;
  else
    vertexPoint.position = QVector3D( pt2.x(), pt2.z(), -pt2.y() );
  if ( mAddNormals )
    vertexPoint.normal = vn;
  if ( mAddTextureCoords )
  {
    vertexPoint.u = textureCoordinates[10];
    vertexPoint.v = textureCoordinates[11];
  }

  addVertexPoint( vertexPoint );
}

QgsTessellator::QgsTessellator() = default;

QgsTessellator::QgsTessellator( double originX, double originY, bool addNormals, bool invertNormals, bool addBackFaces, bool noZ,
                                bool addTextureCoords, int facade, float textureRotation )
{
  setOrigin( QgsVector3D( originX, originY, 0 ) );
  setAddNormals( addNormals );
  setInvertNormals( invertNormals );
  setExtrusionFacesLegacy( facade );
  setBackFacesEnabled( addBackFaces );
  setAddTextureUVs( addTextureCoords );
  setInputZValueIgnored( noZ );
  setTextureRotation( textureRotation );
}

QgsTessellator::QgsTessellator( const QgsRectangle &bounds, bool addNormals, bool invertNormals, bool addBackFaces, bool noZ,
                                bool addTextureCoords, int facade, float textureRotation )
{
  setAddTextureUVs( addTextureCoords );
  setExtrusionFacesLegacy( facade );
  setBounds( bounds );
  setAddNormals( addNormals );
  setInvertNormals( invertNormals );
  setBackFacesEnabled( addBackFaces );
  setInputZValueIgnored( noZ );
  setTextureRotation( textureRotation );
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

void QgsTessellator::setTextureRotation( float rotation )
{
  mTextureRotation = rotation;
}

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

void QgsTessellator::setAddTextureUVs( bool addTextureUVs )
{
  mAddTextureCoords = addTextureUVs;
  updateStride();
}

void QgsTessellator::setTriangulationAlgorithm( Qgis::TriangulationAlgorithm algorithm )
{
  mTriangulationAlgorithm = algorithm;
}

void QgsTessellator::updateStride()
{
  mStride = 3 * sizeof( float );
  if ( mAddNormals )
    mStride += 3 * sizeof( float );
  if ( mAddTextureCoords )
    mStride += 2 * sizeof( float );
}

void QgsTessellator::addVertexPoint( const VertexPoint &vertexPoint )
{
  if ( mVertexBuffer.contains( vertexPoint ) )
  {
    const uint32_t index = mVertexBuffer.value( vertexPoint );
    mIndexBuffer << index;
  }
  else
  {
    const uint32_t index = mVertexBuffer.size();
    mVertexBuffer.insert( vertexPoint, index );
    mIndexBuffer << index;
  }
}

void QgsTessellator::makeWalls( const QgsLineString &ring, bool ccw, float extrusionHeight )
{
  // we need to find out orientation of the ring so that the triangles we generate
  // face the right direction
  // (for exterior we want clockwise order, for holes we want counter-clockwise order)
  const bool is_counter_clockwise = ring.orientation() == Qgis::AngularDirection::CounterClockwise;

  QgsPoint pt;
  QgsPoint ptPrev = ring.pointN( is_counter_clockwise == ccw ? 0 : ring.numPoints() - 1 );
  for ( int i = 1; i < ring.numPoints(); ++i )
  {
    pt = ring.pointN( is_counter_clockwise == ccw ? i : ring.numPoints() - i - 1 );

    const QVector3D pt1(
      static_cast<float>( ptPrev.x() - mOrigin.x() ),
      static_cast<float>( ptPrev.y() - mOrigin.y() ),
      static_cast<float>( std::isnan( ptPrev.z() ) ? 0 : ptPrev.z() - mOrigin.z() ) );

    const QVector3D pt2(
      static_cast<float>( pt.x() - mOrigin.x() ),
      static_cast<float>( pt.y() - mOrigin.y() ),
      static_cast<float>( std::isnan( pt.z() ) ? 0 : pt.z() - mOrigin.z() ) );

    // make a quad
    addExtrusionWallQuad( pt1, pt2, extrusionHeight );
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
  const double exp = 1e10;   // round to 10 decimal digits
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
    QVector4D v( *srcXData++ - pt0.x(),
                 *srcYData++ - pt0.y(),
                 srcZData ? *srcZData++ - pt0.z() : 0,
                 0 );
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

QVector<float> QgsTessellator::vertexBuffer() const
{
  const size_t vertexCount = mVertexBuffer.size();
  if ( vertexCount == 0 )
    return QVector<float>();

  QVector<float> vertexData;
  vertexData.resize( vertexCount * ( stride() / sizeof( float ) ) );

  for ( auto it = mVertexBuffer.constBegin(); it != mVertexBuffer.constEnd(); ++it )
  {
    const VertexPoint &vertex = it.key();
    uint32_t index = it.value();

    int offset = stride() / sizeof( float );

    vertexData[ index * offset ] = vertex.position.x();
    vertexData[ index * offset + 1 ] = vertex.position.y();
    vertexData[ index * offset + 2 ] = vertex.position.z();

    if ( mAddNormals )
    {
      vertexData[ index * offset + 3 ] = vertex.normal.x();
      vertexData[ index * offset + 4 ] = vertex.normal.y();
      vertexData[ index * offset + 5 ] = vertex.normal.z();
    }

    if ( mAddTextureCoords )
    {
      vertexData[ index * offset + ( mAddNormals ? 6 : 3 ) ] = vertex.u;
      vertexData[ index * offset + ( mAddNormals ? 7 : 4 ) ] = vertex.v;
    }
  }

  return vertexData;
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
    *base = QMatrix4x4(
              pXVector.x(), pXVector.y(), pXVector.z(), 0,
              pYVector.x(), pYVector.y(), pYVector.z(), 0,
              pNormal.x(), pNormal.y(), pNormal.z(), 0,
              0, 0, 0, 0 );
  }
  else
  {
    base->setToIdentity();
  }
}

void QgsTessellator::addTriangleVertices(
  const std::array<QVector3D, 3> &points,
  QVector3D pNormal,
  float extrusionHeight,
  QMatrix4x4 *transformMatrix,
  const QgsPoint *originOffset,
  bool reverse
)
{
  // if reverse is true, the triangle vertices are added in reverse order and normal is inverted
  const QVector3D normal = reverse ? -pNormal : pNormal;
  for ( int i = 0; i < 3; ++i )
  {
    const int index = reverse ? 2 - i : i;

    // cppcheck-suppress negativeContainerIndex
    QVector3D point = points[ index ];
    const float z = mInputZValueIgnored ? 0.0f : point.z();
    QVector4D pt( point.x(), point.y(), z, 0 );

    pt = *transformMatrix * pt;

    const double fx = pt.x() - mOrigin.x() + originOffset->x();
    const double fy = pt.y() - mOrigin.y() + originOffset->y();
    const double baseHeight = mInputZValueIgnored ? 0 : pt.z() - mOrigin.z() + originOffset->z();
    const double fz = mInputZValueIgnored ? 0.0 : ( baseHeight + extrusionHeight );

    if ( baseHeight < mZMin )
      mZMin =  static_cast<float>( baseHeight );
    if ( baseHeight > mZMax )
      mZMax = static_cast<float>( baseHeight );
    if ( fz > mZMax )
      mZMax = static_cast<float>( fz );

    if ( mOutputZUp )
    {
      vertexPoint.position = QVector3D( fx, fy, fz );
      if ( mAddNormals )
      {
        vertexPoint.normal = normal;
      }
    }
    else
    {
      vertexPoint.position = QVector3D( fx, fz, -fy );
      if ( mAddNormals )
      {
        vertexPoint.normal = QVector3D( normal.x(), normal.z(), - normal.y() );
      }
    }

    if ( mAddTextureCoords )
    {
      const std::pair<float, float> pr = rotateCoords( static_cast<float>( fx ), static_cast<float>( fy ), 0.0f, 0.0f, mTextureRotation );
      vertexPoint.u = pr.first;
      vertexPoint.v = pr.second;
    }

    addVertexPoint( vertexPoint );
  }
}

void QgsTessellator::ringToEarcutPoints( const QgsLineString *ring, std::vector<std::array<double, 2>> &polyline, QHash<std::array<double, 2>*, float> *zHash )
{
  const int pCount = ring->numPoints();

  polyline.reserve( pCount );

  const double *srcXData = ring->xData();
  const double *srcYData = ring->yData();
  const double *srcZData = ring->zData();

  // earcut handles duplicates, we do not need to remove them here
  for ( int i = 0; i < pCount - 1; ++i )
  {
    const float x = static_cast<float>( *srcXData++ );
    const float y = static_cast<float>( *srcYData++ );

    std::array<double, 2> pt = { x, y };
    polyline.push_back( pt );

    if ( zHash && srcZData )
    {
      ( *zHash )[ &pt ] = *srcZData++;
    }
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
  QHash<std::array<double, 2>*, float> z;
  std::vector<std::vector<std::array<double, 2>>> rings;
  std::vector<std::array<double, 2>> polyline;

  ringToEarcutPoints( qgsgeometry_cast< const QgsLineString * >( polygonNew->exteriorRing() ), polyline, mInputZValueIgnored ? nullptr : &z );
  rings.push_back( polyline );

  for ( int i = 0; i < polygonNew->numInteriorRings(); ++i )
  {
    std::vector<std::array<double, 2>> holePolyline;
    ringToEarcutPoints( qgsgeometry_cast<const QgsLineString *>( polygonNew->interiorRing( i ) ), holePolyline, mInputZValueIgnored ? nullptr : &z );
    rings.push_back( holePolyline );
  }

  std::vector<std::array<double, 2>> points;
  for ( const auto &ring : rings )
  {
    points.insert( points.end(), ring.begin(), ring.end() );
  }

  std::vector<uint32_t> indices = mapbox::earcut<uint32_t>( rings );
  std::vector<QVector3D> trianglePoints;
  trianglePoints.reserve( points.size() );

  for ( size_t i = 0; i < indices.size(); i++ )
  {
    uint32_t vertexIndex = indices[ i ];
    std::array<double, 2> vertex = points[ vertexIndex ];

    double x = vertex[ 0 ];
    double y = vertex[ 1 ];

    float zValue = z.value( &vertex, 0.0f );

    trianglePoints.emplace_back( x / mScale, y / mScale, zValue );
  }

  return trianglePoints;
}

void QgsTessellator::addPolygon( const QgsPolygon &polygon, float extrusionHeight )
{
  const QgsLineString *exterior = qgsgeometry_cast< const QgsLineString * >( polygon.exteriorRing() );
  if ( !exterior )
    return;

  const QVector3D pNormal = !mInputZValueIgnored ? calculateNormal( exterior, mOrigin.x(), mOrigin.y(), mOrigin.z(), mInvertNormals, extrusionHeight ) : QVector3D();
  const int pCount = exterior->numPoints();
  if ( pCount == 0 )
    return;

  QMatrix4x4 base;  // identity matrix by default
  const QgsPoint ptStart( exterior->startPoint() );
  const QgsPoint extrusionOrigin( Qgis::WkbType::PointZ, ptStart.x(), ptStart.y(), std::isnan( ptStart.z() ) ? 0 : ptStart.z() );
  std::unique_ptr<QgsPolygon> polygonNew;

  if ( !mInputZValueIgnored && !qgsDoubleNear( pNormal.length(), 1, 0.001 ) )
    return;  // this should not happen - pNormal should be normalized to unit length

  const bool buildWalls = mExtrusionFaces.testFlag( Qgis::ExtrusionFace::Walls );
  const bool buildFloor = mExtrusionFaces.testFlag( Qgis::ExtrusionFace::Floor );
  const bool buildRoof = mExtrusionFaces.testFlag( Qgis::ExtrusionFace::Roof );

  if ( buildFloor || buildRoof )
  {
    calculateBaseTransform( pNormal, &base );
    polygonNew.reset( transformPolygonToNewBase( polygon, extrusionOrigin, &base, mScale ) );

    // our 3x3 matrix is orthogonal, so for inverse we only need to transpose it
    base = base.transposed();

    if ( pCount == 4 && polygon.numInteriorRings() == 0 )
    {
      Q_ASSERT( polygonNew->exteriorRing()->numPoints() >= 3 );

      const QgsLineString *triangle = qgsgeometry_cast< const QgsLineString * >( polygonNew->exteriorRing() );
      const QVector3D p1( static_cast<float>( triangle->xAt( 0 ) ), static_cast<float>( triangle->yAt( 0 ) ), static_cast<float>( triangle->zAt( 0 ) ) );
      const QVector3D p2( static_cast<float>( triangle->xAt( 1 ) ), static_cast<float>( triangle->yAt( 1 ) ), static_cast<float>( triangle->zAt( 1 ) ) );
      const QVector3D p3( static_cast<float>( triangle->xAt( 2 ) ), static_cast<float>( triangle->yAt( 2 ) ), static_cast<float>( triangle->zAt( 2 ) ) );
      const std::array<QVector3D, 3> points = { { p1, p2, p3 } };

      addTriangleVertices( points, pNormal, extrusionHeight, &base, &extrusionOrigin, false );

      if ( mAddBackFaces )
      {
        addTriangleVertices( points, pNormal, extrusionHeight, &base, &extrusionOrigin, true );
      }

      if ( extrusionHeight != 0 && buildFloor )
      {
        addTriangleVertices( points, pNormal, 0, &base, &extrusionOrigin, false );
        if ( mAddBackFaces )
        {
          addTriangleVertices( points, pNormal, 0, &base, &extrusionOrigin, true );
        }
      }
    }
    else  // we need to triangulate the polygon
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

        for ( size_t i = 0; i < trianglePoints.size(); i += 3 )
        {
          const QVector3D p1 = trianglePoints[ i + 0 ];
          const QVector3D p2 = trianglePoints[ i + 1 ];
          const QVector3D p3 = trianglePoints[ i + 2 ];
          const std::array<QVector3D, 3> points = { { p1, p2, p3 } };

          addTriangleVertices( points, pNormal, extrusionHeight, &base, &extrusionOrigin, false );

          if ( mAddBackFaces )
          {
            addTriangleVertices( points, pNormal, extrusionHeight, &base, &extrusionOrigin, true );
          }

          if ( extrusionHeight != 0 && buildFloor )
          {
            addTriangleVertices( points, pNormal, 0, &base, &extrusionOrigin, true );
            if ( mAddBackFaces )
            {
              addTriangleVertices( points, pNormal, 0, &base, &extrusionOrigin, false );
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

  QVector<VertexPoint> vertexPoints;
  vertexPoints.resize( mIndexBuffer.size() );

  for ( auto it = mVertexBuffer.constBegin(); it != mVertexBuffer.constEnd(); ++it )
  {
    const VertexPoint &vertex = it.key();
    const uint32_t index = it.value();

    vertexPoints[ index ] = vertex;
  }

  for ( size_t i = 0; i + 2 < nVals; i += 3 )
  {
    const uint32_t index1 = mIndexBuffer[ i ];
    const uint32_t index2 = mIndexBuffer[ i + 1 ];
    const uint32_t index3 = mIndexBuffer[ i + 2 ];

    const VertexPoint vertex1 = vertexPoints[ index1 ];
    const VertexPoint vertex2 = vertexPoints[ index2 ];
    const VertexPoint vertex3 = vertexPoints[ index3  ];

    if ( mOutputZUp )
    {
      const QgsPoint p1( vertex1.position.x(), vertex1.position.y(), vertex1.position.z() );
      const QgsPoint p2( vertex2.position.x(), vertex2.position.y(), vertex2.position.z() );
      const QgsPoint p3( vertex3.position.x(), vertex3.position.y(), vertex3.position.z() );
      mp->addGeometry( new QgsTriangle( p1, p2, p3 ) );
    }
    else
    {
      // tessellator geometry is x, z, -y
      const QgsPoint p1( vertex1.position.x(), -vertex1.position.z(), vertex1.position.y() );
      const QgsPoint p2( vertex2.position.x(), -vertex2.position.z(), vertex2.position.y() );
      const QgsPoint p3( vertex3.position.x(), -vertex3.position.z(), vertex3.position.y() );
      mp->addGeometry( new QgsTriangle( p1, p2, p3 ) );
    }
  }

  return mp;
}

QVector<float> QgsTessellator::data() const
{
  const size_t n = mIndexBuffer.size();
  if ( n == 0 )
    return QVector<float>();

  QVector<float> tData;
  tData.reserve( n * ( stride() / sizeof( float ) ) );

  QVector<VertexPoint> vertexPoints;
  vertexPoints.resize( n );

  for ( auto it = mVertexBuffer.constBegin(); it != mVertexBuffer.constEnd(); ++it )
  {
    const VertexPoint &vertex = it.key();
    const uint32_t index = it.value();

    vertexPoints[ index ] = vertex;
  }

  for ( uint32_t i : mIndexBuffer )
  {
    const VertexPoint vertex = vertexPoints[ i ];

    tData << vertex.position.x();
    tData << vertex.position.y();
    tData << vertex.position.z();

    if ( mAddNormals )
    {
      tData << vertex.normal.x();
      tData << vertex.normal.y();
      tData << vertex.normal.z();
    }

    if ( mAddTextureCoords )
    {
      tData << vertex.u;
      tData << vertex.v;
    }
  }

  return tData;
}
