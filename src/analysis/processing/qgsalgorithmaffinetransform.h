/***************************************************************************
                         qgsalgorithmaffinetransform.h
                         ---------------------
    begin                : December 2019
    copyright            : (C) 2019 by Nyall Dawson
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

#ifndef QGSALGORITHMAFFINETRANSFORM_H
#define QGSALGORITHMAFFINETRANSFORM_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native affine transformation algorithm.
 */
class QgsAffineTransformationAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{
  public:
    QgsAffineTransformationAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QgsAffineTransformationAlgorithm *createInstance() const override SIP_FACTORY;
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;
    bool supportInPlaceEdit( const QgsMapLayer *layer ) const override;

  protected:
    QString outputName() const override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    Qgis::WkbType outputWkbType( Qgis::WkbType inputWkbType ) const override;

  private:
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

    double mScaleX = 0.0;
    bool mDynamicScaleX = false;
    QgsProperty mScaleXProperty;

    double mScaleY = 0.0;
    bool mDynamicScaleY = false;
    QgsProperty mScaleYProperty;

    double mScaleZ = 0.0;
    bool mDynamicScaleZ = false;
    QgsProperty mScaleZProperty;

    double mScaleM = 0.0;
    bool mDynamicScaleM = false;
    QgsProperty mScaleMProperty;

    double mRotationZ = 0.0;
    bool mDynamicRotationZ = false;
    QgsProperty mRotationZProperty;
};


///@endcond PRIVATE

#endif // QGSALGORITHMAFFINETRANSFORM_H
