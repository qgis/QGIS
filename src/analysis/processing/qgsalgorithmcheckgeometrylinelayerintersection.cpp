/***************************************************************************
                        qgsalgorithmcheckgeometrylinelayerintersection.cpp
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

#include "qgsalgorithmcheckgeometrylinelayerintersection.h"
#include "qgsgeometrycheckcontext.h"
#include "qgsgeometrycheckerror.h"
#include "qgsgeometrylinelayerintersectioncheck.h"
#include "qgspoint.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataproviderfeaturepool.h"

///@cond PRIVATE

QString QgsGeometryCheckLineLayerIntersectionAlgorithm::name() const
{
  return QStringLiteral( "checkgeometrylinelayerintersection" );
}

QString QgsGeometryCheckLineLayerIntersectionAlgorithm::displayName() const
{
  return QObject::tr( "Check Geometry (Line layer intersection)" );
}

QStringList QgsGeometryCheckLineLayerIntersectionAlgorithm::tags() const
{
  return QObject::tr( "check,geometry,line,layer,intersection" ).split( ',' );
}

QString QgsGeometryCheckLineLayerIntersectionAlgorithm::group() const
{
  return QObject::tr( "Check geometry" );
}

QString QgsGeometryCheckLineLayerIntersectionAlgorithm::groupId() const
{
  return QStringLiteral( "checkgeometry" );
}

QString QgsGeometryCheckLineLayerIntersectionAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm checks the line intersection of geometry (linestring) with another layer." );
}

Qgis::ProcessingAlgorithmFlags QgsGeometryCheckLineLayerIntersectionAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::NoThreading;
}

QgsGeometryCheckLineLayerIntersectionAlgorithm *QgsGeometryCheckLineLayerIntersectionAlgorithm::createInstance() const
{
  return new QgsGeometryCheckLineLayerIntersectionAlgorithm();
}

void QgsGeometryCheckLineLayerIntersectionAlgorithm::initAlgorithm( const QVariantMap &configuration )
{
  Q_UNUSED( configuration )

  addParameter( new QgsProcessingParameterFeatureSource(
    QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ), QList< int >() << static_cast<int>( Qgis::ProcessingSourceType::VectorLine )
  ) );
  addParameter( new QgsProcessingParameterFeatureSource(
    QStringLiteral( "INTERSECTION_LAYER" ),
    QObject::tr( "Intersection layer" ),
    QList< int >() << static_cast<int>( Qgis::ProcessingSourceType::VectorLine ) << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon )
  ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "ERRORS" ), QObject::tr( "Errors layer" ), Qgis::ProcessingSourceType::VectorPoint ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Output layer" ), Qgis::ProcessingSourceType::VectorLine ) );

  std::unique_ptr< QgsProcessingParameterNumber > tolerance = std::make_unique< QgsProcessingParameterNumber >(
    QStringLiteral( "TOLERANCE" ), QObject::tr( "Tolerance" ), Qgis::ProcessingNumberParameterType::Integer, 8, false, 1, 13
  );
  tolerance->setFlags( tolerance->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( tolerance.release() );
}

bool QgsGeometryCheckLineLayerIntersectionAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mTolerance = parameterAsInt( parameters, QStringLiteral( "TOLERANCE" ), context );

  return true;
}

QgsFeaturePool *QgsGeometryCheckLineLayerIntersectionAlgorithm::createFeaturePool( QgsVectorLayer *layer, bool selectedOnly ) const
{
  return new QgsVectorDataProviderFeaturePool( layer, selectedOnly );
}

static QgsFields outputFields()
{
  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "gc_layerid" ), QMetaType::QString ) );
  fields.append( QgsField( QStringLiteral( "gc_layername" ), QMetaType::QString ) );
  fields.append( QgsField( QStringLiteral( "gc_featid" ), QMetaType::Int ) );
  fields.append( QgsField( QStringLiteral( "gc_partidx" ), QMetaType::Int ) );
  fields.append( QgsField( QStringLiteral( "gc_ringidx" ), QMetaType::Int ) );
  fields.append( QgsField( QStringLiteral( "gc_vertidx" ), QMetaType::Int ) );
  fields.append( QgsField( QStringLiteral( "gc_errorx" ), QMetaType::Double ) );
  fields.append( QgsField( QStringLiteral( "gc_errory" ), QMetaType::Double ) );
  fields.append( QgsField( QStringLiteral( "gc_error" ), QMetaType::QString ) );
  return fields;
}


QVariantMap QgsGeometryCheckLineLayerIntersectionAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QString dest_output;
  QString dest_errors;

  std::unique_ptr< QgsProcessingFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  std::unique_ptr< QgsProcessingFeatureSource > intersection_layer( parameterAsSource( parameters, QStringLiteral( "INTERSECTION_LAYER" ), context ) );
  if ( !intersection_layer )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INTERSECTION_LAYER" ) ) );

  mInputLayer.reset( source->materialize( QgsFeatureRequest() ) );
  mIntersectionLayer.reset( intersection_layer->materialize( QgsFeatureRequest() ) );

  if ( !mInputLayer.get() )
    throw QgsProcessingException( QObject::tr( "Could not load source layer for INPUT" ) );

  if ( !mIntersectionLayer.get() )
    throw QgsProcessingException( QObject::tr( "Could not load source layer for INTERSECTION_LAYER" ) );

  QgsFields fields = outputFields();

  std::unique_ptr< QgsFeatureSink > sink_output( parameterAsSink(
    parameters, QStringLiteral( "OUTPUT" ), context, dest_output, fields, mInputLayer->wkbType(), mInputLayer->sourceCrs()
  ) );
  if ( !sink_output )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  std::unique_ptr< QgsFeatureSink > sink_errors( parameterAsSink(
    parameters, QStringLiteral( "ERRORS" ), context, dest_errors, fields, Qgis::WkbType::Point, mInputLayer->sourceCrs()
  ) );
  if ( !sink_errors )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "ERRORS" ) ) );

  QgsProcessingMultiStepFeedback multiStepFeedback( 3, feedback );

  QgsProject *project = mInputLayer->project() ? mInputLayer->project() : QgsProject::instance();
  std::unique_ptr<QgsGeometryCheckContext> checkContext = std::make_unique<QgsGeometryCheckContext>(
    mTolerance, mInputLayer->sourceCrs(), project->transformContext(), project
  );

  // Test detection
  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  QVariantMap configurationCheck;
  configurationCheck.insert( "checkLayer", mIntersectionLayer->id() );
  const QgsGeometryLineLayerIntersectionCheck check( checkContext.get(), configurationCheck );

  multiStepFeedback.setCurrentStep( 1 );
  feedback->setProgressText( QObject::tr( "Preparing features…" ) );
  QMap<QString, QgsFeaturePool *> featurePools;
  featurePools.insert( mInputLayer->id(), createFeaturePool( mInputLayer.get() ) );
  featurePools.insert( mIntersectionLayer->id(), createFeaturePool( mIntersectionLayer.get() ) );

  multiStepFeedback.setCurrentStep( 2 );
  feedback->setProgressText( QObject::tr( "Collecting errors…" ) );
  check.collectErrors( featurePools, checkErrors, messages, feedback );

  multiStepFeedback.setCurrentStep( 3 );
  feedback->setProgressText( QObject::tr( "Exporting errors…" ) );
  double step { checkErrors.size() > 0 ? 100.0 / checkErrors.size() : 1 };
  long i = 0;
  feedback->setProgress( 0.0 );


  for ( QgsGeometryCheckError *error : checkErrors )
  {
    if ( feedback->isCanceled() )
      break;

    QgsFeature f;
    QgsAttributes attrs = f.attributes();

    attrs << error->layerId()
          << mInputLayer->name()
          << error->featureId()
          << error->vidx().part
          << error->vidx().ring
          << error->vidx().vertex
          << error->location().x()
          << error->location().y()
          << error->value().toString();
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
