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

#include "qgspointcloud3dsymbol.h"

#include "qgschunkloader_p.h"
#include "qgsfeature3dhandler_p.h"
#include "qgschunkedentity_p.h"

#include "qgspointcloudlayer3drenderer.h"

#include <QFutureWatcher>
#include <Qt3DRender/QGeometry>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QMaterial>
#include <QVector3D>

#define SIP_NO_FILE

class IndexedPointCloudNode;

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
      QVector<QVector3D> positions;  // contains triplets of float x,y,z for each point
      QVector<float> parameter;
      QVector<QVector3D> colors;
      QByteArray triangles; // in case of points triangulation, contains index of point in the array positions
      QByteArray normals; // in case of points triangulation, contains the normals of the solid surface on each vertex
    };

  protected:
    float mZMin = std::numeric_limits<float>::max();
    float mZMax = std::numeric_limits<float>::lowest();

    void makeEntity( Qt3DCore::QEntity *parent, const QgsPointCloud3DRenderContext &context, const PointData &out, bool selected );
    virtual Qt3DRender::QGeometry *makeGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride ) = 0;
    QgsPointCloudBlock *pointCloudBlock( QgsPointCloudIndex *pc, const IndexedPointCloudNode &node, const QgsPointCloudRequest &request, const QgsPointCloud3DRenderContext &context );

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
     * - if options are selected, skips triangles with horizontale or vertical size greater than a threshold
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
    Qt3DRender::QGeometry *makeGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride ) override;
};

class QgsColorRampPointCloud3DSymbolHandler : public QgsPointCloud3DSymbolHandler
{
  public:
    QgsColorRampPointCloud3DSymbolHandler();

    bool prepare( const QgsPointCloud3DRenderContext &context ) override;
    void processNode( QgsPointCloudIndex *pc, const IndexedPointCloudNode &n, const QgsPointCloud3DRenderContext &context ) override;
    void finalize( Qt3DCore::QEntity *parent, const QgsPointCloud3DRenderContext &context ) override;

  private:
    Qt3DRender::QGeometry *makeGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride ) override;
};

class QgsRGBPointCloud3DSymbolHandler : public QgsPointCloud3DSymbolHandler
{
  public:
    QgsRGBPointCloud3DSymbolHandler();

    bool prepare( const QgsPointCloud3DRenderContext &context ) override;
    void processNode( QgsPointCloudIndex *pc, const IndexedPointCloudNode &n, const QgsPointCloud3DRenderContext &context ) override;
    void finalize( Qt3DCore::QEntity *parent, const QgsPointCloud3DRenderContext &context ) override;

  private:
    Qt3DRender::QGeometry *makeGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride ) override;
};

class QgsClassificationPointCloud3DSymbolHandler : public QgsPointCloud3DSymbolHandler
{
  public:
    QgsClassificationPointCloud3DSymbolHandler();

    bool prepare( const QgsPointCloud3DRenderContext &context ) override;
    void processNode( QgsPointCloudIndex *pc, const IndexedPointCloudNode &n, const QgsPointCloud3DRenderContext &context ) override;
    void finalize( Qt3DCore::QEntity *parent, const QgsPointCloud3DRenderContext &context ) override;

  private:
    Qt3DRender::QGeometry *makeGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride ) override;
};


class QgsPointCloud3DGeometry: public Qt3DRender::QGeometry
{
  public:
    QgsPointCloud3DGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride );

  protected:
    virtual void makeVertexBuffer( const QgsPointCloud3DSymbolHandler::PointData &data ) = 0;

    Qt3DRender::QAttribute *mPositionAttribute = nullptr;
    Qt3DRender::QAttribute *mParameterAttribute = nullptr;
    Qt3DRender::QAttribute *mColorAttribute = nullptr;
    Qt3DRender::QAttribute *mTriangleIndexAttribute = nullptr;
    Qt3DRender::QAttribute *mNormalsAttribute = nullptr;
    Qt3DRender::QBuffer *mVertexBuffer = nullptr;
    Qt3DRender::QBuffer *mTriangleBuffer = nullptr;
    Qt3DRender::QBuffer *mNormalsBuffer = nullptr;
    int mVertexCount = 0;

    unsigned int mByteStride = 16;
};

class QgsSingleColorPointCloud3DGeometry : public QgsPointCloud3DGeometry
{
  public:
    QgsSingleColorPointCloud3DGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride );

  private:
    void makeVertexBuffer( const QgsPointCloud3DSymbolHandler::PointData &data ) override;
};

class QgsColorRampPointCloud3DGeometry : public QgsPointCloud3DGeometry
{
  public:
    QgsColorRampPointCloud3DGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride );

  private:
    void makeVertexBuffer( const QgsPointCloud3DSymbolHandler::PointData &data ) override;
};

class QgsRGBPointCloud3DGeometry : public QgsPointCloud3DGeometry
{
  public:
    QgsRGBPointCloud3DGeometry( Qt3DCore::QNode *parent, const QgsPointCloud3DSymbolHandler::PointData &data, unsigned int byteStride );
  private:
    void makeVertexBuffer( const QgsPointCloud3DSymbolHandler::PointData &data ) override;
};


/// @endcond

#endif // QGSPOINTCLOUD3DSYMBOL_P_H
