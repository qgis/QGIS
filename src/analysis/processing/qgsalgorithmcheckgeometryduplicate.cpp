/***************************************************************************
                        qgsalgorithmcheckgeometryduplicate.cpp
                        ---------------------
   begin                : April 2025
   copyright            : (C) 2025 by Jacky Volpes
   email                : jacky dot volpes at oslandia dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmcheckgeometryduplicate.h"
#include "qgsgeometrycheckcontext.h"
#include "qgsgeometrycheckerror.h"
#include "qgsgeometryduplicatecheck.h"
#include "qgspoint.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataproviderfeaturepool.h"

///@cond PRIVATE

QString QgsGeometryCheckDuplicateAlgorithm::name() const
{
  return QStringLiteral( "checkgeometryduplicate" );
}

QString QgsGeometryCheckDuplicateAlgorithm::displayName() const
{
  return QObject::tr( "Duplicated geometries" );
}

QString QgsGeometryCheckDuplicateAlgorithm::shortDescription() const
{
  return QObject::tr( "Detects duplicate geometries." );
}

QStringList QgsGeometryCheckDuplicateAlgorithm::tags() const
{
  return QObject::tr( "check,geometry,duplicate" ).split( ',' );
}

QString QgsGeometryCheckDuplicateAlgorithm::group() const
{
  return QObject::tr( "Check geometry" );
}

QString QgsGeometryCheckDuplicateAlgorithm::groupId() const
{
  return QStringLiteral( "checkgeometry" );
}

QString QgsGeometryCheckDuplicateAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm checks the duplicate geometries.\n"
                      "Duplicate geometries are errors." );
}

Qgis::ProcessingAlgorithmFlags QgsGeometryCheckDuplicateAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::NoThreading;
}

QgsGeometryCheckDuplicateAlgorithm *QgsGeometryCheckDuplicateAlgorithm::createInstance() const
{
  return new QgsGeometryCheckDuplicateAlgorithm();
}

void QgsGeometryCheckDuplicateAlgorithm::initAlgorithm( const QVariantMap &configuration )
{
  Q_UNUSED( configuration )

  addParameter(
    new QgsProcessingParameterFeatureSource(
      QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ),
      QList<int>()
        << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon )
        << static_cast<int>( Qgis::ProcessingSourceType::VectorLine )
        << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint )
    )
  );
  addParameter( new QgsProcessingParameterField(
    QStringLiteral( "UNIQUE_ID" ), QObject::tr( "Unique feature identifier" ), QString(), QStringLiteral( "INPUT" )
  ) );
  addParameter( new QgsProcessingParameterFeatureSink(
    QStringLiteral( "ERRORS" ), QObject::tr( "Duplicate geometries errors" ), Qgis::ProcessingSourceType::VectorPoint
  ) );
  addParameter( new QgsProcessingParameterFeatureSink(
    QStringLiteral( "OUTPUT" ), QObject::tr( "Duplicate geometries" ), Qgis::ProcessingSourceType::VectorAnyGeometry, QVariant(), true, false
  ) );

  std::unique_ptr<QgsProcessingParameterNumber> tolerance = std::make_unique<QgsProcessingParameterNumber>(
    QStringLiteral( "TOLERANCE" ), QObject::tr( "Tolerance" ), Qgis::ProcessingNumberParameterType::Integer, 8, false, 1, 13
  );
  tolerance->setFlags( tolerance->flags() | Qgis::ProcessingParameterFlag::Advanced );
  tolerance->setHelp( QObject::tr( "The \"Tolerance\" advanced parameter defines the numerical precision of geometric operations, "
                                   "given as an integer n, meaning that any difference smaller than 10⁻ⁿ (in map units) is considered zero." ) );
  addParameter( tolerance.release() );
}

bool QgsGeometryCheckDuplicateAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mTolerance = parameterAsInt( parameters, QStringLiteral( "TOLERANCE" ), context );

  return true;
}

QgsFields QgsGeometryCheckDuplicateAlgorithm::outputFields()
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


QVariantMap QgsGeometryCheckDuplicateAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QString dest_output;
  QString dest_errors;

  const std::unique_ptr<QgsProcessingFeatureSource> input( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !input )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  const QString uniqueIdFieldName( parameterAsString( parameters, QStringLiteral( "UNIQUE_ID" ), context ) );
  const int uniqueIdFieldIdx = input->fields().indexFromName( uniqueIdFieldName );
  if ( uniqueIdFieldIdx == -1 )
    throw QgsProcessingException( QObject::tr( "Missing field %1 in input layer" ).arg( uniqueIdFieldName ) );

  const QgsField uniqueIdField = input->fields().at( uniqueIdFieldIdx );

  QgsFields fields = outputFields();
  fields.append( uniqueIdField );

  const std::unique_ptr<QgsFeatureSink> sink_output( parameterAsSink(
    parameters, QStringLiteral( "OUTPUT" ), context, dest_output, fields, input->wkbType(), input->sourceCrs()
  ) );
  const std::unique_ptr<QgsFeatureSink> sink_errors( parameterAsSink(
    parameters, QStringLiteral( "ERRORS" ), context, dest_errors, fields, Qgis::WkbType::Point, input->sourceCrs()
  ) );
  if ( !sink_errors )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "ERRORS" ) ) );

  QgsProcessingMultiStepFeedback multiStepFeedback( 3, feedback );

  const QgsProject *project = QgsProject::instance();
  QgsGeometryCheckContext checkContext = QgsGeometryCheckContext( mTolerance, input->sourceCrs(), project->transformContext(), project );

  // Test detection
  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  const QgsGeometryDuplicateCheck check( &checkContext, QVariantMap() );

  multiStepFeedback.setCurrentStep( 1 );
  feedback->setProgressText( QObject::tr( "Preparing features…" ) );
  QMap<QString, QgsFeaturePool *> checkerFeaturePools;

  std::unique_ptr<QgsVectorLayer> inputLayer( input->materialize( QgsFeatureRequest() ) );
  QgsVectorDataProviderFeaturePool featurePool = QgsVectorDataProviderFeaturePool( inputLayer.get() );
  checkerFeaturePools.insert( inputLayer->id(), &featurePool );

  multiStepFeedback.setCurrentStep( 2 );
  feedback->setProgressText( QObject::tr( "Collecting errors…" ) );
  check.collectErrors( checkerFeaturePools, checkErrors, messages, feedback );

  multiStepFeedback.setCurrentStep( 3 );
  feedback->setProgressText( QObject::tr( "Exporting errors…" ) );
  const double step { checkErrors.size() > 0 ? 100.0 / checkErrors.size() : 1 };
  long i = 0;
  feedback->setProgress( 0.0 );

  for ( const QgsGeometryCheckError *error : checkErrors )
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
    if ( sink_output && !sink_output->addFeature( f, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink_output.get(), parameters, QStringLiteral( "OUTPUT" ) ) );

    f.setGeometry( QgsGeometry::fromPoint( QgsPoint( error->location().x(), error->location().y() ) ) );
    if ( !sink_errors->addFeature( f, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink_errors.get(), parameters, QStringLiteral( "ERRORS" ) ) );

    i++;
    feedback->setProgress( 100.0 * step * static_cast<double>( i ) );
  }

  // Place the point layer above the other layer
  if ( context.willLoadLayerOnCompletion( dest_output ) && context.willLoadLayerOnCompletion( dest_errors ) )
  {
    context.layerToLoadOnCompletionDetails( dest_errors ).layerSortKey = 0;
    context.layerToLoadOnCompletionDetails( dest_output ).layerSortKey = 1;
  }

  // cleanup memory of the pointed data
  for ( const QgsGeometryCheckError *error : checkErrors )
  {
    delete error;
  }

  QVariantMap outputs;
  if ( sink_output )
    outputs.insert( QStringLiteral( "OUTPUT" ), dest_output );
  outputs.insert( QStringLiteral( "ERRORS" ), dest_errors );

  return outputs;
}

///@endcond
