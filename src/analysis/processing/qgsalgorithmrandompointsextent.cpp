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

#include <random>

#include "qgsspatialindex.h"

///@cond PRIVATE

QString QgsRandomPointsExtentAlgorithm::name() const
{
  return u"randompointsinextent"_s;
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
  return u"vectorcreation"_s;
}

void QgsRandomPointsExtentAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterExtent( u"EXTENT"_s, QObject::tr( "Input extent" ) ) );
  addParameter( new QgsProcessingParameterNumber( u"POINTS_NUMBER"_s, QObject::tr( "Number of points" ), Qgis::ProcessingNumberParameterType::Integer, 1, false, 1 ) );
  addParameter( new QgsProcessingParameterDistance( u"MIN_DISTANCE"_s, QObject::tr( "Minimum distance between points" ), 0, u"TARGET_CRS"_s, true, 0 ) );
  addParameter( new QgsProcessingParameterCrs( u"TARGET_CRS"_s, QObject::tr( "Target CRS" ), u"ProjectCrs"_s, false ) );

  auto maxAttempts_param = std::make_unique<QgsProcessingParameterNumber>( u"MAX_ATTEMPTS"_s, QObject::tr( "Maximum number of search attempts given the minimum distance" ), Qgis::ProcessingNumberParameterType::Integer, 200, true, 1 );
  maxAttempts_param->setFlags( maxAttempts_param->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( maxAttempts_param.release() );

  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Random points" ), Qgis::ProcessingSourceType::VectorPoint ) );
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

QString QgsRandomPointsExtentAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates a point layer with a given number of random points, all of them within a given extent." );
}

QgsRandomPointsExtentAlgorithm *QgsRandomPointsExtentAlgorithm::createInstance() const
{
  return new QgsRandomPointsExtentAlgorithm();
}

bool QgsRandomPointsExtentAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mCrs = parameterAsCrs( parameters, u"TARGET_CRS"_s, context );
  mExtent = parameterAsExtent( parameters, u"EXTENT"_s, context, mCrs );
  mNumPoints = parameterAsInt( parameters, u"POINTS_NUMBER"_s, context );
  mDistance = parameterAsDouble( parameters, u"MIN_DISTANCE"_s, context );
  mMaxAttempts = parameterAsInt( parameters, u"MAX_ATTEMPTS"_s, context );

  return true;
}

QVariantMap QgsRandomPointsExtentAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsFields fields = QgsFields();
  fields.append( QgsField( u"id"_s, QMetaType::Type::LongLong ) );

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, fields, Qgis::WkbType::Point, mCrs ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

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
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
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
        if ( !index.addFeature( f ) || !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
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
                                                     "of attempts for searching new points." )
                                          .arg( i )
                                          .arg( mNumPoints ) );
        }
        else
        {
          distCheckIterations++;
          continue; //retry with new point
        }
      }
    }
  }

  sink->finalize();

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, dest );

  return outputs;
}

///@endcond
