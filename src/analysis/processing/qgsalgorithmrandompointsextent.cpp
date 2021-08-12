/***************************************************************************
                         qgsalgorithmrandompointsextent.cpp
                         ---------------------
    begin                : November 2019
    copyright            : (C) 2019 by Clemens Raffler
    email                : clemens dot raffler at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

//Disclaimer: The algorithm optimizes the original Random points in extent algorithm, (C) Alexander Bruy, 2014

#include "qgsalgorithmrandompointsextent.h"
#include "random"

///@cond PRIVATE

QString QgsRandomPointsExtentAlgorithm::name() const
{
  return QStringLiteral( "randompointsinextent" );
}

QString QgsRandomPointsExtentAlgorithm::displayName() const
{
  return QObject::tr( "Random points in extent" );
}

QStringList QgsRandomPointsExtentAlgorithm::tags() const
{
  return QObject::tr( "random,points,extent,create" ).split( ',' );
}

QString QgsRandomPointsExtentAlgorithm::group() const
{
  return QObject::tr( "Vector creation" );
}

QString QgsRandomPointsExtentAlgorithm::groupId() const
{
  return QStringLiteral( "vectorcreation" );
}

void QgsRandomPointsExtentAlgorithm::initAlgorithm( const QVariantMap & )
{

  addParameter( new QgsProcessingParameterExtent( QStringLiteral( "EXTENT" ), QObject::tr( "Input extent" ) ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "POINTS_NUMBER" ), QObject::tr( "Number of points" ), QgsProcessingParameterNumber::Integer, 1, false, 1 ) );
  addParameter( new QgsProcessingParameterDistance( QStringLiteral( "MIN_DISTANCE" ), QObject::tr( "Minimum distance between points" ), 0, QStringLiteral( "TARGET_CRS" ), true, 0 ) );
  addParameter( new QgsProcessingParameterCrs( QStringLiteral( "TARGET_CRS" ), QObject::tr( "Target CRS" ), QStringLiteral( "ProjectCrs" ), false ) );

  std::unique_ptr< QgsProcessingParameterNumber > maxAttempts_param = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "MAX_ATTEMPTS" ), QObject::tr( "Maximum number of search attempts given the minimum distance" ), QgsProcessingParameterNumber::Integer, 200, true, 1 );
  maxAttempts_param->setFlags( maxAttempts_param->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( maxAttempts_param.release() );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Random points" ), QgsProcessing::TypeVectorPoint ) );
}

QString QgsRandomPointsExtentAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a new point layer with a given "
                      "number of random points, all of them within a given extent. "
                      "A distance factor can be specified, to avoid points being "
                      "too close to each other. If the minimum distance between points "
                      "makes it impossible to create new points, either "
                      "distance can be decreased or the maximum number of attempts may be "
                      "increased."
                    );
}

QgsRandomPointsExtentAlgorithm *QgsRandomPointsExtentAlgorithm::createInstance() const
{
  return new QgsRandomPointsExtentAlgorithm();
}

bool QgsRandomPointsExtentAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mCrs = parameterAsCrs( parameters, QStringLiteral( "TARGET_CRS" ), context );
  mExtent = parameterAsExtent( parameters, QStringLiteral( "EXTENT" ), context, mCrs );
  mNumPoints = parameterAsInt( parameters, QStringLiteral( "POINTS_NUMBER" ), context );
  mDistance = parameterAsDouble( parameters, QStringLiteral( "MIN_DISTANCE" ), context );
  mMaxAttempts = parameterAsInt( parameters, QStringLiteral( "MAX_ATTEMPTS" ), context );

  return true;
}

QVariantMap QgsRandomPointsExtentAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{

  QgsFields fields = QgsFields();
  fields.append( QgsField( QStringLiteral( "id" ), QVariant::LongLong ) );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields, QgsWkbTypes::Point, mCrs ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  //initialize random engine
  std::random_device random_device;
  const std::mt19937 mersenne_twister( random_device() );

  std::uniform_real_distribution<double> x_distribution( mExtent.xMinimum(), mExtent.xMaximum() );
  std::uniform_real_distribution<double> y_distribution( mExtent.yMinimum(), mExtent.yMaximum() );

  if ( mDistance == 0 )
  {
    int i = 0;
    while ( i < mNumPoints )
    {
      if ( feedback->isCanceled() )
        break;

      const double rx = x_distribution( random_device );
      const double ry = y_distribution( random_device );

      QgsFeature f = QgsFeature( i );

      f.setGeometry( QgsGeometry( new QgsPoint( rx, ry ) ) );
      f.setAttributes( QgsAttributes() << i );
      if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
      i++;
      feedback->setProgress( static_cast<int>( static_cast<double>( i ) / static_cast<double>( mNumPoints ) * 100 ) );
    }
  }
  else
  {
    QgsSpatialIndex index = QgsSpatialIndex();
    int distCheckIterations = 0;

    int i = 0;
    while ( i < mNumPoints )
    {
      if ( feedback->isCanceled() )
        break;

      const double rx = x_distribution( random_device );
      const double ry = y_distribution( random_device );

      //check if new random point is inside searching distance to existing points
      const QList<QgsFeatureId> neighbors = index.nearestNeighbor( QgsPointXY( rx, ry ), 1, mDistance );
      if ( neighbors.empty() )
      {
        QgsFeature f = QgsFeature( i );
        f.setAttributes( QgsAttributes() << i );
        const QgsGeometry randomPointGeom = QgsGeometry( new QgsPoint( rx, ry ) );
        f.setGeometry( randomPointGeom );
        if ( !index.addFeature( f ) ||
             !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
        i++;
        distCheckIterations = 0; //reset distCheckIterations if a point is added
        feedback->setProgress( static_cast<int>( static_cast<double>( i ) / static_cast<double>( mNumPoints ) * 100 ) );
      }
      else
      {
        if ( distCheckIterations == mMaxAttempts )
        {
          throw QgsProcessingException( QObject::tr( "%1 of %2 points have been successfully created, but no more random points could be found "
                                        "due to the given minimum distance between points. Either choose a larger extent, "
                                        "lower the minimum distance between points or try increasing the number "
                                        "of attempts for searching new points." ).arg( i ).arg( mNumPoints ) );
        }
        else
        {
          distCheckIterations++;
          continue; //retry with new point
        }

      }
    }
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );

  return outputs;
}

///@endcond
