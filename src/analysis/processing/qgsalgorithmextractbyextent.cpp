/***************************************************************************
                         qgsalgorithmextractbyextent.cpp
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

#include "qgsalgorithmextractbyextent.h"

///@cond PRIVATE

QString QgsExtractByExtentAlgorithm::name() const
{
  return QStringLiteral( "extractbyextent" );
}

QString QgsExtractByExtentAlgorithm::displayName() const
{
  return QObject::tr( "Extract/clip by extent" );
}

QStringList QgsExtractByExtentAlgorithm::tags() const
{
  return QObject::tr( "clip,extract,intersect,intersection,mask,extent" ).split( ',' );
}

QString QgsExtractByExtentAlgorithm::group() const
{
  return QObject::tr( "Vector overlay" );
}

QString QgsExtractByExtentAlgorithm::groupId() const
{
  return QStringLiteral( "vectoroverlay" );
}
void QgsExtractByExtentAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterExtent( QStringLiteral( "EXTENT" ), QObject::tr( "Extent" ) ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "CLIP" ), QObject::tr( "Clip features to extent" ), false ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Extracted" ) ) );
}

QString QgsExtractByExtentAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a new vector layer that only contains features which fall within a specified extent. "
                      "Any features which intersect the extent will be included.\n\n"
                      "Optionally, feature geometries can also be clipped to the extent. If this option is selected, then the output "
                      "geometries will automatically be converted to multi geometries to ensure uniform output geometry types." );
}

QgsExtractByExtentAlgorithm *QgsExtractByExtentAlgorithm::createInstance() const
{
  return new QgsExtractByExtentAlgorithm();
}

QVariantMap QgsExtractByExtentAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsFeatureSource > featureSource( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !featureSource )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  if ( featureSource->hasSpatialIndex() == QgsFeatureSource::SpatialIndexNotPresent )
    feedback->pushWarning( QObject::tr( "No spatial index exists for input layer, performance will be severely degraded" ) );

  const QgsRectangle extent = parameterAsExtent( parameters, QStringLiteral( "EXTENT" ), context, featureSource->sourceCrs() );
  const bool clip = parameterAsBoolean( parameters, QStringLiteral( "CLIP" ), context );

  // if clipping, we force multi output
  const QgsWkbTypes::Type outType = clip ? QgsWkbTypes::promoteNonPointTypesToMulti( featureSource->wkbType() ) : featureSource->wkbType();

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, featureSource->fields(), outType, featureSource->sourceCrs() ) );

  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  const QgsGeometry clipGeom = parameterAsExtentGeometry( parameters, QStringLiteral( "EXTENT" ), context, featureSource->sourceCrs() );

  const double step = featureSource->featureCount() > 0 ? 100.0 / featureSource->featureCount() : 1;
  QgsFeatureIterator inputIt = featureSource->getFeatures( QgsFeatureRequest().setFilterRect( extent ).setFlags( QgsFeatureRequest::ExactIntersect ) );
  QgsFeature f;
  int i = -1;
  while ( inputIt.nextFeature( f ) )
  {
    i++;
    if ( feedback->isCanceled() )
    {
      break;
    }

    if ( clip )
    {
      QgsGeometry g = f.geometry().intersection( clipGeom );
      g.convertToMultiType();
      f.setGeometry( g );
    }

    if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
    feedback->setProgress( i * step );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

///@endcond

