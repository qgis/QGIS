/***************************************************************************
                         qgsalgorithmshortestpathlayertopoint.h
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

#ifndef QGSALGORITHMSHORTESTPATHLAYERTOPOINT_H
#define QGSALGORITHMSHORTESTPATHLAYERTOPOINT_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsalgorithmnetworkanalysisbase.h"

///@cond PRIVATE

/**
 * Native shortest path (layer to point) algorithm.
 */
class QgsShortestPathLayerToPointAlgorithm : public QgsNetworkAnalysisAlgorithmBase
{

  public:

    QgsShortestPathLayerToPointAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString shortHelpString() const override;
    QgsShortestPathLayerToPointAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

};

///@endcond PRIVATE

#endif // QGSALGORITHMSHORTESTPATHLAYERTOPOINT_H
