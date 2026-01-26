/***************************************************************************
                         qgsalgorithmzonalminmaxpoint.h
                         ------------------------------
    begin                : November 2024
    copyright            : (C) 2024 Nyall Dawson
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

#ifndef QGSALGORITHMZONALMINMAXPOINT_H
#define QGSALGORITHMZONALMINMAXPOINT_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native zonal minimum and maximum point algorithm.
 */
class QgsZonalMinimumMaximumPointAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{
  public:
    QgsZonalMinimumMaximumPointAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortDescription() const override;
    QString shortHelpString() const override;
    QList<int> inputLayerTypes() const override;
    bool supportInPlaceEdit( const QgsMapLayer *layer ) const override;
    QgsZonalMinimumMaximumPointAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;
    QString outputName() const override;
    QgsFields outputFields( const QgsFields &inputFields ) const override;
    Qgis::ProcessingSourceType outputLayerType() const override;
    Qgis::WkbType outputWkbType( Qgis::WkbType inputWkbType ) const override;
    QgsFeatureSink::SinkFlags sinkFlags() const override;
    Qgis::ProcessingAlgorithmDocumentationFlags documentationFlags() const override;
    QgsCoordinateReferenceSystem outputCrs( const QgsCoordinateReferenceSystem &inputCrs ) const override;

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    std::unique_ptr<QgsRasterInterface> mRaster;
    int mBand = 1;
    QgsCoordinateReferenceSystem mCrs;
    bool mCreatedTransform = false;
    QgsCoordinateTransform mFeatureToRasterTransform;
    double mPixelSizeX = 0;
    double mPixelSizeY = 0;
    QgsFields mOutputFields;
};

///@endcond PRIVATE

#endif // QGSALGORITHMZONALMINMAXPOINT_H
