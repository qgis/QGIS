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

#include "qgsmeshdataset.h"
#include "qgsmeshutils.h"
#include "qgsvectorfieldengine.h"

#include <QSize>
#include <QVector>

#define SIP_NO_FILE

class QgsMeshLayerRendererFeedback;
class QgsPointXY;
class QgsRenderContext;
class QgsTriangularMesh;
class QgsMeshDataBlock;

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
    static QgsMeshVectorRenderer *makeVectorRenderer(
      const QgsTriangularMesh &m,
      const QgsMeshDataBlock &datasetVectorValues,
      const QgsMeshDataBlock &scalarActiveFaceFlagValues,
      const QVector<double> &datasetValuesMag,
      double datasetMagMaximumValue,
      double datasetMagMinimumValue,
      QgsMeshDatasetGroupMetadata::DataType dataType,
      const QgsVectorFieldSettings &settings,
      QgsRenderContext &context,
      const QgsRectangle &layerExtent,
      QgsMeshLayerRendererFeedback *feedBack,
      const QSize &size
    );
};

/**
 * \ingroup core
 *
 * \brief Helper private class for rendering vector datasets (e.g. velocity) with a glyph per
 * data point, that is with arrows or with wind barbs.
 *
 * \note not available in Python bindings
 * \since QGIS 4.4
 */
class QgsMeshVectorGlyphRenderer : public QgsMeshVectorRenderer
{
  public:
    //! Ctor
    QgsMeshVectorGlyphRenderer(
      const QgsTriangularMesh &m,
      const QgsMeshDataBlock &datasetValues,
      const QVector<double> &datasetValuesMag,
      double datasetMagMaximumValue,
      double datasetMagMinimumValue,
      QgsMeshDatasetGroupMetadata::DataType dataType,
      const QgsVectorFieldSettings &settings,
      QgsRenderContext &context,
      QSize size
    );
    ~QgsMeshVectorGlyphRenderer() override;

    /**
     * Draws vector glyphs in the context's painter based on settings
     */
    void draw() override;

  private:
    //! Draws for data defined on vertices
    void drawVectorDataOnVertices();
    //! Draws for data defined on face centers
    void drawVectorDataOnFaces();
    //! Draws for data defined on edge centers
    void drawVectorDataOnEdges();
    //! Draws for data defined on edge centers or face centers
    void drawVectorDataOnPoints( const QSet<int> indexesToRender, const QVector<QgsMeshVertex> &points );
    //! Draws data on user-defined grid
    void drawVectorDataOnGrid();

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

    QgsRenderContext &mContext;
    const QgsVectorFieldSettings mCfg;
    QSize mOutputSize;

    QgsVectorFieldEngine mEngine;
};

///@endcond


#endif // QGSMESHVECTORRENDERER_H
