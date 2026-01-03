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
#include "qgsvectordataproviderfeaturepool.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE

QString QgsGeometryCheckContainedAlgorithm::name() const
{
  return u"checkgeometrycontained"_s;
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
  return u"checkgeometry"_s;
}

QString QgsGeometryCheckContainedAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm checks the input geometries contained in the polygons from the polygon layers list.\n"
                      "A polygon layer can be checked against itself.\n"
                      "Input features contained in the polygon layers features are errors.\n" );
}

Qgis::ProcessingAlgorithmFlags QgsGeometryCheckContainedAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::NoThreading | Qgis::ProcessingAlgorithmFlag::RequiresProject;
}

QgsGeometryCheckContainedAlgorithm *QgsGeometryCheckContainedAlgorithm::createInstance() const
{
  return new QgsGeometryCheckContainedAlgorithm();
}

void QgsGeometryCheckContainedAlgorithm::initAlgorithm( const QVariantMap &configuration )
{
  Q_UNUSED( configuration )

  addParameter( new QgsProcessingParameterFeatureSource(
    u"INPUT"_s, QObject::tr( "Input layer" ),
    QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint )
                 << static_cast<int>( Qgis::ProcessingSourceType::VectorLine )
                 << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon )
  ) );
  addParameter( new QgsProcessingParameterField(
    u"UNIQUE_ID"_s, QObject::tr( "Unique feature identifier" ), QString(), u"INPUT"_s
  ) );
  addParameter( new QgsProcessingParameterMultipleLayers(
    u"POLYGONS"_s, QObject::tr( "Polygon layers" ), Qgis::ProcessingSourceType::VectorPolygon
  ) );

  addParameter( new QgsProcessingParameterFeatureSink(
    u"ERRORS"_s, QObject::tr( "Errors from contained features" ), Qgis::ProcessingSourceType::VectorPoint
  ) );
  addParameter( new QgsProcessingParameterFeatureSink(
    u"OUTPUT"_s, QObject::tr( "Contained features" ), Qgis::ProcessingSourceType::VectorAnyGeometry, QVariant(), true, false
  ) );

  auto tolerance = std::make_unique<QgsProcessingParameterNumber>(
    u"TOLERANCE"_s, QObject::tr( "Tolerance" ), Qgis::ProcessingNumberParameterType::Integer, 8, false, 1, 13
  );

  tolerance->setFlags( tolerance->flags() | Qgis::ProcessingParameterFlag::Advanced );
  tolerance->setHelp( QObject::tr( "The \"Tolerance\" advanced parameter defines the numerical precision of geometric operations, "
                                   "given as an integer n, meaning that any difference smaller than 10⁻ⁿ (in map units) is considered zero." ) );
  addParameter( tolerance.release() );
}

bool QgsGeometryCheckContainedAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mTolerance = parameterAsInt( parameters, u"TOLERANCE"_s, context );
  return true;
}

QgsFields QgsGeometryCheckContainedAlgorithm::outputFields()
{
  QgsFields fields;
  fields.append( QgsField( u"gc_layerid"_s, QMetaType::QString ) );
  fields.append( QgsField( u"gc_layername"_s, QMetaType::QString ) );
  fields.append( QgsField( u"gc_partidx"_s, QMetaType::Int ) );
  fields.append( QgsField( u"gc_ringidx"_s, QMetaType::Int ) );
  fields.append( QgsField( u"gc_vertidx"_s, QMetaType::Int ) );
  fields.append( QgsField( u"gc_errorx"_s, QMetaType::Double ) );
  fields.append( QgsField( u"gc_errory"_s, QMetaType::Double ) );
  fields.append( QgsField( u"gc_error"_s, QMetaType::QString ) );
  return fields;
}


QVariantMap QgsGeometryCheckContainedAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const std::unique_ptr<QgsProcessingFeatureSource> input( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !input )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  QList<QgsMapLayer *> polygonLayers = parameterAsLayerList( parameters, u"POLYGONS"_s, context );
  if ( polygonLayers.isEmpty() )
    throw QgsProcessingException( invalidSourceError( parameters, u"POLYGONS"_s ) );

  const QString uniqueIdFieldName( parameterAsString( parameters, u"UNIQUE_ID"_s, context ) );
  const int uniqueIdFieldIdx = input->fields().indexFromName( uniqueIdFieldName );
  if ( uniqueIdFieldIdx == -1 )
    throw QgsProcessingException( QObject::tr( "Missing field %1 in input layer" ).arg( uniqueIdFieldName ) );

  const QgsField uniqueIdField = input->fields().at( uniqueIdFieldIdx );

  QgsFields fields = outputFields();
  fields.append( uniqueIdField );

  QString dest_output, dest_errors;
  const std::unique_ptr<QgsFeatureSink> sink_output( parameterAsSink(
    parameters, u"OUTPUT"_s, context, dest_output, fields, input->wkbType(), input->sourceCrs()
  ) );

  std::unique_ptr<QgsFeatureSink> sink_errors( parameterAsSink(
    parameters, u"ERRORS"_s, context, dest_errors, fields, Qgis::WkbType::Point, input->sourceCrs()
  ) );
  if ( !sink_errors )
    throw QgsProcessingException( invalidSinkError( parameters, u"ERRORS"_s ) );

  QgsProcessingMultiStepFeedback multiStepFeedback( 3, feedback );

  QgsGeometryCheckContext checkContext = QgsGeometryCheckContext( mTolerance, input->sourceCrs(), context.transformContext(), context.project(), uniqueIdFieldIdx );

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
  QgsGeometryCheck::Result res = check.collectErrors( checkerFeaturePools, checkErrors, messages, feedback );
  if ( res == QgsGeometryCheck::Result::Success )
  {
    feedback->pushInfo( QObject::tr( "Errors collected successfully." ) );
  }
  else if ( res == QgsGeometryCheck::Result::Canceled )
  {
    throw QgsProcessingException( QObject::tr( "Operation was canceled." ) );
  }
  else if ( res == QgsGeometryCheck::Result::DuplicatedUniqueId )
  {
    throw QgsProcessingException( QObject::tr( "Field '%1' contains non-unique values and can not be used as unique ID." ).arg( uniqueIdFieldName ) );
  }

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
      throw QgsProcessingException( writeFeatureError( sink_output.get(), parameters, u"OUTPUT"_s ) );

    f.setGeometry( QgsGeometry::fromPoint( QgsPoint( error->location().x(), error->location().y() ) ) );
    if ( !sink_errors->addFeature( f, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink_errors.get(), parameters, u"ERRORS"_s ) );

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
    outputs.insert( u"OUTPUT"_s, dest_output );
  outputs.insert( u"ERRORS"_s, dest_errors );

  return outputs;
}

///@endcond
