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

#include <random>
#include <unordered_map>

///@cond PRIVATE

constexpr uint KMEANS_MAX_ITERATIONS = 1000;

QString QgsKMeansClusteringAlgorithm::name() const
{
  return u"kmeansclustering"_s;
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
  return u"vectoranalysis"_s;
}

void QgsKMeansClusteringAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorAnyGeometry ) ) );
  addParameter( new QgsProcessingParameterNumber( u"CLUSTERS"_s, QObject::tr( "Number of clusters" ), Qgis::ProcessingNumberParameterType::Integer, 5, false, 1 ) );

  QStringList initializationMethods;
  initializationMethods << QObject::tr( "Farthest points" )
                        << QObject::tr( "K-means++" );
  addParameter( new QgsProcessingParameterEnum( u"METHOD"_s, QObject::tr( "Method" ), initializationMethods, false, 0, false ) );

  auto fieldNameParam = std::make_unique<QgsProcessingParameterString>( u"FIELD_NAME"_s, QObject::tr( "Cluster field name" ), u"CLUSTER_ID"_s );
  fieldNameParam->setFlags( fieldNameParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( fieldNameParam.release() );
  auto sizeFieldNameParam = std::make_unique<QgsProcessingParameterString>( u"SIZE_FIELD_NAME"_s, QObject::tr( "Cluster size field name" ), u"CLUSTER_SIZE"_s );
  sizeFieldNameParam->setFlags( sizeFieldNameParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( sizeFieldNameParam.release() );

  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Clusters" ), Qgis::ProcessingSourceType::VectorAnyGeometry ) );
}

QString QgsKMeansClusteringAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates the 2D distance based k-means cluster number for each input feature.\n\n"
                      "If input geometries are lines or polygons, the clustering is based on the centroid of the feature.\n\n"
                      "References:\n"
                      "Arthur, David & Vassilvitskii, Sergei. (2007). K-Means++: The Advantages of Careful Seeding. Proc. of the Annu. ACM-SIAM Symp. on Discrete Algorithms. 8.\n\n"
                      "Bhattacharya, Anup & Eube, Jan & Röglin, Heiko & Schmidt, Melanie. (2019). Noisy, Greedy and Not So Greedy k-means++" );
}

QString QgsKMeansClusteringAlgorithm::shortDescription() const
{
  return QObject::tr( "Calculates the 2D distance based k-means cluster number for each input feature." );
}

QgsKMeansClusteringAlgorithm *QgsKMeansClusteringAlgorithm::createInstance() const
{
  return new QgsKMeansClusteringAlgorithm();
}

