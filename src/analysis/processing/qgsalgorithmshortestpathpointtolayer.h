/***************************************************************************
                         qgsalgorithmshortestpathpointtolayer.h
                         ---------------------
    begin                : JUly 2018
    copyright            : (C) 2018 by Alexander Bruy
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

#ifndef QGSALGORITHMSHORTESTPATHPOINTTOLAYER_H
#define QGSALGORITHMSHORTESTPATHPOINTTOLAYER_H

#define SIP_NO_FILE

#include "qgis.h"
#include "qgsalgorithmnetworkanalysisbase.h"

///@cond PRIVATE

/**
 * Native shortest path (point to layer) algorithm.
 */
class QgsShortestPathPointToLayerAlgorithm : public QgsNetworkAnalysisAlgorithmBase
{

  public:

    QgsShortestPathPointToLayerAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QgsShortestPathPointToLayerAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};

///@endcond PRIVATE

#endif // QGSALGORITHMSHORTESTPATHPOINTTOLAYER_H
