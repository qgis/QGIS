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
#include "qgssinglebandpseudocolorrenderer.h"
#include "qgsrastershader.h"

class QgsRenderContext;

#ifdef SIP_RUN
% ModuleHeaderCode
#include "qgsmeshlayerinterpolator.h"
  % End
#endif

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

namespace QgsMeshUtils
{

  /**
   * Exports mesh layer's dataset values as raster block
   *
   * The function always fetches native mesh and dataset data
   * from data provider and calculates triangular mesh
   *
   * \param layer mesh layer
   * \param datasetIndex index from layer defining group and dataset (time) to export
   * \param destinationCrs destination/map CRS. Used to create triangular mesh from native mesh
   * \param transformContext Transform context to transform layer CRS to destination CRS
   * \param mapUnitsPerPixel map units per pixel for block
   * \param extent extent of block in destination CRS
   * \param feedback optional raster feedback object for cancellation/preview
   * \returns raster block with Float::64 values. NULLPTR on error
   *
   * \since QGIS 3.6
   */
  CORE_EXPORT QgsRasterBlock *exportRasterBlock(
    const QgsMeshLayer &layer,
    const QgsMeshDatasetIndex &datasetIndex,
    const QgsCoordinateReferenceSystem &destinationCrs,
    const QgsCoordinateTransformContext &transformContext,
    double mapUnitsPerPixel,
    const QgsRectangle &extent,
    QgsRasterBlockFeedback *feedback = nullptr ) SIP_FACTORY;


  /**
   * Exports mesh layer's dataset values as raster block
   *
   * \param triangularMesh the triangular mesh of the mesh layer
   * \param datasetValues dataset values used to build the raster block
   * \param activeFlags active flag values
   * \param dataType the data type iof the dataset values
   * \param transform the coordinate transform used to export the raster block
   * \param mapUnitsPerPixel map units per pixel for block
   * \param extent extent of block in destination CRS
   * \param feedback optional raster feedback object for cancellation/preview
   * \returns raster block with Float::64 values. NULLPTR on error
   *
   * \since QGIS 3.18
   */
  CORE_EXPORT QgsRasterBlock *exportRasterBlock(
    const QgsTriangularMesh &triangularMesh,
    const QgsMeshDataBlock &datasetValues,
    const QgsMeshDataBlock &activeFlags,
    const QgsMeshDatasetGroupMetadata::DataType dataType,
    const QgsCoordinateTransform &transform,
    double mapUnitsPerPixel,
    const QgsRectangle &extent,
    QgsRasterBlockFeedback *feedback = nullptr ) SIP_SKIP;
}; // namespace QgsMeshUtils

#endif // QGSMESHLAYERINTERPOLATOR_H
