/***************************************************************************
  qgschunkboundsentity_p.h
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

#ifndef QGSCHUNKBOUNDSENTITY_P_H
#define QGSCHUNKBOUNDSENTITY_P_H

///@cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

#include <Qt3DCore/QEntity>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QGeometry>
typedef Qt3DRender::QAttribute Qt3DQAttribute;
typedef Qt3DRender::QGeometry Qt3DQGeometry;
#else
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QGeometry>
typedef Qt3DCore::QAttribute Qt3DQAttribute;
typedef Qt3DCore::QGeometry Qt3DQGeometry;
#endif
#include <QVector3D>
#include <Qt3DRender/QGeometryRenderer>

class QgsAABB;
class AABBMesh;

#define SIP_NO_FILE


/**
 * \ingroup 3d
 * \brief Draws bounds of axis aligned bounding boxes
 * \note Not available in Python bindings
 * \since QGIS 3.0
 */
class QgsChunkBoundsEntity : public Qt3DCore::QEntity
{
    Q_OBJECT

  public:
    //! Constructs the entity
    QgsChunkBoundsEntity( Qt3DCore::QNode *parent = nullptr );

    //! Sets a list of bounding boxes to be rendered by the entity
    void setBoxes( const QList<QgsAABB> &bboxes );

  private:
    AABBMesh *mAabbMesh = nullptr;
};


class LineMeshGeometry : public Qt3DQGeometry
{
    Q_OBJECT

  public:
    LineMeshGeometry( Qt3DCore::QNode *parent = nullptr );

    int vertexCount()
    {
      return mVertexCount;
    }

    void setVertices( const QList<QVector3D> &vertices );

  private:
    Qt3DQAttribute *mPositionAttribute = nullptr;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    Qt3DRender::QBuffer *mVertexBuffer = nullptr;
#else
    Qt3DCore::QBuffer *mVertexBuffer = nullptr;
#endif
    int mVertexCount = 0;

};


//! Geometry renderer for axis aligned bounding boxes - draws a box edges as lines
class AABBMesh : public Qt3DRender::QGeometryRenderer
{
    Q_OBJECT

  public:
    AABBMesh( Qt3DCore::QNode *parent = nullptr );

    void setBoxes( const QList<QgsAABB> &bboxes );

  private:
    LineMeshGeometry *mLineMeshGeo = nullptr;
};

/// @endcond

#endif // QGSCHUNKBOUNDSENTITY_P_H
