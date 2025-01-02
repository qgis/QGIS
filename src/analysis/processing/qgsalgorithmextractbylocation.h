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
#include "qgsprocessingalgorithm.h"
#include "qgsapplication.h"

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
    Predicate reversePredicate( Predicate predicate ) const;
    QStringList predicateOptionsList() const;
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
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmSelectLocation.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmSelectLocation.svg" ) ); }
    QString name() const override;
    Qgis::ProcessingAlgorithmFlags flags() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsSelectByLocationAlgorithm *createInstance() const override SIP_FACTORY;

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
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsExtractByLocationAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
};

///@endcond PRIVATE

#endif // QGSALGORITHMEXTRACTBYLOCATION_H