QVariantMap QgsKMeansClusteringAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  int k = parameterAsInt( parameters, u"CLUSTERS"_s, context );
  int initializationMethod = parameterAsInt( parameters, u"METHOD"_s, context );

  QgsFields outputFields = source->fields();
  QgsFields newFields;
  const QString clusterFieldName = parameterAsString( parameters, u"FIELD_NAME"_s, context );
  newFields.append( QgsField( clusterFieldName, QMetaType::Type::Int ) );
  const QString clusterSizeFieldName = parameterAsString( parameters, u"SIZE_FIELD_NAME"_s, context );
  newFields.append( QgsField( clusterSizeFieldName, QMetaType::Type::Int ) );
  outputFields = QgsProcessingUtils::combineFields( outputFields, newFields );

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, outputFields, source->wkbType(), source->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  // build list of point inputs - if it's already a point, use that. If not, take the centroid.
  feedback->pushInfo( QObject::tr( "Collecting input points" ) );
  const double step = source->featureCount() > 0 ? 50.0 / static_cast< double >( source->featureCount() ) : 1;
  int i = 0;
  int n = 0;
  int featureWithGeometryCount = 0;
  QgsFeature feat;

  std::vector<Feature> clusterFeatures;
  QgsFeatureIterator features = source->getFeatures( QgsFeatureRequest().setNoAttributes() );
  QHash<QgsFeatureId, std::size_t> idToObj;
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
    if ( QgsWkbTypes::flatType( feat.geometry().wkbType() ) == Qgis::WkbType::Point )
      point = QgsPointXY( *qgsgeometry_cast<const QgsPoint *>( feat.geometry().constGet() ) );
    else
    {
      const QgsGeometry centroid = feat.geometry().centroid();
      if ( centroid.isNull() )
        continue; // centroid failed, e.g. empty linestring

      point = QgsPointXY( *qgsgeometry_cast<const QgsPoint *>( centroid.constGet() ) );
    }

    n++;

    idToObj[feat.id()] = clusterFeatures.size();
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
    std::vector<QgsPointXY> centers( k );
    switch ( initializationMethod )
    {
      case 0: // farthest points
        initClustersFarthestPoints( clusterFeatures, centers, k, feedback );
        break;
      case 1: // k-means++
        initClustersPlusPlus( clusterFeatures, centers, k, feedback );
        break;
      default:
        break;
    }
    calculateKMeans( clusterFeatures, centers, k, feedback );
  }

  // cluster size
  std::unordered_map<int, int> clusterSize;
  for ( auto it = idToObj.constBegin(); it != idToObj.constEnd(); ++it )
  {
    clusterSize[clusterFeatures[it.value()].cluster]++;
  }

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
      const int cluster = clusterFeatures[*obj].cluster;
      attr << cluster << clusterSize[cluster];
    }
    feat.setAttributes( attr );
    if ( !sink->addFeature( feat, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
  }

  sink->finalize();

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, dest );
  return outputs;
}

// ported from https://github.com/postgis/postgis/blob/svn-trunk/liblwgeom/lwkmeans.c

