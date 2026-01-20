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

#include "qgsgeometryareacheck.h"
#include "qgsgeometrycheckcontext.h"
#include "qgsgeometrycheckerror.h"
#include "qgspoint.h"
#include "qgsvectordataproviderfeaturepool.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE

QString QgsGeometryCheckAreaAlgorithm::name() const
{
  return u"checkgeometryarea"_s;
}

QString QgsGeometryCheckAreaAlgorithm::displayName() const
{
  return QObject::tr( "Small polygons" );
}

QString QgsGeometryCheckAreaAlgorithm::shortDescription() const
{
  return QObject::tr( "Detects polygons smaller than a given area." );
}

QStringList QgsGeometryCheckAreaAlgorithm::tags() const
{
  return QObject::tr( "check,geometry,area" ).split( ',' );
}

QString QgsGeometryCheckAreaAlgorithm::group() const
{
  return QObject::tr( "Check geometry" );
}

QString QgsGeometryCheckAreaAlgorithm::groupId() const
{
  return u"checkgeometry"_s;
}

QString QgsGeometryCheckAreaAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm checks the areas of polygon geometries.\n"
                      "Areas below the area threshold are errors." );
}

Qgis::ProcessingAlgorithmFlags QgsGeometryCheckAreaAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::NoThreading | Qgis::ProcessingAlgorithmFlag::RequiresProject;
}

QgsGeometryCheckAreaAlgorithm *QgsGeometryCheckAreaAlgorithm::createInstance() const
{
  return new QgsGeometryCheckAreaAlgorithm();
}

void QgsGeometryCheckAreaAlgorithm::initAlgorithm( const QVariantMap &configuration )
{
  Q_UNUSED( configuration )

  addParameter(
    new QgsProcessingParameterFeatureSource(
      u"INPUT"_s, QObject::tr( "Input layer" ),
      QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon )
    )
  );
  addParameter( new QgsProcessingParameterField(
    u"UNIQUE_ID"_s, QObject::tr( "Unique feature identifier" ), QString(), u"INPUT"_s
  ) );
  addParameter( new QgsProcessingParameterArea(
    u"AREATHRESHOLD"_s, QObject::tr( "Area threshold" ), 0, u"INPUT"_s
  ) );

  addParameter( new QgsProcessingParameterFeatureSink(
    u"ERRORS"_s, QObject::tr( "Small polygons errors" ), Qgis::ProcessingSourceType::VectorPoint
  ) );
  addParameter( new QgsProcessingParameterFeatureSink(
    u"OUTPUT"_s, QObject::tr( "Small polygons features" ), Qgis::ProcessingSourceType::VectorPolygon, QVariant(), true, false
  ) );

  auto tolerance = std::make_unique<QgsProcessingParameterNumber>(
    u"TOLERANCE"_s, QObject::tr( "Tolerance" ), Qgis::ProcessingNumberParameterType::Integer, 8, false, 1, 13
  );
  tolerance->setFlags( tolerance->flags() | Qgis::ProcessingParameterFlag::Advanced );
  tolerance->setHelp( QObject::tr( "The \"Tolerance\" advanced parameter defines the numerical precision of geometric operations, "
                                   "given as an integer n, meaning that any difference smaller than 10⁻ⁿ (in map units) is considered zero." ) );
  addParameter( tolerance.release() );
}

bool QgsGeometryCheckAreaAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mTolerance = parameterAsInt( parameters, u"TOLERANCE"_s, context );
  return true;
}

QgsFields QgsGeometryCheckAreaAlgorithm::outputFields()
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

QVariantMap QgsGeometryCheckAreaAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QString dest_output;
  QString dest_errors;
  const std::unique_ptr<QgsProcessingFeatureSource> input( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !input )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  const QString uniqueIdFieldName( parameterAsString( parameters, u"UNIQUE_ID"_s, context ) );
  const int uniqueIdFieldIdx = input->fields().indexFromName( uniqueIdFieldName );
  if ( uniqueIdFieldIdx == -1 )
    throw QgsProcessingException( QObject::tr( "Missing field %1 in input layer" ).arg( uniqueIdFieldName ) );

  const QgsField uniqueIdField = input->fields().at( uniqueIdFieldIdx );

  QgsFields fields = outputFields();
  fields.append( uniqueIdField );

  const std::unique_ptr<QgsFeatureSink> sink_output( parameterAsSink(
    parameters, u"OUTPUT"_s, context, dest_output, fields, input->wkbType(), input->sourceCrs()
  ) );

  const std::unique_ptr<QgsFeatureSink> sink_errors( parameterAsSink(
    parameters, u"ERRORS"_s, context, dest_errors, fields, Qgis::WkbType::Point, input->sourceCrs()
  ) );
  if ( !sink_errors )
    throw QgsProcessingException( invalidSinkError( parameters, u"ERRORS"_s ) );

  QgsProcessingMultiStepFeedback multiStepFeedback( 3, feedback );

  QgsGeometryCheckContext checkContext = QgsGeometryCheckContext( mTolerance, input->sourceCrs(), context.transformContext(), context.project(), uniqueIdFieldIdx );

  // Test detection
  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  const double areaThreshold = parameterAsDouble( parameters, u"AREATHRESHOLD"_s, context );

  QVariantMap configurationCheck;
  configurationCheck.insert( "areaThreshold", areaThreshold );
  const QgsGeometryAreaCheck check( &checkContext, configurationCheck );

  multiStepFeedback.setCurrentStep( 1 );
  feedback->setProgressText( QObject::tr( "Preparing features…" ) );
  QMap<QString, QgsFeaturePool *> featurePools;

  std::unique_ptr<QgsVectorLayer> inputLayer( input->materialize( QgsFeatureRequest() ) );
  QgsVectorDataProviderFeaturePool featurePool = QgsVectorDataProviderFeaturePool( inputLayer.get() );
  featurePools.insert( inputLayer->id(), &featurePool );

  multiStepFeedback.setCurrentStep( 2 );
  feedback->setProgressText( QObject::tr( "Collecting errors…" ) );
  QgsGeometryCheck::Result res = check.collectErrors( featurePools, checkErrors, messages, feedback );
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
      throw QgsProcessingException( writeFeatureError( sink_output.get(), parameters, u"OUTPUT"_s ) );

    f.setGeometry( QgsGeometry::fromPoint( QgsPoint( error->location().x(), error->location().y() ) ) );
    if ( !sink_errors->addFeature( f, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink_errors.get(), parameters, u"ERRORS"_s ) );

    i++;
    feedback->setProgress( 100.0 * step * static_cast<double>( i ) );
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
