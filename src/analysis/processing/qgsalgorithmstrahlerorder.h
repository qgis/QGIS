/***************************************************************************
                         qgsalgorithmstrahlerorder.h
                         ---------------------
    begin                : September 2026
    copyright            : (C) 2026 by Nyall Dawson
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

#ifndef QGSALGORITHMSTRAHLERORDER_H
#define QGSALGORITHMSTRAHLERORDER_H


#include "qgis_sip.h"
#include "qgsalgorithmd8base.h"
#include "qgsprocessingalgorithm.h"

#define SIP_NO_FILE

///@cond PRIVATE

/**
 * \brief Common base class for Strahler stream order processing algorithms.
 */
class QgsStrahlerOrderAlgorithmBase : public QgsD8AnalysisAlgorithmBase
{
  public:
    QgsStrahlerOrderAlgorithmBase() = default;

    QStringList tags() const override;

  protected:
    void addCommonParameters();

    /**
     * Writes the processed int16 raster block to the destination file.
     */
    QVariantMap writeOutputRaster( QgsRasterBlock *outputBlock, const QString &outputFile, const QString &outputFormat, const QString &creationOptions );

    int mLayerWidth = 0;
    int mLayerHeight = 0;

    QgsRectangle mExtent;
    QgsCoordinateReferenceSystem mCrs;

    std::unique_ptr<QgsRasterInterface> mDemInterface;

    int mOutputNoData = -9999;
};

/**
 * \brief Native processing algorithm for computing Strahler stream order directly from a DEM raster.
 */
class QgsStrahlerOrderFromDemAlgorithm : public QgsStrahlerOrderAlgorithmBase
{
  public:
    QgsStrahlerOrderFromDemAlgorithm() = default;

    QString name() const override;
    QString displayName() const override;
    QString shortDescription() const override;
    QString shortHelpString() const override;
    QgsProcessingAlgorithm *createInstance() const override;

    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    double mCellSizeX = 0.0;
    double mCellSizeY = 0.0;
};

/**
 * \brief Native processing algorithm for computing Strahler stream order from a DEM raster and a D8 flow direction raster.
 */
class QgsStrahlerOrderFromFlowDirectionAlgorithm : public QgsStrahlerOrderAlgorithmBase
{
  public:
    QgsStrahlerOrderFromFlowDirectionAlgorithm() = default;

    QString name() const override;
    QString displayName() const override;
    QString shortDescription() const override;
    QString shortHelpString() const override;
    QgsProcessingAlgorithm *createInstance() const override;

    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    std::unique_ptr<QgsRasterInterface> mFlowDirInterface;
};

///@endcond PRIVATE

#endif // QGSALGORITHMSTRAHLERORDER_H
