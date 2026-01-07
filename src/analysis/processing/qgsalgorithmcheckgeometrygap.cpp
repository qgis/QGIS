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
#include "qgsvectordataproviderfeaturepool.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE

QString QgsGeometryCheckGapAlgorithm::name() const
{
  return u"checkgeometrygap"_s;
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
  return u"checkgeometry"_s;
}

QString QgsGeometryCheckGapAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm checks the gaps between polygons.\n"
                      "Gaps with an area smaller than the gap threshold are errors.\n\n"
                      "If an allowed gaps layer is given, the gaps contained in polygons from this layer will be ignored.\n"
                      "An optional buffer can be applied to the allowed gaps.\n\n"
                      "The neighbors output layer is needed for the fix geometry (gaps) algorithm. It is a 1-N "
                      "relational table for correspondence between a gap and the unique id of its neighbor features." );
}

Qgis::ProcessingAlgorithmFlags QgsGeometryCheckGapAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::NoThreading | Qgis::ProcessingAlgorithmFlag::RequiresProject;
}

QgsGeometryCheckGapAlgorithm *QgsGeometryCheckGapAlgorithm::createInstance() const
{
  return new QgsGeometryCheckGapAlgorithm();
}

void QgsGeometryCheckGapAlgorithm::initAlgorithm( const QVariantMap &configuration )
{
  Q_UNUSED( configuration )

  addParameter( new QgsProcessingParameterFeatureSource(
    u"INPUT"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon )
  ) );
  addParameter( new QgsProcessingParameterField(
    u"UNIQUE_ID"_s, QObject::tr( "Unique feature identifier" ), QString(), u"INPUT"_s
  ) );
  addParameter( new QgsProcessingParameterNumber(
    u"GAP_THRESHOLD"_s, QObject::tr( "Gap threshold" ), Qgis::ProcessingNumberParameterType::Double, 0, false, 0.0
  ) );

  // Optional allowed gaps layer and buffer value
  addParameter( new QgsProcessingParameterVectorLayer(
    u"ALLOWED_GAPS_LAYER"_s, QObject::tr( "Allowed gaps layer" ),
    QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon ), QVariant(), true
  ) );
  addParameter( new QgsProcessingParameterDistance(
    u"ALLOWED_GAPS_BUFFER"_s, QObject::tr( "Allowed gaps buffer" ), QVariant(), u"ALLOWED_GAPS_LAYER"_s, true, 0.0
  ) );

  addParameter( new QgsProcessingParameterFeatureSink(
    u"NEIGHBORS"_s, QObject::tr( "Neighbors layer" ), Qgis::ProcessingSourceType::Vector
  ) );
  addParameter( new QgsProcessingParameterFeatureSink(
    u"ERRORS"_s, QObject::tr( "Gap errors" ), Qgis::ProcessingSourceType::VectorPoint, QVariant(), true, false
  ) );
  addParameter( new QgsProcessingParameterFeatureSink(
    u"OUTPUT"_s, QObject::tr( "Gap features" ), Qgis::ProcessingSourceType::VectorPolygon
  ) );

  auto tolerance = std::make_unique<QgsProcessingParameterNumber>(
    u"TOLERANCE"_s, QObject::tr( "Tolerance" ), Qgis::ProcessingNumberParameterType::Integer, 8, false, 1, 13
  );
  tolerance->setFlags( tolerance->flags() | Qgis::ProcessingParameterFlag::Advanced );
  tolerance->setHelp( QObject::tr( "The \"Tolerance\" advanced parameter defines the numerical precision of geometric operations, "
                                   "given as an integer n, meaning that any difference smaller than 10⁻ⁿ (in map units) is considered zero." ) );
  addParameter( tolerance.release() );
}

bool QgsGeometryCheckGapAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mTolerance = parameterAsInt( parameters, u"TOLERANCE"_s, context );

  return true;
}

QgsFields QgsGeometryCheckGapAlgorithm::outputFields()
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
  fields.append( QgsField( u"gc_errorid"_s, QMetaType::LongLong ) );
  return fields;
}

QVariantMap QgsGeometryCheckGapAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QString dest_output;
  QString dest_errors;
  QString dest_neighbors;
  const std::unique_ptr<QgsProcessingFeatureSource> input( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !input )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  QgsVectorLayer *allowedGapsLayer = parameterAsVectorLayer( parameters, u"ALLOWED_GAPS_LAYER"_s, context );

  const double allowedGapsBuffer = parameterAsDouble( parameters, u"ALLOWED_GAPS_BUFFER"_s, context );
  const double gapThreshold = parameterAsDouble( parameters, u"GAP_THRESHOLD"_s, context );

  const QgsFields fields = outputFields();

  const std::unique_ptr<QgsFeatureSink> sink_output( parameterAsSink(
    parameters, u"OUTPUT"_s, context, dest_output, fields, input->wkbType(), input->sourceCrs()
  ) );
  if ( !sink_output )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  const std::unique_ptr<QgsFeatureSink> sink_errors( parameterAsSink(
    parameters, u"ERRORS"_s, context, dest_errors, fields, Qgis::WkbType::Point, input->sourceCrs()
  ) );

  const QString uniqueIdFieldName( parameterAsString( parameters, u"UNIQUE_ID"_s, context ) );
  const int uniqueIdFieldIdx = input->fields().indexFromName( uniqueIdFieldName );
  if ( uniqueIdFieldIdx == -1 )
    throw QgsProcessingException( QObject::tr( "Missing field %1 in input layer" ).arg( uniqueIdFieldName ) );

  QgsFields neighborsFields = QgsFields();
  neighborsFields.append( QgsField( "gc_errorid", QMetaType::LongLong ) );
  neighborsFields.append( input->fields().at( uniqueIdFieldIdx ) );
  const std::unique_ptr<QgsFeatureSink> sink_neighbors( parameterAsSink(
    parameters, u"NEIGHBORS"_s, context, dest_neighbors, neighborsFields, Qgis::WkbType::NoGeometry
  ) );
  if ( !sink_neighbors )
    throw QgsProcessingException( invalidSinkError( parameters, u"NEIGHBORS"_s ) );

  QgsProcessingMultiStepFeedback multiStepFeedback( 3, feedback );

  QgsGeometryCheckContext checkContext = QgsGeometryCheckContext( mTolerance, input->sourceCrs(), context.transformContext(), context.project(), uniqueIdFieldIdx );

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
  else if ( res == QgsGeometryCheck::Result::GeometryOverlayError )
  {
    throw QgsProcessingException( QObject::tr( "Failed to perform geometry overlay operation." ) );
  }

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
        throw QgsProcessingException( writeFeatureError( sink_neighbors.get(), parameters, u"NEIGHBORS"_s ) );
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
    if ( !sink_output->addFeature( f, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink_output.get(), parameters, u"OUTPUT"_s ) );

    f.setGeometry( QgsGeometry::fromPoint( QgsPoint( error->location().x(), error->location().y() ) ) );
    if ( sink_errors && !sink_errors->addFeature( f, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink_errors.get(), parameters, u"ERRORS"_s ) );

    i++;
    feedback->setProgress( 100.0 * step * static_cast<double>( i ) );
  }

  // Place the point layer above the polygon layer
  if ( context.willLoadLayerOnCompletion( dest_output ) && context.willLoadLayerOnCompletion( dest_errors ) )
  {
    context.layerToLoadOnCompletionDetails( dest_errors ).layerSortKey = 1;
    context.layerToLoadOnCompletionDetails( dest_output ).layerSortKey = 0;
  }

  // cleanup memory of the pointed data
  for ( const QgsGeometryCheckError *error : checkErrors )
  {
    delete error;
  }

  QVariantMap outputs;
  outputs.insert( u"NEIGHBORS"_s, dest_neighbors );
  outputs.insert( u"OUTPUT"_s, dest_output );
  if ( sink_errors )
    outputs.insert( u"ERRORS"_s, dest_errors );

  return outputs;
}

///@endcond
