/***************************************************************************
                         qgsalgorithmstatisticsbycategories.h
                         ------------------------------
    begin                : September 2026
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

#ifndef QGSALGORITHMSTATISTICSBYCATEGORIES_H
#define QGSALGORITHMSTATISTICSBYCATEGORIES_H

#include "qgis_sip.h"
#include "qgsapplication.h"
#include "qgsprocessingalgorithm.h"

#include <QString>

#define SIP_NO_FILE

using namespace Qt::StringLiterals;

///@cond PRIVATE

/**
 * Native statistics by categories algorithm.
 */
class QgsStatisticsByCategoriesAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsStatisticsByCategoriesAlgorithm() = default;
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
    QgsStatisticsByCategoriesAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    void saveCounts( const QVariantMap &parameters, const QMap<QVariantList, int> &values, QgsFeatureSink *sink, QgsProcessingFeedback *feedback );
    void calculateNumericStatistics( const QVariantMap &parameters, const QMap<QVariantList, QList<double>> &values, QgsFeatureSink *sink, QgsProcessingFeedback *feedback );
    void calculateDateTimeStatistics( const QVariantMap &parameters, const QMap<QVariantList, QVariantList> &values, QgsFeatureSink *sink, QgsProcessingFeedback *feedback );
    void calculateStringStatistics( const QVariantMap &parameters, const QMap<QVariantList, QStringList> &values, QgsFeatureSink *sink, QgsProcessingFeedback *feedback );
};

///@endcond PRIVATE

#endif // QGSALGORITHMSTATISTICSBYCATEGORIES_H
