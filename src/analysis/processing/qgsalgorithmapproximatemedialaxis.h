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


#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

#define SIP_NO_FILE

///@cond PRIVATE


/**
 * Approximate medial axis algorithm with SFCGAL backend.
 */
class QgsApproximateMedialAxisAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{
  public:
    QgsApproximateMedialAxisAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QgsApproximateMedialAxisAlgorithm *createInstance() const override SIP_FACTORY;
    QList<int> inputLayerTypes() const override;

  protected:
    QString outputName() const override;
    Qgis::ProcessingSourceType outputLayerType() const override;
    Qgis::WkbType outputWkbType( Qgis::WkbType inputWkbType ) const override;
    QgsFields outputFields( const QgsFields &inputFields ) const override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
};

///@endcond PRIVATE
#endif // QGSALGORITHMAPPROXIMATEMEDIALAXIS_H
