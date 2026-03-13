/***************************************************************************
                         qgsalgorithmrandomextractwithinsubsets.h
                         ---------------------
    begin                : January 2026
    copyright            : (C) 2026 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALGORITHMRANDOMEXTRACTWITHINSUBSETS_H
#define QGSALGORITHMRANDOMEXTRACTWITHINSUBSETS_H

#include "qgis_sip.h"
#include "qgsapplication.h"
#include "qgsprocessingalgorithm.h"

#include <QString>

#define SIP_NO_FILE

using namespace Qt::StringLiterals;

///@cond PRIVATE

/**
 * Base class for random extraction/selection within subsets algorithms.
 */
class QgsRandomExtractWithinSubsetsAlgorithmBase : public QgsProcessingAlgorithm
{
  public:
    QString group() const override;
    QString groupId() const override;

  protected:
    /**
     * Selects a \a value number/percentage of random feature IDs from the \a source.
     */
    void sampleFeatureIds( QgsFeatureSource *source, const double value, const QString &fieldName, QgsProcessingFeedback *feedback );

    QgsFeatureIds mSelectedFeatureIds;
    int mMethod = 0;
};

/**
 * Native random extract within subsets algorithm.
 */
class QgsRandomExtractWithinSubsetsAlgorithm : public QgsRandomExtractWithinSubsetsAlgorithmBase
{
  public:
    QgsRandomExtractWithinSubsetsAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    Qgis::ProcessingAlgorithmDocumentationFlags documentationFlags() const override;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QgsRandomExtractWithinSubsetsAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
};

/**
 * Native random selection algorithm.
 */
class QgsRandomSelectionWithinSubsetsAlgorithm : public QgsRandomExtractWithinSubsetsAlgorithmBase
{
  public:
    QgsRandomSelectionWithinSubsetsAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QIcon icon() const override { return QgsApplication::getThemeIcon( u"/algorithms/mAlgorithmSelectRandom.svg"_s ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( u"/algorithms/mAlgorithmSelectRandom.svg"_s ); }
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QgsRandomSelectionWithinSubsetsAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap postProcessAlgorithm( QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    QVariant mInput;
    QgsVectorLayer *mTargetLayer = nullptr;
};

///@endcond PRIVATE

#endif // QGSALGORITHMRANDOMEXTRACTWITHINSUBSETS_H
