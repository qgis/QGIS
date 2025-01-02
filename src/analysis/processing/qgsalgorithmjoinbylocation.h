/***************************************************************************
                         qgsalgorithmjoinbylocation.h
                         ---------------------
    begin                : January 2020
    copyright            : (C) 2020 by Alexis Roy-Lizotte
    email                : roya2 at premiertech dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMJOINBYLOCATION_H
#define QGSALGORITHMJOINBYLOCATION_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgsprocessingalgorithm.h"
#include "qgsfeature.h"


///@cond PRIVATE

/**
 * Native join by location algorithm
 *
 * \ since QGIS 3.12
 */
class QgsJoinByLocationAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsJoinByLocationAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    Qgis::ProcessingAlgorithmDocumentationFlags documentationFlags() const override;
    QgsJoinByLocationAlgorithm *createInstance() const override SIP_FACTORY;

    /**
     * Returns a translated list of predicate names, in the order expected by the algorithm's enum parameter.
     */
    static QStringList translatedPredicates();

    /**
     * Sorts a list of predicates so that faster ones are tested first
     */
    static void sortPredicates( QList<int> &predicates );

    /**
     * Returns TRUE if \a feature satisfies any of the predicates.
     */
    static bool featureFilter( const QgsFeature &feature, QgsGeometryEngine *engine, bool comparingToJoinedFeature, const QList<int> &predicates );

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    bool processFeatureFromJoinSource( QgsFeature &joinFeature, QgsProcessingFeedback *feedback );
    bool processFeatureFromInputSource( QgsFeature &inputFeature, QgsProcessingContext &context, QgsProcessingFeedback *feedback );

  private:
    void processAlgorithmByIteratingOverJoinedSource( QgsProcessingContext &context, QgsProcessingFeedback *feedback );
    void processAlgorithmByIteratingOverInputSource( QgsProcessingContext &context, QgsProcessingFeedback *feedback );

    enum JoinMethod
    {
      OneToMany = 0,
      JoinToFirst = 1,
      JoinToLargestOverlap = 2
    };
    long mJoinedCount = 0;
    std::unique_ptr<QgsProcessingFeatureSource> mBaseSource;
    std::unique_ptr<QgsProcessingFeatureSource> mJoinSource;
    QgsAttributeList mJoinedFieldIndices;
    bool mDiscardNonMatching = false;
    std::unique_ptr<QgsFeatureSink> mJoinedFeatures;
    QgsFeatureIds mAddedIds;
    std::unique_ptr<QgsFeatureSink> mUnjoinedFeatures;
    JoinMethod mJoinMethod = OneToMany;
    QList<int> mPredicates;
};

///@endcond PRIVATE

#endif // QGSALGORITHMJOINBYLOCATION_H
