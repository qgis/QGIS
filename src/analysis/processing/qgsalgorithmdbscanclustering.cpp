/***************************************************************************
                         qgsalgorithmdbscanclustering.cpp
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

#include "qgsalgorithmdbscanclustering.h"
#include "qgsspatialindexkdbush.h"
#include <unordered_set>

///@cond PRIVATE

QString QgsDbscanClusteringAlgorithm::name() const
{
  return QStringLiteral( "dbscanclustering" );
}

QString QgsDbscanClusteringAlgorithm::displayName() const
{
  return QObject::tr( "DBSCAN clustering" );
}

QString QgsDbscanClusteringAlgorithm::shortDescription() const
{
  return QObject::tr( "Clusters point features using a density based scan algorithm." );
}

QStringList QgsDbscanClusteringAlgorithm::tags() const
{
  return QObject::tr( "clustering,clusters,density,based,points,distance" ).split( ',' );
}

QString QgsDbscanClusteringAlgorithm::group() const
{
  return QObject::tr( "Vector analysis" );
}

QString QgsDbscanClusteringAlgorithm::groupId() const
{
  return QStringLiteral( "vectoranalysis" );
}

void QgsDbscanClusteringAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ),
                QObject::tr( "Input layer" ), QList< int >() << QgsProcessing::TypeVectorPoint ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "MIN_SIZE" ), QObject::tr( "Minimum cluster size" ),
                QgsProcessingParameterNumber::Integer, 5, false, 1 ) );
  addParameter( new QgsProcessingParameterDistance( QStringLiteral( "EPS" ),
                QObject::tr( "Maximum distance between clustered points" ), 1, QStringLiteral( "INPUT" ), false, 0 ) );

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

QString QgsDbscanClusteringAlgorithm::shortHelpString() const
{
  return QObject::tr( "Clusters point features based on a 2D implementation of Density-based spatial clustering of applications with noise (DBSCAN) algorithm.\n\n"
                      "The algorithm requires two parameters, a minimum cluster size (“minPts”), and the maximum distance allowed between clustered points (“eps”)." );
}

QgsDbscanClusteringAlgorithm *QgsDbscanClusteringAlgorithm::createInstance() const
{
  return new QgsDbscanClusteringAlgorithm();
}

struct KDBushDataEqualById
{
  bool operator()( const QgsSpatialIndexKDBushData &a, const QgsSpatialIndexKDBushData &b ) const
  {
    return  a.id == b.id;
  }
};

struct KDBushDataHashById
{
  std::size_t operator()( const QgsSpatialIndexKDBushData &a ) const
  {
    return std::hash< QgsFeatureId > {}( a.id );
  }
};

QVariantMap QgsDbscanClusteringAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsProcessingFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  const std::size_t minSize = static_cast< std::size_t>( parameterAsInt( parameters, QStringLiteral( "MIN_SIZE" ), context ) );
  const double eps1 = parameterAsDouble( parameters, QStringLiteral( "EPS" ), context );
  const double eps2 = parameterAsDouble( parameters, QStringLiteral( "EPS2" ), context );
  const bool borderPointsAreNoise = parameterAsBoolean( parameters, QStringLiteral( "DBSCAN*" ), context );

  QgsFields outputFields = source->fields();
  QgsFields newFields;
  const QString clusterFieldName = parameterAsString( parameters, QStringLiteral( "FIELD_NAME" ), context );
  newFields.append( QgsField( clusterFieldName, QVariant::Int ) );
  const QString clusterSizeFieldName = parameterAsString( parameters, QStringLiteral( "SIZE_FIELD_NAME" ), context );
  newFields.append( QgsField( clusterSizeFieldName, QVariant::Int ) );
  outputFields = QgsProcessingUtils::combineFields( outputFields, newFields );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, outputFields, source->wkbType(), source->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  QgsFeatureRequest indexRequest;

  std::unordered_map< QgsFeatureId, QDateTime> idToDateTime;
  const QString dateTimeFieldName = parameterAsString( parameters, QStringLiteral( "DATETIME_FIELD" ), context );
  int dateTimefieldIndex = -1;
  if ( !dateTimeFieldName.isEmpty() )
  {
    dateTimefieldIndex = source->fields().lookupField( dateTimeFieldName );
    if ( dateTimefieldIndex == -1 )
      throw QgsProcessingException( QObject::tr( "Datetime field missing" ) );

    indexRequest.setSubsetOfAttributes( QgsAttributeList() << dateTimefieldIndex );
  }
  else
  {
    indexRequest.setNoAttributes();
  }

  // build spatial index, also collecting feature datetimes if required
  feedback->pushInfo( QObject::tr( "Building spatial index" ) );
  QgsFeatureIterator indexIterator = source->getFeatures( indexRequest );
  QgsSpatialIndexKDBush index( indexIterator, [&idToDateTime, dateTimefieldIndex]( const QgsFeature & feature )->bool
  {
    if ( dateTimefieldIndex >= 0 )
      idToDateTime[ feature.id() ] = feature.attributes().at( dateTimefieldIndex ).toDateTime();
    return true;
  }, feedback );

  if ( feedback->isCanceled() )
    return QVariantMap();

  // stdbscan!
  feedback->pushInfo( QObject::tr( "Analysing clusters" ) );
  std::unordered_map< QgsFeatureId, int> idToCluster;
  idToCluster.reserve( index.size() );
  const long featureCount = source->featureCount();
  QgsFeatureIterator features = source->getFeatures( QgsFeatureRequest().setNoAttributes() );
  stdbscan( minSize, eps1, eps2, borderPointsAreNoise, featureCount, features, index, idToCluster, idToDateTime, feedback );

  // cluster size
  std::unordered_map< int, int> clusterSize;
  std::for_each( idToCluster.begin(), idToCluster.end(), [ &clusterSize ]( std::pair< QgsFeatureId, int > idCluster ) { clusterSize[ idCluster.second ]++; } );

  // write clusters
  const double writeStep = featureCount > 0 ? 10.0 / featureCount : 1;
  features = source->getFeatures();
  int i = 0;
  QgsFeature feat;
  while ( features.nextFeature( feat ) )
  {
    i++;
    if ( feedback->isCanceled() )
    {
      break;
    }

    feedback->setProgress( 90 + i * writeStep );
    QgsAttributes attr = feat.attributes();
    const auto cluster = idToCluster.find( feat.id() );
    if ( cluster != idToCluster.end() )
    {
      attr << cluster->second  << clusterSize[ cluster->second ];
    }
    else
    {
      attr << QVariant() << QVariant();
    }
    feat.setAttributes( attr );
    if ( !sink->addFeature( feat, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  outputs.insert( QStringLiteral( "NUM_CLUSTERS" ), static_cast< unsigned int >( clusterSize.size() ) );
  return outputs;
}

void QgsDbscanClusteringAlgorithm::stdbscan( const std::size_t minSize,
    const double eps1,
    const double eps2,
    const bool borderPointsAreNoise,
    const long featureCount,
    QgsFeatureIterator features,
    QgsSpatialIndexKDBush &index,
    std::unordered_map< QgsFeatureId, int> &idToCluster,
    std::unordered_map< QgsFeatureId, QDateTime> &idToDateTime,
    QgsProcessingFeedback *feedback )
{
  const double step = featureCount > 0 ? 90.0 / featureCount : 1;

  std::unordered_set< QgsFeatureId > visited;
  visited.reserve( index.size() );

  QgsFeature feat;
  int i = 0;
  int clusterCount = 0;

  while ( features.nextFeature( feat ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    if ( !feat.hasGeometry() )
    {
      feedback->setProgress( ++i * step );
      continue;
    }

    if ( visited.find( feat.id() ) != visited.end() )
    {
      // already visited!
      continue;
    }

    QgsPointXY point;
    if ( QgsWkbTypes::flatType( feat.geometry().wkbType() ) == QgsWkbTypes::Point )
      point = QgsPointXY( *qgsgeometry_cast< const QgsPoint * >( feat.geometry().constGet() ) );
    else
    {
      // not a point geometry
      feedback->reportError( QObject::tr( "Feature %1 is a %2 feature, not a point." ).arg( feat.id() ).arg( QgsWkbTypes::displayString( feat.geometry().wkbType() ) ) );
      feedback->setProgress( ++i * step );
      continue;
    }

    if ( !idToDateTime.empty() && !idToDateTime[ feat.id() ].isValid() )
    {
      // missing datetime value
      feedback->reportError( QObject::tr( "Feature %1 is missing a valid datetime value." ).arg( feat.id() ).arg( QgsWkbTypes::displayString( feat.geometry().wkbType() ) ) );
      feedback->setProgress( ++i * step );
      continue;
    }

    std::unordered_set< QgsSpatialIndexKDBushData, KDBushDataHashById, KDBushDataEqualById> within;

    if ( minSize > 1 )
    {
      index.within( point, eps1, [&within, pointId = feat.id(), &idToDateTime, &eps2]( const QgsSpatialIndexKDBushData & data )
      {
        if ( idToDateTime.empty() || ( idToDateTime[ data.id ].isValid() && std::abs( idToDateTime[ pointId ].msecsTo( idToDateTime[ data.id ] ) ) <= eps2 ) )
          within.insert( data );
      } );
      if ( within.size() < minSize )
        continue;

      visited.insert( feat.id() );
    }
    else
    {
      // optimised case for minSize == 1, we can skip the initial check
      within.insert( QgsSpatialIndexKDBushData( feat.id(), point.x(), point.y() ) );
    }

    // start new cluster
    clusterCount++;
    idToCluster[ feat.id() ] = clusterCount;
    feedback->setProgress( ++i * step );

    while ( !within.empty() )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }

      const QgsSpatialIndexKDBushData j = *within.begin();
      within.erase( within.begin() );

      if ( visited.find( j.id ) != visited.end() )
      {
        // already visited!
        continue;
      }

      visited.insert( j.id );
      feedback->setProgress( ++i * step );

      // check from this point
      const QgsPointXY point2 = j.point();

      std::unordered_set< QgsSpatialIndexKDBushData, KDBushDataHashById, KDBushDataEqualById > within2;
      index.within( point2, eps1, [&within2, point2Id = j.id, &idToDateTime, &eps2]( const QgsSpatialIndexKDBushData & data )
      {
        if ( idToDateTime.empty() || ( idToDateTime[ data.id ].isValid() && std::abs( idToDateTime[ point2Id ].msecsTo( idToDateTime[ data.id ] ) ) <= eps2 ) )
          within2.insert( data );
      } );

      if ( within2.size() >= minSize )
      {
        // expand neighbourhood
        std::copy_if( within2.begin(),
                      within2.end(),
                      std::inserter( within, within.end() ),
                      [&visited]( const QgsSpatialIndexKDBushData & needle )
        {
          return visited.find( needle.id ) == visited.end();
        } );
      }
      if ( !borderPointsAreNoise || within2.size() >= minSize )
      {
        idToCluster[ j.id ] = clusterCount;
      }
    }
  }
}

///@endcond


