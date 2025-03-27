/***************************************************************************
                         qgsmeshlayerinterpolator.h
                         --------------------------
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

#ifndef QGSMESHLAYERINTERPOLATOR_H
#define QGSMESHLAYERINTERPOLATOR_H

class QgsMeshLayer;
class QgsSymbol;
class QgsCoordinateReferenceSystem;
class QgsCoordinateTransformContext;
class QgsMeshDatasetIndex;

#include "qgis.h"
#include "qgis_sip.h"

#include <QSize>
#include "qgsmaplayerrenderer.h"
#include "qgstriangularmesh.h"
#include "qgsrasterinterface.h"

class QgsRenderContext;

///@cond PRIVATE

/**
 * \ingroup core
 * \brief Interpolate mesh scalar dataset to raster block
 *
 * \note not available in Python bindings
 * \since QGIS 3.2
 */
class QgsMeshLayerInterpolator : public QgsRasterInterface SIP_SKIP
{
  public:
    //! Ctor
    QgsMeshLayerInterpolator( const QgsTriangularMesh &m,
                              const QVector<double> &datasetValues,
                              const QgsMeshDataBlock &activeFaceFlagValues,
                              QgsMeshDatasetGroupMetadata::DataType dataType,
                              const QgsRenderContext &context,
                              const QSize &size );
    ~QgsMeshLayerInterpolator() override;

    QgsRasterInterface *clone() const override;
    Qgis::DataType dataType( int ) const override;
    int bandCount() const override;
    QgsRasterBlock *block( int, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback = nullptr ) override;

    void setSpatialIndexActive( bool active );

    void setElevationMapSettings( bool renderElevationMap, double elevationScale, double elevationOffset );

  private:
    const QgsTriangularMesh &mTriangularMesh;
    const QVector<double> &mDatasetValues;
    const QgsMeshDataBlock &mActiveFaceFlagValues;
    const QgsRenderContext &mContext;
    QgsMeshDatasetGroupMetadata::DataType mDataType = QgsMeshDatasetGroupMetadata::DataType::DataOnVertices;
    QSize mOutputSize;
    bool mSpatialIndexActive = false;

    bool mRenderElevation = false;
    double mElevationScale = 1.0;
    double mElevationOffset = 0.0;
};

///@endcond


#endif // QGSMESHLAYERINTERPOLATOR_H
