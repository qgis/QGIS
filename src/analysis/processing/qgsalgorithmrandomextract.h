/***************************************************************************
                         qgsalgorithmrandomextract.h
                         ---------------------
    begin                : December 2019
    copyright            : (C) 2019 by Alexander Bruy
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

#ifndef QGSALGORITHMRANDOMEXTRACT_H
#define QGSALGORITHMRANDOMEXTRACT_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsapplication.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Base class for random extraction/selection algorithms.
 */
class QgsRandomExtractSelectAlgorithmBase : public QgsProcessingAlgorithm
{
  public:
    QString group() const override;
    QString groupId() const override;

  protected:
    /**
     * Selects \a count random feature IDs from the \a source.
     */
    void sampleFeatureIds( QgsFeatureSource *source, const long long count, QgsProcessingFeedback *feedback );

    QgsFeatureIds mSelectedFeatureIds;
};

/**
 * Native random extract algorithm.
 */
class QgsRandomExtractAlgorithm : public QgsRandomExtractSelectAlgorithmBase
{
  public:
    QgsRandomExtractAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    Qgis::ProcessingAlgorithmDocumentationFlags documentationFlags() const override;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QgsRandomExtractAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
};

/**
 * Native random selection algorithm.
 */
class QgsRandomSelectionAlgorithm : public QgsRandomExtractSelectAlgorithmBase
{
  public:
    QgsRandomSelectionAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QIcon icon() const override { return QgsApplication::getThemeIcon( u"/algorithms/mAlgorithmSelectRandom.svg"_s ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( u"/algorithms/mAlgorithmSelectRandom.svg"_s ); }
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QgsRandomSelectionAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap postProcessAlgorithm( QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    QVariant mInput;
    QgsVectorLayer *mTargetLayer = nullptr;
};

///@endcond PRIVATE

#endif // QGSALGORITHMRANDOMEXTRACT_H
