/***************************************************************************
                         qgsalgorithmclip.cpp
                         ---------------------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgsalgorithmclip.h"
#include "qgsgeometryengine.h"
#include "qgsoverlayutils.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE

QString QgsClipAlgorithm::name() const
{
  return QStringLiteral( "clip" );
}

QgsProcessingAlgorithm::Flags QgsClipAlgorithm::flags() const
{
  Flags f = QgsProcessingAlgorithm::flags();
  f |= QgsProcessingAlgorithm::FlagSupportsInPlaceEdits;
  return f;
}

QString QgsClipAlgorithm::displayName() const
{
  return QObject::tr( "Clip" );
}

QStringList QgsClipAlgorithm::tags() const
{
  return QObject::tr( "clip,intersect,intersection,mask" ).split( ',' );
}

QString QgsClipAlgorithm::group() const
{
  return QObject::tr( "Vector overlay" );
}

QString QgsClipAlgorithm::groupId() const
{
  return QStringLiteral( "vectoroverlay" );
}

void QgsClipAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "OVERLAY" ), QObject::tr( "Overlay layer" ), QList< int >() << QgsProcessing::TypeVectorPolygon ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Clipped" ) ) );
}

QString QgsClipAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm clips a vector layer using the features of an additional polygon layer. Only the parts of the features "
                      "in the Input layer that fall within the polygons of the Overlay layer will be added to the resulting layer." )
         + QStringLiteral( "\n\n" )
         + QObject::tr( "The attributes of the features are not modified, although properties such as area or length of the features will "
                        "be modified by the clipping operation. If such properties are stored as attributes, those attributes will have to "
                        "be manually updated." );
}

QgsClipAlgorithm *QgsClipAlgorithm::createInstance() const
{
  return new QgsClipAlgorithm();
}

bool QgsClipAlgorithm::supportInPlaceEdit( const QgsMapLayer *l ) const
{
  const QgsVectorLayer *layer = qobject_cast< const QgsVectorLayer * >( l );
  if ( !layer )
    return false;

  return layer->isSpatial();
}

QVariantMap QgsClipAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsFeatureSource > featureSource( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !featureSource )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  std::unique_ptr< QgsFeatureSource > maskSource( parameterAsSource( parameters, QStringLiteral( "OVERLAY" ), context ) );
  if ( !maskSource )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "OVERLAY" ) ) );

  if ( featureSource->hasSpatialIndex() == QgsFeatureSource::SpatialIndexNotPresent )
    feedback->pushWarning( QObject::tr( "No spatial index exists for input layer, performance will be severely degraded" ) );

  QString dest;
  const QgsWkbTypes::GeometryType sinkType = QgsWkbTypes::geometryType( featureSource->wkbType() );
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, featureSource->fields(), QgsWkbTypes::promoteNonPointTypesToMulti( featureSource->wkbType() ), featureSource->sourceCrs() ) );

  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  // first build up a list of clip geometries
  QVector< QgsGeometry > clipGeoms;
  QgsFeatureIterator it = maskSource->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( QList< int >() ).setDestinationCrs( featureSource->sourceCrs(), context.transformContext() ) );
  QgsFeature f;
  while ( it.nextFeature( f ) )
  {
    if ( f.hasGeometry() )
      clipGeoms << f.geometry();
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );

  if ( clipGeoms.isEmpty() )
    return outputs;

  // are we clipping against a single feature? if so, we can show finer progress reports
  bool singleClipFeature = false;
  QgsGeometry combinedClipGeom;
  if ( clipGeoms.length() > 1 )
  {
    combinedClipGeom = QgsGeometry::unaryUnion( clipGeoms );
    if ( combinedClipGeom.isEmpty() )
    {
      throw QgsProcessingException( QObject::tr( "Could not create the combined clip geometry: %1" ).arg( combinedClipGeom.lastError() ) );
    }
    singleClipFeature = false;
  }
  else
  {
    combinedClipGeom = clipGeoms.at( 0 );
    singleClipFeature = true;
  }

  // use prepared geometries for faster intersection tests
  std::unique_ptr< QgsGeometryEngine > engine( QgsGeometry::createGeometryEngine( combinedClipGeom.constGet() ) );
  engine->prepareGeometry();

  QgsFeatureIds testedFeatureIds;

  int i = -1;
  const auto constClipGeoms = clipGeoms;
  for ( const QgsGeometry &clipGeom : constClipGeoms )
  {
    i++;
    if ( feedback->isCanceled() )
    {
      break;
    }
    QgsFeatureIterator inputIt = featureSource->getFeatures( QgsFeatureRequest().setFilterRect( clipGeom.boundingBox() ) );
    QgsFeatureList inputFeatures;
    QgsFeature f;
    while ( inputIt.nextFeature( f ) )
      inputFeatures << f;

    if ( inputFeatures.isEmpty() )
      continue;

    double step = 0;
    if ( singleClipFeature )
      step = 100.0 / inputFeatures.length();

    const int current = 0;
    const auto constInputFeatures = inputFeatures;
    for ( const QgsFeature &inputFeature : constInputFeatures )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }

      if ( !inputFeature.hasGeometry() )
        continue;

      if ( testedFeatureIds.contains( inputFeature.id() ) )
      {
        // don't retest a feature we have already checked
        continue;
      }
      testedFeatureIds.insert( inputFeature.id() );

      if ( !engine->intersects( inputFeature.geometry().constGet() ) )
        continue;

      QgsGeometry newGeometry;
      if ( !engine->contains( inputFeature.geometry().constGet() ) )
      {
        const QgsGeometry currentGeometry = inputFeature.geometry();
        newGeometry = combinedClipGeom.intersection( currentGeometry );
        if ( newGeometry.wkbType() == QgsWkbTypes::Unknown || QgsWkbTypes::flatType( newGeometry.wkbType() ) == QgsWkbTypes::GeometryCollection )
        {
          const QgsGeometry intCom = inputFeature.geometry().combine( newGeometry );
          const QgsGeometry intSym = inputFeature.geometry().symDifference( newGeometry );
          newGeometry = intCom.difference( intSym );
        }
      }
      else
      {
        // clip geometry totally contains feature geometry, so no need to perform intersection
        newGeometry = inputFeature.geometry();
      }

      if ( !QgsOverlayUtils::sanitizeIntersectionResult( newGeometry, sinkType ) )
        continue;

      QgsFeature outputFeature;
      outputFeature.setGeometry( newGeometry );
      outputFeature.setAttributes( inputFeature.attributes() );
      if ( !sink->addFeature( outputFeature, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );


      if ( singleClipFeature )
        feedback->setProgress( current * step );
    }

    if ( !singleClipFeature )
    {
      // coarse progress report for multiple clip geometries
      feedback->setProgress( 100.0 * static_cast< double >( i ) / clipGeoms.length() );
    }
  }

  return outputs;
}

///@endcond
