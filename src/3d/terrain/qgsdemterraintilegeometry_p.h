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

#define SIP_NO_FILE

#include <Qt3DExtras/qt3dextras_global.h>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <Qt3DRender/QGeometry>
#else
#include <Qt3DCore/QGeometry>
#endif
#include <QSize>

#include <QImage>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
namespace Qt3DRender
{
  class QAttribute;
  class QBuffer;
}
#else
namespace Qt3DCore
{
  class QAttribute;
  class QBuffer;
}
#endif

namespace QgsRayCastingUtils
{
  class Ray3D;
}

/**
 * \ingroup 3d
 * \brief Stores attributes and vertex/index buffers for one terrain tile based on DEM.
 * \since QGIS 3.0
 */
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
class DemTerrainTileGeometry : public Qt3DRender::QGeometry
#else
class DemTerrainTileGeometry : public Qt3DCore::QGeometry
#endif
{
    Q_OBJECT

  public:

    /**
     * Constructs a terrain tile geometry. Resolution is the number of vertices on one side of the tile,
     * heightMap is array of float values with one height value for each vertex
     */
    explicit DemTerrainTileGeometry( int resolution, float side, float vertScale, float skirtHeight, const QByteArray &heightMap, QNode *parent = nullptr );

    bool rayIntersection( const QgsRayCastingUtils::Ray3D &ray, const QMatrix4x4 &worldTransform, QVector3D &intersectionPoint );

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    Qt3DRender::QAttribute *positionAttribute() { return mPositionAttribute; }
    Qt3DRender::QAttribute *normalAttribute() { return mNormalAttribute; }
    Qt3DRender::QAttribute *texCoordsAttribute() { return mTexCoordAttribute; }
    Qt3DRender::QAttribute *indexAttribute() { return mIndexAttribute; }
#else
    Qt3DCore::QAttribute *positionAttribute() { return mPositionAttribute; }
    Qt3DCore::QAttribute *normalAttribute() { return mNormalAttribute; }
    Qt3DCore::QAttribute *texCoordsAttribute() { return mTexCoordAttribute; }
    Qt3DCore::QAttribute *indexAttribute() { return mIndexAttribute; }
#endif

  private:
    void init();

    int mResolution;
    float mSide;
    float mVertScale;
    float mSkirtHeight;
    QByteArray mHeightMap;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    Qt3DRender::QAttribute *mPositionAttribute = nullptr;
    Qt3DRender::QAttribute *mNormalAttribute = nullptr;
    Qt3DRender::QAttribute *mTexCoordAttribute = nullptr;
    Qt3DRender::QAttribute *mIndexAttribute = nullptr;
    Qt3DRender::QBuffer *mVertexBuffer = nullptr;
    Qt3DRender::QBuffer *mIndexBuffer = nullptr;
#else
    Qt3DCore::QAttribute *mPositionAttribute = nullptr;
    Qt3DCore::QAttribute *mNormalAttribute = nullptr;
    Qt3DCore::QAttribute *mTexCoordAttribute = nullptr;
    Qt3DCore::QAttribute *mIndexAttribute = nullptr;
    Qt3DCore::QBuffer *mVertexBuffer = nullptr;
    Qt3DCore::QBuffer *mIndexBuffer = nullptr;
#endif
};

/// @endcond

#endif  // QGSDEMTERRAINTILEGEOMETRY_P_H
