/***************************************************************************
                         qgsalgorithmbasicstatistics.h
                         ------------------------------
    begin                : June 2024
    copyright            : (C) 2024 by Alexander Bruy
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

#ifndef QGSALGORITHMBASICSTATISTICS_H
#define QGSALGORITHMBASICSTATISTICS_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsapplication.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native basic statistics algorithm.
 */
class QgsBasicStatisticsAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsBasicStatisticsAlgorithm() = default;
    QIcon icon() const override { return QgsApplication::getThemeIcon( u"/algorithms/mAlgorithmBasicStatistics.svg"_s ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( u"/algorithms/mAlgorithmBasicStatistics.svg"_s ); }
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QgsBasicStatisticsAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    QVariantMap calculateNumericStatistics( const QVariantMap &parameters, const int fieldIndex, QgsFeatureIterator features, const long long count, QgsFeatureSink *sink, QStringList &data, QgsProcessingFeedback *feedback );
    QVariantMap calculateDateTimeStatistics( const QVariantMap &parameters, const int fieldIndex, QgsField field, QgsFeatureIterator features, const long long count, QgsFeatureSink *sink, QStringList &data, QgsProcessingFeedback *feedback );
    QVariantMap calculateStringStatistics( const QVariantMap &parameters, const int fieldIndex, QgsFeatureIterator features, const long long count, QgsFeatureSink *sink, QStringList &data, QgsProcessingFeedback *feedback );
};

///@endcond PRIVATE

#endif // QGSALGORITHMBASICSTATISTICS_H
