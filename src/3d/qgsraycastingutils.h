/***************************************************************************
  qgsraycastingutils.h
  --------------------------------------
  Date                 : April 2024
  Copyright            : (C) 2024 by Matthias Kuhn
  Email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRAYCASTINGUTILS_H
#define QGSRAYCASTINGUTILS_H

#include <QVector3D>
#include <QMatrix4x4>

#define SIP_NO_FILE

class QgsAABB;
class QgsRay3D;

namespace Qt3DRender
{
  class QGeometryRenderer;
} // namespace Qt3DRender

namespace QgsRayCastingUtils
{

  /**
   * Tests whether an axis aligned box is intersected by a ray.
   * \since QGIS 4.0
   */
  bool rayBoxIntersection( const QgsRay3D &ray, const QgsAABB &nodeBbox );

  /**
   * Tests whether a triangle is intersected by a ray.
   * \since QGIS 4.0
   */
  bool rayTriangleIntersection( const QgsRay3D &ray, float maxDist, const QVector3D &a, const QVector3D &b, const QVector3D &c, QVector3D &uvw, float &t );

  /**
   * Tests whether a triangular mesh is intersected by a ray. Returns whether an intersection
   * was found. If found, it outputs the point at which the intersection happened in world coordinates and
   * the index of the intersecting triangle.
   * \since QGIS 4.0
   */
  bool rayMeshIntersection( Qt3DRender::QGeometryRenderer *geometryRenderer, const QgsRay3D &r, float maxDist, const QMatrix4x4 &worldTransform, QVector3D &intPt, int &triangleIndex );

} // namespace QgsRayCastingUtils

#endif // QGSRAYCASTINGUTILS_H
