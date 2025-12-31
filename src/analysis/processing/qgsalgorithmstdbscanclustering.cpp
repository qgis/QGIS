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
  return u"stdbscanclustering"_s;
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
  return u"vectoranalysis"_s;
}

void QgsStDbscanClusteringAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint ) ) );
  addParameter( new QgsProcessingParameterField( u"DATETIME_FIELD"_s, QObject::tr( "Date/time field" ), QVariant(), u"INPUT"_s, Qgis::ProcessingFieldParameterDataType::DateTime, false, false ) );

  addParameter( new QgsProcessingParameterNumber( u"MIN_SIZE"_s, QObject::tr( "Minimum cluster size" ), Qgis::ProcessingNumberParameterType::Integer, 5, false, 1 ) );
  addParameter( new QgsProcessingParameterDistance( u"EPS"_s, QObject::tr( "Maximum distance between clustered points" ), 1, u"INPUT"_s, false, 0 ) );
  auto durationParameter = std::make_unique<QgsProcessingParameterDuration>( u"EPS2"_s, QObject::tr( "Maximum time duration between clustered points" ), 0, false, 0 );
  durationParameter->setDefaultUnit( Qgis::TemporalUnit::Hours );
  addParameter( durationParameter.release() );

  auto dbscanStarParam = std::make_unique<QgsProcessingParameterBoolean>( u"DBSCAN*"_s, QObject::tr( "Treat border points as noise (DBSCAN*)" ), false, true );
  dbscanStarParam->setFlags( dbscanStarParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( dbscanStarParam.release() );

  auto fieldNameParam = std::make_unique<QgsProcessingParameterString>( u"FIELD_NAME"_s, QObject::tr( "Cluster field name" ), u"CLUSTER_ID"_s );
  fieldNameParam->setFlags( fieldNameParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( fieldNameParam.release() );
  auto sizeFieldNameParam = std::make_unique<QgsProcessingParameterString>( u"SIZE_FIELD_NAME"_s, QObject::tr( "Cluster size field name" ), u"CLUSTER_SIZE"_s );
  sizeFieldNameParam->setFlags( sizeFieldNameParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( sizeFieldNameParam.release() );

  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Clusters" ), Qgis::ProcessingSourceType::VectorPoint ) );

  addOutput( new QgsProcessingOutputNumber( u"NUM_CLUSTERS"_s, QObject::tr( "Number of clusters" ) ) );
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
