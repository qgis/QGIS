/***************************************************************************
                         qgsalgorithmkmeansclustering.cpp
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

#include "qgsalgorithmkmeansclustering.h"
#include <unordered_map>

///@cond PRIVATE

const int KMEANS_MAX_ITERATIONS = 1000;

QString QgsKMeansClusteringAlgorithm::name() const
{
  return QStringLiteral( "kmeansclustering" );
}

QString QgsKMeansClusteringAlgorithm::displayName() const
{
  return QObject::tr( "K-means clustering" );
}

QStringList QgsKMeansClusteringAlgorithm::tags() const
{
  return QObject::tr( "clustering,clusters,kmeans,points" ).split( ',' );
}

QString QgsKMeansClusteringAlgorithm::group() const
{
  return QObject::tr( "Vector analysis" );
}

QString QgsKMeansClusteringAlgorithm::groupId() const
{
  return QStringLiteral( "vectoranalysis" );
}

void QgsKMeansClusteringAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ),
                QObject::tr( "Input layer" ), QList< int >() << QgsProcessing::TypeVectorAnyGeometry ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "CLUSTERS" ), QObject::tr( "Number of clusters" ),
                QgsProcessingParameterNumber::Integer, 5, false, 1 ) );

  auto fieldNameParam = std::make_unique<QgsProcessingParameterString>( QStringLiteral( "FIELD_NAME" ),
                        QObject::tr( "Cluster field name" ), QStringLiteral( "CLUSTER_ID" ) );
  fieldNameParam->setFlags( fieldNameParam->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( fieldNameParam.release() );
  auto sizeFieldNameParam = std::make_unique<QgsProcessingParameterString>( QStringLiteral( "SIZE_FIELD_NAME" ),
                            QObject::tr( "Cluster size field name" ), QStringLiteral( "CLUSTER_SIZE" ) );
  sizeFieldNameParam->setFlags( sizeFieldNameParam->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( sizeFieldNameParam.release() );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Clusters" ), QgsProcessing::TypeVectorAnyGeometry ) );
}

QString QgsKMeansClusteringAlgorithm::shortHelpString() const
{
  return QObject::tr( "Calculates the 2D distance based k-means cluster number for each input feature.\n\n"
                      "If input geometries are lines or polygons, the clustering is based on the centroid of the feature." );
}

QgsKMeansClusteringAlgorithm *QgsKMeansClusteringAlgorithm::createInstance() const
{
  return new QgsKMeansClusteringAlgorithm();
}

QVariantMap QgsKMeansClusteringAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsProcessingFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  int k = parameterAsInt( parameters, QStringLiteral( "CLUSTERS" ), context );

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

  // build list of point inputs - if it's already a point, use that. If not, take the centroid.
  feedback->pushInfo( QObject::tr( "Collecting input points" ) );
  const double step = source->featureCount() > 0 ? 50.0 / source->featureCount() : 1;
  int i = 0;
  int n = 0;
  int featureWithGeometryCount = 0;
  QgsFeature feat;

  std::vector< Feature > clusterFeatures;
  QgsFeatureIterator features = source->getFeatures( QgsFeatureRequest().setNoAttributes() );
  QHash< QgsFeatureId, int > idToObj;
  while ( features.nextFeature( feat ) )
  {
    i++;
    if ( feedback->isCanceled() )
    {
      break;
    }

    feedback->setProgress( i * step );
    if ( !feat.hasGeometry() )
      continue;
    featureWithGeometryCount++;

    QgsPointXY point;
    if ( QgsWkbTypes::flatType( feat.geometry().wkbType() ) == QgsWkbTypes::Point )
      point = QgsPointXY( *qgsgeometry_cast< const QgsPoint * >( feat.geometry().constGet() ) );
    else
    {
      const QgsGeometry centroid = feat.geometry().centroid();
      if ( centroid.isNull() )
        continue; // centroid failed, e.g. empty linestring

      point = QgsPointXY( *qgsgeometry_cast< const QgsPoint * >( centroid.constGet() ) );
    }

    n++;

    idToObj[ feat.id() ] = clusterFeatures.size();
    clusterFeatures.emplace_back( Feature( point ) );
  }

  if ( n < k )
  {
    feedback->reportError( QObject::tr( "Number of geometries is less than the number of clusters requested, not all clusters will get data" ) );
    k = n;
  }

  if ( k > 1 )
  {
    feedback->pushInfo( QObject::tr( "Calculating clusters" ) );

    // cluster centers
    std::vector< QgsPointXY > centers( k );

    initClusters( clusterFeatures, centers, k, feedback );
    calculateKMeans( clusterFeatures, centers, k, feedback );
  }

  // cluster size
  std::unordered_map< int, int> clusterSize;
  for ( const int obj : idToObj )
    clusterSize[ clusterFeatures[ obj ].cluster ]++;

  features = source->getFeatures();
  i = 0;
  while ( features.nextFeature( feat ) )
  {
    i++;
    if ( feedback->isCanceled() )
    {
      break;
    }

    feedback->setProgress( 50 + i * step );
    QgsAttributes attr = feat.attributes();
    const auto obj = idToObj.find( feat.id() );
    if ( !feat.hasGeometry() || obj == idToObj.end() )
    {
      attr << QVariant() << QVariant();
    }
    else if ( k <= 1 )
    {
      attr << 0 << featureWithGeometryCount;
    }
    else
    {
      const int cluster = clusterFeatures[ *obj ].cluster;
      attr << cluster << clusterSize[ cluster ];
    }
    feat.setAttributes( attr );
    if ( !sink->addFeature( feat, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

// ported from https://github.com/postgis/postgis/blob/svn-trunk/liblwgeom/lwkmeans.c

void QgsKMeansClusteringAlgorithm::initClusters( std::vector<Feature> &points, std::vector<QgsPointXY> &centers, const int k, QgsProcessingFeedback *feedback )
{
  const std::size_t n = points.size();
  if ( n == 0 )
    return;

  if ( n == 1 )
  {
    for ( int i = 0; i < k; i++ )
      centers[ i ] = points[ 0 ].point;
    return;
  }

  long duplicateCount = 1;
  // initially scan for two most distance points from each other, p1 and p2
  std::size_t p1 = 0;
  std::size_t p2 = 0;
  double distanceP1 = 0;
  double distanceP2 = 0;
  double maxDistance = -1;
  for ( std::size_t i = 1; i < n; i++ )
  {
    distanceP1 = points[i].point.sqrDist( points[p1].point );
    distanceP2 = points[i].point.sqrDist( points[p2].point );

    // if this point is further then existing candidates, replace our choice
    if ( ( distanceP1 > maxDistance ) || ( distanceP2 > maxDistance ) )
    {
      maxDistance = std::max( distanceP1, distanceP2 );
      if ( distanceP1 > distanceP2 )
        p2 = i;
      else
        p1 = i;
    }

    // also record count of duplicate points
    if ( qgsDoubleNear( distanceP1, 0 ) || qgsDoubleNear( distanceP2, 0 ) )
      duplicateCount++;
  }

  if ( feedback && duplicateCount > 1 )
  {
    feedback->pushInfo( QObject::tr( "There are at least %n duplicate input(s), the number of output clusters may be less than was requested", nullptr, duplicateCount ) );
  }

  // By now two points should be found and be not the same
  // Q_ASSERT( p1 != p2 && maxDistance >= 0 );

  // Accept these two points as our initial centers
  centers[0] = points[p1].point;
  centers[1] = points[p2].point;

  if ( k > 2 )
  {
    // array of minimum distance to a point from accepted cluster centers
    std::vector< double > distances( n );

    // initialize array with distance to first object
    for ( std::size_t j = 0; j < n; j++ )
    {
      distances[j] = points[j].point.sqrDist( centers[0] );
    }
    distances[p1] = -1;
    distances[p2] = -1;

    // loop i on clusters, skip 0 and 1 as found already
    for ( int i = 2; i < k; i++ )
    {
      std::size_t candidateCenter = 0;
      double maxDistance = std::numeric_limits<double>::lowest();

      // loop j on points
      for ( std::size_t j = 0; j < n; j++ )
      {
        // accepted clusters are already marked with distance = -1
        if ( distances[j] < 0 )
          continue;

        // update minimal distance with previously accepted cluster
        distances[j] = std::min( points[j].point.sqrDist( centers[i - 1] ), distances[j] );

        // greedily take a point that's farthest from any of accepted clusters
        if ( distances[j] > maxDistance )
        {
          candidateCenter = j;
          maxDistance = distances[j];
        }
      }

      // checked earlier by counting entries on input, just in case
      Q_ASSERT( maxDistance >= 0 );

      // accept candidate to centers
      distances[candidateCenter] = -1;
      // copy the point coordinates into the initial centers array
      centers[i] = points[candidateCenter].point;
    }
  }
}

// ported from https://github.com/postgis/postgis/blob/svn-trunk/liblwgeom/lwkmeans.c

void QgsKMeansClusteringAlgorithm::calculateKMeans( std::vector<QgsKMeansClusteringAlgorithm::Feature> &objs, std::vector<QgsPointXY> &centers, int k, QgsProcessingFeedback *feedback )
{
  int converged = false;
  bool changed = false;

  // avoid reallocating weights array for every iteration
  std::vector< uint > weights( k );

  uint i = 0;
  for ( i = 0; i < KMEANS_MAX_ITERATIONS && !converged; i++ )
  {
    if ( feedback && feedback->isCanceled() )
      break;

    findNearest( objs, centers, k, changed );
    updateMeans( objs, centers, weights, k );
    converged = !changed;
  }

  if ( !converged && feedback )
    feedback->reportError( QObject::tr( "Clustering did not converge after %n iteration(s)", nullptr, i ) );
  else if ( feedback )
    feedback->pushInfo( QObject::tr( "Clustering converged after %n iteration(s)", nullptr, i ) );
}

// ported from https://github.com/postgis/postgis/blob/svn-trunk/liblwgeom/lwkmeans.c

void QgsKMeansClusteringAlgorithm::findNearest( std::vector<QgsKMeansClusteringAlgorithm::Feature> &points, const std::vector<QgsPointXY> &centers, const int k, bool &changed )
{
  changed = false;
  const std::size_t n = points.size();
  for ( std::size_t i = 0; i < n; i++ )
  {
    Feature &point = points[i];

    // Initialize with distance to first cluster
    double currentDistance = point.point.sqrDist( centers[0] );
    int currentCluster = 0;

    // Check all other cluster centers and find the nearest
    for ( int cluster = 1; cluster < k; cluster++ )
    {
      const double distance = point.point.sqrDist( centers[cluster] );
      if ( distance < currentDistance )
      {
        currentDistance = distance;
        currentCluster = cluster;
      }
    }

    // Store the nearest cluster this object is in
    if ( point.cluster != currentCluster )
    {
      changed = true;
      point.cluster = currentCluster;
    }
  }
}

// ported from https://github.com/postgis/postgis/blob/svn-trunk/liblwgeom/lwkmeans.c

void QgsKMeansClusteringAlgorithm::updateMeans( const std::vector<Feature> &points, std::vector<QgsPointXY> &centers, std::vector<uint> &weights, const int k )
{
  const uint n = points.size();
  std::fill( weights.begin(), weights.end(), 0 );
  for ( int i = 0; i < k; i++ )
  {
    centers[i].setX( 0.0 );
    centers[i].setY( 0.0 );
  }
  for ( uint i = 0; i < n; i++ )
  {
    const int cluster = points[i].cluster;
    centers[cluster] += QgsVector( points[i].point.x(),
                                   points[i].point.y() );
    weights[cluster] += 1;
  }
  for ( int i = 0; i < k; i++ )
  {
    centers[i] /= weights[i];
  }
}


///@endcond


