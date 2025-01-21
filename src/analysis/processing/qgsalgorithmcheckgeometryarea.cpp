/***************************************************************************
                        qgsalgorithmcheckgeometryarea.cpp
                        ---------------------
   begin                : November 2023
   copyright            : (C) 2023 by Loïc Bartoletti
   email                : loic dot bartoletti at oslandia dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmcheckgeometryarea.h"
#include "qgsgeometrycheckcontext.h"
#include "qgsgeometrycheckerror.h"
#include "qgsgeometryareacheck.h"
#include "qgspoint.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataproviderfeaturepool.h"

///@cond PRIVATE

auto QgsGeometryCheckAreaAlgorithm::name() const -> QString
{
  return QStringLiteral( "checkgeometryarea" );
}

auto QgsGeometryCheckAreaAlgorithm::displayName() const -> QString
{
  return QObject::tr( "Check geometry (Area)" );
}

auto QgsGeometryCheckAreaAlgorithm::tags() const -> QStringList
{
  return QObject::tr( "check,geometry,area" ).split( ',' );
}

auto QgsGeometryCheckAreaAlgorithm::group() const -> QString
{
  return QObject::tr( "Check geometry" );
}

auto QgsGeometryCheckAreaAlgorithm::groupId() const -> QString
{
  return QStringLiteral( "checkgeometry" );
}

auto QgsGeometryCheckAreaAlgorithm::shortHelpString() const -> QString
{
  return QObject::tr( "This algorithm checks the areas of polygon geometries." );
}

auto QgsGeometryCheckAreaAlgorithm::flags() const -> Qgis::ProcessingAlgorithmFlags
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::NoThreading;
}

auto QgsGeometryCheckAreaAlgorithm::createInstance() const -> QgsGeometryCheckAreaAlgorithm *
{
  return new QgsGeometryCheckAreaAlgorithm();
}

void QgsGeometryCheckAreaAlgorithm::initAlgorithm( const QVariantMap &configuration )
{
  Q_UNUSED( configuration )

  // inputs
  addParameter(
    new QgsProcessingParameterFeatureSource(
      QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ),
      QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon )
    )
  );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "UNIQUE_ID" ), QObject::tr( "Unique feature identifier" ), QString(), QStringLiteral( "INPUT" ) ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "AREATHRESHOLD" ), QObject::tr( "area threshold" ), Qgis::ProcessingNumberParameterType::Double, 0, false, 0.0 ) );

  // outputs
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "ERRORS" ), QObject::tr( "Error layer" ), Qgis::ProcessingSourceType::VectorPoint ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Output layer" ), Qgis::ProcessingSourceType::VectorPolygon ) );

  std::unique_ptr<QgsProcessingParameterNumber> tolerance = std::make_unique<QgsProcessingParameterNumber>( QStringLiteral( "TOLERANCE" ), QObject::tr( "Tolerance" ), Qgis::ProcessingNumberParameterType::Integer, 8, false, 1, 13 );
  tolerance->setFlags( tolerance->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( tolerance.release() );
}

auto QgsGeometryCheckAreaAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * ) -> bool
{
  mTolerance = parameterAsInt( parameters, QStringLiteral( "TOLERANCE" ), context );
  return true;
}

auto QgsGeometryCheckAreaAlgorithm::outputFields() -> QgsFields
{
  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "gc_layerid" ), QMetaType::QString ) );
  fields.append( QgsField( QStringLiteral( "gc_layername" ), QMetaType::QString ) );
  fields.append( QgsField( QStringLiteral( "gc_partidx" ), QMetaType::Int ) );
  fields.append( QgsField( QStringLiteral( "gc_ringidx" ), QMetaType::Int ) );
  fields.append( QgsField( QStringLiteral( "gc_vertidx" ), QMetaType::Int ) );
  fields.append( QgsField( QStringLiteral( "gc_errorx" ), QMetaType::Double ) );
  fields.append( QgsField( QStringLiteral( "gc_errory" ), QMetaType::Double ) );
  fields.append( QgsField( QStringLiteral( "gc_error" ), QMetaType::QString ) );
  return fields;
}


auto QgsGeometryCheckAreaAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) -> QVariantMap
{
  QString dest_output;
  QString dest_errors;
  QgsProcessingFeatureSource *input = parameterAsSource( parameters, QStringLiteral( "INPUT" ), context );

  QString uniqueIdFieldName( parameterAsString( parameters, QStringLiteral( "UNIQUE_ID" ), context ) );
  int uniqueIdFieldIdx = input->fields().indexFromName( uniqueIdFieldName );
  if ( uniqueIdFieldIdx == -1 )
    throw QgsProcessingException( QObject::tr( "Missing field %1 in input layer" ).arg( uniqueIdFieldName ) );

  const QgsField uniqueIdField = input->fields().at( uniqueIdFieldIdx );

  QgsFields fields = outputFields();
  fields.append( uniqueIdField );

  const std::unique_ptr<QgsFeatureSink> sink_output( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest_output, fields, input->wkbType(), input->sourceCrs() ) );
  if ( !sink_output )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  const std::unique_ptr<QgsFeatureSink> sink_errors( parameterAsSink( parameters, QStringLiteral( "ERRORS" ), context, dest_errors, fields, Qgis::WkbType::Point, input->sourceCrs() ) );
  if ( !sink_errors )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "ERRORS" ) ) );

  QgsProcessingMultiStepFeedback multiStepFeedback( 3, feedback );

  QgsProject *project = QgsProject::instance();

  const std::unique_ptr<QgsGeometryCheckContext> checkContext = std::make_unique<QgsGeometryCheckContext>( mTolerance, input->sourceCrs(), project->transformContext(), project );

  // Test detection
  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  const double areaThreshold = parameterAsDouble( parameters, QStringLiteral( "AREATHRESHOLD" ), context );

  QVariantMap configurationCheck;
  configurationCheck.insert( "areaThreshold", areaThreshold );
  const QgsGeometryAreaCheck check( checkContext.get(), configurationCheck );

  multiStepFeedback.setCurrentStep( 1 );
  feedback->setProgressText( QObject::tr( "Preparing features…" ) );
  QMap<QString, QgsFeaturePool *> featurePools;

  QgsVectorLayer *inputLayer = input->materialize( QgsFeatureRequest() );
  featurePools.insert( inputLayer->id(), new QgsVectorDataProviderFeaturePool( inputLayer ) );

  multiStepFeedback.setCurrentStep( 2 );
  feedback->setProgressText( QObject::tr( "Collecting errors…" ) );
  check.collectErrors( featurePools, checkErrors, messages, feedback );

  multiStepFeedback.setCurrentStep( 3 );
  feedback->setProgressText( QObject::tr( "Exporting errors…" ) );
  const double step { checkErrors.size() > 0 ? 100.0 / checkErrors.size() : 1 };
  long i = 0;
  feedback->setProgress( 0.0 );

  for ( QgsGeometryCheckError *error : checkErrors )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }
    QgsFeature f;
    QgsAttributes attrs = f.attributes();

    attrs << error->layerId()
          << inputLayer->name()
          << error->vidx().part
          << error->vidx().ring
          << error->vidx().vertex
          << error->location().x()
          << error->location().y()
          << error->value().toString()
          << inputLayer->getFeature( error->featureId() ).attribute( uniqueIdField.name() );
    f.setAttributes( attrs );

    f.setGeometry( error->geometry() );
    if ( !sink_output->addFeature( f, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink_output.get(), parameters, QStringLiteral( "OUTPUT" ) ) );

    f.setGeometry( QgsGeometry::fromPoint( QgsPoint( error->location().x(), error->location().y() ) ) );
    if ( !sink_errors->addFeature( f, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink_errors.get(), parameters, QStringLiteral( "ERRORS" ) ) );

    i++;
    feedback->setProgress( 100.0 * step * static_cast<double>( i ) );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest_output );
  outputs.insert( QStringLiteral( "ERRORS" ), dest_errors );

  return outputs;
}

///@endcond
