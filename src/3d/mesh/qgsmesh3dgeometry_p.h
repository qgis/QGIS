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

#include <Qt3DRender/QBufferDataGenerator>

#include <qgsvector3d.h>

#include "qgsmaplayerref.h"
#include "qgsmesh3dsymbol.h"
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

#define SIP_NO_FILE

namespace Qt3DRender
{
  class QAttribute;
  class QBuffer;
}

class QgsMeshLayer;

/**
 * Creates attributes and vertex/index buffers for a mesh layer
 */
class QgsMesh3dGeometry: public  Qt3DRender::QGeometry
{
  protected:
    //! Constructor
    explicit QgsMesh3dGeometry( const QgsTriangularMesh &triangularMesh,
                                const QgsVector3D &origin,
                                const QgsMesh3DSymbol *symbol,
                                QNode *parent );
    virtual ~QgsMesh3dGeometry() = default;
    void prepareVerticesPositionAttribute( Qt3DRender::QBuffer *buffer, int count, int stride, int offset );
    void prepareVerticesNormalAttribute( Qt3DRender::QBuffer *buffer, int count, int stride, int offset );
    void prepareIndexesAttribute( Qt3DRender::QBuffer *buffer, int count );

    QgsVector3D mOrigin;
    float mVertScale;
    QgsTriangularMesh mTriangulaMesh;
};

/**
 * Creates attributes and vertex/index buffers for a mesh layer that renders the dataset
 */
class QgsMeshDataset3dGeometry: public  QgsMesh3dGeometry
{
  public:
    //! Constructs a mesh layer geometry from triangular mesh.
    explicit QgsMeshDataset3dGeometry( const QgsTriangularMesh &triangularMesh,
                                       QgsMeshLayer *layer,
                                       const QgsDateTimeRange &timeRange,
                                       const QgsVector3D &origin,
                                       const QgsMesh3DSymbol *symbol,
                                       QNode *parent );

  private:
    void init();

    //! Returns the number of active faces
    int extractDataset( QVector<double> &verticaleMagnitude, QVector<double> &scalarMagnitude, QgsMeshDataBlock &verticalActiveFaceFlagValues );
    void prepareVerticesDatasetAttribute( Qt3DRender::QBuffer *buffer, int count, int stride, int offset );

    bool mIsVerticalMagnitudeRelative;
    int mVerticalGroupDatasetIndex;
    QgsDateTimeRange mTimeRange;
    QgsMapLayerRef mLayerRef;

    QgsMeshLayer *meshLayer() const;

};

/**
 * Creates attributes and vertex/index buffers for a mesh layer that renders terrain
 */
class QgsMeshTerrain3dGeometry: public  QgsMesh3dGeometry
{
  public:
    //! Constructs a mesh layer geometry from triangular mesh.
    explicit QgsMeshTerrain3dGeometry( const QgsTriangularMesh &triangularMesh,
                                       const QgsVector3D &origin,
                                       const QgsMesh3DSymbol *symbol,
                                       QNode *parent );
  private:
    void init();
};


///@endcond

#endif // QGSMESHGEOMETRY_P_H
