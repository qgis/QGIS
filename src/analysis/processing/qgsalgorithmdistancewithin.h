/***************************************************************************
                         qgsalgorithmdistancewithin.h
                         ---------------------
    begin                : August 2021
    copyright            : (C) 2021 by Nyall Dawson
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

#ifndef QGSALGORITHMDISTANCEWITHIN_H
#define QGSALGORITHMDISTANCEWITHIN_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsapplication.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE


/**
 * Base class for distance within based extraction/selection algorithms.
 */
class QgsDistanceWithinAlgorithm : public QgsProcessingAlgorithm
{
  protected:
    void addDistanceParameter();
    void process( const QgsProcessingContext &context, QgsFeatureSource *targetSource, QgsFeatureSource *referenceSource, double distance, const QgsProperty &distanceProperty, const std::function<void( const QgsFeature & )> &handleFeatureFunction, bool onlyRequireTargetIds, QgsProcessingFeedback *feedback, QgsExpressionContext &expressionContext );

  private:
    void processByIteratingOverTargetSource( const QgsProcessingContext &context, QgsFeatureSource *targetSource, QgsFeatureSource *referenceSource, double distance, const QgsProperty &distanceProperty, const std::function<void( const QgsFeature & )> &handleFeatureFunction, bool onlyRequireTargetIds, QgsProcessingFeedback *feedback, QgsExpressionContext &expressionContext );
    void processByIteratingOverReferenceSource( const QgsProcessingContext &context, QgsFeatureSource *targetSource, QgsFeatureSource *referenceSource, double distance, const std::function<void( const QgsFeature & )> &handleFeatureFunction, bool onlyRequireTargetIds, QgsProcessingFeedback *feedback );
};


/**
 * Native select within distance algorithm
 */
class QgsSelectWithinDistanceAlgorithm : public QgsDistanceWithinAlgorithm
{
  public:
    QgsSelectWithinDistanceAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    [[nodiscard]] QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmSelectDistance.svg" ) ); }
    [[nodiscard]] QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmSelectDistance.svg" ) ); }
    [[nodiscard]] QString name() const override;
    [[nodiscard]] Qgis::ProcessingAlgorithmFlags flags() const override;
    [[nodiscard]] QString displayName() const override;
    [[nodiscard]] QStringList tags() const override;
    [[nodiscard]] QString group() const override;
    [[nodiscard]] QString groupId() const override;
    [[nodiscard]] QString shortHelpString() const override;
    [[nodiscard]] QString shortDescription() const override;
    [[nodiscard]] QgsSelectWithinDistanceAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
};

/**
 * Native extract by distance within algorithm
 */
class QgsExtractWithinDistanceAlgorithm : public QgsDistanceWithinAlgorithm
{
  public:
    QgsExtractWithinDistanceAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    [[nodiscard]] QString name() const override;
    [[nodiscard]] QString displayName() const override;
    [[nodiscard]] QStringList tags() const override;
    [[nodiscard]] QString group() const override;
    [[nodiscard]] QString groupId() const override;
    [[nodiscard]] QString shortHelpString() const override;
    [[nodiscard]] QString shortDescription() const override;
    [[nodiscard]] QgsExtractWithinDistanceAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
};

///@endcond PRIVATE

#endif // QGSALGORITHMDISTANCEWITHIN_H
