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
  return u"extractbyextent"_s;
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
  return u"vectoroverlay"_s;
}
void QgsExtractByExtentAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterExtent( u"EXTENT"_s, QObject::tr( "Extent" ) ) );
  addParameter( new QgsProcessingParameterBoolean( u"CLIP"_s, QObject::tr( "Clip features to extent" ), false ) );
  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Extracted" ) ) );
}

QString QgsExtractByExtentAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a new vector layer that only contains features which fall within a specified extent. "
                      "Any features which intersect the extent will be included.\n\n"
                      "Optionally, feature geometries can also be clipped to the extent. If this option is selected, then the output "
                      "geometries will automatically be converted to multi geometries to ensure uniform output geometry types." );
}

QString QgsExtractByExtentAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates a vector layer that only contains features which intersect a specified extent." );
}

QgsExtractByExtentAlgorithm *QgsExtractByExtentAlgorithm::createInstance() const
{
  return new QgsExtractByExtentAlgorithm();
}

QVariantMap QgsExtractByExtentAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsFeatureSource> featureSource( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !featureSource )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  if ( featureSource->hasSpatialIndex() == Qgis::SpatialIndexPresence::NotPresent )
    feedback->pushWarning( QObject::tr( "No spatial index exists for input layer, performance will be severely degraded" ) );

  const QgsRectangle extent = parameterAsExtent( parameters, u"EXTENT"_s, context, featureSource->sourceCrs() );
  const bool clip = parameterAsBoolean( parameters, u"CLIP"_s, context );

  // if clipping, we force multi output
  const Qgis::WkbType outType = clip ? QgsWkbTypes::promoteNonPointTypesToMulti( featureSource->wkbType() ) : featureSource->wkbType();

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, featureSource->fields(), outType, featureSource->sourceCrs() ) );

  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  const QgsGeometry clipGeom = parameterAsExtentGeometry( parameters, u"EXTENT"_s, context, featureSource->sourceCrs() );

  const double step = featureSource->featureCount() > 0 ? 100.0 / featureSource->featureCount() : 1;
  QgsFeatureIterator inputIt = featureSource->getFeatures( QgsFeatureRequest().setFilterRect( extent ).setFlags( Qgis::FeatureRequestFlag::ExactIntersect ) );
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

      if ( g.type() != Qgis::GeometryType::Point )
      {
        // some data providers are picky about the geometries we pass to them: we can't add single-part geometries
        // when we promised multi-part geometries, so ensure we have the right type
        g.convertToMultiType();
      }

      f.setGeometry( g );
    }

    if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
    feedback->setProgress( i * step );
  }

  sink->finalize();

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, dest );
  return outputs;
}

///@endcond
