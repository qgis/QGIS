/***************************************************************************
                         qgsalgorithmconverttocurves.h
                         ---------------------
    begin                : March 2018
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

#ifndef QGSALGORITHMCONVERTTOCURVES_H
#define QGSALGORITHMCONVERTTOCURVES_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgsprocessingalgorithm.h"
#include "qgsmaptopixelgeometrysimplifier.h"

///@cond PRIVATE

/**
 * Native segmentize by maximum distance algorithm.
 */
class QgsConvertToCurvesAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{
  public:
    QgsConvertToCurvesAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsConvertToCurvesAlgorithm *createInstance() const override SIP_FACTORY;
    QList<int> inputLayerTypes() const override;
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;

  protected:
    QString outputName() const override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback *feedback ) override;
    Qgis::WkbType outputWkbType( Qgis::WkbType inputWkbType ) const override;

  private:
    double mTolerance = 0.000001;
    bool mDynamicTolerance = false;
    QgsProperty mToleranceProperty;

    double mAngleTolerance = 0.000001;
    bool mDynamicAngleTolerance = false;
    QgsProperty mAngleToleranceProperty;
};

///@endcond PRIVATE

#endif // QGSALGORITHMCONVERTTOCURVES_H
