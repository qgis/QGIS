/***************************************************************************
                         qgsalgorithmstdbscanclustering.cpp
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

#include "qgsalgorithmstdbscanclustering.h"

///@cond PRIVATE

QString QgsStDbscanClusteringAlgorithm::name() const
{
  return QStringLiteral( "stdbscanclustering" );
}

QString QgsStDbscanClusteringAlgorithm::displayName() const
{
  return QObject::tr( "ST-DBSCAN clustering" );
}

QString QgsStDbscanClusteringAlgorithm::shortDescription() const
{
  return QObject::tr( "Clusters spatiotemporal point features using a time and density based scan algorithm." );
}

QStringList QgsStDbscanClusteringAlgorithm::tags() const
{
  return QObject::tr( "clustering,clusters,density,based,points,temporal,time,interval,duration,distance" ).split( ',' );
}

QString QgsStDbscanClusteringAlgorithm::group() const
{
  return QObject::tr( "Vector analysis" );
}

QString QgsStDbscanClusteringAlgorithm::groupId() const
{
  return QStringLiteral( "vectoranalysis" );
}

void QgsStDbscanClusteringAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ),
                QObject::tr( "Input layer" ), QList< int >() << QgsProcessing::TypeVectorPoint ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "DATETIME_FIELD" ),
                QObject::tr( "Date/time field" ), QVariant(),
                QStringLiteral( "INPUT" ), QgsProcessingParameterField::DateTime, false, false ) );

  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "MIN_SIZE" ), QObject::tr( "Minimum cluster size" ),
                QgsProcessingParameterNumber::Integer, 5, false, 1 ) );
  addParameter( new QgsProcessingParameterDistance( QStringLiteral( "EPS" ),
                QObject::tr( "Maximum distance between clustered points" ), 1, QStringLiteral( "INPUT" ), false, 0 ) );
  auto durationParameter = std::make_unique<QgsProcessingParameterDuration>( QStringLiteral( "EPS2" ),
                           QObject::tr( "Maximum time duration between clustered points" ), 0, false, 0 );
  durationParameter->setDefaultUnit( QgsUnitTypes::TemporalHours );
  addParameter( durationParameter.release() );

  auto dbscanStarParam = std::make_unique<QgsProcessingParameterBoolean>( QStringLiteral( "DBSCAN*" ),
                         QObject::tr( "Treat border points as noise (DBSCAN*)" ), false, true );
  dbscanStarParam->setFlags( dbscanStarParam->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( dbscanStarParam.release() );

  auto fieldNameParam = std::make_unique<QgsProcessingParameterString>( QStringLiteral( "FIELD_NAME" ),
                        QObject::tr( "Cluster field name" ), QStringLiteral( "CLUSTER_ID" ) );
  fieldNameParam->setFlags( fieldNameParam->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( fieldNameParam.release() );
  auto sizeFieldNameParam = std::make_unique<QgsProcessingParameterString>( QStringLiteral( "SIZE_FIELD_NAME" ),
                            QObject::tr( "Cluster size field name" ), QStringLiteral( "CLUSTER_SIZE" ) );
  sizeFieldNameParam->setFlags( sizeFieldNameParam->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( sizeFieldNameParam.release() );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Clusters" ), QgsProcessing::TypeVectorPoint ) );

  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "NUM_CLUSTERS" ), QObject::tr( "Number of clusters" ) ) );
}

QString QgsStDbscanClusteringAlgorithm::shortHelpString() const
{
  return QObject::tr( "Clusters point features based on a 2D implementation of spatiotemporal density-based clustering of applications with noise (ST-DBSCAN) algorithm.\n\n"
                      "For more details, please see the following papers:\n"
                      "* Ester, M., H. P. Kriegel, J. Sander, and X. Xu, \"A Density-Based Algorithm for Discovering Clusters in Large Spatial Databases with Noise\". In: Proceedings of the 2nd International Conference on Knowledge Discovery and Data Mining, Portland, OR, AAAI Press, pp. 226-231. 1996\n"
                      "* Birant, Derya, and Alp Kut. \"ST-DBSCAN: An algorithm for clustering spatial–temporal data.\" Data & Knowledge Engineering 60.1 (2007): 208-221.\n"
                      "* Peca, I., Fuchs, G., Vrotsou, K., Andrienko, N. V., & Andrienko, G. L. (2012). Scalable Cluster Analysis of Spatial Events. In EuroVA@ EuroVis." );
}

QgsStDbscanClusteringAlgorithm *QgsStDbscanClusteringAlgorithm::createInstance() const
{
  return new QgsStDbscanClusteringAlgorithm();
}

///@endcond


