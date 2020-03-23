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
  return QObject::tr( "random,points,lines,seed,attributes,create" ).split( ',' );
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
  addParameter( new QgsProcessingParameterDistance( QStringLiteral( "MIN_DISTANCE" ), QObject::tr( "Minimum distance between points" ), 0, QStringLiteral( "TARGET_CRS" ), true, 0 ) );
  addParameter( new QgsProcessingParameterCrs( QStringLiteral( "TARGET_CRS" ), QObject::tr( "Target CRS" ), QStringLiteral( "ProjectCrs" ), false ) );

  //addParameter( new QgsProcessingParameterNumber( QStringLiteral( "MAX_TRIES_PER_POINT" ), QObject::tr( "Maximum number of search attempts (relevant when Minimum distance between points > 0)" ), QgsProcessingParameterNumber::Integer, 10, false, 1 ) );
  std::unique_ptr< QgsProcessingParameterNumber > maxAttempts_param = qgis::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "MAX_TRIES_PER_POINT" ), QObject::tr( "Maximum number of search attempts (relevant when Minimum distance between points > 0)" ), QgsProcessingParameterNumber::Integer, 10, true, 1, 1000 );
  maxAttempts_param->setFlags( maxAttempts_param->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( maxAttempts_param.release() );

  //addParameter( new QgsProcessingParameterNumber( QStringLiteral( "SEED" ), QObject::tr( "Random seed" ), QgsProcessingParameterNumber::Integer, 1, false, 1 ) );
  std::unique_ptr< QgsProcessingParameterNumber > randomSeed_param = qgis::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "SEED" ), QObject::tr( "Random seed" ), QgsProcessingParameterNumber::Integer, 1, false, 1 );
  randomSeed_param->setFlags( randomSeed_param->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( randomSeed_param.release() );

  //addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "INCLUDE_LINE_ATTRIBUTES" ), QObject::tr( "Include line feature attributes" ), true ) );
  std::unique_ptr< QgsProcessingParameterBoolean > includeLineAttr_param = qgis::make_unique< QgsProcessingParameterBoolean >( QStringLiteral( "INCLUDE_LINE_ATTRIBUTES" ), QObject::tr( "Include line attributes" ), true );
  includeLineAttr_param->setFlags( includeLineAttr_param->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( includeLineAttr_param.release() );

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
                      "on the lines of the <i>Input line layer</i>.</p> "
                      "<ul><li>For each feature in the <i>Input line layer</i>, the algorithm attempts to add "
                      "the specified <i>Number of points for each feature</i> to the output layer.</li> "
                      "<li>A <i>Minimum distance between points</i> can be specified.<br> "
                      "A point will not be generated if there is an already generated point " 
                      "(on any line feature) within this (Euclidean) distance from "
                      "the generated location. "
                      "If the <i>Minimum distance between points</i> is too large, it may not be possible to generate "
                      "the specified <i>Number of points for each feature</i>.</li> "
                      "<li>The <i>Maximum number of attempts per point</i> "
                      "is only relevant if the <i>Minimum distance between points</i> is greater than 0. "
                      "The total number of points will be<br> 'number of input features' * "
                      "<i>Number of points for each feature</i><br> if there are no misses.</li> "
                      "<li>The seed for the random generator can be provided (<i>Random seed</i> "
                      "- integer, greater than 0).</li> "
                      "<li>The user can choose to <i>Include line feature attributes</i> in "
                      "the attributes of the generated point features.</li> "
                      "</ul> "
                      "<p>Output from the algorithm:</p> "
                      "<ul> "
                      "<li> A point layer containing the random points (OUTPUT).</li> "
                      "<li> The number of generated features (POINTS_GENERATED).</li> "
                      "<li> The number of missed points (POINTS_MISSED).</li> "
                      "<li> The number of features with missing points (LINES_WITH_MISSED_POINTS).</li> "
                      "<li> The number of features with an empty or no geometry (LINES_WITH_EMPTY_OR_NO_GEOMETRY).</li> "
                      "</ul>"
                      );
}


QgsRandomPointsOnLinesAlgorithm *QgsRandomPointsOnLinesAlgorithm::createInstance() const
{
  return new QgsRandomPointsOnLinesAlgorithm();
}

bool QgsRandomPointsOnLinesAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mCrs = parameterAsCrs( parameters, QStringLiteral( "TARGET_CRS" ), context );
  mNumPoints = parameterAsInt( parameters, QStringLiteral( "POINTS_NUMBER" ), context );
  mMinDistance = parameterAsDouble( parameters, QStringLiteral( "MIN_DISTANCE" ), context );
  mMaxAttempts = parameterAsInt( parameters, QStringLiteral( "MAX_TRIES_PER_POINT" ), context );
  mRandSeed = parameterAsInt( parameters, QStringLiteral( "SEED" ), context );
  mIncludeLineAttr = parameterAsBoolean( parameters, QStringLiteral( "INCLUDE_LINE_ATTRIBUTES" ), context );

  return true;
}

QVariantMap QgsRandomPointsOnLinesAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{

  std::unique_ptr< QgsFeatureSource > lineSource( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !lineSource )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  QgsFields fields = QgsFields();
  fields.append( QgsField( QStringLiteral( "rand_point_id" ), QVariant::LongLong ) );
  if ( mIncludeLineAttr )
    fields.extend(lineSource->fields());

  QString ldest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, ldest, fields, QgsWkbTypes::Point, mCrs ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  //initialize random engine
  srand(mRandSeed);

  //index for finding nearest neighbours (mMinDistance > 0)
  QgsSpatialIndex index = QgsSpatialIndex();

  int totNPoints = 0;
  int missedPoints = 0;
  int missedLines = 0;
  int emptyOrNullGeom = 0;

  int featureCount = 0;
  QgsFeature lFeat;
  QgsFeatureIterator fitL = lineSource->getFeatures();
  while ( fitL.nextFeature( lFeat ) )
  {
    featureCount++;
    if ( feedback->isCanceled() )
    {
      feedback->pushInfo("cancelled");
      break;
    }
    if ( not lFeat.hasGeometry() )
    {
      // Increment invalid features count
      feedback->pushInfo("No geometry");
      emptyOrNullGeom++;
      continue;
    }
    QgsGeometry lGeom( lFeat.geometry() );
    if (lGeom.isNull())
    {
      // Increment invalid features count
      feedback->pushInfo("Null geometry");
      emptyOrNullGeom++;
      continue;
    }
    if (lGeom.isEmpty())
    {
      // Increment invalid features count
      feedback->pushInfo("Empty geometry");
      emptyOrNullGeom++;
      continue;
    }
    float lineLength = lGeom.length();
    int distCheckIterations = 0;
    int i = 0;
    while ( i < mNumPoints and distCheckIterations < mMaxAttempts )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }

      float randPos = lineLength * (float) rand() / RAND_MAX;
      QgsGeometry rpGeom = QgsGeometry( lGeom.interpolate( randPos ) );
      //if ( not rpGeom.isNull() and not rpGeom.isEmpty() )
      if ( rpGeom.isNull() or rpGeom.isEmpty() )
      {
        distCheckIterations++;
        continue;
      }

      QgsPointXY rPoint;
      rPoint = QgsPoint( *qgsgeometry_cast< const QgsPoint * >( rpGeom.constGet() ) );

      if ( not mMinDistance == 0 )
      {
        if ( i > 0 )
        {
          QList<QgsFeatureId> neighbors = index.nearestNeighbor( rPoint, 1, mMinDistance );
          if (not neighbors.empty() )
          {
            distCheckIterations++;
            continue;
          }
        }
      }
      QgsFeature f = QgsFeature();
      QgsAttributes pAttrs = QgsAttributes();
      pAttrs.append( totNPoints );
      if ( mIncludeLineAttr ) {
          pAttrs.append( lFeat.attributes() );
      }
      f.setAttributes( pAttrs );
      f.setGeometry( rpGeom );
      if ( not mMinDistance == 0 )
      {
        index.addFeature( f );
      }
      sink->addFeature( f, QgsFeatureSink::FastInsert );
      totNPoints++;
      i++;
      distCheckIterations = 0; //reset distCheckIterations if a point is added
      feedback->setProgress( static_cast<int>( static_cast<double>( totNPoints ) / static_cast<double>( mNumPoints ) * 100 ) );
    }
    if (i < mNumPoints)
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
