/***************************************************************************
                         qgsalgorithmaggregate.h
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

#ifndef QGSALGORITHMAGGREGATE_H
#define QGSALGORITHMAGGREGATE_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include "qgsdistancearea.h"

///@cond PRIVATE

/**
 * Native aggregate algorithm.
 */
class QgsAggregateAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsAggregateAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsAggregateAlgorithm *createInstance() const override SIP_FACTORY;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;

  protected:

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

    bool supportInPlaceEdit( const QgsMapLayer *layer ) const override;

  private:

    QgsExpression createExpression( const QString &expressionString, QgsProcessingContext &context ) const;

    std::unique_ptr< QgsProcessingFeatureSource > mSource;
    QString mGroupBy;

    QgsExpression mGroupByExpression;
    QgsExpression mGeometryExpression;

    QgsFields mFields;
    QList< QgsExpression > mExpressions;
    QList< int > mAttributesRequireLastFeature;
    QgsDistanceArea mDa;

    struct Group
    {
      QgsFeatureSink *sink = nullptr;
      QgsMapLayer *layer = nullptr;
      QgsFeature firstFeature;
      QgsFeature lastFeature;
    };

};

///@endcond PRIVATE

#endif // QGSALGORITHMAGGREGATE_H
