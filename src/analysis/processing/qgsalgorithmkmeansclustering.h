/***************************************************************************
                         qgsalgorithmkmeansclustering.h
                         ---------------------
    begin                : June 2018
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

#ifndef QGSALGORITHMKMEANSCLUSTERING_H
#define QGSALGORITHMKMEANSCLUSTERING_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgis_analysis.h"
#include "qgsprocessingalgorithm.h"

///@cond PRIVATE


/**
 * Native k-means clustering algorithm.
 */
class ANALYSIS_EXPORT QgsKMeansClusteringAlgorithm : public QgsProcessingAlgorithm
{

  public:

    QgsKMeansClusteringAlgorithm() = default;
    void initAlgorithm( const QVariantMap &configuration = QVariantMap() ) override;
    QString name() const override;
    QString displayName() const override;
    QStringList tags() const override;
    QString group() const override;
    QString groupId() const override;
    QString shortHelpString() const override;
    QgsKMeansClusteringAlgorithm *createInstance() const override SIP_FACTORY;

  protected:

    QVariantMap processAlgorithm( const QVariantMap &parameters,
                                  QgsProcessingContext &context, QgsProcessingFeedback *feedback ) override;

  private:

    struct Feature
    {
      Feature( QgsPointXY point )
        : point( point )
      {}

      QgsPointXY point;
      int cluster = -1;
    };

    static void initClusters( std::vector< Feature > &points, std::vector< QgsPointXY > &centers, int k, QgsProcessingFeedback *feedback );
    static void calculateKMeans( std::vector< Feature > &points, std::vector< QgsPointXY > &centers, int k, QgsProcessingFeedback *feedback );
    static void findNearest( std::vector< Feature > &points, const std::vector< QgsPointXY > &centers, int k, bool &changed );
    static void updateMeans( const std::vector< Feature > &points, std::vector< QgsPointXY > &centers, std::vector< uint > &weights, int k );

    friend class TestQgsProcessingAlgsPt1;
};

///@endcond PRIVATE

#endif // QGSALGORITHMKMEANSCLUSTERING_H


