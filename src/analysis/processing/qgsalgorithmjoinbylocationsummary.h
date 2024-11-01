/***************************************************************************
                         qgsalgorithmjoinbylocationsummary.h
                         ---------------------
    begin                : September 2023
    copyright            : (C) 2023 by Nyall Dawson
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

#ifndef QGSALGORITHMJOINBYLOCATIONSUMMARY_H
#define QGSALGORITHMJOINBYLOCATIONSUMMARY_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native join by location (summary) algorithm
 */
class QgsJoinByLocationSummaryAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsJoinByLocationSummaryAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString shortDescription() const override;
    QIcon icon() const override;
    QString svgIconPath() const override;
    QgsJoinByLocationSummaryAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:
    QStringList mAllSummaries;
};

///@endcond PRIVATE

#endif // QGSALGORITHMJOINBYLOCATIONSUMMARY_H
