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

#include "qgscurve.h"
#include "qgsgeometry.h"
#include "qgsmessagelog.h"
#include "qgsmultipolygon.h"
#include "qgspoint.h"
#include "qgspolygon.h"
#include "qgstriangle.h"
#include "qgis_sip.h"
#include "qgsgeometryengine.h"

#include "poly2tri.h"

#include <QtDebug>
#include <QMatrix4x4>
#include <QVector3D>
#include <QtMath>
#include <algorithm>
#include <unordered_set>

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

static void make_quad( float x0, float y0, float z0, float x1, float y1, float z1, float height, QVector<float> &data, bool addNormals, bool addTextureCoords, float textureRotation )
{
  const float dx = x1 - x0;
  const float dy = -( y1 - y0 );

  // perpendicular vector in plane to [x,y] is [-y,x]
  QVector3D vn( -dy, 0, dx );
  vn = -vn;
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
    u0 = x0;
    v0 = z0 + height;

    u1 = x1;
    v1 = z1 + height;

    u2 = x0;
    v2 = z0;

    u3 = x1;
    v3 = z1;
  }
  else
  {
    // consider y and z as the texture coowallsTextureRotationrdinates
    u0 = -y0;
    v0 = z0 + height;

    u1 = -y1;
    v1 = z1 + height;

    u2 = -y0;
    v2 = z0;

    u3 = -y1;
    v3 = z1;
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
    const std::pair<float, float> rotated = rotateCoords( textureCoordinates[i], textureCoordinates[i + 1], 0, 0, textureRotation );
    textureCoordinates[i] = rotated.first;
    textureCoordinates[i + 1] = rotated.second;
  }

  // triangle 1
  // vertice 1
  data << x0 << z0 + height << -y0;
  if ( addNormals )
    data << vn.x() << vn.y() << vn.z();
  if ( addTextureCoords )
    data << textureCoordinates[0] << textureCoordinates[1];
  // vertice 2
  data << x1 << z1 + height << -y1;
  if ( addNormals )
    data << vn.x() << vn.y() << vn.z();
  if ( addTextureCoords )
    data << textureCoordinates[2] << textureCoordinates[3];
  // verice 3
  data << x0 << z0 << -y0;
  if ( addNormals )
    data << vn.x() << vn.y() << vn.z();
  if ( addTextureCoords )
    data << textureCoordinates[4] << textureCoordinates[5];

  // triangle 2
  // vertice 1
  data << x0 << z0 << -y0;
  if ( addNormals )
    data << vn.x() << vn.y() << vn.z();
  if ( addTextureCoords )
    data << textureCoordinates[6] << textureCoordinates[7];
  // vertice 2
  data << x1 << z1 + height << -y1;
  if ( addNormals )
    data << vn.x() << vn.y() << vn.z();
  if ( addTextureCoords )
    data << textureCoordinates[8] << textureCoordinates[9];
  // vertice 3
  data << x1 << z1 << -y1;
  if ( addNormals )
    data << vn.x() << vn.y() << vn.z();
  if ( addTextureCoords )
    data << textureCoordinates[10] << textureCoordinates[11];
}


QgsTessellator::QgsTessellator( double originX, double originY, bool addNormals, bool invertNormals, bool addBackFaces, bool noZ,
                                bool addTextureCoords, int facade, float textureRotation )
  : mOriginX( originX )
  , mOriginY( originY )
  , mAddNormals( addNormals )
  , mInvertNormals( invertNormals )
  , mAddBackFaces( addBackFaces )
  , mAddTextureCoords( addTextureCoords )
  , mNoZ( noZ )
  , mTessellatedFacade( facade )
  , mTextureRotation( textureRotation )
{
  init();
}

