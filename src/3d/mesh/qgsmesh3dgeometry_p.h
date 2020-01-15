/***************************************************************************
                         qgsmesh3dgeometry_p.h
                         -------------------------
    begin                : january 2020
    copyright            : (C) 2020 by Vincent Cloarec
    email                : vcloarec at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHGEOMETRY_P_H
#define QGSMESHGEOMETRY_P_H

#include <Qt3DExtras/qt3dextras_global.h>
#include <Qt3DRender/qgeometry.h>
#include <QVector3D>

#include "qgstriangularmesh.h"

///@cond PRIVATE

//
//  W A R N I N G
//  -------------
//
// This file is not part of the QGIS API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//

namespace Qt3DRender
{
  class QAttribute;
  class QBuffer;
}

/**
 * \ingroup 3d
 * Stores attributes and vertex/index buffers for a mesh layer
 * \since QGIS 3.12
 */
class QgsMesh3dGeometry: public  Qt3DRender::QGeometry
{
  public:

    //! Constructs a mesh layer geometry from triangular mesh.
    explicit QgsMesh3dGeometry( const QgsTriangularMesh &mesh, const QgsRectangle &extent, float verticaleScale, QNode *parent );

  private:
    void init();

    QgsTriangularMesh mTriangularMesh;
    QgsRectangle mExtent;
    float mVertScale;

    Qt3DRender::QAttribute *mPositionAttribute = nullptr;
    Qt3DRender::QAttribute *mNormalAttribute = nullptr;
    Qt3DRender::QAttribute *mTexCoordAttribute = nullptr;
    Qt3DRender::QAttribute *mIndexAttribute = nullptr;
    Qt3DRender::QBuffer *mVertexBuffer = nullptr;
    Qt3DRender::QBuffer *mIndexBuffer = nullptr;

};

class QgsMesh3dDatasetGeometry : public Qt3DRender::QGeometry
{
  public:
    //! Constructs a mesh layer geometry from triangular mesh.
    explicit QgsMesh3dDatasetGeometry( const QgsTriangularMesh &mesh,
                                       const QVector<double> &verticaleMagnitude,
                                       const QVector<double> &scalarMagnitude,
                                       const QgsRectangle &extent,
                                       float verticaleScale, QNode *parent );
  private:
    void init();

    QgsTriangularMesh mTriangularMesh;
    QVector<double> mVerticaleMagnitude;
    QVector<double> mScalarMagnitude;
    QgsRectangle mExtent;
    float mVertScale;

    Qt3DRender::QAttribute *mPositionAttribute = nullptr;
    Qt3DRender::QAttribute *mNormalAttribute = nullptr;

    //! attribute used to store the scalarMagnitude used to render color
    Qt3DRender::QAttribute *mVertexScalarMagnitudeAttribute = nullptr;

    Qt3DRender::QAttribute *mTexCoordAttribute = nullptr;
    Qt3DRender::QAttribute *mIndexAttribute = nullptr;
    Qt3DRender::QBuffer *mVertexBuffer = nullptr;
    Qt3DRender::QBuffer *mIndexBuffer = nullptr;
};

///@endcond

#endif // QGSMESHGEOMETRY_P_H
