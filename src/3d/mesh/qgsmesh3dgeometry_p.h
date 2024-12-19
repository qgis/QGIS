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

#include <QFuture>

#include <Qt3DExtras/qt3dextras_global.h>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <Qt3DRender/QGeometry>
#else
#include <Qt3DCore/QGeometry>
#endif
#include <QVector3D>

#include <qgsvector3d.h>

#include "qgsmaplayerref.h"
#include "qgsmesh3dsymbol.h"
#include "qgsrectangle.h"
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

class QgsMeshLayer;

/**
 * Class that constructs the geometry 3D mesh on another thread
 */
class QgsMesh3DGeometryBuilder: public QObject
{
    Q_OBJECT
  public:
    QgsMesh3DGeometryBuilder( const QgsTriangularMesh &mesh,
                              const QgsVector3D &origin,
                              const QgsRectangle &extent,
                              float vertScale,
                              QObject *parent );

    virtual void start();

    QByteArray vertexData() const {return mFutureVertex.result();}
    QByteArray indexData() const {return mFutureIndex.result();}

  signals:
    void dataIsReady();

  protected slots:
    void vertexFinished();
    void indexFinished();

  protected:
    QFuture<QByteArray> mFutureVertex;
    QFuture<QByteArray> mFutureIndex;
    QFutureWatcher<QByteArray> *mWatcherVertex = nullptr;
    QFutureWatcher<QByteArray> *mWatcherIndex = nullptr;

    QgsTriangularMesh mMesh;
    QgsVector3D mOrigin;
    QgsRectangle mExtent;
    float mVertScale;

    mutable QMutex mMutex;
    bool mVertexFinished = false;
    bool mIndexFinished = false;
    bool mIsCanceled = false;
};


/**
* Base class for creating attributes and vertex/index buffers for a mesh layer
*/
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
class QgsMesh3DGeometry: public Qt3DRender::QGeometry
#else
class QgsMesh3DGeometry: public Qt3DCore::QGeometry
#endif
{
    Q_OBJECT
  protected:
    //! Constructor
    explicit QgsMesh3DGeometry( const QgsTriangularMesh &triangularMesh,
                                const QgsVector3D &origin,
                                const QgsRectangle &extent,
                                double verticalScale,
                                QNode *parent );

    ~QgsMesh3DGeometry() = default;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    void prepareVerticesPositionAttribute( Qt3DRender::QBuffer *buffer, int stride, int offset );
    void prepareVerticesNormalAttribute( Qt3DRender::QBuffer *buffer, int stride, int offset );
    void prepareIndexesAttribute( Qt3DRender::QBuffer *buffer );
#else
    void prepareVerticesPositionAttribute( Qt3DCore::QBuffer *buffer, int stride, int offset );
    void prepareVerticesNormalAttribute( Qt3DCore::QBuffer *buffer, int stride, int offset );
    void prepareIndexesAttribute( Qt3DCore::QBuffer *buffer );
#endif

    QgsVector3D mOrigin;
    QgsRectangle mExtent;
    float mVertScale;
    QgsTriangularMesh mTriangulaMesh;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    Qt3DRender::QBuffer *mVertexBuffer = nullptr;
    Qt3DRender::QBuffer *mIndexBuffer = nullptr;

    Qt3DRender::QAttribute *mPositionAttribute = nullptr;
    Qt3DRender::QAttribute *mNormalAttribute = nullptr;
    Qt3DRender::QAttribute *mIndexAttribute = nullptr;
#else
    Qt3DCore::QBuffer *mVertexBuffer = nullptr;
    Qt3DCore::QBuffer *mIndexBuffer = nullptr;

    Qt3DCore::QAttribute *mPositionAttribute = nullptr;
    Qt3DCore::QAttribute *mNormalAttribute = nullptr;
    Qt3DCore::QAttribute *mIndexAttribute = nullptr;
#endif

    QgsMesh3DGeometryBuilder *mBuilder;

  protected slots:
    virtual void getData();
};

