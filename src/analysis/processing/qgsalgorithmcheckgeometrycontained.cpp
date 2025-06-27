/***************************************************************************
                        qgsalgorithmcheckgeometrycontained.cpp
                        ---------------------
   begin                : January 2025
   copyright            : (C) 2024 by Jacky Volpes
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

#include "qgsalgorithmcheckgeometrycontained.h"
#include "qgsgeometrycheckcontext.h"
#include "qgsgeometrycheckerror.h"
#include "qgsgeometrycontainedcheck.h"
#include "qgspoint.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataproviderfeaturepool.h"

///@cond PRIVATE

QString QgsGeometryCheckContainedAlgorithm::name() const
{
  return QStringLiteral( "checkgeometrycontained" );
}

QString QgsGeometryCheckContainedAlgorithm::displayName() const
{
  return QObject::tr( "Features inside polygon" );
}

QString QgsGeometryCheckContainedAlgorithm::shortDescription() const
{
  return QObject::tr( "Detects features contained inside polygons from a list of polygon layers." );
}

QStringList QgsGeometryCheckContainedAlgorithm::tags() const
{
  return QObject::tr( "check,geometry,contained" ).split( ',' );
}

QString QgsGeometryCheckContainedAlgorithm::group() const
{
  return QObject::tr( "Check geometry" );
}

QString QgsGeometryCheckContainedAlgorithm::groupId() const
{
  return QStringLiteral( "checkgeometry" );
}

QString QgsGeometryCheckContainedAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm checks the input geometries contained in the polygons from the polygon layers list.\n"
                      "A polygon layer can be checked against itself.\n"
                      "Input features contained in the polygon layers features are errors.\n" );
}

Qgis::ProcessingAlgorithmFlags QgsGeometryCheckContainedAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::NoThreading;
}

QgsGeometryCheckContainedAlgorithm *QgsGeometryCheckContainedAlgorithm::createInstance() const
{
  return new QgsGeometryCheckContainedAlgorithm();
}

void QgsGeometryCheckContainedAlgorithm::initAlgorithm( const QVariantMap &configuration )
{
  Q_UNUSED( configuration )

  // inputs
  addParameter( new QgsProcessingParameterFeatureSource(
    QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ),
    QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint )
                 << static_cast<int>( Qgis::ProcessingSourceType::VectorLine )
                 << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon )
  ) );
  addParameter( new QgsProcessingParameterField(
    QStringLiteral( "UNIQUE_ID" ), QObject::tr( "Unique feature identifier" ), QString(), QStringLiteral( "INPUT" )
  ) );
  addParameter( new QgsProcessingParameterMultipleLayers(
    QStringLiteral( "POLYGONS" ), QObject::tr( "Polygon layers" ), Qgis::ProcessingSourceType::VectorPolygon
  ) );

  // outputs
  addParameter( new QgsProcessingParameterFeatureSink(
    QStringLiteral( "OUTPUT" ), QObject::tr( "Errors from contained features" ), Qgis::ProcessingSourceType::VectorAnyGeometry, QVariant(), true, false
  ) );
  addParameter( new QgsProcessingParameterFeatureSink(
    QStringLiteral( "ERRORS" ), QObject::tr( "Contained features" ), Qgis::ProcessingSourceType::VectorPoint
  ) );

  std::unique_ptr<QgsProcessingParameterNumber> tolerance = std::make_unique<QgsProcessingParameterNumber>(
    QStringLiteral( "TOLERANCE" ), QObject::tr( "Tolerance" ), Qgis::ProcessingNumberParameterType::Integer, 8, false, 1, 13
  );

  tolerance->setFlags( tolerance->flags() | Qgis::ProcessingParameterFlag::Advanced );
  tolerance->setHelp( QObject::tr( "The \"Tolerance\" advanced parameter defines the numerical precision of geometric operations, "
                                   "given as an integer n, meaning that any difference smaller than 10⁻ⁿ (in map units) is considered zero." ) );
  addParameter( tolerance.release() );
}

bool QgsGeometryCheckContainedAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mTolerance = parameterAsInt( parameters, QStringLiteral( "TOLERANCE" ), context );
  return true;
}

QgsFields QgsGeometryCheckContainedAlgorithm::outputFields()
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


QVariantMap QgsGeometryCheckContainedAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const std::unique_ptr<QgsProcessingFeatureSource> input( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !input )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  QList<QgsMapLayer *> polygonLayers = parameterAsLayerList( parameters, QStringLiteral( "POLYGONS" ), context );
  if ( polygonLayers.isEmpty() )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "POLYGONS" ) ) );


  const QString uniqueIdFieldName( parameterAsString( parameters, QStringLiteral( "UNIQUE_ID" ), context ) );
  const int uniqueIdFieldIdx = input->fields().indexFromName( uniqueIdFieldName );
  if ( uniqueIdFieldIdx == -1 )
    throw QgsProcessingException( QObject::tr( "Missing field %1 in input layer" ).arg( uniqueIdFieldName ) );

  const QgsField uniqueIdField = input->fields().at( uniqueIdFieldIdx );

  QgsFields fields = outputFields();
  fields.append( uniqueIdField );

  QString dest_output, dest_errors;
  const std::unique_ptr<QgsFeatureSink> sink_output( parameterAsSink(
    parameters, QStringLiteral( "OUTPUT" ), context, dest_output, fields, input->wkbType(), input->sourceCrs()
  ) );

  std::unique_ptr<QgsFeatureSink> sink_errors( parameterAsSink(
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

  const QgsGeometryContainedCheck check( &checkContext, QVariantMap() );

  multiStepFeedback.setCurrentStep( 1 );
  feedback->setProgressText( QObject::tr( "Preparing features…" ) );
  QMap<QString, QgsFeaturePool *> checkerFeaturePools;
  QList<std::shared_ptr<QgsVectorDataProviderFeaturePool>> featurePools;

  std::unique_ptr<QgsVectorLayer> inputLayer( input->materialize( QgsFeatureRequest() ) );
  featurePools << std::make_shared<QgsVectorDataProviderFeaturePool>( inputLayer.get() );
  checkerFeaturePools.insert( inputLayer->id(), featurePools.last().get() );

  for ( QgsMapLayer *polygonLayer : polygonLayers )
  {
    auto vlayer = dynamic_cast<QgsVectorLayer *>( polygonLayer );
    if ( vlayer && vlayer->geometryType() == Qgis::GeometryType::Polygon )
    {
      featurePools << std::make_shared<QgsVectorDataProviderFeaturePool>( vlayer );
      checkerFeaturePools.insert( vlayer->id(), featurePools.last().get() );
    }
  }

  multiStepFeedback.setCurrentStep( 2 );
  feedback->setProgressText( QObject::tr( "Collecting errors…" ) );
  check.collectErrors( checkerFeaturePools, checkErrors, messages, feedback );

  multiStepFeedback.setCurrentStep( 3 );
  feedback->setProgressText( QObject::tr( "Exporting errors…" ) );
  const double step { checkErrors.size() > 0 ? 100.0 / checkErrors.size() : 1 };
  long i = 0;
  feedback->setProgress( 0.0 );

  QVariantList uniqueIds;
  for ( const QgsGeometryCheckError *error : checkErrors )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }
    QgsFeature f;
    QgsAttributes attrs = f.attributes();

    // the geometry_checker adds errors from input + polygon layers comapred together, here
    // we only want errors from the input layer.
    if ( error->layerId() != inputLayer->id() )
    {
      continue;
    }

    // if the user wants to check a polygon layer against itself we avoid adding duplicate errors.
    QVariant uniqueId = inputLayer->getFeature( error->featureId() ).attribute( uniqueIdField.name() );
    if ( uniqueIds.contains( uniqueId ) )
    {
      continue;
    }
    else
    {
      uniqueIds << uniqueId;
    }

    attrs << error->layerId()
          << inputLayer->name()
          << error->vidx().part
          << error->vidx().ring
          << error->vidx().vertex
          << error->location().x()
          << error->location().y()
          << error->value().toString()
          << uniqueId;
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