void QgsKMeansClusteringAlgorithm::initClustersFarthestPoints( std::vector<Feature> &points, std::vector<QgsPointXY> &centers, const int k, QgsProcessingFeedback *feedback )
{
  const std::size_t n = points.size();
  if ( n == 0 )
    return;

  if ( n == 1 )
  {
    for ( int i = 0; i < k; i++ )
      centers[i] = points[0].point;
    return;
  }

  std::size_t duplicateCount = 1;
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
    feedback->pushWarning( QObject::tr( "There are at least %n duplicate input(s), the number of output clusters may be less than was requested", nullptr, static_cast< int >( duplicateCount ) ) );
  }

  // By now two points should be found and be not the same
  // Q_ASSERT( p1 != p2 && maxDistance >= 0 );

  // Accept these two points as our initial centers
  centers[0] = points[p1].point;
  centers[1] = points[p2].point;

  if ( k > 2 )
  {
    // array of minimum distance to a point from accepted cluster centers
    std::vector<double> distances( n );

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

void QgsKMeansClusteringAlgorithm::initClustersPlusPlus( std::vector<Feature> &points, std::vector<QgsPointXY> &centers, const int k, QgsProcessingFeedback *feedback )
{
  const std::size_t n = points.size();
  if ( n == 0 )
    return;

  if ( n == 1 )
  {
    for ( int i = 0; i < k; i++ )
      centers[i] = points[0].point;
    return;
  }

  // randomly select the first point
  std::random_device rd;
  std::mt19937 gen( rd() );
  std::uniform_int_distribution<size_t> distrib( 0, n - 1 );

  std::size_t p1 = distrib( gen );
  centers[0] = points[p1].point;

  // calculate distances and total error (sum of distances of points to center)
  std::vector<double> distances( n );
  double totalError = 0;
  std::size_t duplicateCount = 1;
  for ( size_t i = 0; i < n; i++ )
  {
    double distance = points[i].point.sqrDist( centers[0] );
    distances[i] = distance;
    totalError += distance;
    if ( qgsDoubleNear( distance, 0 ) )
    {
      duplicateCount++;
    }
  }
  if ( feedback && duplicateCount > 1 )
  {
    feedback->pushWarning( QObject::tr( "There are at least %n duplicate input(s), the number of output clusters may be less than was requested", nullptr, static_cast< int >( duplicateCount ) ) );
  }

  // greedy kmeans++
  // test not only one center but L possible centers
  // chosen independently according to the same probability distribution), and then among these L
  // centers, the one that decreases the k-means cost the most is chosen
  // Bhattacharya, Anup & Eube, Jan & Röglin, Heiko & Schmidt, Melanie. (2019). Noisy, greedy and Not So greedy k-means++
  unsigned int numCandidateCenters = 2 + static_cast< int >( std::floor( std::log( k ) ) );
  std::vector<double> randomNumbers( numCandidateCenters );
  std::vector<size_t> candidateCenters( numCandidateCenters );

  std::uniform_real_distribution<double> dis( 0.0, 1.0 );
  for ( int i = 1; i < k; i++ )
  {
    // sampling with probability proportional to the squared distance to the closest existing center
    for ( unsigned int j = 0; j < numCandidateCenters; j++ )
    {
      randomNumbers[j] = dis( gen ) * totalError;
    }

    // cumulative sum, keep distances for later use
    std::vector<double> cumSum = distances;
    for ( size_t j = 1; j < n; j++ )
    {
      cumSum[j] += cumSum[j - 1];
    }

    // binary search for the index of the first element greater than or equal to random numbers
    for ( unsigned int j = 0; j < numCandidateCenters; j++ )
    {
      size_t low = 0;
      size_t high = n - 1;

      while ( low <= high )
      {
        size_t mid = low + ( high - low ) / 2;
        if ( cumSum[mid] < randomNumbers[j] )
        {
          low = mid + 1;
        }
        else
        {
          // size_t cannot be negative
          if ( mid == 0 )
            break;

          high = mid - 1;
        }
      }
      // clip candidate center to the number of points
      if ( low >= n )
      {
        low = n - 1;
      }
      candidateCenters[j] = low;
    }

    std::vector<std::vector<double>> distancesCandidateCenters( numCandidateCenters, std::vector<double>( n ) );
    ;

    // store distances between previous and new candidate center, error and best candidate index
    double currentError = 0;
    double lowestError = std::numeric_limits<double>::max();
    unsigned int bestCandidateIndex = 0;
    for ( unsigned int j = 0; j < numCandidateCenters; j++ )
    {
      for ( size_t z = 0; z < n; z++ )
      {
        // distance to candidate center
        double distance = points[candidateCenters[j]].point.sqrDist( points[z].point );
        // if distance to previous center is less than the current distance, use that
        if ( distance > distances[z] )
        {
          distance = distances[z];
        }
        distancesCandidateCenters[j][z] = distance;
        currentError += distance;
      }
      if ( lowestError > currentError )
      {
        lowestError = currentError;
        bestCandidateIndex = j;
      }
    }

    // update distances with the best candidate center values
    for ( size_t j = 0; j < n; j++ )
    {
      distances[j] = distancesCandidateCenters[bestCandidateIndex][j];
    }
    // store the best candidate center
    centers[i] = points[candidateCenters[bestCandidateIndex]].point;
    // update error
    totalError = lowestError;
  }
}

// ported from https://github.com/postgis/postgis/blob/svn-trunk/liblwgeom/lwkmeans.c

void QgsKMeansClusteringAlgorithm::calculateKMeans( std::vector<QgsKMeansClusteringAlgorithm::Feature> &objs, std::vector<QgsPointXY> &centers, int k, QgsProcessingFeedback *feedback )
{
  int converged = false;
  bool changed = false;

  // avoid reallocating weights array for every iteration
  std::vector<uint> weights( k );

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
    feedback->reportError( QObject::tr( "Clustering did not converge after %n iteration(s)", nullptr, static_cast<int>( i ) ) );
  else if ( feedback )
    feedback->pushInfo( QObject::tr( "Clustering converged after %n iteration(s)", nullptr, static_cast<int>( i ) ) );
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
    centers[cluster] += QgsVector( points[i].point.x(), points[i].point.y() );
    weights[cluster] += 1;
  }
  for ( int i = 0; i < k; i++ )
  {
    centers[i] /= weights[i];
  }
}


///@endcond
