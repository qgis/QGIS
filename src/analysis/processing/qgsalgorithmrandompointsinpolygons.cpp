/***************************************************************************
                         qgsalgorithmrandompointsinpolygons.cpp
                         ---------------------
    begin                : March 2020
    copyright            : (C) 2020 by Håvard Tveite
    email                : havard dot tveite at nmbu dot no
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsalgorithmrandompointsinpolygons.h"
#include <random>

// The algorithm parameter names:
static const QString INPUT = QStringLiteral( "INPUT" );
static const QString POINTS_NUMBER = QStringLiteral( "POINTS_NUMBER" );
static const QString MIN_DISTANCE_GLOBAL = QStringLiteral( "MIN_DISTANCE_GLOBAL" );
static const QString MIN_DISTANCE = QStringLiteral( "MIN_DISTANCE" );
static const QString MAX_TRIES_PER_POINT = QStringLiteral( "MAX_TRIES_PER_POINT" );
static const QString SEED = QStringLiteral( "SEED" );
static const QString INCLUDE_POLYGON_ATTRIBUTES = QStringLiteral( "INCLUDE_POLYGON_ATTRIBUTES" );
static const QString OUTPUT = QStringLiteral( "OUTPUT" );
static const QString OUTPUT_POINTS = QStringLiteral( "OUTPUT_POINTS" );
static const QString POINTS_MISSED = QStringLiteral( "POINTS_MISSED" );
static const QString POLYGONS_WITH_MISSED_POINTS = QStringLiteral( "POLYGONS_WITH_MISSED_POINTS" );
static const QString FEATURES_WITH_EMPTY_OR_NO_GEOMETRY = QStringLiteral( "FEATURES_WITH_EMPTY_OR_NO_GEOMETRY" );
///@cond PRIVATE

QString QgsRandomPointsInPolygonsAlgorithm::name() const
{
  return QStringLiteral( "randompointsinpolygons" );
}

QString QgsRandomPointsInPolygonsAlgorithm::displayName() const
{
  return QObject::tr( "Random points in polygons" );
}

QStringList QgsRandomPointsInPolygonsAlgorithm::tags() const
{
  return QObject::tr( "seed,attributes,create" ).split( ',' );
}

QString QgsRandomPointsInPolygonsAlgorithm::group() const
{
  return QObject::tr( "Vector creation" );
}

QString QgsRandomPointsInPolygonsAlgorithm::groupId() const
{
  return QStringLiteral( "vectorcreation" );
}

void QgsRandomPointsInPolygonsAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( INPUT, QObject::tr( "Input polygon layer" ), QList< int >() << QgsProcessing::TypeVectorPolygon ) );
  std::unique_ptr< QgsProcessingParameterNumber > numberPointsParam = std::make_unique< QgsProcessingParameterNumber >( POINTS_NUMBER, QObject::tr( "Number of points for each feature" ), QgsProcessingParameterNumber::Integer, 1, false, 1 );
  numberPointsParam->setIsDynamic( true );
  numberPointsParam->setDynamicPropertyDefinition( QgsPropertyDefinition( POINTS_NUMBER, QObject::tr( "Number of points for each feature" ), QgsPropertyDefinition::IntegerPositive ) );
  numberPointsParam->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( numberPointsParam.release() );

  std::unique_ptr< QgsProcessingParameterDistance > minDistParam = std::make_unique< QgsProcessingParameterDistance >( MIN_DISTANCE, QObject::tr( "Minimum distance between points" ), 0, INPUT, true, 0 );
  minDistParam->setIsDynamic( true );
  minDistParam->setDynamicPropertyDefinition( QgsPropertyDefinition( MIN_DISTANCE, QObject::tr( "Minimum distance between points" ), QgsPropertyDefinition::DoublePositive ) );
  minDistParam->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( minDistParam.release() );

  std::unique_ptr< QgsProcessingParameterDistance > minDistGlobalParam = std::make_unique< QgsProcessingParameterDistance >( MIN_DISTANCE_GLOBAL, QObject::tr( "Global minimum distance between points" ), 0, INPUT, true, 0 );
  minDistGlobalParam->setFlags( minDistGlobalParam->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( minDistGlobalParam.release() );

  std::unique_ptr< QgsProcessingParameterNumber > maxAttemptsParam = std::make_unique< QgsProcessingParameterNumber >( MAX_TRIES_PER_POINT, QObject::tr( "Maximum number of search attempts (for Min. dist. > 0)" ), QgsProcessingParameterNumber::Integer, 10, true, 1 );
  maxAttemptsParam->setFlags( maxAttemptsParam->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  maxAttemptsParam->setIsDynamic( true );
  maxAttemptsParam->setDynamicPropertyDefinition( QgsPropertyDefinition( MAX_TRIES_PER_POINT, QObject::tr( "Maximum number of attempts per point (for Min. dist. > 0)" ), QgsPropertyDefinition::IntegerPositiveGreaterZero ) );
  maxAttemptsParam->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( maxAttemptsParam.release() );

  std::unique_ptr< QgsProcessingParameterNumber > randomSeedParam = std::make_unique< QgsProcessingParameterNumber >( SEED, QObject::tr( "Random seed" ), QgsProcessingParameterNumber::Integer, QVariant(), true, 1 );
  randomSeedParam->setFlags( randomSeedParam->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( randomSeedParam.release() );

  std::unique_ptr< QgsProcessingParameterBoolean > includePolygonAttrParam = std::make_unique< QgsProcessingParameterBoolean >( INCLUDE_POLYGON_ATTRIBUTES, QObject::tr( "Include polygon attributes" ), true );
  includePolygonAttrParam->setFlags( includePolygonAttrParam->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( includePolygonAttrParam.release() );

  addParameter( new
                QgsProcessingParameterFeatureSink( OUTPUT, QObject::tr( "Random points in polygons" ), QgsProcessing::TypeVectorPoint ) );

  addOutput( new QgsProcessingOutputNumber( OUTPUT_POINTS, QObject::tr( "Total number of points generated" ) ) );
  addOutput( new QgsProcessingOutputNumber( POINTS_MISSED, QObject::tr( "Number of missed points" ) ) );
  addOutput( new QgsProcessingOutputNumber( POLYGONS_WITH_MISSED_POINTS, QObject::tr( "Number of polygons with missed points" ) ) );
  addOutput( new QgsProcessingOutputNumber( FEATURES_WITH_EMPTY_OR_NO_GEOMETRY, QObject::tr( "Number of features with empty or no geometry" ) ) );
}

QString QgsRandomPointsInPolygonsAlgorithm::shortHelpString() const
{
  return QObject::tr( "<p>This algorithm creates a point layer, with points placed randomly "
                      "in the polygons of the <i><b>Input polygon layer</b></i>.</p> "
                      "<ul><li>For each feature in the <i><b>Input polygon layer</b></i>, the algorithm attempts to add "
                      "the specified <i><b>Number of points for each feature</b></i> to the output layer.</li> "
                      "<li>A <i><b>Minimum distance between points</b></i> and a "
                      "<i><b>Global minimum distance between points</b></i> can be specified.<br> "
                      "A point will not be added if there is an already generated point within "
                      "this (Euclidean) distance from the generated location. "
                      "With <i>Minimum distance between points</i>, only points in the same "
                      "polygon feature are considered, while for <i>Global minimum distance "
                      "between points</i> all previously generated points are considered. "
                      "If the <i>Global minimum distance between points</i> is set equal to "
                      "or larger than the (local) <i>Minimum distance between points</i>, the "
                      "latter has no effect.<br> "
                      "If the <i>Minimum distance between points</i> is too large, "
                      "it may not be possible to generate the specified <i>Number of points "
                      "for each feature</i>, but all the generated points are returned.</li> "
                      "<li>The <i><b>Maximum number of attempts per point</b></i> can be specified.</li> "
                      "<li>The seed for the random generator can be provided (<b><i>Random seed</i></b> "
                      "- integer, greater than 0).</li> "
                      "<li>The user can choose not to <i><b>Include polygon feature attributes</b></i> in "
                      "the attributes of the generated point features.</li> "
                      "</ul> "
                      "The total number of points will be<br> <b>'number of input features'</b> * "
                      "<i><b>Number of points for each feature</b></i><br> if there are no misses. "
                      "The <i>Number of points for each feature</i>, <i>Minimum distance between points</i> "
                      "and <i>Maximum number of attempts per point</i> can be data defined. "
                      "<p>Output from the algorithm:</p> "
                      "<ul> "
                      "<li> The number of features with an empty or no geometry "
                      "(<code>FEATURES_WITH_EMPTY_OR_NO_GEOMETRY</code>).</li> "
                      "<li> A point layer containing the random points (<code>OUTPUT</code>).</li> "
                      "<li> The number of generated features (<code>OUTPUT_POINTS</code>).</li> "
                      "<li> The number of missed points (<code>POINTS_MISSED</code>).</li> "
                      "<li> The number of features with non-empty geometry and missing points "
                      "(<code>POLYGONS_WITH_MISSED_POINTS</code>).</li> "
                      "</ul>"
                    );
}


QgsRandomPointsInPolygonsAlgorithm *QgsRandomPointsInPolygonsAlgorithm::createInstance() const
{
  return new QgsRandomPointsInPolygonsAlgorithm();
}

bool QgsRandomPointsInPolygonsAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mNumPoints = parameterAsInt( parameters, POINTS_NUMBER, context );
  mDynamicNumPoints = QgsProcessingParameters::isDynamic( parameters, POINTS_NUMBER );
  if ( mDynamicNumPoints )
    mNumPointsProperty = parameters.value( POINTS_NUMBER ).value< QgsProperty >();

  mMinDistance = parameterAsDouble( parameters, MIN_DISTANCE, context );
  mDynamicMinDistance = QgsProcessingParameters::isDynamic( parameters, MIN_DISTANCE );
  if ( mDynamicMinDistance )
    mMinDistanceProperty = parameters.value( MIN_DISTANCE ).value< QgsProperty >();

  mMaxAttempts = parameterAsInt( parameters, MAX_TRIES_PER_POINT, context );
  mDynamicMaxAttempts = QgsProcessingParameters::isDynamic( parameters, MAX_TRIES_PER_POINT );
  if ( mDynamicMaxAttempts )
    mMaxAttemptsProperty = parameters.value( MAX_TRIES_PER_POINT ).value< QgsProperty >();

  mMinDistanceGlobal = parameterAsDouble( parameters, MIN_DISTANCE_GLOBAL, context );

  mUseRandomSeed = parameters.value( SEED ).isValid();
  mRandSeed = parameterAsInt( parameters, SEED, context );
  mIncludePolygonAttr = parameterAsBoolean( parameters, INCLUDE_POLYGON_ATTRIBUTES, context );
  return true;
}

QVariantMap QgsRandomPointsInPolygonsAlgorithm::processAlgorithm( const QVariantMap &parameters,
    QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsProcessingFeatureSource > polygonSource( parameterAsSource( parameters, INPUT, context ) );
  if ( !polygonSource )
    throw QgsProcessingException( invalidSourceError( parameters, INPUT ) );

  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "rand_point_id" ), QVariant::LongLong ) );
  if ( mIncludePolygonAttr )
    fields.extend( polygonSource->fields() );

  QString ldest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, OUTPUT,
                                          context, ldest, fields, QgsWkbTypes::Point, polygonSource->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, OUTPUT ) );

  QgsExpressionContext expressionContext = createExpressionContext( parameters, context, polygonSource.get() );

  // Initialize random engine -- note that we only use this if the user has specified a fixed seed
  std::random_device rd;
  std::mt19937 mt( !mUseRandomSeed ? rd() : mRandSeed );
  const std::uniform_real_distribution<> uniformDist( 0, 1 );
  std::uniform_int_distribution<> uniformIntDist( 1, 999999999 );

  // Index for finding global close points (mMinDistance != 0)
  QgsSpatialIndex globalIndex;
  int indexPoints = 0;

  int totNPoints = 0;
  int missedPoints = 0;
  int missedPolygons = 0;
  int emptyOrNullGeom = 0;

  long featureCount = 0;
  long long attempts = 0; // used for unique feature IDs in the indexes
  const long numberOfFeatures = polygonSource->featureCount();
  long long desiredNumberOfPoints = 0;
  const double featureProgressStep = 100.0 / ( numberOfFeatures > 0 ? numberOfFeatures : 1 );
  double baseFeatureProgress = 0.0;
  QgsFeature polyFeat;
  QgsFeatureIterator fitL = mIncludePolygonAttr || mDynamicNumPoints || mDynamicMinDistance || mDynamicMaxAttempts ? polygonSource->getFeatures()
                            : polygonSource->getFeatures( QgsFeatureRequest().setNoAttributes() );
  while ( fitL.nextFeature( polyFeat ) )
  {
    if ( feedback->isCanceled() )
    {
      feedback->setProgress( 0 );
      break;
    }
    if ( !polyFeat.hasGeometry() )
    {
      // Increment invalid features count
      emptyOrNullGeom++;
      featureCount++;
      baseFeatureProgress += featureProgressStep;
      feedback->setProgress( baseFeatureProgress );
      continue;
    }
    const QgsGeometry polyGeom( polyFeat.geometry() );
    if ( polyGeom.isEmpty() )
    {
      // Increment invalid features count
      emptyOrNullGeom++;
      featureCount++;
      baseFeatureProgress += featureProgressStep;
      feedback->setProgress( baseFeatureProgress );
      continue;
    }
    if ( mDynamicNumPoints || mDynamicMinDistance || mDynamicMaxAttempts )
    {
      expressionContext.setFeature( polyFeat );
    }
    // (Re)initialize the local (per polygon) index
    QgsSpatialIndex localIndex;
    int localIndexPoints = 0;
    int pointsAddedForThisFeature = 0;
    // Get data defined parameters
    int numberPointsForThisFeature = mNumPoints;
    if ( mDynamicNumPoints )
      numberPointsForThisFeature = mNumPointsProperty.valueAsInt( expressionContext, numberPointsForThisFeature );
    desiredNumberOfPoints += numberPointsForThisFeature;
    int maxAttemptsForThisFeature = mMaxAttempts;
    if ( mDynamicMaxAttempts )
      maxAttemptsForThisFeature = mMaxAttemptsProperty.valueAsInt( expressionContext, maxAttemptsForThisFeature );
    double minDistanceForThisFeature = mMinDistance;
    if ( mDynamicMinDistance )
      minDistanceForThisFeature = mMinDistanceProperty.valueAsDouble( expressionContext, minDistanceForThisFeature );
    const double pointProgressIncrement = featureProgressStep / ( numberPointsForThisFeature * maxAttemptsForThisFeature );
    double pointProgress = 0.0;
    // Check if we can avoid using the acceptPoint function
    if ( ( minDistanceForThisFeature == 0 ) && ( mMinDistanceGlobal == 0 ) )
    {
      QVector< QgsPointXY > newPoints = polyGeom.randomPointsInPolygon( numberPointsForThisFeature, mUseRandomSeed ? uniformIntDist( mt ) : 0 );
      for ( int i = 0; i < newPoints.length(); i++ )
      {
        // add the point
        const QgsPointXY pt = newPoints[i];
        QgsFeature f = QgsFeature( totNPoints );
        QgsAttributes pAttrs = QgsAttributes();
        pAttrs.append( totNPoints );
        if ( mIncludePolygonAttr )
        {
          pAttrs.append( polyFeat.attributes() );
        }
        f.setAttributes( pAttrs );
        const QgsGeometry newGeom = QgsGeometry::fromPointXY( pt );
        f.setGeometry( newGeom );
        if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
        totNPoints++;
        pointsAddedForThisFeature++;
        pointProgress += pointProgressIncrement * ( maxAttemptsForThisFeature );
      }
      feedback->setProgress( baseFeatureProgress + pointProgress );
      continue;
    }
    else
    {
      // Have to check for minimum distance, provide the acceptPoints function
      QVector< QgsPointXY > newPoints = polyGeom.randomPointsInPolygon( numberPointsForThisFeature, [ & ]( const QgsPointXY & newPoint ) -> bool
      {
        attempts++;
        // May have to check minimum distance to existing points
        // The first point can always be added
        // Local first (if larger than global)
        if ( minDistanceForThisFeature != 0 && mMinDistanceGlobal < minDistanceForThisFeature && localIndexPoints > 0 )
        {
          const QList<QgsFeatureId> neighbors = localIndex.nearestNeighbor( newPoint, 1, minDistanceForThisFeature );
          //if ( totNPoints > 0 && !neighbors.empty() )
          if ( !neighbors.empty() )
          {
            return false;
          }
        }
        // The global
        if ( mMinDistanceGlobal != 0.0 && indexPoints > 0 )
        {
          const QList<QgsFeatureId> neighbors = globalIndex.nearestNeighbor( newPoint, 1, mMinDistanceGlobal );
          //if ( totNPoints > 0 && !neighbors.empty() )
          if ( !neighbors.empty() )
          {
            return false;
          }
        }
        // Point is accepted - add it to the indexes
        QgsFeature f = QgsFeature( attempts );
        QgsAttributes pAttrs = QgsAttributes();
        pAttrs.append( attempts );
        f.setAttributes( pAttrs );
        const QgsGeometry newGeom = QgsGeometry::fromPointXY( newPoint );

        f.setGeometry( newGeom );
        //totNPoints++;

        if ( minDistanceForThisFeature != 0 )
        {
          if ( !localIndex.addFeature( f ) )
            throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QString() ) );
          localIndexPoints++;
        }
        if ( mMinDistanceGlobal != 0.0 )
        {
          if ( !globalIndex.addFeature( f ) )
            throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QString() ) );
          indexPoints++;
        }
        return true;
      },  mUseRandomSeed ? uniformIntDist( mt ) : 0, feedback, maxAttemptsForThisFeature );

      // create and output features for the generated points
      for ( int i = 0; i < newPoints.length(); i++ )
      {
        const QgsPointXY pt = newPoints[i];
        QgsFeature f = QgsFeature( totNPoints );
        QgsAttributes pAttrs = QgsAttributes();
        pAttrs.append( totNPoints );
        if ( mIncludePolygonAttr )
        {
          pAttrs.append( polyFeat.attributes() );
        }
        f.setAttributes( pAttrs );
        const QgsGeometry newGeom = QgsGeometry::fromPointXY( pt );
        f.setGeometry( newGeom );
        if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
        totNPoints++;
        pointsAddedForThisFeature++;
        pointProgress += pointProgressIncrement * ( maxAttemptsForThisFeature );
      }
      feedback->setProgress( baseFeatureProgress + pointProgress );
    }

    baseFeatureProgress += featureProgressStep;
    if ( pointsAddedForThisFeature < numberPointsForThisFeature )
    {
      missedPolygons++;
    }
    featureCount++;
    feedback->setProgress( baseFeatureProgress );
  } // while features
  missedPoints = desiredNumberOfPoints - totNPoints;
  feedback->pushInfo( QObject::tr( "Total number of points generated: "
                                   "%1\nNumber of missed points: "
                                   "%2\nPolygons with missing points: "
                                   "%3\nFeatures with empty or missing "
                                   "geometries: %4"
                                 ).arg( totNPoints ).arg( missedPoints ).arg( missedPolygons ).arg( emptyOrNullGeom ) );
  QVariantMap outputs;
  outputs.insert( OUTPUT, ldest );
  outputs.insert( OUTPUT_POINTS, totNPoints );
  outputs.insert( POINTS_MISSED, missedPoints );
  outputs.insert( POLYGONS_WITH_MISSED_POINTS, missedPolygons );
  outputs.insert( FEATURES_WITH_EMPTY_OR_NO_GEOMETRY, emptyOrNullGeom );

  return outputs;
}

///@endcond
