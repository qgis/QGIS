/***************************************************************************
                         qgsalgorithmextractzmvalues.h
                         ---------------------------------
    begin                : January 2019
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

#ifndef QGSALGORITHMEXTRACTZMVALUES_H
#define QGSALGORITHMEXTRACTZMVALUES_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include "qgsstatisticalsummary.h"
#include <functional>

///@cond PRIVATE

/**
 * Native add incremental field algorithm.
 */
class QgsExtractZMValuesAlgorithmBase : public QgsProcessingFeatureBasedAlgorithm
{

  public:

    QgsExtractZMValuesAlgorithmBase() = default;
    QString group() const override;
    QString groupId() const override;
    QList<int> inputLayerTypes() const override;

  protected:

    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;
    QString outputName() const override;
    QgsFields outputFields( const QgsFields &inputFields ) const override;
    QgsProcessingFeatureSource::Flag sourceFlags() const override;

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature,  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    bool supportInPlaceEdit( const QgsMapLayer *layer ) const override;

  protected:

    std::function<double( const QgsPoint & )> mExtractValFunc;
    std::function<bool( const QgsGeometry & )> mTestGeomFunc;
    QString mDefaultFieldPrefix;
  private:

    QList< QgsStatisticalSummary::Statistic > mSelectedStats;
    QgsStatisticalSummary::Statistics mStats = QgsStatisticalSummary::All;
    QString mPrefix;
    QgsFields mNewFields;


};

/**
 * Native extract z values algorithm.
 */
class QgsExtractZValuesAlgorithm : public QgsExtractZMValuesAlgorithmBase
{
  public:
    QgsExtractZValuesAlgorithm();

    QString name() const override;
    QString displayName() const override;
    QgsProcessingAlgorithm *createInstance() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;

};


/**
 * Native extract m values algorithm.
 */
class QgsExtractMValuesAlgorithm : public QgsExtractZMValuesAlgorithmBase
{
  public:
    QgsExtractMValuesAlgorithm();

    QString name() const override;
    QString displayName() const override;
    QgsProcessingAlgorithm *createInstance() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;

};

///@endcond PRIVATE

#endif // QGSALGORITHMEXTRACTZMVALUES_H


