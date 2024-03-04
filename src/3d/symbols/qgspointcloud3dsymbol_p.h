/***************************************************************************
  qgspointcloud3dsymbol_p.h
  ------------------------------
  Date                 : December 2020
  Copyright            : (C) 2020 by Nedjima Belgacem
  Email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUD3DSYMBOL_P_H
#define QGSPOINTCLOUD3DSYMBOL_P_H

///@cond PRIVATE

#include "qgspointcloudlayer3drenderer.h"

#include <QFutureWatcher>
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <Qt3DRender/QGeometry>
#include <Qt3DRender/QBuffer>
#else
#include <Qt3DCore/QGeometry>
#include <Qt3DCore/QBuffer>
#endif
#include <Qt3DRender/QMaterial>
#include <QVector3D>

#define SIP_NO_FILE

class IndexedPointCloudNode;
class QgsAABB;

class QgsPointCloud3DSymbolHandler // : public QgsFeature3DHandler
{
  public:
    QgsPointCloud3DSymbolHandler();

    virtual ~QgsPointCloud3DSymbolHandler() = default;

    virtual bool prepare( const QgsPointCloud3DRenderContext &context ) = 0;// override;
    virtual void processNode( QgsPointCloudIndex *pc, const IndexedPointCloudNode &n, const QgsPointCloud3DRenderContext &context ) = 0; // override;
    virtual void finalize( Qt3DCore::QEntity *parent, const QgsPointCloud3DRenderContext &context ) = 0;// override;

    void triangulate( QgsPointCloudIndex *pc, const IndexedPointCloudNode &n, const QgsPointCloud3DRenderContext &context, const QgsAABB &bbox );

    float zMinimum() const { return mZMin; }
    float zMaximum() const { return mZMax; }

    //! temporary data we will pass to the tessellator
    struct PointData
    {
      QVector<QVector3D> positions;  // Contains triplets of float x,y,z for each point
      QVector<float> parameter;
      QVector<float> pointSizes; // Contains point sizes, in case they are overridden for classification renderer
      QVector<QVector3D> colors;
      QByteArray triangles; // In case of points triangulation, contains index of point in the array positions
      QByteArray normals; // In case of points triangulation, contains the normals of the solid surface on each vertex
    };

  protected:
    float mZMin = std::numeric_limits<float>::max();
    float mZMax = std::numeric_limits<float>::lowest();

    void makeEntity( Qt3DCore::QEntity *parent, const QgsPointCloud3DRenderContext &context, const PointData &out, bool selected );

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    virtual Qt3DRender::QGeometry *makeGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride ) = 0;
#else
    virtual Qt3DCore::QGeometry *makeGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride ) = 0;
#endif
    std::unique_ptr<QgsPointCloudBlock> pointCloudBlock( QgsPointCloudIndex *pc, const IndexedPointCloudNode &node, const QgsPointCloudRequest &request, const QgsPointCloud3DRenderContext &context );

    // outputs
    PointData outNormal;  //!< Features that are not selected

  private:
    //! Returns all vertices of the node \a n, and of its parents contained in \a bbox and in an extension of this box depending of the density of the points
    std::vector<double> getVertices( QgsPointCloudIndex *pc, const IndexedPointCloudNode &n, const QgsPointCloud3DRenderContext &context, const QgsAABB &bbox );

    //! Calculates the normals of triangles dedined by index contained in \a triangles. Must be used only in the method triangulate().
    void calculateNormals( const std::vector<size_t> &triangles );

    /**
     * Applies a filter on triangles to improve the rendering:
     *
     * - keeps only triangles that have a least one point in the bounding box \a bbox
     * - if options are selected, skips triangles with horizontal or vertical size greater than a threshold
     *
     * Must be used only in the method triangulate().
     */
    void filterTriangles( const std::vector<size_t> &triangleIndexes, const QgsPointCloud3DRenderContext &context, const QgsAABB &bbox );
};

class QgsSingleColorPointCloud3DSymbolHandler : public QgsPointCloud3DSymbolHandler
{
  public:
    QgsSingleColorPointCloud3DSymbolHandler();

    bool prepare( const QgsPointCloud3DRenderContext &context ) override;
    void processNode( QgsPointCloudIndex *pc, const IndexedPointCloudNode &n, const QgsPointCloud3DRenderContext &context ) override;
    void finalize( Qt3DCore::QEntity *parent, const QgsPointCloud3DRenderContext &context ) override;

  private:
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    Qt3DRender::QGeometry *makeGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride ) override;
#else
    Qt3DCore::QGeometry *makeGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride ) override;
#endif
};

class QgsColorRampPointCloud3DSymbolHandler : public QgsPointCloud3DSymbolHandler
{
  public:
    QgsColorRampPointCloud3DSymbolHandler();

