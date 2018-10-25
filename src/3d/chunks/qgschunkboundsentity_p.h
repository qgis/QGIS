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
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QGeometry>
#include <QVector3D>
#include <Qt3DRender/QGeometryRenderer>

class QgsAABB;
class AABBMesh;


/**
 * \ingroup 3d
 * Draws bounds of axis aligned bounding boxes
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


class LineMeshGeometry : public Qt3DRender::QGeometry
{
    Q_OBJECT

  public:
    LineMeshGeometry( Qt3DCore::QNode *parent = nullptr );

    int vertexCount()
    {
      return mVertices.size();
    }

    void setVertices( const QList<QVector3D> &vertices );

  private:
    Qt3DRender::QAttribute *mPositionAttribute = nullptr;
    Qt3DRender::QBuffer *mVertexBuffer = nullptr;
    QList<QVector3D> mVertices;

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