QgsTessellator::QgsTessellator( const QgsRectangle &bounds, bool addNormals, bool invertNormals, bool addBackFaces, bool noZ,
                                bool addTextureCoords, int facade, float textureRotation )
  : mBounds( bounds )
  , mOriginX( mBounds.xMinimum() )
  , mOriginY( mBounds.yMinimum() )
  , mAddNormals( addNormals )
  , mInvertNormals( invertNormals )
  , mAddBackFaces( addBackFaces )
  , mAddTextureCoords( addTextureCoords )
  , mNoZ( noZ )
  , mTessellatedFacade( facade )
  , mTextureRotation( textureRotation )
{
  init();
}

void QgsTessellator::init()
{
  mStride = 3 * sizeof( float );
  if ( mAddNormals )
    mStride += 3 * sizeof( float );
  if ( mAddTextureCoords )
    mStride += 2 * sizeof( float );
}

static bool _isRingCounterClockWise( const QgsCurve &ring )
{
  double a = 0;
  const int count = ring.numPoints();
  Qgis::VertexType vt;
  QgsPoint pt, ptPrev;
  ring.pointAt( 0, ptPrev, vt );
  for ( int i = 1; i < count + 1; ++i )
  {
    ring.pointAt( i % count, pt, vt );
    a += ptPrev.x() * pt.y() - ptPrev.y() * pt.x();
    ptPrev = pt;
  }
  return a > 0; // clockwise if a is negative
}

static void _makeWalls( const QgsLineString &ring, bool ccw, float extrusionHeight, QVector<float> &data,
                        bool addNormals, bool addTextureCoords, double originX, double originY, float textureRotation )
{
  // we need to find out orientation of the ring so that the triangles we generate
  // face the right direction
  // (for exterior we want clockwise order, for holes we want counter-clockwise order)
  const bool is_counter_clockwise = _isRingCounterClockWise( ring );

  QgsPoint pt;
  QgsPoint ptPrev = ring.pointN( is_counter_clockwise == ccw ? 0 : ring.numPoints() - 1 );
  for ( int i = 1; i < ring.numPoints(); ++i )
  {
    pt = ring.pointN( is_counter_clockwise == ccw ? i : ring.numPoints() - i - 1 );
    float x0 = ptPrev.x() - originX, y0 = ptPrev.y() - originY;
    float x1 = pt.x() - originX, y1 = pt.y() - originY;
    const float z0 = std::isnan( ptPrev.z() ) ? 0 : ptPrev.z();
    const float z1 = std::isnan( pt.z() ) ? 0 : pt.z();

    // make a quad
    make_quad( x0, y0, z0, x1, y1, z1, extrusionHeight, data, addNormals, addTextureCoords, textureRotation );
    ptPrev = pt;
  }
}

static QVector3D _calculateNormal( const QgsLineString *curve, double originX, double originY, bool invertNormal )
{
  // if it is just plain 2D curve there is no need to calculate anything
  // because it will be a flat horizontally oriented patch
  if ( !QgsWkbTypes::hasZ( curve->wkbType() ) || curve->isEmpty() )
    return QVector3D( 0, 0, 1 );

  // often we have 3D coordinates, but Z is the same for all vertices
  // so in order to save calculation and avoid possible issues with order of vertices
  // (the calculation below may decide that a polygon faces downwards)
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
  if ( sameZ )
    return QVector3D( 0, 0, 1 );

  // Calculate the polygon's normal vector, based on Newell's method
  // https://www.khronos.org/opengl/wiki/Calculating_a_Surface_Normal
  //
  // Order of vertices is important here as it determines the front/back face of the polygon

  double nx = 0, ny = 0, nz = 0;
  pt1 = curve->pointN( 0 );

  // shift points by the tessellator's origin - this does not affect normal calculation and it may save us from losing some precision
  pt1.setX( pt1.x() - originX );
  pt1.setY( pt1.y() - originY );
  for ( int i = 1; i < curve->numPoints(); i++ )
  {
    pt2 = curve->pointN( i );
    pt2.setX( pt2.x() - originX );
    pt2.setY( pt2.y() - originY );

    if ( std::isnan( pt1.z() ) || std::isnan( pt2.z() ) )
      continue;

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


static void _normalVectorToXYVectors( const QVector3D &pNormal, QVector3D &pXVector, QVector3D &pYVector )
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

static void _ringToPoly2tri( const QgsLineString *ring, std::vector<p2t::Point *> &polyline, QHash<p2t::Point *, float> *zHash )
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


inline double _round_coord( double x )
{
  const double exp = 1e10;   // round to 10 decimal digits
  return round( x * exp ) / exp;
}


static QgsCurve *_transform_ring_to_new_base( const QgsLineString &curve, const QgsPoint &pt0, const QMatrix4x4 *toNewBase, const float scale )
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
    *xData++ = _round_coord( v.x() );
    *yData++ = _round_coord( v.y() );
    *zData++ = _round_coord( v.z() );
  }
  return new QgsLineString( x, y, z );
}


