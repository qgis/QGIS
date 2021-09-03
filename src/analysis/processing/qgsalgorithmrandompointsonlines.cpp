/***************************************************************************
                         qgsalgorithmrandompointsonlines.cpp
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


#include "qgsalgorithmrandompointsonlines.h"
#include "random"

// The algorithm parameter names:
static const QString INPUT = QStringLiteral( "INPUT" );
static const QString POINTS_NUMBER = QStringLiteral( "POINTS_NUMBER" );
static const QString MIN_DISTANCE_GLOBAL = QStringLiteral( "MIN_DISTANCE_GLOBAL" );
static const QString MIN_DISTANCE = QStringLiteral( "MIN_DISTANCE" );
static const QString MAX_TRIES_PER_POINT = QStringLiteral( "MAX_TRIES_PER_POINT" );
static const QString SEED = QStringLiteral( "SEED" );
static const QString INCLUDE_LINE_ATTRIBUTES = QStringLiteral( "INCLUDE_LINE_ATTRIBUTES" );
static const QString OUTPUT = QStringLiteral( "OUTPUT" );
static const QString OUTPUT_POINTS = QStringLiteral( "OUTPUT_POINTS" );
static const QString POINTS_MISSED = QStringLiteral( "POINTS_MISSED" );
static const QString LINES_WITH_MISSED_POINTS = QStringLiteral( "LINES_WITH_MISSED_POINTS" );
static const QString FEATURES_WITH_EMPTY_OR_NO_GEOMETRY = QStringLiteral( "FEATURES_WITH_EMPTY_OR_NO_GEOMETRY" );

///@cond PRIVATE

QString QgsRandomPointsOnLinesAlgorithm::name() const
{
  return QStringLiteral( "randompointsonlines" );
}

QString QgsRandomPointsOnLinesAlgorithm::displayName() const
{
  return QObject::tr( "Random points on lines" );
}

QStringList QgsRandomPointsOnLinesAlgorithm::tags() const
{
  return QObject::tr( "seed,attributes,create" ).split( ',' );
}

QString QgsRandomPointsOnLinesAlgorithm::group() const
{
  return QObject::tr( "Vector creation" );
}

QString QgsRandomPointsOnLinesAlgorithm::groupId() const
{
  return QStringLiteral( "vectorcreation" );
}

void QgsRandomPointsOnLinesAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( INPUT, QObject::tr( "Input line layer" ), QList< int >() << QgsProcessing::TypeVectorLine ) );
  std::unique_ptr< QgsProcessingParameterNumber > numberPointsParam = std::make_unique< QgsProcessingParameterNumber >( POINTS_NUMBER, QObject::tr( "Number of points for each feature" ), QgsProcessingParameterNumber::Integer, 1, false, 1 );
  numberPointsParam->setIsDynamic( true );
  numberPointsParam->setDynamicPropertyDefinition( QgsPropertyDefinition( POINTS_NUMBER, QObject::tr( "Number of points for each feature" ), QgsPropertyDefinition::IntegerPositive ) );
  numberPointsParam->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( numberPointsParam.release() );

  std::unique_ptr< QgsProcessingParameterDistance > minDistParam = std::make_unique< QgsProcessingParameterDistance >( MIN_DISTANCE, QObject::tr( "Minimum distance between points" ), 0, INPUT, true, 0 );
  minDistParam->setIsDynamic( true );
  minDistParam->setDynamicPropertyDefinition( QgsPropertyDefinition( MIN_DISTANCE, QObject::tr( "Minimum distance between points (per feature)" ), QgsPropertyDefinition::DoublePositive ) );
  minDistParam->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( minDistParam.release() );

  std::unique_ptr< QgsProcessingParameterDistance > minDistGlobalParam = std::make_unique< QgsProcessingParameterDistance >( MIN_DISTANCE_GLOBAL, QObject::tr( "Global minimum distance between points" ), 0, INPUT, true, 0 );
  minDistGlobalParam->setFlags( minDistGlobalParam->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( minDistGlobalParam.release() );

  std::unique_ptr< QgsProcessingParameterNumber > maxAttemptsParam = std::make_unique< QgsProcessingParameterNumber >( MAX_TRIES_PER_POINT, QObject::tr( "Maximum number of search attempts (for Min. dist. > 0)" ), QgsProcessingParameterNumber::Integer, 10, true, 1 );
  maxAttemptsParam->setFlags( maxAttemptsParam->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  maxAttemptsParam->setIsDynamic( true );
  maxAttemptsParam->setDynamicPropertyDefinition( QgsPropertyDefinition( MAX_TRIES_PER_POINT, QObject::tr( "Maximum number of search attempts (for Min. dist. > 0)" ), QgsPropertyDefinition::IntegerPositiveGreaterZero ) );
  maxAttemptsParam->setDynamicLayerParameterName( QStringLiteral( "INPUT" ) );
  addParameter( maxAttemptsParam.release() );

  std::unique_ptr< QgsProcessingParameterNumber > randomSeedParam = std::make_unique< QgsProcessingParameterNumber >( SEED, QObject::tr( "Random seed" ), QgsProcessingParameterNumber::Integer, QVariant(), true, 1 );
  randomSeedParam->setFlags( randomSeedParam->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( randomSeedParam.release() );

  std::unique_ptr< QgsProcessingParameterBoolean > includeLineAttrParam = std::make_unique< QgsProcessingParameterBoolean >( INCLUDE_LINE_ATTRIBUTES, QObject::tr( "Include line attributes" ), true );
  includeLineAttrParam->setFlags( includeLineAttrParam->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( includeLineAttrParam.release() );

  addParameter( new
                QgsProcessingParameterFeatureSink( OUTPUT, QObject::tr( "Random points on lines" ), QgsProcessing::TypeVectorPoint ) );

  addOutput( new QgsProcessingOutputNumber( OUTPUT_POINTS, QObject::tr( "Total number of points generated" ) ) );
  addOutput( new QgsProcessingOutputNumber( POINTS_MISSED, QObject::tr( "Number of missed points" ) ) );
  addOutput( new QgsProcessingOutputNumber( LINES_WITH_MISSED_POINTS, QObject::tr( "Number of features with missed points" ) ) );
  addOutput( new QgsProcessingOutputNumber( FEATURES_WITH_EMPTY_OR_NO_GEOMETRY, QObject::tr( "Number of features with empty or no geometry" ) ) );
}

QString QgsRandomPointsOnLinesAlgorithm::shortHelpString() const
{
  return QObject::tr( "<p>This algorithm creates a point layer, with points placed randomly "
                      "on the lines of the <i>Input line layer</i>. "
                      "The default behavior is that the generated point features inherit "
                      "the attributes of the line feature on which they were was generated.</p>"
                      "<p>Parameters / options:</p> "
                      "<ul> "
                      "<li>For each feature in the <i><b>Input line layer</b></i>, the "
                      "algorithm attempts to add the specified <i><b>Number of points for "
                      "each feature</b></i> to the output layer.</li> "
                      "<li>A <i><b>Minimum distance between points</b></i> and a "
                      "<i><b>Global minimum distance between points</b></i> can be specified. "
                      "A point will not be added if there is an already generated point within "
                      "this (Euclidean) distance from the generated location. "
                      "With <i>Minimum distance between points</i>, only points on the same "
                      "line feature are considered, while for <i>Global minimum distance "
                      "between points</i> all previously generated points are considered. "
                      "If the <i>Global minimum distance between points</i> is set larger "
                      "than the (local) <i>Minimum distance between points</i>, the latter "
                      "has no effect.<br> "
                      "If the <i>Minimum distance between points</i> is too large, "
                      "it may not be possible to generate the specified <i>Number of points "
                      "for each feature</i>.</li> "
                      "<li>The <i><b>Maximum number of attempts per point</b></i> "
                      "is only relevant if <i>Minimum distance between points</i> or <i>Global "
                      "minimum distance between points</i> is greater than 0. "
                      "The total number of points will be<br> <b>number of input features</b> * "
                      "<b>Number of points for each feature</i><br> if there are no "
                      "misses and all features have proper geometries.</li> "
                      "<li>The seed for the random generator can be provided (<i>Random seed</i> "
                      "- integer, greater than 0).</li> "
                      "<li>The user can choose not to <i><b>Include line feature attributes</b></i> "
                      "in the generated point features.</li> "
                      "</ul> "
                      "<p>Output from the algorithm:</p> "
                      "<ul> "
                      "<li> A point layer containing the random points (<code>OUTPUT</code>).</li> "
                      "<li> The number of generated features (<code>POINTS_GENERATED</code>).</li> "
                      "<li> The number of missed points (<code>POINTS_MISSED</code>).</li> "
                      "<li> The number of features with non-empty geometry and missing points "
                      "(<code>LINES_WITH_MISSED_POINTS</code>).</li> "
                      "<li> The number of features with an empty or no geometry "
                      "(<code>LINES_WITH_EMPTY_OR_NO_GEOMETRY</code>).</li> "
                      "</ul>"
                    );
}


QgsRandomPointsOnLinesAlgorithm *QgsRandomPointsOnLinesAlgorithm::createInstance() const
{
  return new QgsRandomPointsOnLinesAlgorithm();
}

bool QgsRandomPointsOnLinesAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
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
  mIncludeLineAttr = parameterAsBoolean( parameters, INCLUDE_LINE_ATTRIBUTES, context );
  return true;
}

QVariantMap QgsRandomPointsOnLinesAlgorithm::processAlgorithm( const QVariantMap &parameters,
    QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsProcessingFeatureSource > lineSource( parameterAsSource( parameters, INPUT, context ) );
  if ( !lineSource )
    throw QgsProcessingException( invalidSourceError( parameters, INPUT ) );

  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "rand_point_id" ), QVariant::LongLong ) );
  if ( mIncludeLineAttr )
    fields.extend( lineSource->fields() );

  QString ldest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, OUTPUT,
                                          context, ldest, fields, QgsWkbTypes::Point, lineSource->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, OUTPUT ) );

  QgsExpressionContext expressionContext = createExpressionContext( parameters, context, lineSource.get() );

  // Initialize random engine
  std::random_device rd;
  std::mt19937 mt( !mUseRandomSeed ? rd() : mRandSeed );
  std::uniform_real_distribution<> uniformDist( 0, 1 );

  // Index for finding global close points (mMinDistance > 0)
  QgsSpatialIndex index;

  int totNPoints = 0;
  int missedPoints = 0;
  int missedLines = 0;
  int emptyOrNullGeom = 0;

  long featureCount = 0;
  const long numberOfFeatures = lineSource->featureCount();
  long long desiredNumberOfPoints = 0;
  const double featureProgressStep = 100.0 / ( numberOfFeatures > 0 ? numberOfFeatures : 1 );
  double baseFeatureProgress = 0.0;
  QgsFeature lFeat;
  QgsFeatureIterator fitL = mIncludeLineAttr || mDynamicNumPoints || mDynamicMinDistance || mDynamicMaxAttempts ? lineSource->getFeatures()
                            : lineSource->getFeatures( QgsFeatureRequest().setNoAttributes() );
  while ( fitL.nextFeature( lFeat ) )
  {
    if ( feedback->isCanceled() )
    {
      feedback->setProgress( 0 );
      break;
    }
    if ( !lFeat.hasGeometry() )
    {
      // Increment invalid features count
      emptyOrNullGeom++;
      featureCount++;
      baseFeatureProgress += featureProgressStep;
      feedback->setProgress( baseFeatureProgress );
      continue;
    }
    const QgsGeometry lGeom( lFeat.geometry() );
    if ( lGeom.isEmpty() )
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
      expressionContext.setFeature( lFeat );
    }

    const double lineLength = lGeom.length();
    int pointsAddedForThisFeature = 0;

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
    QgsSpatialIndex localIndex;

    for ( long pointIndex = 0; pointIndex < numberPointsForThisFeature; pointIndex++ )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }
      // Try to add a point (mMaxAttempts attempts)
      int distCheckIterations = 0;
      while ( distCheckIterations < maxAttemptsForThisFeature )
      {
        if ( feedback->isCanceled() )
        {
          break;
        }
        // Generate a random point
        const double randPos = lineLength * uniformDist( mt );
        const QgsGeometry rpGeom = QgsGeometry( lGeom.interpolate( randPos ) );
        distCheckIterations++;
        pointProgress += pointProgressIncrement;

        if ( !rpGeom.isNull() && !rpGeom.isEmpty() )
        {
          if ( ( minDistanceForThisFeature != 0 ) || ( mMinDistanceGlobal != 0 ) )
          {
            // Check minimum distance to existing points
            // Per feature first
            if ( ( minDistanceForThisFeature != 0 ) && ( pointsAddedForThisFeature > 0 ) )
            {
              const QList<QgsFeatureId> neighbors = localIndex.nearestNeighbor( rpGeom, 1, minDistanceForThisFeature );
              if ( !neighbors.empty() )
              {
                feedback->setProgress( baseFeatureProgress + pointProgress );
                continue;
              }
            }
            // Then check globally
            if ( ( mMinDistanceGlobal != 0 ) && ( totNPoints > 0 ) )
            {
              const QList<QgsFeatureId> neighbors = index.nearestNeighbor( rpGeom, 1, mMinDistanceGlobal );
              if ( !neighbors.empty() )
              {
                feedback->setProgress( baseFeatureProgress + pointProgress );
                continue;
              }
            }
          }
          // OK to add point
          QgsFeature f = QgsFeature( totNPoints );
          QgsAttributes pAttrs = QgsAttributes();
          pAttrs.append( totNPoints );
          if ( mIncludeLineAttr )
          {
            pAttrs.append( lFeat.attributes() );
          }
          f.setAttributes( pAttrs );
          f.setGeometry( rpGeom );

          if ( mMinDistanceGlobal != 0 )
          {
            if ( !index.addFeature( f ) )
              throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QString() ) );
          }
          if ( minDistanceForThisFeature != 0 )
          {
            if ( !localIndex.addFeature( f ) )
              throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QString() ) );
          }
          if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
            throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
          totNPoints++;
          pointsAddedForThisFeature++;
          pointProgress += pointProgressIncrement * ( maxAttemptsForThisFeature - distCheckIterations );
          break;
        }
        else
        {
          feedback->setProgress( baseFeatureProgress + pointProgress );
        }
      } // while not maxattempts
      feedback->setProgress( baseFeatureProgress + pointProgress );
    } // for points
    baseFeatureProgress += featureProgressStep;
    if ( pointsAddedForThisFeature < numberPointsForThisFeature )
    {
      missedLines++;
    }
    featureCount++;
    feedback->setProgress( baseFeatureProgress );
  } // while features
  missedPoints = desiredNumberOfPoints - totNPoints;
  feedback->pushInfo( QObject::tr( "Total number of points generated: "
                                   " %1\nNumber of missed points: %2\nFeatures with missing points: "
                                   " %3\nFeatures with empty or missing geometries: %4"
                                 ).arg( totNPoints ).arg( missedPoints ).arg( missedLines ).arg( emptyOrNullGeom ) );
  QVariantMap outputs;
  outputs.insert( OUTPUT, ldest );
  outputs.insert( OUTPUT_POINTS, totNPoints );
  outputs.insert( POINTS_MISSED, missedPoints );
  outputs.insert( LINES_WITH_MISSED_POINTS, missedLines );
  outputs.insert( FEATURES_WITH_EMPTY_OR_NO_GEOMETRY, emptyOrNullGeom );

  return outputs;
}

///@endcond
