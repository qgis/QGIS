/***************************************************************************
                         qgsalgorithmrandompointslayer.cpp
                         ---------------------
    begin                : November 2026
    copyright            : (C) 2026 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmrandompointslayer.h"

#include <random>

#include "qgsspatialindex.h"

#include <QString>

using namespace Qt::StringLiterals;

///@cond PRIVATE

QString QgsRandomPointsLayerAlgorithm::name() const
{
  return u"randompointsinlayerbounds"_s;
}

QString QgsRandomPointsLayerAlgorithm::displayName() const
{
  return QObject::tr( "Random points in layer bounds" );
}

QStringList QgsRandomPointsLayerAlgorithm::tags() const
{
  return QObject::tr( "random,points,bounds,polygon,create" ).split( ',' );
}

QString QgsRandomPointsLayerAlgorithm::group() const
{
  return QObject::tr( "Vector creation" );
}

QString QgsRandomPointsLayerAlgorithm::groupId() const
{
  return u"vectorcreation"_s;
}

void QgsRandomPointsLayerAlgorithm::initAlgorithm( const QVariantMap & )
{
  auto inputParam = std::make_unique<QgsProcessingParameterFeatureSource>( u"INPUT"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon ) );
  inputParam->setHelp( QObject::tr( "Polygon layer defining the area." ) );
  addParameter( inputParam.release() );

  auto pointNumberParam = std::make_unique<QgsProcessingParameterNumber>( u"POINTS_NUMBER"_s, QObject::tr( "Number of points" ), Qgis::ProcessingNumberParameterType::Integer, 1, false, 1 );
  pointNumberParam->setHelp( QObject::tr( "Number of points to create." ) );
  addParameter( pointNumberParam.release() );

  auto minDistanceParam = std::make_unique<QgsProcessingParameterDistance>( u"MIN_DISTANCE"_s, QObject::tr( "Minimum distance between points" ), 0, u"INPUT"_s, true, 0 );
  minDistanceParam->setHelp( "The minimum distance between points." );
  addParameter( minDistanceParam.release() );

  auto maxAttemptsParam = std::make_unique<
    QgsProcessingParameterNumber>( u"MAX_ATTEMPTS"_s, QObject::tr( "Maximum number of search attempts given the minimum distance" ), Qgis::ProcessingNumberParameterType::Integer, 200, true, 1 );
  maxAttemptsParam->setHelp( QObject::tr( "Maximum number of attempts to place the points." ) );
  maxAttemptsParam->setFlags( maxAttemptsParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( maxAttemptsParam.release() );

  auto outputParam = std::make_unique<QgsProcessingParameterFeatureSink>( u"OUTPUT"_s, QObject::tr( "Random points" ), Qgis::ProcessingSourceType::VectorPoint );
  outputParam->setHelp( QObject::tr( "Output random points layer." ) );
  addParameter( outputParam.release() );
}

QString QgsRandomPointsLayerAlgorithm::shortHelpString() const
{
  return QObject::tr(
    "This algorithm creates a new point layer with a given "
    "number of random points, all of them within the bounds "
    "of a given polygon layer. A distance factor can be specified, "
    "to avoid points being too close to each other. "
    "If the minimum distance between points makes it impossible "
    "to create new points, either the distance must be decreased "
    "or the maximum number of attempts must be increased."
  );
}

QString QgsRandomPointsLayerAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates a new point layer with a given number of random points, all of them within the boundaries of a given layer." );
}

QgsRandomPointsLayerAlgorithm *QgsRandomPointsLayerAlgorithm::createInstance() const
{
  return new QgsRandomPointsLayerAlgorithm();
}

bool QgsRandomPointsLayerAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mSource = parameterAsSource( parameters, u"INPUT"_s, context );
  if ( !mSource )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  mExtent = mSource->sourceExtent();
  mNumPoints = parameterAsInt( parameters, u"POINTS_NUMBER"_s, context );
  mDistance = parameterAsDouble( parameters, u"MIN_DISTANCE"_s, context );
  mMaxAttempts = parameterAsInt( parameters, u"MAX_ATTEMPTS"_s, context );

  return true;
}

QVariantMap QgsRandomPointsLayerAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QGS_MARK_ALGORITHM_SOURCE

  QgsFields fields = QgsFields();
  fields.append( QgsField( u"id"_s, QMetaType::Type::LongLong ) );

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, fields, Qgis::WkbType::Point, mSource->sourceCrs() ) );
  if ( !sink )
  {
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );
  }

  std::random_device random_device;
  const std::mt19937 mersenne_twister( random_device() );

  std::uniform_real_distribution<double> x_distribution( mExtent.xMinimum(), mExtent.xMaximum() );
  std::uniform_real_distribution<double> y_distribution( mExtent.yMinimum(), mExtent.yMaximum() );

  QgsSpatialIndex sourceIndex( *mSource, feedback );

  if ( mDistance == 0 )
  {
    int i = 0;
    while ( i < mNumPoints )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }

      const double rx = x_distribution( random_device );
      const double ry = y_distribution( random_device );

      const QgsGeometry geom = QgsGeometry::fromPointXY( QgsPointXY( rx, ry ) );
      if ( isWithinLayerBounds( geom, sourceIndex, feedback ) )
      {
        QgsFeature f = QgsFeature( i );
        f.setGeometry( geom );
        f.setAttributes( QgsAttributes() << i );
        if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
        {
          throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
        }
        else
        {
          feedback->featureAddedToSink( dest );
        }
        i++;
        feedback->setProgress( static_cast<int>( static_cast<double>( i ) / static_cast<double>( mNumPoints ) * 100 ) );
      }
    }
  }
  else
  {
    int i = 0;
    int nIterations = 0;
    QgsSpatialIndex newPointIndex = QgsSpatialIndex();
    while ( i < mNumPoints )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }

      const double rx = x_distribution( random_device );
      const double ry = y_distribution( random_device );

      const QList<QgsFeatureId> neighbors = newPointIndex.nearestNeighbor( QgsPointXY( rx, ry ), 1, mDistance );
      if ( neighbors.empty() )
      {
        const QgsGeometry geom = QgsGeometry::fromPointXY( QgsPointXY( rx, ry ) );
        if ( isWithinLayerBounds( geom, sourceIndex, feedback ) )
        {
          QgsFeature f = QgsFeature( i );
          f.setGeometry( geom );
          f.setAttributes( QgsAttributes() << i );
          if ( !newPointIndex.addFeature( f ) || !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
          {
            throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
          }
          else
          {
            feedback->featureAddedToSink( u"OUTPUT"_s );
          }
          i++;
          nIterations = 0;
          feedback->setProgress( static_cast<int>( static_cast<double>( i ) / static_cast<double>( mNumPoints ) * 100 ) );
        }
      }
      else
      {
        if ( nIterations == mMaxAttempts )
        {
          throw QgsProcessingException(
            QObject::tr(
              "%1 of %2 points have been successfully created, but no more random points could be found "
              "due to the given minimum distance between points. Either lower the minimum distance "
              "between points or try increasing the number of attempts for searching new points."
            )
              .arg( i )
              .arg( mNumPoints )
          );
        }
        else
        {
          nIterations++;
          continue;
        }
      }
    }
  }

  sink->finalize();
  feedback->featureSinkFinalized( u"OUTPUT"_s );

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, dest );

  return outputs;
}

bool QgsRandomPointsLayerAlgorithm::isWithinLayerBounds( const QgsGeometry &geom, const QgsSpatialIndex &sourceIndex, QgsProcessingFeedback *feedback ) const
{
  const QgsFeatureIds candidateIds = qgis::listToSet( sourceIndex.intersects( geom.boundingBox() ) );
  if ( !candidateIds.isEmpty() )
  {
    QgsFeatureIterator fit = mSource->getFeatures( QgsFeatureRequest().setFilterFids( candidateIds ).setNoAttributes() );
    QgsFeature f;
    while ( fit.nextFeature( f ) )
    {
      if ( feedback && feedback->isCanceled() )
      {
        break;
      }

      if ( geom.within( f.geometry() ) )
      {
        return true;
      }
    }
  }

  return false;
}

///@endcond