static QgsPolygon *_transform_polygon_to_new_base( const QgsPolygon &polygon, const QgsPoint &pt0, const QMatrix4x4 *toNewBase, const float scale )
{
  QgsPolygon *p = new QgsPolygon;
  p->setExteriorRing( _transform_ring_to_new_base( *qgsgeometry_cast< const QgsLineString * >( polygon.exteriorRing() ), pt0, toNewBase, scale ) );
  for ( int i = 0; i < polygon.numInteriorRings(); ++i )
    p->addInteriorRing( _transform_ring_to_new_base( *qgsgeometry_cast< const QgsLineString * >( polygon.interiorRing( i ) ), pt0, toNewBase, scale ) );
  return p;
}


double _minimum_distance_between_coordinates( const QgsPolygon &polygon )
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
      const double d = ( x0 - x1 ) * ( x0 - x1 ) + ( y0 - y1 ) * ( y0 - y1 );
      if ( d < min_d )
        min_d = d;
      x0 = x1;
      y0 = y1;
    }
  }

  return min_d != 1e20 ? std::sqrt( min_d ) : 1e20;
}

void QgsTessellator::addPolygon( const QgsPolygon &polygon, float extrusionHeight )
{
  const QgsLineString *exterior = qgsgeometry_cast< const QgsLineString * >( polygon.exteriorRing() );
  if ( !exterior )
    return;

  const QVector3D pNormal = !mNoZ ? _calculateNormal( exterior, mOriginX, mOriginY, mInvertNormals ) : QVector3D();
  const int pCount = exterior->numPoints();
  if ( pCount == 0 )
    return;

  float zMin = std::numeric_limits<float>::max();
  float zMaxBase = -std::numeric_limits<float>::max();
  float zMaxExtruded = -std::numeric_limits<float>::max();

  const float scale = mBounds.isNull() ? 1.0 : std::max( 10000.0 / mBounds.width(), 10000.0 / mBounds.height() );

  std::unique_ptr<QMatrix4x4> toNewBase, toOldBase;
  QgsPoint ptStart, pt0;
  std::unique_ptr<QgsPolygon> polygonNew;
  auto rotatePolygonToXYPlane = [&]()
  {
    if ( !mNoZ && pNormal != QVector3D( 0, 0, 1 ) )
    {
      // this is not a horizontal plane - need to reproject the polygon to a new base so that
      // we can do the triangulation in a plane
      QVector3D pXVector, pYVector;
      _normalVectorToXYVectors( pNormal, pXVector, pYVector );

      // so now we have three orthogonal unit vectors defining new base
      // let's build transform matrix. We actually need just a 3x3 matrix,
      // but Qt does not have good support for it, so using 4x4 matrix instead.
      toNewBase.reset( new QMatrix4x4(
                         pXVector.x(), pXVector.y(), pXVector.z(), 0,
                         pYVector.x(), pYVector.y(), pYVector.z(), 0,
                         pNormal.x(), pNormal.y(), pNormal.z(), 0,
                         0, 0, 0, 0 ) );

      // our 3x3 matrix is orthogonal, so for inverse we only need to transpose it
      toOldBase.reset( new QMatrix4x4( toNewBase->transposed() ) );
    }

    ptStart = QgsPoint( exterior->startPoint() );
    pt0 = QgsPoint( QgsWkbTypes::PointZ, ptStart.x(), ptStart.y(), std::isnan( ptStart.z() ) ? 0 : ptStart.z() );

    // subtract ptFirst from geometry for better numerical stability in triangulation
    // and apply new 3D vector base if the polygon is not horizontal

    polygonNew.reset( _transform_polygon_to_new_base( polygon, pt0, toNewBase.get(), scale ) );
  };

  if ( !mNoZ && !qgsDoubleNear( pNormal.length(), 1, 0.001 ) )
    return;  // this should not happen - pNormal should be normalized to unit length

  const QVector3D upVector( 0, 0, 1 );
  const float pNormalUpVectorDotProduct = QVector3D::dotProduct( upVector, pNormal );
  const float radsBetwwenUpNormal = qAcos( pNormalUpVectorDotProduct );

  const float detectionDelta = qDegreesToRadians( 10.0f );
  int facade = 0;
  if ( radsBetwwenUpNormal > M_PI_2 - detectionDelta && radsBetwwenUpNormal < M_PI_2 + detectionDelta ) facade = 1;
  else if ( radsBetwwenUpNormal > - M_PI_2 - detectionDelta && radsBetwwenUpNormal < -M_PI_2 + detectionDelta ) facade = 1;
  else facade = 2;

  if ( pCount == 4 && polygon.numInteriorRings() == 0 && ( mTessellatedFacade & facade ) )
  {
    QgsLineString *triangle = nullptr;
    if ( mAddTextureCoords )
    {
      rotatePolygonToXYPlane();
      triangle = qgsgeometry_cast< QgsLineString * >( polygonNew->exteriorRing() );
      Q_ASSERT( polygonNew->exteriorRing()->numPoints() >= 3 );
    }

    // polygon is a triangle - write vertices to the output data array without triangulation
    const double *xData = exterior->xData();
    const double *yData = exterior->yData();
    const double *zData = !mNoZ ? exterior->zData() : nullptr;
    for ( int i = 0; i < 3; i++ )
    {
      const float z = !zData ? 0 : *zData;
      if ( z < zMin )
        zMin = z;
      if ( z > zMaxBase )
        zMaxBase = z;
      if ( z > zMaxExtruded )
        zMaxExtruded = z;

      mData << *xData - mOriginX << z << - *yData + mOriginY;
      if ( mAddNormals )
        mData << pNormal.x() << pNormal.z() << - pNormal.y();
      if ( mAddTextureCoords )
      {
        std::pair<float, float> p( triangle->xAt( i ), triangle->yAt( i ) );
        if ( facade & 1 )
        {
          p = rotateCoords( p.first, p.second, 0.0f, 0.0f, mTextureRotation );
        }
        else if ( facade & 2 )
        {
          p = rotateCoords( p.first, p.second, 0.0f, 0.0f, mTextureRotation );
        }
        mData << p.first << p.second;
      }
      xData++; yData++;
      // zData can be nullptr if mNoZ or triangle is 2D
      if ( zData )
        zData++;
    }

    if ( mAddBackFaces )
    {
      // the same triangle with reversed order of coordinates and inverted normal
      for ( int i = 2; i >= 0; i-- )
      {
        mData << exterior->xAt( i ) - mOriginX << ( mNoZ ? 0 : exterior->zAt( i ) ) << - exterior->yAt( i ) + mOriginY;
        if ( mAddNormals )
          mData << -pNormal.x() << -pNormal.z() << pNormal.y();
        if ( mAddTextureCoords )
        {
          std::pair<float, float> p( triangle->xAt( i ), triangle->yAt( i ) );
          if ( facade & 1 )
          {
            p = rotateCoords( p.first, p.second, 0.0f, 0.0f, mTextureRotation );
          }
          else if ( facade & 2 )
          {
            p = rotateCoords( p.first, p.second, 0.0f, 0.0f, mTextureRotation );
          }
          mData << p.first << p.second;
        }
      }
    }
  }
  else if ( mTessellatedFacade & facade )
  {

    rotatePolygonToXYPlane();

    if ( _minimum_distance_between_coordinates( *polygonNew ) < 0.001 )
    {
      // when the distances between coordinates of input points are very small,
      // the triangulation likes to crash on numerical errors - when the distances are ~ 1e-5
      // Assuming that the coordinates should be in a projected CRS, we should be able
      // to simplify geometries that may cause problems and avoid possible crashes
      const QgsGeometry polygonSimplified = QgsGeometry( polygonNew->clone() ).simplify( 0.001 );
      if ( polygonSimplified.isNull() )
      {
        QgsMessageLog::logMessage( QObject::tr( "geometry simplification failed - skipping" ), QObject::tr( "3D" ) );
        return;
      }
      const QgsPolygon *polygonSimplifiedData = qgsgeometry_cast<const QgsPolygon *>( polygonSimplified.constGet() );
      if ( polygonSimplifiedData == nullptr || _minimum_distance_between_coordinates( *polygonSimplifiedData ) < 0.001 )
      {
        // Failed to fix that. It could be a really tiny geometry... or maybe they gave us
        // geometry in unprojected lat/lon coordinates
        QgsMessageLog::logMessage( QObject::tr( "geometry's coordinates are too close to each other and simplification failed - skipping" ), QObject::tr( "3D" ) );
        return;
      }
      else
      {
        polygonNew.reset( polygonSimplifiedData->clone() );
      }
    }

    QList< std::vector<p2t::Point *> > polylinesToDelete;
    QHash<p2t::Point *, float> z;

    // polygon exterior
    std::vector<p2t::Point *> polyline;
    _ringToPoly2tri( qgsgeometry_cast< const QgsLineString * >( polygonNew->exteriorRing() ), polyline, mNoZ ? nullptr : &z );
    polylinesToDelete << polyline;

    std::unique_ptr<p2t::CDT> cdt( new p2t::CDT( polyline ) );

    // polygon holes
    for ( int i = 0; i < polygonNew->numInteriorRings(); ++i )
    {
      std::vector<p2t::Point *> holePolyline;
      const QgsLineString *hole = qgsgeometry_cast< const QgsLineString *>( polygonNew->interiorRing( i ) );

      _ringToPoly2tri( hole, holePolyline, mNoZ ? nullptr : &z );

      cdt->AddHole( holePolyline );
      polylinesToDelete << holePolyline;
    }

    // run triangulation and write vertices to the output data array
    try
    {
      cdt->Triangulate();

      std::vector<p2t::Triangle *> triangles = cdt->GetTriangles();

      mData.reserve( mData.size() + 3 * triangles.size() * ( stride() / sizeof( float ) ) );
      for ( size_t i = 0; i < triangles.size(); ++i )
      {
        p2t::Triangle *t = triangles[i];
        for ( int j = 0; j < 3; ++j )
        {
          p2t::Point *p = t->GetPoint( j );
          QVector4D pt( p->x, p->y, mNoZ ? 0 : z[p], 0 );
          if ( toOldBase )
            pt = *toOldBase * pt;
          const double fx = ( pt.x() / scale ) - mOriginX + pt0.x();
          const double fy = ( pt.y() / scale ) - mOriginY + pt0.y();
          const double baseHeight = mNoZ ? 0 : ( pt.z() + pt0.z() );
          const double fz = mNoZ ? 0 : ( pt.z() + extrusionHeight + pt0.z() );
          if ( baseHeight < zMin )
            zMin = baseHeight;
          if ( baseHeight > zMaxBase )
            zMaxBase = baseHeight;
          if ( fz > zMaxExtruded )
            zMaxExtruded = fz;

          mData << fx << fz << -fy;
          if ( mAddNormals )
            mData << pNormal.x() << pNormal.z() << - pNormal.y();
          if ( mAddTextureCoords )
          {
            const std::pair<float, float> pr = rotateCoords( p->x, p->y, 0.0f, 0.0f, mTextureRotation );
            mData << pr.first << pr.second;
          }
        }

        if ( mAddBackFaces )
        {
          // the same triangle with reversed order of coordinates and inverted normal
          for ( int j = 2; j >= 0; --j )
          {
            p2t::Point *p = t->GetPoint( j );
            QVector4D pt( p->x, p->y, mNoZ ? 0 : z[p], 0 );
            if ( toOldBase )
              pt = *toOldBase * pt;
            const double fx = ( pt.x() / scale ) - mOriginX + pt0.x();
            const double fy = ( pt.y() / scale ) - mOriginY + pt0.y();
            const double fz = mNoZ ? 0 : ( pt.z() + extrusionHeight + pt0.z() );
            mData << fx << fz << -fy;
            if ( mAddNormals )
              mData << -pNormal.x() << -pNormal.z() << pNormal.y();
            if ( mAddTextureCoords )
            {
              const std::pair<float, float> pr = rotateCoords( p->x, p->y, 0.0f, 0.0f, mTextureRotation );
              mData << pr.first << pr.second;
            }
          }
        }
      }
    }
    catch ( ... )
    {
      QgsMessageLog::logMessage( QObject::tr( "Triangulation failed. Skipping polygon…" ), QObject::tr( "3D" ) );
    }

    for ( int i = 0; i < polylinesToDelete.count(); ++i )
      qDeleteAll( polylinesToDelete[i] );
  }

  // add walls if extrusion is enabled
  if ( extrusionHeight != 0 && ( mTessellatedFacade & 1 ) )
  {
    _makeWalls( *exterior, false, extrusionHeight, mData, mAddNormals, mAddTextureCoords, mOriginX, mOriginY, mTextureRotation );

    for ( int i = 0; i < polygon.numInteriorRings(); ++i )
      _makeWalls( *qgsgeometry_cast< const QgsLineString * >( polygon.interiorRing( i ) ), true, extrusionHeight, mData, mAddNormals, mAddTextureCoords, mOriginX, mOriginY, mTextureRotation );

    if ( zMaxBase + extrusionHeight > zMaxExtruded )
      zMaxExtruded = zMaxBase + extrusionHeight;
  }

  if ( zMin < mZMin )
    mZMin = zMin;
  if ( zMaxExtruded > mZMax )
    mZMax = zMaxExtruded;
  if ( zMaxBase > mZMax )
    mZMax = zMaxBase;
}

int QgsTessellator::dataVerticesCount() const
{
  return mData.size() / ( stride() / sizeof( float ) );
}

std::unique_ptr<QgsMultiPolygon> QgsTessellator::asMultiPolygon() const
{
  std::unique_ptr< QgsMultiPolygon > mp = std::make_unique< QgsMultiPolygon >();
  const auto nVals = mData.size();
  mp->reserve( nVals / 9 );
  for ( auto i = decltype( nVals ) {0}; i + 8 < nVals; i += 9 )
  {
    // tessellator geometry is x, z, -y
    const QgsPoint p1( mData[i + 0], -mData[i + 2], mData[i + 1] );
    const QgsPoint p2( mData[i + 3], -mData[i + 5], mData[i + 4] );
    const QgsPoint p3( mData[i + 6], -mData[i + 8], mData[i + 7] );
    mp->addGeometry( new QgsTriangle( p1, p2, p3 ) );
  }
  return mp;
}
