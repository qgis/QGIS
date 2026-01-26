/***************************************************************************
                         qgsalgorithmfindprojection.cpp
                         ---------------------
    begin                : March 2025
    copyright            : (C) 2025 by Alexander Bruy
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

#include "qgsalgorithmfindprojection.h"

#include "qgsgeometryengine.h"

///@cond PRIVATE

QString QgsFindProjectionAlgorithm::name() const
{
  return u"findprojection"_s;
}

QString QgsFindProjectionAlgorithm::displayName() const
{
  return QObject::tr( "Find projection" );
}

QStringList QgsFindProjectionAlgorithm::tags() const
{
  return QObject::tr( "crs,srs,coordinate,reference,system,guess,estimate,finder,determine" ).split( ',' );
}

QString QgsFindProjectionAlgorithm::group() const
{
  return QObject::tr( "Vector general" );
}

QString QgsFindProjectionAlgorithm::groupId() const
{
  return u"vectorgeneral"_s;
}

QString QgsFindProjectionAlgorithm::shortHelpString() const
{
  return QObject::tr( "Creates a list of possible candidate coordinate reference systems for a layer "
                      "with an unknown projection.\n\n"
                      "The expected area which the layer should reside in must be specified via the "
                      "target area parameter.\n\n"
                      "The algorithm operates by testing the layer's extent in every known reference "
                      "system and listing any in which the bounds would fall within the target area if "
                      "the layer was in this projection." );
}

QString QgsFindProjectionAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates a list of possible candidate coordinate reference systems for a layer with an unknown projection." );
}

QgsFindProjectionAlgorithm *QgsFindProjectionAlgorithm::createInstance() const
{
  return new QgsFindProjectionAlgorithm();
}

void QgsFindProjectionAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterExtent( u"TARGET_AREA"_s, QObject::tr( "Target area for layer" ) ) );

  // deprecated
  auto crsParam = std::make_unique<QgsProcessingParameterCrs>( u"TARGET_AREA_CRS"_s, QObject::tr( "Target area CRS" ), QVariant(), true );
  crsParam->setFlags( crsParam->flags() | Qgis::ProcessingParameterFlag::Hidden );
  addParameter( crsParam.release() );

  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "CRS candidates" ), Qgis::ProcessingSourceType::Vector ) );
}

QVariantMap QgsFindProjectionAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  const QgsRectangle extent = parameterAsExtent( parameters, u"TARGET_AREA"_s, context );
  QgsCoordinateReferenceSystem targetCrs = parameterAsExtentCrs( parameters, u"TARGET_AREA"_s, context );
  if ( parameters.contains( u"TARGET_AREA_CRS"_s ) )
  {
    QgsCoordinateReferenceSystem crs = parameterAsCrs( parameters, u"TARGET_AREA_CRS"_s, context );
    if ( crs.isValid() )
    {
      targetCrs = crs;
    }
  }

  QgsGeometry targetGeometry = QgsGeometry::fromRect( extent );

  QgsFields fields;
  fields.append( QgsField( "auth_id", QMetaType::Type::QString ) );

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, fields, Qgis::WkbType::NoGeometry, QgsCoordinateReferenceSystem() ) );
  if ( !sink )
  {
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );
  }

  std::unique_ptr<QgsGeometryEngine> engine( QgsGeometry::createGeometryEngine( targetGeometry.constGet() ) );
  engine->prepareGeometry();

  QgsGeometry layerBounds = QgsGeometry::fromRect( source->sourceExtent() );

  const QList< long > crsesToCheck = QgsCoordinateReferenceSystem::validSrsIds();
  double step = crsesToCheck.count() > 0 ? 100.0 / crsesToCheck.count() : 0;
  long foundResults = 0;
  long i = 0;

  QgsCoordinateTransformContext transformContext;

  for ( long srsId : crsesToCheck )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    QgsCoordinateReferenceSystem candidateCrs = QgsCoordinateReferenceSystem::fromSrsId( srsId );
    if ( !candidateCrs.isValid() )
    {
      continue;
    }

    QgsCoordinateTransform transformCandidate = QgsCoordinateTransform( candidateCrs, targetCrs, transformContext );
    transformCandidate.setBallparkTransformsAreAppropriate( true );
    transformCandidate.disableFallbackOperationHandler( true );
    QgsGeometry transformedBounds = QgsGeometry( layerBounds );

    try
    {
      if ( transformedBounds.transform( transformCandidate ) != Qgis::GeometryOperationResult::Success )
      {
        continue;
      }
    }
    catch ( QgsCsException & )
    {
      continue;
    }

    try
    {
      if ( engine->intersects( transformedBounds.constGet() ) )
      {
        feedback->pushInfo( QObject::tr( "Found candidate CRS: %1." ).arg( candidateCrs.authid() ) );
        QgsFeature f = QgsFeature( fields );
        f.setAttributes( QgsAttributes() << candidateCrs.authid() );
        if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );

        foundResults++;
      }
    }
    catch ( QgsCsException & )
    {
      continue;
    }

    feedback->setProgress( i * step );
    i++;
  }

  if ( foundResults == 0 )
  {
    feedback->reportError( QObject::tr( "No matching projections found." ) );
  }

  sink->finalize();

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, dest );
  return outputs;
}

///@endcond
