/***************************************************************************
                         qgsalgorithmextractbylocation.h
                         ---------------------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#ifndef QGSALGORITHMEXTRACTBYLOCATION_H
#define QGSALGORITHMEXTRACTBYLOCATION_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsapplication.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE


/**
 * Base class for location based extraction/selection algorithms.
 */
class QgsLocationBasedAlgorithm : public QgsProcessingAlgorithm
{
  protected:
    enum Predicate
    {
      Intersects,
      Contains,
      Disjoint,
      IsEqual,
      Touches,
      Overlaps,
      Within,
      Crosses,
    };

    void addPredicateParameter();
    [[nodiscard]] Predicate reversePredicate( Predicate predicate ) const;
    [[nodiscard]] QStringList predicateOptionsList() const;
    void process( const QgsProcessingContext &context, QgsFeatureSource *targetSource, QgsFeatureSource *intersectSource, const QList<int> &selectedPredicates, const std::function<void( const QgsFeature & )> &handleFeatureFunction, bool onlyRequireTargetIds, QgsProcessingFeedback *feedback, const QgsFeatureIds &skipTargetFeatureIds = QgsFeatureIds() );

  protected:
    QgsCoordinateReferenceSystem mTargetCrs;
    long long mTargetFeatureCount = 0;
    long long mIntersectFeatureCount = 0;

  private:
    void processByIteratingOverTargetSource( const QgsProcessingContext &context, QgsFeatureSource *targetSource, QgsFeatureSource *intersectSource, const QList<int> &selectedPredicates, const std::function<void( const QgsFeature & )> &handleFeatureFunction, bool onlyRequireTargetIds, QgsProcessingFeedback *feedback, const QgsFeatureIds &skipTargetFeatureIds );
    void processByIteratingOverIntersectSource( const QgsProcessingContext &context, QgsFeatureSource *targetSource, QgsFeatureSource *intersectSource, const QList<int> &selectedPredicates, const std::function<void( const QgsFeature & )> &handleFeatureFunction, bool onlyRequireTargetIds, QgsProcessingFeedback *feedback, const QgsFeatureIds &skipTargetFeatureIds );
};


/**
 * Native select by location algorithm
 */
class QgsSelectByLocationAlgorithm : public QgsLocationBasedAlgorithm
{
  public:
    QgsSelectByLocationAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    [[nodiscard]] QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmSelectLocation.svg" ) ); }
    [[nodiscard]] QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmSelectLocation.svg" ) ); }
    [[nodiscard]] QString name() const override;
    [[nodiscard]] Qgis::ProcessingAlgorithmFlags flags() const override;
    [[nodiscard]] QString displayName() const override;
    [[nodiscard]] QStringList tags() const override;
    [[nodiscard]] QString group() const override;
    [[nodiscard]] QString groupId() const override;
    [[nodiscard]] QString shortHelpString() const override;
    [[nodiscard]] QString shortDescription() const override;
    [[nodiscard]] QgsSelectByLocationAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
};

/**
 * Native extract by location algorithm
 */
class QgsExtractByLocationAlgorithm : public QgsLocationBasedAlgorithm
{
  public:
    QgsExtractByLocationAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    [[nodiscard]] QString name() const override;
    [[nodiscard]] QString displayName() const override;
    [[nodiscard]] QStringList tags() const override;
    [[nodiscard]] QString group() const override;
    [[nodiscard]] QString groupId() const override;
    [[nodiscard]] QString shortHelpString() const override;
    [[nodiscard]] QString shortDescription() const override;
    [[nodiscard]] QgsExtractByLocationAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
};

///@endcond PRIVATE

#endif // QGSALGORITHMEXTRACTBYLOCATION_H
