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
#include "qgsmeshdataprovider.h"
#include "qgsmeshtracerenderer.h"
#include "qgsmapclippingregion.h"

class QgsRenderContext;
class QgsMeshLayerLabelProvider;

///@cond PRIVATE

/**
 * Feedback for mesh layer rendering - right now derived from raster block feedback so that we
 * can pass it to block reading in the raster interface.
 */
class QgsMeshLayerRendererFeedback : public QgsRasterBlockFeedback
{
    Q_OBJECT
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
  QgsMeshDatasetGroupMetadata::DataType mScalarDataType = QgsMeshDatasetGroupMetadata::DataType::DataOnVertices;
  double mScalarDatasetMinimum = std::numeric_limits<double>::quiet_NaN();
  double mScalarDatasetMaximum = std::numeric_limits<double>::quiet_NaN();
  QgsMeshRendererScalarSettings::DataResamplingMethod mDataInterpolationMethod = QgsMeshRendererScalarSettings::NoResampling;
  std::unique_ptr<QgsMesh3DAveragingMethod> mScalarAveragingMethod;

  // vector dataset
  QgsMeshDatasetIndex mActiveVectorDatasetIndex;
  QgsMeshDataBlock mVectorDatasetValues;
  QgsMeshDataBlock mVectorActiveFaceFlagValues;
  QVector<double> mVectorDatasetValuesMag;
  double mVectorDatasetMagMinimum = std::numeric_limits<double>::quiet_NaN();
  double mVectorDatasetMagMaximum = std::numeric_limits<double>::quiet_NaN();
  double mVectorDatasetGroupMagMinimum = std::numeric_limits<double>::quiet_NaN();
  double mVectorDatasetGroupMagMaximum = std::numeric_limits<double>::quiet_NaN();
  QgsMeshDatasetGroupMetadata::DataType mVectorDataType = QgsMeshDatasetGroupMetadata::DataType::DataOnVertices;
  std::unique_ptr<QgsMesh3DAveragingMethod> mVectorAveragingMethod;
};


///@endcond

/**
 * \ingroup core
 * \brief Implementation of threaded rendering for mesh layers.
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
    bool forceRasterRender() const override;

  private:
    void prepareLabeling( QgsMeshLayer *layer, QSet<QString> &attributeNames );
    void renderMesh();
    void renderEdgeMesh( const QgsMeshRendererMeshSettings &settings, const QList<int> &edgesInExtent );
    void renderFaceMesh( const QgsMeshRendererMeshSettings &settings, const QVector<QgsMeshFace> &faces, const QList<int> &facesInExtent );
    void renderScalarDataset();
    void renderScalarDatasetOnEdges( const QgsMeshRendererScalarSettings &scalarSettings );
    void renderScalarDatasetOnFaces( const QgsMeshRendererScalarSettings &scalarSettings );
    void registerLabelFeatures();

    void renderVectorDataset();
    void copyTriangularMeshes( QgsMeshLayer *layer, QgsRenderContext &context );
    void copyScalarDatasetValues( QgsMeshLayer *layer );
    void copyVectorDatasetValues( QgsMeshLayer *layer );
    void calculateOutputSize();
    QgsPointXY fractionPoint( const QgsPointXY &p1, const QgsPointXY &p2, double fraction ) const;
    bool mIsMeshSimplificationActive = false;
    QColor colorAt( QgsColorRampShader *shader, double val ) const;
    bool mIsEditable = false;

    /**
     * used with new labeling engine (QgsLabelingEngine): provider for labels.
     * may be NULLPTR. no need to delete: if exists it is owned by labeling engine
     */
    QgsMeshLayerLabelProvider *mLabelProvider = nullptr;

  protected:
    QString mLayerName;

    //! feedback class for cancellation
    std::unique_ptr<QgsMeshLayerRendererFeedback> mFeedback;

    // copy from mesh layer
    QgsMesh mNativeMesh;

    // copy from mesh layer
    QgsTriangularMesh mTriangularMesh;

    // copy from mesh layer
    QgsRectangle mLayerExtent;

    // copy of the scalar dataset
    QVector<double> mScalarDatasetValues;
    QgsMeshDataBlock mScalarActiveFaceFlagValues;
    QgsMeshDatasetGroupMetadata::DataType mScalarDataType = QgsMeshDatasetGroupMetadata::DataOnVertices;
    double mScalarDatasetMinimum = std::numeric_limits<double>::quiet_NaN();
    double mScalarDatasetMaximum = std::numeric_limits<double>::quiet_NaN();

    // copy of the vector dataset
    QgsMeshDataBlock mVectorDatasetValues;
    QgsMeshDataBlock mVectorActiveFaceFlagValues;
    QVector<double> mVectorDatasetValuesMag;
    double mVectorDatasetMagMinimum = std::numeric_limits<double>::quiet_NaN();
    double mVectorDatasetMagMaximum = std::numeric_limits<double>::quiet_NaN();
    double mVectorDatasetGroupMagMinimum = std::numeric_limits<double>::quiet_NaN();
    double mVectorDatasetGroupMagMaximum = std::numeric_limits<double>::quiet_NaN();
    QgsMeshDatasetGroupMetadata::DataType mVectorDataType = QgsMeshDatasetGroupMetadata::DataOnVertices;

    // copy of rendering settings
    QgsMeshRendererSettings mRendererSettings;

    QList< QgsMapClippingRegion > mClippingRegions;

    // output screen size
    QSize mOutputSize;

    double mElevationScale = 1.0;
    double mElevationOffset = 0.0;
    bool mRenderElevationMap = false;

    bool mEnableProfile = false;
    quint64 mPreparationTime = 0;

  private:

    double mLayerOpacity = 1.0;
};


#endif // QGSMESHLAYERRENDERER_H
