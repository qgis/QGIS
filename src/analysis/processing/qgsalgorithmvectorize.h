/***************************************************************************
                         qgsalgorithmvectorize.h
                         ---------------------
    begin                : June, 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMVECTORIZE_H
#define QGSALGORITHMVECTORIZE_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include "qgsreclassifyutils.h"

///@cond PRIVATE

/**
 * Base class for vectorize algorithms.
 */
class QgsVectorizeAlgorithmBase : public QgsProcessingAlgorithm
{
  public:
    [[nodiscard]] QString group() const final;
    [[nodiscard]] QString groupId() const final;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) final;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) final;

    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) final;

    std::unique_ptr<QgsRasterInterface> mInterface;

    Qgis::DataType mDataType = Qgis::DataType::Float32;
    double mNoDataValue = -9999;
    int mBand = 1;
    QgsRectangle mExtent;
    QgsCoordinateReferenceSystem mCrs;
    double mRasterUnitsPerPixelX = 0;
    double mRasterUnitsPerPixelY = 0;
    int mNbCellsXProvider = 0;
    int mNbCellsYProvider = 0;
    QgsReclassifyUtils::RasterClass::BoundsType mBoundsType = QgsReclassifyUtils::RasterClass::IncludeMax;
    bool mUseNoDataForMissingValues = false;

  private:
    [[nodiscard]] virtual QString outputName() const = 0;
    [[nodiscard]] virtual Qgis::ProcessingSourceType outputType() const = 0;
    [[nodiscard]] virtual Qgis::WkbType sinkType() const = 0;
    [[nodiscard]] virtual QgsGeometry createGeometryForPixel( double centerX, double centerY, double pixelWidthX, double pixelWidthY ) const = 0;
};

/**
 * Native raster pixels to polygons algorithm.
 */
class QgsRasterPixelsToPolygonsAlgorithm : public QgsVectorizeAlgorithmBase
{
  public:
    QgsRasterPixelsToPolygonsAlgorithm() = default;
    [[nodiscard]] QString name() const override;
    [[nodiscard]] QString displayName() const override;
    [[nodiscard]] QStringList tags() const override;
    [[nodiscard]] QString shortHelpString() const override;
    [[nodiscard]] QString shortDescription() const override;
    [[nodiscard]] QgsRasterPixelsToPolygonsAlgorithm *createInstance() const override SIP_FACTORY;

  private:
    [[nodiscard]] QString outputName() const override;
    [[nodiscard]] Qgis::ProcessingSourceType outputType() const override;
    [[nodiscard]] Qgis::WkbType sinkType() const override;
    [[nodiscard]] QgsGeometry createGeometryForPixel( double centerX, double centerY, double pixelWidthX, double pixelWidthY ) const override;
};

/**
 * Native raster pixels to points algorithm.
 */
class QgsRasterPixelsToPointsAlgorithm : public QgsVectorizeAlgorithmBase
{
  public:
    QgsRasterPixelsToPointsAlgorithm() = default;
    [[nodiscard]] QString name() const override;
    [[nodiscard]] QString displayName() const override;
    [[nodiscard]] QStringList tags() const override;
    [[nodiscard]] QString shortHelpString() const override;
    [[nodiscard]] QString shortDescription() const override;
    [[nodiscard]] QgsRasterPixelsToPointsAlgorithm *createInstance() const override SIP_FACTORY;

  private:
    [[nodiscard]] QString outputName() const override;
    [[nodiscard]] Qgis::ProcessingSourceType outputType() const override;
    [[nodiscard]] Qgis::WkbType sinkType() const override;
    [[nodiscard]] QgsGeometry createGeometryForPixel( double centerX, double centerY, double pixelWidthX, double pixelWidthY ) const override;
};

///@endcond PRIVATE

#endif // QGSALGORITHMVECTORIZE_H
