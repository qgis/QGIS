/***************************************************************************
                         qgsmeshvectorrenderer.h
                         -------------------
    begin                : May 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHVECTORRENDERER_H
#define QGSMESHVECTORRENDERER_H


#define SIP_NO_FILE

#include <QVector>
#include <QSize>

#include "qgis_core.h"
#include "qgsmeshdataprovider.h"
#include "qgstriangularmesh.h"
#include "qgsmeshlayer.h"
#include "qgspointxy.h"

class QgsRenderContext;
class QgsInterpolatedLineColor;
class QgsMeshLayerRendererFeedback;
///@cond PRIVATE


class QgsMeshVectorRenderer
{
  public:

    QgsMeshVectorRenderer() = default;

    /**
     * Draws vector arrows in the context's painter based on settings
     */
    virtual ~QgsMeshVectorRenderer();

    virtual void draw() = 0;

    //! Vector renderer factory. The returned renderer type depend on the settings
    static QgsMeshVectorRenderer *makeVectorRenderer( const QgsTriangularMesh &m,
        const QgsMeshDataBlock &datasetVectorValues,
        const QgsMeshDataBlock &scalarActiveFaceFlagValues,
        const QVector<double> &datasetValuesMag,
        double datasetMagMaximumValue,
        double datasetMagMinimumValue,
        QgsMeshDatasetGroupMetadata::DataType dataType,
        const QgsMeshRendererVectorSettings &settings,
        QgsRenderContext &context,
        const QgsRectangle &layerExtent,
        QgsMeshLayerRendererFeedback *feedBack,
        const QSize &size );
};

/**
 * \ingroup core
 *
 * \brief Helper private class for rendering vector datasets (e.g. velocity)
 *
 * \note not available in Python bindings
 * \since QGIS 3.2
 */
class QgsMeshVectorArrowRenderer : public QgsMeshVectorRenderer
{
  public:
    //! Ctor
    QgsMeshVectorArrowRenderer( const QgsTriangularMesh &m,
                                const QgsMeshDataBlock &datasetValues,
                                const QVector<double> &datasetValuesMag,
                                double datasetMagMaximumValue,
                                double datasetMagMinimumValue,
                                QgsMeshDatasetGroupMetadata::DataType dataType,
                                const QgsMeshRendererVectorSettings &settings,
                                QgsRenderContext &context,
                                QSize size );
    ~QgsMeshVectorArrowRenderer() override;

    /**
     * Draws vector arrows in the context's painter based on settings
     */
    void draw() override;

  private:
    //! Draws for data defined on vertices
    void drawVectorDataOnVertices( );
    //! Draws for data defined on face centers
    void drawVectorDataOnFaces( );
    //! Draws for data defined on edge centers
    void drawVectorDataOnEdges( );
    //! Draws for data defined on edge centers or face centers
    void drawVectorDataOnPoints( const QSet<int> indexesToRender, const QVector<QgsMeshVertex> &points );
    //! Draws data on user-defined grid
    void drawVectorDataOnGrid( );
    //! Draws arrow from start point and vector data
    virtual void drawVector( const QgsPointXY &lineStart, double xVal, double yVal, double magnitude );
    //! Calculates the end point of the arrow based on start point and vector data
    bool calcVectorLineEnd( QgsPointXY &lineEnd,
                            double &vectorLength,
                            double &cosAlpha,
                            double &sinAlpha, //out
                            const QgsPointXY &lineStart,
                            double xVal,
                            double yVal,
                            double magnitude //in
                          );

    /**
     * Calculates the buffer size
     * needed to draw arrows which have
     * start or end point outside the
     * visible canvas extent (in pixels)
     */
    double calcExtentBufferSize() const;

    const QgsTriangularMesh &mTriangularMesh;
    const QgsMeshDataBlock &mDatasetValues;
    const QVector<double> &mDatasetValuesMag; //magnitudes
    double mMinMag = 0.0;
    double mMaxMag = 0.0;
    QgsMeshDatasetGroupMetadata::DataType mDataType = QgsMeshDatasetGroupMetadata::DataType::DataOnVertices;
    QgsRectangle mBufferedExtent;
    QPen mPen;

  protected:
    QgsRenderContext &mContext;
    const QgsMeshRendererVectorSettings mCfg;
    QSize mOutputSize;
    QgsInterpolatedLineColor mVectorColoring;

};

/**
 * \ingroup core
 *
 * \brief Helper private class for rendering vector datasets using Wind Barbs
 *
 * \note not available in Python bindings
 * \since QGIS 3.38
 */
class QgsMeshVectorWindBarbRenderer : public QgsMeshVectorArrowRenderer
{
  public:
    //! Ctor
    QgsMeshVectorWindBarbRenderer( const QgsTriangularMesh &m,
                                   const QgsMeshDataBlock &datasetValues,
                                   const QVector<double> &datasetValuesMag,
                                   double datasetMagMaximumValue,
                                   double datasetMagMinimumValue,
                                   QgsMeshDatasetGroupMetadata::DataType dataType,
                                   const QgsMeshRendererVectorSettings &settings,
                                   QgsRenderContext &context,
                                   QSize size );
    ~QgsMeshVectorWindBarbRenderer() override;

  private:
    void drawVector( const QgsPointXY &lineStart, double xVal, double yVal, double magnitude ) override;

    QgsCoordinateTransform mGeographicTransform;

};

///@endcond



#endif // QGSMESHVECTORRENDERER_H
