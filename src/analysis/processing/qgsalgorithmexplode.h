/***************************************************************************
                         qgsalgorithmexplode.h
                         ---------------------
    begin                : April 2018
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

#ifndef QGSALGORITHMEXPLODE_H
#define QGSALGORITHMEXPLODE_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native explode lines algorithm.
 */
class QgsExplodeAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{
  public:
    QgsExplodeAlgorithm() = default;
    [[nodiscard]] QString name() const override;
    [[nodiscard]] QString displayName() const override;
    [[nodiscard]] QStringList tags() const override;
    [[nodiscard]] QString group() const override;
    [[nodiscard]] QString groupId() const override;
    [[nodiscard]] QString shortHelpString() const override;
    [[nodiscard]] QString shortDescription() const override;
    [[nodiscard]] Qgis::ProcessingAlgorithmDocumentationFlags documentationFlags() const override;
    [[nodiscard]] QList<int> inputLayerTypes() const override;
    [[nodiscard]] Qgis::ProcessingSourceType outputLayerType() const override;
    [[nodiscard]] QgsExplodeAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    [[nodiscard]] QString outputName() const override;
    [[nodiscard]] Qgis::WkbType outputWkbType( Qgis::WkbType inputWkbType ) const override;
    QgsFeatureList processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    [[nodiscard]] Qgis::ProcessingFeatureSourceFlags sourceFlags() const override;
    [[nodiscard]] QgsFeatureSink::SinkFlags sinkFlags() const override;

    [[nodiscard]] std::vector<QgsGeometry> extractAsParts( const QgsGeometry &geometry ) const;
    std::vector<QgsGeometry> curveAsSingleSegments( const QgsCurve *curve, bool useCompoundCurves = false ) const;
};

///@endcond PRIVATE

#endif // QGSALGORITHMEXPLODE_H
