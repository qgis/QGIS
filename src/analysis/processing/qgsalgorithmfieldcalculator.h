/***************************************************************************
                         qgsalgorithmfieldcalculator.h
                         ----------------------
    begin                : September 2020
    copyright            : (C) 2020 by Ivan Ivanov
    email                : ivan@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSALGORITHMFIELDSCALCULATOR_H
#define QGSALGORITHMFIELDSCALCULATOR_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include "qgsdistancearea.h"

///@cond PRIVATE

/**
 * Native field calculator algorithm.
 */
class QgsFieldCalculatorAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsFieldCalculatorAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QList<int> inputLayerTypes() const override;
    QgsFieldCalculatorAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;
    QString outputName() const override;
    QgsFields outputFields( const QgsFields &inputFields ) const override;
    QgsProcessingFeatureSource::Flag sourceFlags() const override;

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature,  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    bool supportInPlaceEdit( const QgsMapLayer *layer ) const override;

  private:
    QgsFields mFields;
    int mFieldIdx;
    QgsExpression mExpression;
    QgsExpressionContext mExpressionContext;
    QgsDistanceArea mDa;
    int mRowNumber;
};

///@endcond PRIVATE

#endif // QGSALGORITHMFIELDSCALCULATOR_H
