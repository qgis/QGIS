/***************************************************************************
                         qgsalgorithmconcavehull.h
                         ---------------------
    begin                : July 2023
    copyright            : (C) 2023 by Alexander Bruy
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

#ifndef QGSALGORITHMCONCAVEHULL_H
#define QGSALGORITHMCONCAVEHULL_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"
#include "qgsapplication.h"

///@cond PRIVATE

/**
 * Native concave hull algorithm.
 */
class QgsConcaveHullAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsConcaveHullAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmConcaveHull.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmConcaveHull.svg" ) ); }
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsConcaveHullAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    bool prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    void concaveHullGeos( std::unique_ptr<QgsFeatureSink> &sink, const QVariantMap &parameters, QgsProcessingFeedback *feedback );
    void concaveHullQgis( std::unique_ptr<QgsFeatureSink> &sink, const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback );

    std::unique_ptr<QgsProcessingFeatureSource> mSource;
    double mPercentage = 0;
    bool mAllowHoles = false;
    bool mSplitMultipart = false;
    double mStep = 0;
};

///@endcond PRIVATE

#endif // QGSALGORITHMCONCAVEHULL_H
