/***************************************************************************
                         qgsalgorithmcoveragevalidate.cpp
                         ---------------------
    begin                : October 2023
    copyright            : (C) 2023 by Nyall Dawson
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


#include "qgsalgorithmcoveragevalidate.h"

#include "qgsgeometrycollection.h"
#include "qgsgeos.h"
#include "qgsprocessingoutputs.h"

///@cond PRIVATE

QString QgsCoverageValidateAlgorithm::name() const
{
  return u"coveragevalidate"_s;
}

QString QgsCoverageValidateAlgorithm::displayName() const
{
  return QObject::tr( "Validate coverage" );
}

QStringList QgsCoverageValidateAlgorithm::tags() const
{
  return QObject::tr( "validity,overlaps,gaps,topological,boundary" ).split( ',' );
}

QString QgsCoverageValidateAlgorithm::group() const
{
  return QObject::tr( "Vector coverage" );
}

QString QgsCoverageValidateAlgorithm::groupId() const
{
  return u"vectorcoverage"_s;
}

void QgsCoverageValidateAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon ) ) );
  auto gapWidthParam = std::make_unique<QgsProcessingParameterDistance>( u"GAP_WIDTH"_s, QObject::tr( "Gap width" ), 0.0, u"INPUT"_s, false, 0, 10000000.0 );
  gapWidthParam->setHelp( QObject::tr( "The maximum width of gaps to detect" ) );
  addParameter( gapWidthParam.release() );

  addParameter( new QgsProcessingParameterFeatureSink( u"INVALID_EDGES"_s, QObject::tr( "Invalid edges" ), Qgis::ProcessingSourceType::VectorLine, QVariant(), true, true ) );
  addOutput( new QgsProcessingOutputBoolean( u"IS_VALID"_s, QObject::tr( "Coverage is valid" ) ) );
}

QString QgsCoverageValidateAlgorithm::shortDescription() const
{
  return QObject::tr( "Analyzes a coverage of polygon features to find places where the assumption of exactly matching edges is not met." );
}

QString QgsCoverageValidateAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm analyzes a coverage (represented as a set of polygon features "
                      "with exactly matching edge geometry) to find places where the "
                      "assumption of exactly matching edges is not met.\n\n"
                      "Invalidity includes polygons that overlap "
                      "or that have gaps smaller than the specified gap width." );
}

QgsCoverageValidateAlgorithm *QgsCoverageValidateAlgorithm::createInstance() const
{
  return new QgsCoverageValidateAlgorithm();
}

QVariantMap QgsCoverageValidateAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  const double gapWidth = parameterAsDouble( parameters, u"GAP_WIDTH"_s, context );

  QString sinkId;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"INVALID_EDGES"_s, context, sinkId, QgsFields(), Qgis::WkbType::LineString, source->sourceCrs() ) );
  if ( !sink && parameters.value( u"INVALID_EDGES"_s ).isValid() )
    throw QgsProcessingException( invalidSinkError( parameters, u"INVALID_EDGES"_s ) );

  QgsGeometryCollection collection;

  const long count = source->featureCount();
  if ( count > 0 )
  {
    collection.reserve( count );
  }

  const double step = count > 0 ? 100.0 / count : 1;
  int current = 0;

  feedback->pushInfo( QObject::tr( "Collecting features" ) );

  QgsFeature inFeature;
  QgsFeatureIterator features = source->getFeatures();
  while ( features.nextFeature( inFeature ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    if ( inFeature.hasGeometry() )
    {
      collection.addGeometry( inFeature.geometry().constGet()->clone() );
    }

    feedback->setProgress( current * step * 0.2 );
    current++;
  }

  feedback->pushInfo( QObject::tr( "Validating coverage" ) );

  QgsGeos geos( &collection );
  QString error;
  std::unique_ptr<QgsAbstractGeometry> invalidEdges;
  Qgis::CoverageValidityResult result;
  try
  {
    result = geos.validateCoverage( gapWidth, &invalidEdges, &error );
  }
  catch ( QgsNotSupportedException &e )
  {
    throw QgsProcessingException( e.what() );
  }

  switch ( result )
  {
    case Qgis::CoverageValidityResult::Invalid:
      feedback->reportError( QObject::tr( "Coverage is not valid" ) );
      if ( invalidEdges )
      {
        if ( sink )
        {
          for ( auto partsIt = invalidEdges->const_parts_begin(); partsIt != invalidEdges->const_parts_end(); ++partsIt )
          {
            QgsFeature outFeature;
            outFeature.setGeometry( QgsGeometry( *partsIt ? ( *partsIt )->clone() : nullptr ) );
            if ( !sink->addFeature( outFeature, QgsFeatureSink::FastInsert ) )
              throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
          }
        }
      }
      break;
    case Qgis::CoverageValidityResult::Valid:
      feedback->pushInfo( QObject::tr( "Coverage is valid" ) );
      break;
    case Qgis::CoverageValidityResult::Error:
      if ( !error.isEmpty() )
        throw QgsProcessingException( QObject::tr( "An error occurred validating coverage: %1" ).arg( error ) );
      else
        throw QgsProcessingException( QObject::tr( "An error occurred validating coverage" ) );
  }

  feedback->setProgress( 100 );
  if ( sink )
    sink->finalize();

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, sinkId );
  outputs.insert( u"IS_VALID"_s, result == Qgis::CoverageValidityResult::Valid );
  return outputs;
}

///@endcond
