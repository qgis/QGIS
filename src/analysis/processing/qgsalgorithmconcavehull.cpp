/***************************************************************************
                         qgsalgorithmconcavehull.cpp
                         ---------------------
    begin                : July 2023
    copyright            : (C) 2023 by Alexander Bruy
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

#include "qgsalgorithmconcavehull.h"

#include "qgsmultipoint.h"
#include "qgsprocessingregistry.h"
#include "qgsspatialindex.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE

QString QgsConcaveHullAlgorithm::name() const
{
  return u"concavehull"_s;
}

QString QgsConcaveHullAlgorithm::displayName() const
{
  return QObject::tr( "Concave hull (by layer)" );
}

QStringList QgsConcaveHullAlgorithm::tags() const
{
  return QObject::tr( "concave,hull,bounds,bounding" ).split( ',' );
}

QString QgsConcaveHullAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsConcaveHullAlgorithm::groupId() const
{
  return u"vectorgeometry"_s;
}

QString QgsConcaveHullAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm computes the concave hull covering all features from an input point layer." ) + u"\n\n"_s
         + QObject::tr( "See the 'Concave hull (by feature)' algorithm for a concave hull calculation which covers individual features from a layer." );
}

QString QgsConcaveHullAlgorithm::shortDescription() const
{
  return QObject::tr( "Computes the concave hull of all features from an input point layer." );
}

QgsConcaveHullAlgorithm *QgsConcaveHullAlgorithm::createInstance() const
{
  return new QgsConcaveHullAlgorithm();
}

void QgsConcaveHullAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint ) ) );
  addParameter( new QgsProcessingParameterNumber( u"ALPHA"_s, QObject::tr( "Threshold (0-1, where 1 is equivalent with Convex Hull)" ), Qgis::ProcessingNumberParameterType::Double, 0.3, false, 0, 1 ) );
  addParameter( new QgsProcessingParameterBoolean( u"HOLES"_s, QObject::tr( "Allow holes" ), true ) );
  addParameter( new QgsProcessingParameterBoolean( u"NO_MULTIGEOMETRY"_s, QObject::tr( "Split multipart geometry into singleparts" ), false ) );
  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Concave hull" ), Qgis::ProcessingSourceType::VectorPolygon ) );
}

bool QgsConcaveHullAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mSource.reset( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !mSource )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  if ( mSource->featureCount() < 3 )
    throw QgsProcessingException( QObject::tr( "Input layer should contain at least 3 points." ) );

  mStep = mSource->featureCount() > 0 ? 50.0 / mSource->featureCount() : 1;

  mPercentage = parameterAsDouble( parameters, u"ALPHA"_s, context );
  mAllowHoles = parameterAsBool( parameters, u"HOLES"_s, context );
  mSplitMultipart = parameterAsBool( parameters, u"NO_MULTIGEOMETRY"_s, context );

  return true;
}

QVariantMap QgsConcaveHullAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, QgsFields(), Qgis::WkbType::Polygon, mSource->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

#if GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR < 11
  concaveHullQgis( sink, parameters, context, feedback );
#else
  concaveHullGeos( sink, parameters, feedback );
#endif

  sink->finalize();

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, dest );
  return outputs;
}

void QgsConcaveHullAlgorithm::concaveHullGeos( std::unique_ptr<QgsFeatureSink> &sink, const QVariantMap &parameters, QgsProcessingFeedback *feedback )
{
  long long i = 0;

  QgsFeatureIterator it = mSource->getFeatures( QgsFeatureRequest().setNoAttributes(), Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks );
  QgsFeature f;
  QgsGeometry allPoints;
  while ( it.nextFeature( f ) )
  {
    i++;
    if ( feedback->isCanceled() )
      return;

    feedback->setProgress( i * mStep );

    if ( !f.hasGeometry() )
      continue;

    const QgsAbstractGeometry *geom = f.geometry().constGet();
    if ( QgsWkbTypes::isMultiType( geom->wkbType() ) )
    {
      const QgsMultiPoint mp( *qgsgeometry_cast<const QgsMultiPoint *>( geom ) );
      for ( auto pit = mp.const_parts_begin(); pit != mp.const_parts_end(); ++pit )
      {
        allPoints.addPartV2( qgsgeometry_cast<const QgsPoint *>( *pit )->clone(), Qgis::WkbType::Point );
      }
    }
    else
    {
      allPoints.addPartV2( qgsgeometry_cast<const QgsPoint *>( geom )->clone(), Qgis::WkbType::Point );
    }
  }
  const QgsGeometry concaveHull = allPoints.concaveHull( mPercentage, mAllowHoles );

  if ( mSplitMultipart && concaveHull.isMultipart() )
  {
    QVector<QgsGeometry> collection = concaveHull.asGeometryCollection();
    mStep = collection.length() > 0 ? 50.0 / collection.length() : 1;
    for ( int i = 0; i < collection.length(); i++ )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }

      QgsGeometry geom = collection[i];
      if ( !mAllowHoles )
      {
        geom = collection[i].removeInteriorRings();
      }
      QgsFeature f;
      f.setGeometry( geom );
      if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );

      feedback->setProgress( 50 + i * mStep );
    }
  }
  else
  {
    QgsGeometry geom( concaveHull );
    if ( !mAllowHoles )
    {
      geom = concaveHull.removeInteriorRings();
    }
    QgsFeature f;
    f.setGeometry( geom );
    if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
    feedback->setProgress( 100 );
  }
}

void QgsConcaveHullAlgorithm::concaveHullQgis( std::unique_ptr<QgsFeatureSink> &sink, const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsProcessingMultiStepFeedback multiStepFeedback( 5, feedback );

  feedback->setProgressText( QObject::tr( "Creating Delaunay triangles…" ) );
  multiStepFeedback.setCurrentStep( 1 );

  QVariantMap params;
  params["INPUT"] = parameters["INPUT"];
  params["TOLERANCE"] = 0.0;
  params["ADD_ATTRIBUTES"] = false;
  params["OUTPUT"] = QgsProcessing::TEMPORARY_OUTPUT;
  const QgsProcessingAlgorithm *delaunayAlg = QgsApplication::processingRegistry()->algorithmById( u"native:delaunaytriangulation"_s );
  if ( !delaunayAlg )
  {
    throw QgsProcessingException( QObject::tr( "Failed to compute concave hull: Delaunay triangulation algorithm not found!" ) );
  }

  std::unique_ptr<QgsProcessingAlgorithm> algorithm;
  algorithm.reset( delaunayAlg->create() );
  QVariantMap results = algorithm->run( params, context, &multiStepFeedback );
  QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( QgsProcessingUtils::mapLayerFromString( results["OUTPUT"].toString(), context ) );

  if ( !layer )
  {
    throw QgsProcessingException( QObject::tr( "Failed to compute Delaunay triangulation." ) );
  }

  if ( layer->featureCount() == 0 )
  {
    throw QgsProcessingException( QObject::tr( "No Delaunay triangles created." ) );
  }

  feedback->setProgressText( QObject::tr( "Computing edges max length…" ) );
  multiStepFeedback.setCurrentStep( 2 );

  QVector<double> length;
  QMap<long, double> edges;
  long i = 0;
  double step = layer->featureCount() > 0 ? 100.0 / layer->featureCount() : 1;
  QgsFeatureIterator it = layer->getFeatures( QgsFeatureRequest().setNoAttributes() );
  QgsFeature f;
  while ( it.nextFeature( f ) )
  {
    i++;
    if ( feedback->isCanceled() )
      return;

    multiStepFeedback.setProgress( i * step );

    if ( !f.hasGeometry() )
      continue;

    QVector<QgsPointXY> points = f.geometry().asPolygon().at( 0 );
    for ( int j = 0; j < points.size() - 1; j++ )
    {
      length << std::sqrt( points.at( j ).sqrDist( points.at( j + 1 ) ) );
    }
    QVector<double> vec = length.mid( length.size() - 3, -1 );
    edges[f.id()] = *std::max_element( vec.constBegin(), vec.constEnd() );
  }
  const double maxLength = *std::max_element( length.constBegin(), length.constEnd() );

  feedback->setProgressText( QObject::tr( "Removing features…" ) );
  multiStepFeedback.setCurrentStep( 3 );
  i = 0;
  step = edges.size() > 0 ? 100.0 / edges.size() : 1;
  QgsFeatureIds toDelete;
  QMap<long, double>::iterator edgesIt = edges.begin();
  while ( edgesIt != edges.end() )
  {
    if ( feedback->isCanceled() )
      return;

    if ( edgesIt.value() > mPercentage * maxLength )
    {
      toDelete << edgesIt.key();
    }

    ++edgesIt;
    i++;
    multiStepFeedback.setProgress( i * step );
  }
  layer->dataProvider()->deleteFeatures( toDelete );

  feedback->setProgressText( QObject::tr( "Dissolving Delaunay triangles…" ) );
  multiStepFeedback.setCurrentStep( 4 );
  params.clear();
  params["INPUT"] = layer->source();
  params["OUTPUT"] = QgsProcessing::TEMPORARY_OUTPUT;
  const QgsProcessingAlgorithm *dissolveAlg = QgsApplication::processingRegistry()->algorithmById( u"native:dissolve"_s );
  if ( !dissolveAlg )
  {
    throw QgsProcessingException( QObject::tr( "Failed to compute concave hull: Dissolve algorithm not found!" ) );
  }
  algorithm.reset( dissolveAlg->create() );
  results = algorithm->run( params, context, &multiStepFeedback );
  layer = qobject_cast<QgsVectorLayer *>( QgsProcessingUtils::mapLayerFromString( results["OUTPUT"].toString(), context ) );
  if ( !layer )
  {
    throw QgsProcessingException( QObject::tr( "Failed to dissolve Delaunay triangles." ) );
  }
  if ( layer->featureCount() == 0 )
  {
    throw QgsProcessingException( QObject::tr( "There are no features in the dissolved layer." ) );
  }

  layer->getFeatures().nextFeature( f );
  QgsGeometry concaveHull = f.geometry();

  // save result
  multiStepFeedback.setCurrentStep( 5 );

  if ( mSplitMultipart && concaveHull.isMultipart() )
  {
    const QVector<QgsGeometry> collection = concaveHull.asGeometryCollection();
    step = collection.length() > 0 ? 50.0 / collection.length() : 1;
    for ( int i = 0; i < collection.length(); i++ )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }

      QgsGeometry geom = collection[i];
      if ( !mAllowHoles )
      {
        geom = collection[i].removeInteriorRings();
      }
      QgsFeature f;
      f.setGeometry( geom );
      if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );

      multiStepFeedback.setProgress( i * step );
    }
  }
  else
  {
    QgsGeometry geom( concaveHull );
    if ( !mAllowHoles )
    {
      geom = concaveHull.removeInteriorRings();
    }
    QgsFeature f;
    f.setGeometry( geom );
    if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
    multiStepFeedback.setProgress( 100 );
  }
}

///@endcond
