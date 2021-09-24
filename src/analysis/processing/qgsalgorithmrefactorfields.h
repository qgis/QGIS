/***************************************************************************
                         qgsalgorithmrefactorfields.h
                         ---------------------------------
    begin                : June 2020
    copyright            : (C) 2020 by Nyall Dawson
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

#ifndef QGSALGORITHMREFACTORFIELDS_H
#define QGSALGORITHMREFACTORFIELDS_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include "qgsdistancearea.h"

///@cond PRIVATE

/**
 * Native refactor fields algorithm.
 */
class QgsRefactorFieldsAlgorithm : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsRefactorFieldsAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QList<int> inputLayerTypes() const override;
    QgsRefactorFieldsAlgorithm *createInstance() const override SIP_FACTORY;

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
    QList< QgsExpression > mExpressions;
    bool mExpressionsPrepared = false;
    QgsExpressionContext mExpressionContext;
    QgsDistanceArea mDa;
    long long mRowNumber = 0;

};

///@endcond PRIVATE

#endif // QGSALGORITHMREFACTORFIELDS_H
