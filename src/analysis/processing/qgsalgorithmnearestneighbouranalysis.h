/***************************************************************************
                         qgsalgorithmnearestneighbouranalysis.h
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

#ifndef QGSALGORITHMNEARESTNEIGHBOURANALYSIS_H
#define QGSALGORITHMNEARESTNEIGHBOURANALYSIS_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE

/**
 * Native nearest neighbour analysis algorithm.
 */
class QgsNearestNeighbourAnalysisAlgorithm : public QgsProcessingAlgorithm
{
  public:
    QgsNearestNeighbourAnalysisAlgorithm() = default;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QString svgIconPath() const override;
    QIcon icon() const override;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QgsNearestNeighbourAnalysisAlgorithm *createInstance() const override SIP_FACTORY;

  protected:
    QVariantMap processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
};

///@endcond PRIVATE

#endif // QGSALGORITHMNEARESTNEIGHBOURANALYSIS_H
