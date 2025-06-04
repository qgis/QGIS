/***************************************************************************
                        qgsalgorithmcheckgeometrygap.cpp
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

#include "qgsalgorithmcheckgeometrygap.h"
#include "qgsgeometrycheckcontext.h"
#include "qgsgeometrycheckerror.h"
#include "qgsgeometrygapcheck.h"
#include "qgspoint.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataproviderfeaturepool.h"

///@cond PRIVATE

QString QgsGeometryCheckGapAlgorithm::name() const
{
  return QStringLiteral( "checkgeometrygap" );
}

QString QgsGeometryCheckGapAlgorithm::displayName() const
{
  return QObject::tr( "Small gaps" );
}

QString QgsGeometryCheckGapAlgorithm::shortDescription() const
{
  return QObject::tr( "Detects gaps between polygons smaller than a given area." );
}

QStringList QgsGeometryCheckGapAlgorithm::tags() const
{
  return QObject::tr( "check,geometry,gap" ).split( ',' );
}

QString QgsGeometryCheckGapAlgorithm::group() const
{
  return QObject::tr( "Check geometry" );
}

QString QgsGeometryCheckGapAlgorithm::groupId() const
{
  return QStringLiteral( "checkgeometry" );
}

QString QgsGeometryCheckGapAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm checks the gaps between polygons.\n"
                      "Gaps with an area smaller than the gap threshold are errors.\n\n"
                      "If an allowed gaps layer is given, the gaps contained in polygons from this layer will be ignored.\n"
                      "An optional buffer can be applied to the allowed gaps.\n\n"
                      "The neighbors output layer is needed for the fix geometry (gaps) algorithm. It is a 1-N "
                      "relational table for correspondance between a gap and the unique id of its neighbor features." );
}

Qgis::ProcessingAlgorithmFlags QgsGeometryCheckGapAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::NoThreading;
}

QgsGeometryCheckGapAlgorithm *QgsGeometryCheckGapAlgorithm::createInstance() const
{
  return new QgsGeometryCheckGapAlgorithm();
}

void QgsGeometryCheckGapAlgorithm::initAlgorithm( const QVariantMap &configuration )
{
  Q_UNUSED( configuration )

  addParameter( new QgsProcessingParameterFeatureSource(
    QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon )
  ) );
  addParameter( new QgsProcessingParameterField(
    QStringLiteral( "UNIQUE_ID" ), QObject::tr( "Unique feature identifier" ), QString(), QStringLiteral( "INPUT" )
  ) );
  addParameter( new QgsProcessingParameterNumber(
    QStringLiteral( "GAP_THRESHOLD" ), QObject::tr( "Gap threshold" ), Qgis::ProcessingNumberParameterType::Double, 0, false, 0.0
  ) );

  // Optional allowed gaps layer and buffer value
  addParameter( new QgsProcessingParameterVectorLayer(
    QStringLiteral( "ALLOWED_GAPS_LAYER" ), QObject::tr( "Allowed gaps layer" ),
    QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon ), QVariant(), true
  ) );
  addParameter( new QgsProcessingParameterDistance(
    QStringLiteral( "ALLOWED_GAPS_BUFFER" ), QObject::tr( "Allowed gaps buffer" ), QVariant(), QStringLiteral( "ALLOWED_GAPS_LAYER" ), true, 0.0
  ) );

  addParameter( new QgsProcessingParameterFeatureSink(
    QStringLiteral( "NEIGHBORS" ), QObject::tr( "Neighbors layer" ), Qgis::ProcessingSourceType::Vector
  ) );
  addParameter( new QgsProcessingParameterFeatureSink(
    QStringLiteral( "ERRORS" ), QObject::tr( "Gap errors" ), Qgis::ProcessingSourceType::VectorPoint, QVariant(), true, false
  ) );
  addParameter( new QgsProcessingParameterFeatureSink(
    QStringLiteral( "OUTPUT" ), QObject::tr( "Gap features" ), Qgis::ProcessingSourceType::VectorPolygon
  ) );

  std::unique_ptr<QgsProcessingParameterNumber> tolerance = std::make_unique<QgsProcessingParameterNumber>(
    QStringLiteral( "TOLERANCE" ), QObject::tr( "Tolerance" ), Qgis::ProcessingNumberParameterType::Integer, 8, false, 1, 13
  );
  tolerance->setFlags( tolerance->flags() | Qgis::ProcessingParameterFlag::Advanced );
  tolerance->setHelp( QObject::tr( "The \"Tolerance\" advanced parameter defines the numerical precision of geometric operations, "
                                   "given as an integer n, meaning that any difference smaller than 10⁻ⁿ (in map units) is considered zero." ) );
  addParameter( tolerance.release() );
}

bool QgsGeometryCheckGapAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mTolerance = parameterAsInt( parameters, QStringLiteral( "TOLERANCE" ), context );

  return true;
}

QgsFields QgsGeometryCheckGapAlgorithm::outputFields()
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
  fields.append( QgsField( QStringLiteral( "gc_errorid" ), QMetaType::LongLong ) );
  return fields;
}


QVariantMap QgsGeometryCheckGapAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QString dest_output;
  QString dest_errors;
  QString dest_neighbors;
  const std::unique_ptr<QgsProcessingFeatureSource> input( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !input )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  QgsVectorLayer *allowedGapsLayer = parameterAsVectorLayer( parameters, QStringLiteral( "ALLOWED_GAPS_LAYER" ), context );

  const double allowedGapsBuffer = parameterAsDouble( parameters, QStringLiteral( "ALLOWED_GAPS_BUFFER" ), context );
  const double gapThreshold = parameterAsDouble( parameters, QStringLiteral( "GAP_THRESHOLD" ), context );

  const QgsFields fields = outputFields();

  const std::unique_ptr<QgsFeatureSink> sink_output( parameterAsSink(
    parameters, QStringLiteral( "OUTPUT" ), context, dest_output, fields, input->wkbType(), input->sourceCrs()
  ) );

  const std::unique_ptr<QgsFeatureSink> sink_errors( parameterAsSink(
    parameters, QStringLiteral( "ERRORS" ), context, dest_errors, fields, Qgis::WkbType::Point, input->sourceCrs()
  ) );
  if ( !sink_errors )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "ERRORS" ) ) );

  QString uniqueIdFieldName( parameterAsString( parameters, QStringLiteral( "UNIQUE_ID" ), context ) );
  int uniqueIdFieldIdx = input->fields().indexFromName( uniqueIdFieldName );
  if ( uniqueIdFieldIdx == -1 )
    throw QgsProcessingException( QObject::tr( "Missing field %1 in input layer" ).arg( uniqueIdFieldName ) );

  QgsFields neighborsFields = QgsFields();
  neighborsFields.append( QgsField( "gc_errorid", QMetaType::LongLong ) );
  neighborsFields.append( input->fields().at( uniqueIdFieldIdx ) );
  const std::unique_ptr<QgsFeatureSink> sink_neighbors( parameterAsSink(
    parameters, QStringLiteral( "NEIGHBORS" ), context, dest_neighbors, neighborsFields, Qgis::WkbType::NoGeometry
  ) );
  if ( !sink_neighbors )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "NEIGHBORS" ) ) );

  QgsProcessingMultiStepFeedback multiStepFeedback( 3, feedback );

  const QgsProject *project = QgsProject::instance();

  QgsGeometryCheckContext checkContext = QgsGeometryCheckContext( mTolerance, input->sourceCrs(), project->transformContext(), project );

  // Test detection
  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QVariantMap configurationCheck;
  configurationCheck.insert( "gapThreshold", gapThreshold );
  configurationCheck.insert( "allowedGapsEnabled", allowedGapsLayer != nullptr );
  if ( allowedGapsLayer )
  {
    configurationCheck.insert( "allowedGapsLayer", allowedGapsLayer->id() );
    configurationCheck.insert( "allowedGapsBuffer", allowedGapsBuffer );
  }
  QgsGeometryGapCheck check( &checkContext, configurationCheck );
  check.prepare( &checkContext, configurationCheck );

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
  double step { checkErrors.size() > 0 ? 100.0 / checkErrors.size() : 1 };
  long long i = 0;
  feedback->setProgress( 0.0 );

  for ( const QgsGeometryCheckError *error : checkErrors )
  {
    if ( feedback->isCanceled() )
      break;

    const QgsGeometryGapCheckError *gapError = dynamic_cast<const QgsGeometryGapCheckError *>( error );
    if ( !gapError )
      break;

    const QgsFeatureIds neighborIds = gapError->neighbors()[inputLayer->id()];
    for ( QgsFeatureId neighborId : neighborIds )
    {
      QgsFeature neighborFeature;
      neighborFeature.setAttributes(
        QgsAttributes() << i
                        << inputLayer->getFeature( neighborId ).attribute( uniqueIdFieldIdx )
      );
      if ( !sink_neighbors->addFeature( neighborFeature, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink_neighbors.get(), parameters, QStringLiteral( "NEIGHBORS" ) ) );
    }

    QgsFeature f;
    QgsAttributes attrs = f.attributes();
    attrs
      << inputLayer->id()
      << inputLayer->name()
      << error->vidx().part
      << error->vidx().ring
      << error->vidx().vertex
      << error->location().x()
      << error->location().y()
      << error->value().toString()
      << i;
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

  // Place the point layer above the polygon layer
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
  outputs.insert( QStringLiteral( "NEIGHBORS" ), dest_neighbors );
  if ( sink_output )
    outputs.insert( QStringLiteral( "OUTPUT" ), dest_output );
  outputs.insert( QStringLiteral( "ERRORS" ), dest_errors );

  return outputs;
}

///@endcond
