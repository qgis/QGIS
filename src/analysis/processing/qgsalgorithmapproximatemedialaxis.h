/***************************************************************************
                         qgsalgorithmapproximatemedialaxis.h
                         ---------------------
    begin                : September 2025
    copyright            : (C) 2025 by Jean Felder
    email                : jean dot felder at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMAPPROXIMATEMEDIALAXIS_H
#define QGSALGORITHMAPPROXIMATEMEDIALAXIS_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE


/**
 * Approximate medial axis algorithm with SFCGAL backend.
 */
class QgsApproximateMedialAxisAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{
  public:
    QgsApproximateMedialAxisAlgorithm() = default;
    [[nodiscard]] QString name() const override;
    [[nodiscard]] QString displayName() const override;
    [[nodiscard]] QStringList tags() const override;
    [[nodiscard]] QString group() const override;
    [[nodiscard]] QString groupId() const override;
    [[nodiscard]] QString shortHelpString() const override;
    [[nodiscard]] QString shortDescription() const override;
    [[nodiscard]] QgsApproximateMedialAxisAlgorithm *createInstance() const override SIP_FACTORY;
    [[nodiscard]] QList<int> inputLayerTypes() const override;

  protected:
    [[nodiscard]] QString outputName() const override;
    [[nodiscard]] Qgis::ProcessingSourceType outputLayerType() const override;
    [[nodiscard]] Qgis::WkbType outputWkbType( Qgis::WkbType inputWkbType ) const override;
    [[nodiscard]] QgsFields outputFields( const QgsFields &inputFields ) const override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
};

///@endcond PRIVATE
#endif // QGSALGORITHMAPPROXIMATEMEDIALAXIS_H
