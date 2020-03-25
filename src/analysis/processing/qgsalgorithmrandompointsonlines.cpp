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
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input line layer" ), QList< int >() << QgsProcessing::TypeVectorLine ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "POINTS_NUMBER" ), QObject::tr( "Number of points for each feature" ), QgsProcessingParameterNumber::Integer, 1, false, 1 ) );
  addParameter( new QgsProcessingParameterDistance( QStringLiteral( "MIN_DISTANCE" ), QObject::tr( "Minimum distance between points" ), 0, QStringLiteral( "INPUT" ), true, 0 ) );
  
  std::unique_ptr< QgsProcessingParameterNumber > maxAttempts_param = qgis::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "MAX_TRIES_PER_POINT" ), QObject::tr( "Maximum number of search attempts (for Min. dist. > 0)" ), QgsProcessingParameterNumber::Integer, 10, true, 1, 1000 );
  maxAttempts_param->setFlags( maxAttempts_param->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( maxAttempts_param.release() );

  std::unique_ptr< QgsProcessingParameterNumber > randomSeedParam = qgis::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "SEED" ), QObject::tr( "Random seed" ), QgsProcessingParameterNumber::Integer, 1, false, 1 );
  randomSeedParam->setFlags( randomSeedParam->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( randomSeedParam.release() );

  std::unique_ptr< QgsProcessingParameterBoolean > includeLineAttrParam = qgis::make_unique< QgsProcessingParameterBoolean >( QStringLiteral( "INCLUDE_LINE_ATTRIBUTES" ), QObject::tr( "Include line attributes" ), true );
  includeLineAttrParam->setFlags( includeLineAttrParam->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( includeLineAttrParam.release() );

  addParameter( new
                QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Random points on lines" ), QgsProcessing::TypeVectorPoint ) );

  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "OUTPUT_POINTS" ), QObject::tr( "Total number of points generated" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "POINTS_MISSED" ), QObject::tr( "Number of missed points" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "LINES_WITH_MISSED_POINTS" ), QObject::tr( "Number of lines with missed points" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "FEATURES_WITH_EMPTY_OR_NO_GEOMETRY" ), QObject::tr( "Number of features with empty or no geometry" ) ) );
}

QString QgsRandomPointsOnLinesAlgorithm::shortHelpString() const
{
  return QObject::tr( "<p>This algorithm creates a point layer, with points placed randomly "
                      "on the lines of the <i><b>Input line layer</b></i>.</p> "
                      "<ul><li>For each feature in the <i><b>Input line layer</b></i>, the algorithm attempts to add "
                      "the specified <i><b>Number of points for each feature</b></i> to the output layer.</li> "
                      "<li>A <i><b>Minimum distance between points</b></i> can be specified.<br> "
                      "A point will not be generated if there is an already generated point "
                      "(on any line feature) within this (Euclidean) distance from "
                      "the generated location. "
                      "If the <i><b>Minimum distance between points</b></i> is too large, it may not be possible to generate "
                      "the specified <i><b>Number of points for each feature</b></i>.</li> "
                      "<li>The <i><b>Maximum number of attempts per point</b></i> "
                      "is only relevant if the <i><b>Minimum distance between points</b></i> is greater than 0. "
                      "The total number of points will be<br> <b>'number of input features'</b> * "
                      "<i><b>Number of points for each feature</b></i><br> if there are no misses.</li> "
                      "<li>The seed for the random generator can be provided (<i>Random seed</i> "
                      "- integer, greater than 0).</li> "
                      "<li>The user can choose not to <i><b>Include line feature attributes</b></i> in "
                      "the attributes of the generated point features.</li> "
                      "</ul> "
                      "<p>Output from the algorithm:</p> "
                      "<ul> "
                      "<li> A point layer containing the random points (<code>OUTPUT</code>).</li> "
                      "<li> The number of generated features (<code>POINTS_GENERATED</code>).</li> "
                      "<li> The number of missed points (<code>POINTS_MISSED</code>).</li> "
                      "<li> The number of features with missing points (<code>LINES_WITH_MISSED_POINTS</code>).</li> "
                      "<li> The number of features with an empty or no geometry (<code>LINES_WITH_EMPTY_OR_NO_GEOMETRY</code>).</li> "
                      "</ul>"
                    );
}


QgsRandomPointsOnLinesAlgorithm *QgsRandomPointsOnLinesAlgorithm::createInstance() const
{
  return new QgsRandomPointsOnLinesAlgorithm();
}

bool QgsRandomPointsOnLinesAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mNumPoints = parameterAsInt( parameters, QStringLiteral( "POINTS_NUMBER" ), context );
  mMinDistance = parameterAsDouble( parameters, QStringLiteral( "MIN_DISTANCE" ), context );
  mMaxAttempts = parameterAsInt( parameters, QStringLiteral( "MAX_TRIES_PER_POINT" ), context );
  mRandSeed = parameterAsInt( parameters, QStringLiteral( "SEED" ), context );
  mIncludeLineAttr = parameterAsBoolean( parameters, QStringLiteral( "INCLUDE_LINE_ATTRIBUTES" ), context );
  return true;
}

