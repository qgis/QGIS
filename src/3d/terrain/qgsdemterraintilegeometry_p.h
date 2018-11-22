/***************************************************************************
  qgsdemterraintilegeometry_p.h
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

#ifndef QGSDEMTERRAINTILEGEOMETRY_P_H
#define QGSDEMTERRAINTILEGEOMETRY_P_H

///@cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include <Qt3DExtras/qt3dextras_global.h>
#include <Qt3DRender/qgeometry.h>
#include <QSize>

#include <QImage>

namespace Qt3DRender
{

  class QAttribute;
  class QBuffer;

} // Qt3DRender

namespace QgsRayCastingUtils
{
  class Ray3D;
}

/**
 * \ingroup 3d
 * Stores attributes and vertex/index buffers for one terrain tile based on DEM.
 * \since QGIS 3.0
 */
class DemTerrainTileGeometry : public Qt3DRender::QGeometry
{
  public:

    /**
     * Constructs a terrain tile geometry. Resolution is the number of vertices on one side of the tile,
     * heightMap is array of float values with one height value for each vertex
     */
    explicit DemTerrainTileGeometry( int resolution, float side, float vertScale, float skirtHeight, const QByteArray &heightMap, QNode *parent = nullptr );

    bool rayIntersection( const QgsRayCastingUtils::Ray3D &ray, const QMatrix4x4 &worldTransform, QVector3D &intersectionPoint );

  private:
    void init();

    int mResolution;
    float mSide;
    float mVertScale;
    float mSkirtHeight;
    QByteArray mHeightMap;
    Qt3DRender::QAttribute *mPositionAttribute = nullptr;
    Qt3DRender::QAttribute *mNormalAttribute = nullptr;
    Qt3DRender::QAttribute *mTexCoordAttribute = nullptr;
    Qt3DRender::QAttribute *mIndexAttribute = nullptr;
    Qt3DRender::QBuffer *mVertexBuffer = nullptr;
    Qt3DRender::QBuffer *mIndexBuffer = nullptr;
};

/// @endcond

#endif  // QGSDEMTERRAINTILEGEOMETRY_P_H
