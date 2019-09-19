/***************************************************************************
                         qgsalgorithmdbscanclustering.h
                         ---------------------
    begin                : July 2018
    copyright            : (C) 2018 by Nyall Dawson
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

#ifndef QGSALGORITHMDBSCANCLUSTERING_H
#define QGSALGORITHMDBSCANCLUSTERING_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgis_analysis.h"
#include "qgsprocessingalgorithm.h"
#include <unordered_map>

class QgsSpatialIndexKDBush;

///@cond PRIVATE


/**
 * Native DBSCAN density based scan with noise clustering algorithm.
 */
class ANALYSIS_EXPORT QgsDbscanClusteringAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsDbscanClusteringAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QString shortDescription() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsDbscanClusteringAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;
  private:
    static void dbscan( std::size_t minSize,
                        double eps,
                        bool borderPointsAreNoise,
                        long featureCount,
                        QgsFeatureIterator features,
                        QgsSpatialIndexKDBush &index,
                        std::unordered_map< QgsFeatureId, int> &idToCluster,
                        int &clusterCount,
                        QgsProcessingFeedback *feedback );
};

///@endcond PRIVATE

#endif // QGSALGORITHMDBSCANCLUSTERING_H