QVariantMap QgsRandomPointsOnLinesAlgorithm::processAlgorithm( const QVariantMap &parameters,
    QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsFeatureSource > lineSource( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !lineSource )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "rand_point_id" ), QVariant::LongLong ) );
  if ( mIncludeLineAttr )
    fields.extend( lineSource->fields() );

  QString ldest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ),
      context, ldest, fields, QgsWkbTypes::Point, lineSource->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  //initialize random engine
  srand( mRandSeed );

  //index for finding close points (mMinDistance > 0)
  QgsSpatialIndex index;

  int totNPoints = 0;
  int missedPoints = 0;
  int missedLines = 0;
  int emptyOrNullGeom = 0;
  long tries = 0;
  long saved = 0;

  long featureCount = 0;
  long numberOfFeatures = lineSource->featureCount();
  QgsFeature lFeat;
  QgsFeatureIterator fitL = mIncludeLineAttr ? lineSource->getFeatures()
      : lineSource->getFeatures( QgsFeatureRequest().setNoAttributes());
  while ( fitL.nextFeature( lFeat ) )
  {
    featureCount++;
    if ( feedback->isCanceled() )
    {
      feedback->pushInfo( "canceled" );
      break;
    }
    if ( !lFeat.hasGeometry() )
    {
      feedback->pushInfo( "No geometry" );
      // Increment invalid features count
      emptyOrNullGeom++;
      continue;
    }
    QgsGeometry lGeom( lFeat.geometry() );
    if ( lGeom.isEmpty() )
    {
      feedback->pushInfo( "Empty geometry" );
      // Increment invalid features count
      emptyOrNullGeom++;
      continue;
    }
    float lineLength = lGeom.length();
    int pointsAddedForThisFeature = 0;
    for ( long i = 0; i < mNumPoints; i++)
    {
      if ( feedback->isCanceled() )
      {
        break;
      }
      // Try to add a point (mMaxAttempts attempts)
      int distCheckIterations = 0;
      while ( distCheckIterations < mMaxAttempts )
      {
        if ( feedback->isCanceled() )
        {
          break;
        }
        distCheckIterations++;
        tries++;
        // Generate a random point
        float randPos = lineLength * ( float ) rand() / RAND_MAX;
        QgsGeometry rpGeom = QgsGeometry( lGeom.interpolate( randPos ) );

        if ( !rpGeom.isNull() && !rpGeom.isEmpty() )
        {
          if ( mMinDistance != 0 && pointsAddedForThisFeature > 0 )
          {
            // Have to check minimum distance to existing points
            QList<QgsFeatureId> neighbors = index.nearestNeighbor( rpGeom, 1, mMinDistance );
            if ( !neighbors.empty() )
            {
              // Too close!
              continue;
            }
          }
          // point OK to add
          QgsFeature f = QgsFeature( totNPoints );
          QgsAttributes pAttrs = QgsAttributes();
          pAttrs.append( totNPoints );
          if ( mIncludeLineAttr )
          {
            pAttrs.append( lFeat.attributes() );
          }
          f.setAttributes( pAttrs );
          f.setGeometry( rpGeom );

          if ( mMinDistance != 0 )
          {
            index.addFeature( f );
          }
          sink->addFeature( f, QgsFeatureSink::FastInsert );
          totNPoints++;
          pointsAddedForThisFeature++;
          saved += (mMaxAttempts - distCheckIterations);

          feedback->setProgress( static_cast<int>( 100 * static_cast<double>( tries + saved ) / static_cast<double>( mNumPoints * mMaxAttempts * (numberOfFeatures - emptyOrNullGeom) ) ) );
          break;
        }
        else
        {
          feedback->setProgress( static_cast<int>( 100 * static_cast<double>( tries + saved ) / static_cast<double>( mNumPoints * mMaxAttempts * (numberOfFeatures - emptyOrNullGeom) ) ) );
        }
        
      }
    }
    if ( pointsAddedForThisFeature < mNumPoints )
    {
      missedLines++;
    }
  }
  missedPoints = mNumPoints * featureCount - totNPoints;
  feedback->pushInfo( QObject::tr( "Total number of points generated: "
                                   " %1\nNumber of missed points: %2\nLines with missing points: "
                                   " %3\nFeatures with empty or missing geometries: %4"
                                 ).arg( totNPoints ).arg( missedPoints ).arg( missedLines ).arg( emptyOrNullGeom ) );
  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), ldest );
  outputs.insert( QStringLiteral( "OUTPUT_POINTS" ), totNPoints );
  outputs.insert( QStringLiteral( "POINTS_MISSED" ), missedPoints );
  outputs.insert( QStringLiteral( "LINES_WITH_MISSED_POINTS" ), missedLines );
  outputs.insert( QStringLiteral( "FEATURES_WITH_EMPTY_OR_NO_GEOMETRY" ), emptyOrNullGeom );

  return outputs;
}

///@endcond