    bool prepare( const QgsPointCloud3DRenderContext &context ) override;
    void processNode( QgsPointCloudIndex *pc, const IndexedPointCloudNode &n, const QgsPointCloud3DRenderContext &context ) override;
    void finalize( Qt3DCore::QEntity *parent, const QgsPointCloud3DRenderContext &context ) override;

  private:
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    Qt3DRender::QGeometry *makeGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride ) override;
#else
    Qt3DCore::QGeometry *makeGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride ) override;
#endif
};

class QgsRGBPointCloud3DSymbolHandler : public QgsPointCloud3DSymbolHandler
{
  public:
    QgsRGBPointCloud3DSymbolHandler();

    bool prepare( const QgsPointCloud3DRenderContext &context ) override;
    void processNode( QgsPointCloudIndex *pc, const IndexedPointCloudNode &n, const QgsPointCloud3DRenderContext &context ) override;
    void finalize( Qt3DCore::QEntity *parent, const QgsPointCloud3DRenderContext &context ) override;

  private:
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    Qt3DRender::QGeometry *makeGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride ) override;
#else
    Qt3DCore::QGeometry *makeGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride ) override;
#endif
};

class QgsClassificationPointCloud3DSymbolHandler : public QgsPointCloud3DSymbolHandler
{
  public:
    QgsClassificationPointCloud3DSymbolHandler();

    bool prepare( const QgsPointCloud3DRenderContext &context ) override;
    void processNode( QgsPointCloudIndex *pc, const IndexedPointCloudNode &n, const QgsPointCloud3DRenderContext &context ) override;
    void finalize( Qt3DCore::QEntity *parent, const QgsPointCloud3DRenderContext &context ) override;

  private:
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    Qt3DRender::QGeometry *makeGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride ) override;
#else
    Qt3DCore::QGeometry *makeGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride ) override;
#endif
};

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
class QgsPointCloud3DGeometry: public Qt3DRender::QGeometry
#else
class QgsPointCloud3DGeometry: public Qt3DCore::QGeometry
#endif
{
    Q_OBJECT

  public:
    QgsPointCloud3DGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride );

  protected:
    virtual void makeVertexBuffer( const QgsPointCloud3DSymbolHandler::PointData &data ) = 0;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    Qt3DRender::QAttribute *mPositionAttribute = nullptr;
    Qt3DRender::QAttribute *mParameterAttribute = nullptr;
    Qt3DRender::QAttribute *mPointSizeAttribute = nullptr;
    Qt3DRender::QAttribute *mColorAttribute = nullptr;
    Qt3DRender::QAttribute *mTriangleIndexAttribute = nullptr;
    Qt3DRender::QAttribute *mNormalsAttribute = nullptr;
    Qt3DRender::QBuffer *mVertexBuffer = nullptr;
    Qt3DRender::QBuffer *mTriangleBuffer = nullptr;
    Qt3DRender::QBuffer *mNormalsBuffer = nullptr;
#else
    Qt3DCore::QAttribute *mPositionAttribute = nullptr;
    Qt3DCore::QAttribute *mParameterAttribute = nullptr;
    Qt3DCore::QAttribute *mPointSizeAttribute = nullptr;
    Qt3DCore::QAttribute *mColorAttribute = nullptr;
    Qt3DCore::QAttribute *mTriangleIndexAttribute = nullptr;
    Qt3DCore::QAttribute *mNormalsAttribute = nullptr;
    Qt3DCore::QBuffer *mVertexBuffer = nullptr;
    Qt3DCore::QBuffer *mTriangleBuffer = nullptr;
    Qt3DCore::QBuffer *mNormalsBuffer = nullptr;
#endif
    int mVertexCount = 0;

    unsigned int mByteStride = 16;
};

class QgsSingleColorPointCloud3DGeometry : public QgsPointCloud3DGeometry
{
    Q_OBJECT

  public:
    QgsSingleColorPointCloud3DGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride );

  private:
    void makeVertexBuffer( const QgsPointCloud3DSymbolHandler::PointData &data ) override;
};

class QgsColorRampPointCloud3DGeometry : public QgsPointCloud3DGeometry
{
    Q_OBJECT

  public:
    QgsColorRampPointCloud3DGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride );

  private:
    void makeVertexBuffer( const QgsPointCloud3DSymbolHandler::PointData &data ) override;
};

class QgsRGBPointCloud3DGeometry : public QgsPointCloud3DGeometry
{
    Q_OBJECT

  public:
    QgsRGBPointCloud3DGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride );
  private:
    void makeVertexBuffer( const QgsPointCloud3DSymbolHandler::PointData &data ) override;
};

class QgsClassificationPointCloud3DGeometry : public QgsPointCloud3DGeometry
{
    Q_OBJECT

  public:
    QgsClassificationPointCloud3DGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride );

  private:
    void makeVertexBuffer( const QgsPointCloud3DSymbolHandler::PointData &data ) override;
};
/// @endcond

#endif // QGSPOINTCLOUD3DSYMBOL_P_H
