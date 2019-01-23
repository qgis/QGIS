/***************************************************************************
                         qgsalgorithmarraytranslatedfeatures.h
                         ---------------------
    begin                : July 2018
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

#ifndef QGSALGORITHMARRAYTRANSLATEDFEATURES_H
#define QGSALGORITHMARRAYTRANSLATEDFEATURES_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native create array of features algorithm.
 */
class QgsArrayTranslatedFeaturesAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsArrayTranslatedFeaturesAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QgsArrayTranslatedFeaturesAlgorithm *createInstance() const override SIP_FACTORY;
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;

  protected:
    QString outputName() const override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature,  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsWkbTypes::Type outputWkbType( QgsWkbTypes::Type inputWkbType ) const override;
    QgsFields outputFields( const QgsFields &inputFields ) const override;

  private:

    int mCount = 0;
    bool mDynamicCount = false;
    QgsProperty mCountProperty;

    double mDeltaX = 0.0;
    bool mDynamicDeltaX = false;
    QgsProperty mDeltaXProperty;

    double mDeltaY = 0.0;
    bool mDynamicDeltaY = false;
    QgsProperty mDeltaYProperty;

    double mDeltaZ = 0.0;
    bool mDynamicDeltaZ = false;
    QgsProperty mDeltaZProperty;

    double mDeltaM = 0.0;
    bool mDynamicDeltaM = false;
    QgsProperty mDeltaMProperty;

};


///@endcond PRIVATE

#endif // QGSALGORITHMARRAYTRANSLATEDFEATURES_H


