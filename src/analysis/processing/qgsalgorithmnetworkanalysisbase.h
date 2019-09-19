/***************************************************************************
                         qgsalgorithmnetworkanalysisbase.h
                         ---------------------
    begin                : July 2018
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

#ifndef QGSALGORITHMNETWORKANALYSISBASE_H
#define QGSALGORITHMNETWORKANALYSISBASE_H

#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgsprocessingalgorithm.h"

#include "qgsgraph.h"
#include "qgsgraphbuilder.h"
#include "qgsvectorlayerdirector.h"
#include "qgsapplication.h"

///@cond PRIVATE

/**
 * Base class for networkanalysis algorithms.
 */
class QgsNetworkAnalysisAlgorithmBase : public QgsProcessingAlgorithm
{
  public:

    QString group() const final;
    QString groupId() const final;
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmNetworkAnalysis.svg" ) ); }
    QString svgIconPath() const override { return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmNetworkAnalysis.svg" ) ); }

  protected:

    /**
     * Adds common algorithm parameters.
     */
    void addCommonParams();

    /**
     * Populates common parameters with values.
     */
    void loadCommonParams( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback );

    /**
     * Loads point from the feature source for further processing.
     */
    void loadPoints( QgsFeatureSource *source, QVector< QgsPointXY > &points, QHash< int, QgsAttributes > &attributes, QgsProcessingContext &context, QgsProcessingFeedback *feedback );

    std::unique_ptr< QgsFeatureSource > mNetwork;
    QgsVectorLayerDirector *mDirector = nullptr;
    std::unique_ptr< QgsGraphBuilder > mBuilder;
    std::unique_ptr< QgsGraph > mGraph;
    double mMultiplier = 1;
};

///@endcond PRIVATE

#endif // QGSALGORITHMNETWORKANALYSISBASE_H

