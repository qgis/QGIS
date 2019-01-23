/***************************************************************************
                         qgsmeshlayerrenderer.h
                         ----------------------
    begin                : April 2018
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

#ifndef QGSMESHLAYERRENDERER_H
#define QGSMESHLAYERRENDERER_H

class QgsMeshLayer;

#define SIP_NO_FILE

#include <memory>
#include <limits>

#include "qgis.h"

#include "qgsmaplayerrenderer.h"
#include "qgsrasterinterface.h"
#include "qgstriangularmesh.h"
#include "qgsmeshlayer.h"
#include "qgssymbol.h"
#include "qgsmeshdataprovider.h"

class QgsRenderContext;

///@cond PRIVATE

/**
 * Feedback for mesh layer rendering - right now derived from raster block feedback so that we
 * can pass it to block reading in the raster interface.
 */
class QgsMeshLayerRendererFeedback : public QgsRasterBlockFeedback
{
};


/**
 * Cache for data needed to render active datasets
 */
struct CORE_NO_EXPORT QgsMeshLayerRendererCache
{
  int mDatasetGroupsCount = 0;

  // scalar dataset
  QgsMeshDatasetIndex mActiveScalarDatasetIndex;
  QVector<double> mScalarDatasetValues;
  QgsMeshDataBlock mScalarActiveFaceFlagValues;
  bool mScalarDataOnVertices = true;
  double mScalarDatasetMinimum = std::numeric_limits<double>::quiet_NaN();
  double mScalarDatasetMaximum = std::numeric_limits<double>::quiet_NaN();

  // vector dataset
  QgsMeshDatasetIndex mActiveVectorDatasetIndex;
  QgsMeshDataBlock mVectorDatasetValues;
  QVector<double> mVectorDatasetValuesMag;
  double mVectorDatasetMagMinimum = std::numeric_limits<double>::quiet_NaN();
  double mVectorDatasetMagMaximum = std::numeric_limits<double>::quiet_NaN();
  double mVectorDatasetGroupMagMinimum = std::numeric_limits<double>::quiet_NaN();
  double mVectorDatasetGroupMagMaximum = std::numeric_limits<double>::quiet_NaN();
  bool mVectorDataOnVertices = true;
};

///@endcond

/**
 * \ingroup core
 * Implementation of threaded rendering for mesh layers.
 *
 * \note not available in Python bindings
 * \since QGIS 3.2
 */
class QgsMeshLayerRenderer : public QgsMapLayerRenderer
{
  public:
    //! Ctor
    QgsMeshLayerRenderer( QgsMeshLayer *layer, QgsRenderContext &context );
    ~QgsMeshLayerRenderer() override = default;
    QgsFeedback *feedback() const override;
    bool render() override;

  private:
    void renderMesh();
    void renderMesh( const QgsMeshRendererMeshSettings &settings, const QVector<QgsMeshFace> &faces, const QList<int> facesInExtent );
    void renderScalarDataset();
    void renderVectorDataset();
    void copyScalarDatasetValues( QgsMeshLayer *layer );
    void copyVectorDatasetValues( QgsMeshLayer *layer );
    void calculateOutputSize();

  protected:
    //! feedback class for cancelation
    std::unique_ptr<QgsMeshLayerRendererFeedback> mFeedback;

    // copy from mesh layer
    QgsMesh mNativeMesh;

    // copy from mesh layer
    QgsTriangularMesh mTriangularMesh;

    // copy of the scalar dataset
    QVector<double> mScalarDatasetValues;
    QgsMeshDataBlock mScalarActiveFaceFlagValues;
    bool mScalarDataOnVertices = true;
    double mScalarDatasetMinimum = std::numeric_limits<double>::quiet_NaN();
    double mScalarDatasetMaximum = std::numeric_limits<double>::quiet_NaN();

    // copy of the vector dataset
    QgsMeshDataBlock mVectorDatasetValues;
    QVector<double> mVectorDatasetValuesMag;
    double mVectorDatasetMagMinimum = std::numeric_limits<double>::quiet_NaN();
    double mVectorDatasetMagMaximum = std::numeric_limits<double>::quiet_NaN();
    double mVectorDatasetGroupMagMinimum = std::numeric_limits<double>::quiet_NaN();
    double mVectorDatasetGroupMagMaximum = std::numeric_limits<double>::quiet_NaN();
    bool mVectorDataOnVertices = true;

    // rendering context
    QgsRenderContext &mContext;

    // copy of rendering settings
    QgsMeshRendererSettings mRendererSettings;

    // output screen size
    QSize mOutputSize;
};


#endif // QGSMESHLAYERRENDERER_H
