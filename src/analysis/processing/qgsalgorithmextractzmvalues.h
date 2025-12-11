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

#include <functional>

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include "qgsstatisticalsummary.h"

///@cond PRIVATE

/**
 * Native add incremental field algorithm.
 */
class QgsExtractZMValuesAlgorithmBase : public QgsProcessingFeatureBasedAlgorithm
{
  public:
    QgsExtractZMValuesAlgorithmBase() = default;
    [[nodiscard]] QString group() const override;
    [[nodiscard]] QString groupId() const override;
    [[nodiscard]] QList<int> inputLayerTypes() const override;
    bool supportInPlaceEdit( const QgsMapLayer *layer ) const override;

  protected:
    void initParameters( const QVariantMap &configuration = QVariantMap() ) override;
    [[nodiscard]] QString outputName() const override;
    [[nodiscard]] QgsFields outputFields( const QgsFields &inputFields ) const override;
    [[nodiscard]] Qgis::ProcessingFeatureSourceFlags sourceFlags() const override;

    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QgsFeatureList processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  protected:
    std::function<double( const QgsPoint & )> mExtractValFunc;
    std::function<bool( const QgsGeometry & )> mTestGeomFunc;
    QString mDefaultFieldPrefix;

  private:
    QList<Qgis::Statistic> mSelectedStats;
    Qgis::Statistics mStats = Qgis::Statistic::All;
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

    [[nodiscard]] QString name() const override;
    [[nodiscard]] QString displayName() const override;
    [[nodiscard]] QgsProcessingAlgorithm *createInstance() const override;
    [[nodiscard]] QStringList tags() const override;
    [[nodiscard]] QString shortHelpString() const override;
    [[nodiscard]] QString shortDescription() const override;
};


/**
 * Native extract m values algorithm.
 */
class QgsExtractMValuesAlgorithm : public QgsExtractZMValuesAlgorithmBase
{
  public:
    QgsExtractMValuesAlgorithm();

    [[nodiscard]] QString name() const override;
    [[nodiscard]] QString displayName() const override;
    [[nodiscard]] QgsProcessingAlgorithm *createInstance() const override;
    [[nodiscard]] QStringList tags() const override;
    [[nodiscard]] QString shortHelpString() const override;
    [[nodiscard]] QString shortDescription() const override;
};

///@endcond PRIVATE

#endif // QGSALGORITHMEXTRACTZMVALUES_H
