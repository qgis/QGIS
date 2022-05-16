/***************************************************************************
  qgsraycastingutils_h.cpp
  --------------------------------------
  Date                 : June 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsraycastingutils_p.h"

#include "qgis.h"
#include "qgsaabb.h"

#include <Qt3DRender/QCamera>

///@cond PRIVATE


namespace QgsRayCastingUtils
{

// copied from qt3d/src/render/raycasting/qray3d_p.h + qray3d.cpp
// by KDAB, licensed under the terms of LGPL


  Ray3D::Ray3D()
    : m_direction( 0.0f, 0.0f, 1.0f )
  {
  }

  Ray3D::Ray3D( QVector3D origin, QVector3D direction, float distance )
    : m_origin( origin )
    , m_direction( direction )
    , m_distance( distance )
  {}

  QVector3D Ray3D::origin() const
  {
    return m_origin;
  }

  void Ray3D::setOrigin( QVector3D value )
  {
    m_origin = value;
  }

  QVector3D Ray3D::direction() const
  {
    return m_direction;
  }

  void Ray3D::setDirection( QVector3D value )
  {
    if ( value.isNull() )
      return;

    m_direction = value;
  }

  float Ray3D::distance() const
  {
    return m_distance;
  }

  void Ray3D::setDistance( float distance )
  {
    m_distance = distance;
  }

  QVector3D Ray3D::point( float t ) const
  {
    return m_origin + t * m_direction;
  }

  Ray3D &Ray3D::transform( const QMatrix4x4 &matrix )
  {
    m_origin = matrix * m_origin;
    m_direction = matrix.mapVector( m_direction );

    return *this;
  }

  Ray3D Ray3D::transformed( const QMatrix4x4 &matrix ) const
  {
    return Ray3D( matrix * m_origin, matrix.mapVector( m_direction ) );
  }

  bool Ray3D::operator==( const Ray3D &other ) const
  {
    return m_origin == other.origin() && m_direction == other.direction();
  }

  bool Ray3D::operator!=( const Ray3D &other ) const
  {
    return !( *this == other );
  }

  bool Ray3D::contains( QVector3D point ) const
  {
    const QVector3D ppVec( point - m_origin );
    if ( ppVec.isNull() ) // point coincides with origin
      return true;
    const float dot = QVector3D::dotProduct( ppVec, m_direction );
    if ( qFuzzyIsNull( dot ) )
      return false;
    return qFuzzyCompare( dot * dot, ppVec.lengthSquared() * m_direction.lengthSquared() );
  }

  bool Ray3D::contains( const Ray3D &ray ) const
  {
    const float dot = QVector3D::dotProduct( m_direction, ray.direction() );
    if ( !qFuzzyCompare( dot * dot, m_direction.lengthSquared() * ray.direction().lengthSquared() ) )
      return false;
    return contains( ray.origin() );
  }

  float Ray3D::projectedDistance( QVector3D point ) const
  {
    Q_ASSERT( !m_direction.isNull() );

    return QVector3D::dotProduct( point - m_origin, m_direction ) /
           m_direction.lengthSquared();
  }

  QVector3D Ray3D::project( QVector3D vector ) const
  {
    const QVector3D norm = m_direction.normalized();
    return QVector3D::dotProduct( vector, norm ) * norm;
  }


  float Ray3D::distance( QVector3D point ) const
  {
    const float t = projectedDistance( point );
    return ( point - ( m_origin + t * m_direction ) ).length();
  }

  QDebug operator<<( QDebug dbg, const Ray3D &ray )
  {
    const QDebugStateSaver saver( dbg );
    dbg.nospace() << "QRay3D(origin("
                  << ray.origin().x() << ", " << ray.origin().y() << ", "
                  << ray.origin().z() << ") - direction("
                  << ray.direction().x() << ", " << ray.direction().y() << ", "
                  << ray.direction().z() << "))";
    return dbg;
  }

}


////////////////////////////////////////////////////////////////////////////


struct box
{
  box( const QgsAABB &b )
  {
    min[0] = b.xMin; min[1] = b.yMin; min[2] = b.zMin;
    max[0] = b.xMax; max[1] = b.yMax; max[2] = b.zMax;
  }
  double min[3];
  double max[3];
};

struct ray
{
  ray( double xO, double yO, double zO, double xD, double yD, double zD )
  {
    origin[0] = xO; origin[1] = yO; origin[2] = zO;
    dir[0] = xD; dir[1] = yD; dir[2] = zD;
    dir_inv[0] = 1 / dir[0]; dir_inv[1] = 1 / dir[1]; dir_inv[2] = 1 / dir[2];
  }
  ray( const QgsRayCastingUtils::Ray3D &r )
  {
    // ignoring length...
    origin[0] = r.origin().x(); origin[1] = r.origin().y(); origin[2] = r.origin().z();
    dir[0] = r.direction().x(); dir[1] = r.direction().y(); dir[2] = r.direction().z();
    dir_inv[0] = 1 / dir[0]; dir_inv[1] = 1 / dir[1]; dir_inv[2] = 1 / dir[2];
  }
  double origin[3];
  double dir[3];
  double dir_inv[3];
};

// https://tavianator.com/fast-branchless-raybounding-box-intersections/
// https://tavianator.com/fast-branchless-raybounding-box-intersections-part-2-nans/

bool intersection( const box &b, const ray &r )
{
  double t1 = ( b.min[0] - r.origin[0] ) * r.dir_inv[0];
  double t2 = ( b.max[0] - r.origin[0] ) * r.dir_inv[0];

  double tmin = std::min( t1, t2 );
  double tmax = std::max( t1, t2 );

  for ( int i = 1; i < 3; ++i )
  {
    t1 = ( b.min[i] - r.origin[i] ) * r.dir_inv[i];
    t2 = ( b.max[i] - r.origin[i] ) * r.dir_inv[i];

    tmin = std::max( tmin, std::min( std::min( t1, t2 ), tmax ) );
    tmax = std::min( tmax, std::max( std::max( t1, t2 ), tmin ) );
  }

  return tmax > std::max( tmin, 0.0 );
}


namespace QgsRayCastingUtils
{

  bool rayBoxIntersection( const Ray3D &r, const QgsAABB &aabb )
  {
    box b( aabb );

    // intersection() does not like yMin==yMax (excludes borders)
    if ( b.min[0] == b.max[0] ) b.max[0] += 0.1;
    if ( b.min[1] == b.max[1] ) b.max[1] += 0.1;
    if ( b.min[2] == b.max[2] ) b.max[2] += 0.1;

    return intersection( b, ray( r ) );
  }

// copied from intersectsSegmentTriangle() from qt3d/src/render/backend/triangleboundingvolume.cpp
// by KDAB, licensed under the terms of LGPL
  bool rayTriangleIntersection( const Ray3D &ray,
                                QVector3D a,
                                QVector3D b,
                                QVector3D c,
                                QVector3D &uvw,
                                float &t )
  {
    // Note: a, b, c in clockwise order
    // RealTime Collision Detection page 192

    const QVector3D ab = b - a;
    const QVector3D ac = c - a;
    const QVector3D qp = ( ray.origin() - ray.point( ray.distance() ) );

    const QVector3D n = QVector3D::crossProduct( ab, ac );
    const float d = QVector3D::dotProduct( qp, n );

    if ( d <= 0.0f )
      return false;

    const QVector3D ap = ray.origin() - a;
    t = QVector3D::dotProduct( ap, n );

    if ( t < 0.0f || t > d )
      return false;

    const QVector3D e = QVector3D::crossProduct( qp, ap );
    uvw.setY( QVector3D::dotProduct( ac, e ) );

    if ( uvw.y() < 0.0f || uvw.y() > d )
      return false;

    uvw.setZ( -QVector3D::dotProduct( ab, e ) );

    if ( uvw.z() < 0.0f || uvw.y() + uvw.z() > d )
      return false;

    const float ood = 1.0f / d;
    t *= ood;
    uvw.setY( uvw.y() * ood );
    uvw.setZ( uvw.z() * ood );
    uvw.setX( 1.0f - uvw.y() - uvw.z() );

    return true;
  }

}


/// @endcond
