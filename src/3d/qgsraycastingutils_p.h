/***************************************************************************
  qgsraycastingutils_p.h
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

#ifndef QGSRAYCASTINGUTILS_P_H
#define QGSRAYCASTINGUTILS_P_H

///@cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include <QVector3D>

#define SIP_NO_FILE

class QgsAABB;

namespace Qt3DRender
{
  class QCamera;
}


namespace QgsRayCastingUtils
{

  /**
   * Utility class to handle 3D rays. A ray has an origin, a direction and distance.
   * \note With switch to Qt 5.11 we may remove it and use QRayCaster/QScreenRayCaster instead.
   * \since QGIS 3.4
   */
  class Ray3D
  {
    public:
      Ray3D();
      explicit Ray3D( QVector3D origin, QVector3D direction = QVector3D( 0.0f, 0.0f, 1.0f ), float distance = 1.0f );

      QVector3D origin() const;
      void setOrigin( QVector3D value );

      QVector3D direction() const;
      void setDirection( QVector3D value );

      float distance() const;
      void setDistance( float distance );

      bool contains( QVector3D point ) const;
      bool contains( const Ray3D &ray ) const;

      QVector3D point( float t ) const;
      float projectedDistance( QVector3D point ) const;

      QVector3D project( QVector3D vector ) const;

      float distance( QVector3D point ) const;

      Ray3D &transform( const QMatrix4x4 &matrix );
      Ray3D transformed( const QMatrix4x4 &matrix ) const;

      // TODO c++20 - replace with = default
      bool operator==( const Ray3D &other ) const;
      bool operator!=( const Ray3D &other ) const;

    private:
      QVector3D m_origin;
      QVector3D m_direction;
      float m_distance = 1.0f;
  };

  /**
   * Tests whether axis-aligned bounding box is intersected by a ray.
   * \note ray's distance is ignored (considered infinite)
   * \note With switch to Qt 5.11 we may remove it and use QRayCaster/QScreenRayCaster instead.
   * \since QGIS 3.4
   */
  bool rayBoxIntersection( const Ray3D &r, const QgsAABB &aabb );

  /**
   * Tests whether a triangle is intersected by a ray.
   * \note With switch to Qt 5.11 we may remove it and use QRayCaster/QScreenRayCaster instead.
   * \since QGIS 3.4
   */
  bool rayTriangleIntersection( const Ray3D &ray,
                                QVector3D a,
                                QVector3D b,
                                QVector3D c,
                                QVector3D &uvw,
                                float &t );
}

/// @endcond

#endif // QGSRAYCASTINGUTILS_P_H