class QgsMeshDataset3DGeometryBuilder;

/**
 *  Base class for creating attributes and vertex/index buffers for mesh dataset
 *
 *  On creation, the instance prepare all needed data from the mesh layer but 3D geometry is not defined.
 *  Then the instance launches immediately another thread that constructs 3D vertices, faces  and scalar value on vertices
 *  depending on the dataset chosen for vertical magnitude and the one for scalar magnitude (color rendering).
 *
 *  When this job is finished, the mesh dataset 3D geometry node is updated and can be rendered in the 3D scene.
 */
class QgsMeshDataset3DGeometry: public QgsMesh3DGeometry
{
    Q_OBJECT
  public:
    //! Constructs a mesh layer geometry from triangular mesh.
    explicit QgsMeshDataset3DGeometry( const QgsTriangularMesh &triangularMesh,
                                       QgsMeshLayer *layer,
                                       const QgsDateTimeRange &timeRange,
                                       const QgsVector3D &origin,
                                       const QgsRectangle &extent,
                                       const QgsMesh3DSymbol *symbol,
                                       QNode *parent );

    //! Data sended to the other thread to consctruct the 3D geometry
    struct VertexData
    {
      QgsMeshDatasetGroupMetadata verticalGroupMetadata;
      QgsMeshDatasetGroupMetadata scalarGroupMetadata;
      QgsMeshDataBlock verticalData;
      QgsMeshDataBlock scalarData;

      QgsMeshDataBlock activeFaceFlagValues;
      bool isVerticalMagnitudeRelative;
    };

  private slots:
    void getData() override;

  private:
    void prepareData();

    //! Returns the number of active faces
    int extractDataset( QVector<double> &verticaleMagnitude, QVector<double> &scalarMagnitude, QgsMeshDataBlock &verticalActiveFaceFlagValues );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    void prepareVerticesDatasetAttribute( Qt3DRender::QBuffer *buffer, int stride, int offset );
#else
    void prepareVerticesDatasetAttribute( Qt3DCore::QBuffer *buffer, int stride, int offset );
#endif

    bool mIsVerticalMagnitudeRelative;
    int mVerticalGroupDatasetIndex;
    QgsDateTimeRange mTimeRange;
    QgsMapLayerRef mLayerRef;

    QgsMeshLayer *meshLayer() const;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    Qt3DRender::QAttribute *mMagnitudeAttribute = nullptr;
#else
    Qt3DCore::QAttribute *mMagnitudeAttribute = nullptr;
#endif
};

class QgsMeshDataset3DGeometryBuilder: public QgsMesh3DGeometryBuilder
{
    Q_OBJECT

  public:
    QgsMeshDataset3DGeometryBuilder( const QgsTriangularMesh &mesh,
                                     const QgsMesh &nativeMesh,
                                     const QgsVector3D &origin,
                                     const QgsRectangle &extent,
                                     float vertScale,
                                     const QgsMeshDataset3DGeometry::VertexData &vertexData,
                                     QObject *parent );
    void start() override;

  private:
    QgsMesh mNativeMesh;
    QgsMeshDataset3DGeometry::VertexData mVertexData;
};

/**
 *  Base class for creating attributes and vertex/index buffers for a mesh dataset
 *
 *  On creation, the instance launches immediately another thread that constructs 3D vertices, faces of the mesh based on the mesh vertices z value.
 *  When this job is finished, the mesh terrain 3D geometry node is updated and can be rendered in the 3D scene.
 */
class QgsMeshTerrain3DGeometry: public  QgsMesh3DGeometry
{
    Q_OBJECT
  public:
    //! Constructs a mesh layer geometry from triangular mesh.
    explicit QgsMeshTerrain3DGeometry( const QgsTriangularMesh &triangularMesh,
                                       const QgsVector3D &origin,
                                       const QgsRectangle &extent,
                                       double verticalScale,
                                       QNode *parent );
};



///@endcond

#endif // QGSMESHGEOMETRY_P_H
