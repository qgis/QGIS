/***************************************************************************
                        qgsalgorithmcheckgeometrymultipart.cpp
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

#include "qgsalgorithmcheckgeometrymultipart.h"
#include "qgsgeometrycheckcontext.h"
#include "qgsgeometrycheckerror.h"
#include "qgsgeometrymultipartcheck.h"
#include "qgspoint.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataproviderfeaturepool.h"

///@cond PRIVATE

auto QgsGeometryCheckMultipartAlgorithm::name() const -> QString
{
  return QStringLiteral( "checkgeometrymultipart" );
}

auto QgsGeometryCheckMultipartAlgorithm::displayName() const -> QString
{
  return QObject::tr( "Check Geometry (Multipart)" );
}

auto QgsGeometryCheckMultipartAlgorithm::tags() const -> QStringList
{
  return QObject::tr( "check,geometry,multipart" ).split( ',' );
}

auto QgsGeometryCheckMultipartAlgorithm::group() const -> QString
{
  return QObject::tr( "Check geometry" );
}

auto QgsGeometryCheckMultipartAlgorithm::groupId() const -> QString
{
  return QStringLiteral( "checkgeometry" );
}

auto QgsGeometryCheckMultipartAlgorithm::shortHelpString() const -> QString
{
  return QObject::tr( "This algorithm check the multipart of geometry." );
}

auto QgsGeometryCheckMultipartAlgorithm::flags() const -> QgsProcessingAlgorithm::Flags
{
  return QgsProcessingAlgorithm::flags() | QgsProcessingAlgorithm::FlagNoThreading | QgsProcessingAlgorithm::FlagSupportsInPlaceEdits;
}

auto QgsGeometryCheckMultipartAlgorithm::createInstance() const -> QgsGeometryCheckMultipartAlgorithm *
{
  return new QgsGeometryCheckMultipartAlgorithm();
}

static auto resolutionMethods() -> QStringList
{
  return QStringList() << QObject::tr( "Convert to single part feature" )
         << QObject::tr( "Delete feature" )
         << QObject::tr( "No action" );
}

void QgsGeometryCheckMultipartAlgorithm::initAlgorithm( const QVariantMap &configuration )
{

  mIsInPlace = configuration.value( QStringLiteral( "IN_PLACE" ) ).toBool();

  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ), QList< int >() << QgsProcessing::TypeVectorLine ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "ERRORS" ), QObject::tr( "Errors layer" ), QgsProcessing::TypeVectorPoint ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Output layer" ), QgsProcessing::TypeVectorLine ) );

  std::unique_ptr< QgsProcessingParameterNumber > tolerance = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "TOLERANCE" ),
      QObject::tr( "Tolerance" ), QgsProcessingParameterNumber::Integer, 8, false, 1, 13 );
  tolerance->setFlags( tolerance->flags() | QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( tolerance.release() );

  if ( mIsInPlace )
  {
    // resolutionMethod is deprecated and not static and availableResolutionmethod is not static either
    // We need a QStringList here, so copy/paste from resolutionMethod()
//  Q_NOWARN_DEPRECATED_PUSH
//  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "RESOLUTIONMETHOD"), QObject::tr( "Resolution method"), QgsGeometryMultipartCheck::resolutionMethods() ) );
//  Q_NOWARN_DEPRECATED_POP
    addParameter( new QgsProcessingParameterEnum( QStringLiteral( "RESOLUTIONMETHOD" ), QObject::tr( "Resolution method" ), resolutionMethods(), false, resolutionMethods().size() - 1 ) );
  }
}

auto QgsGeometryCheckMultipartAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * ) -> bool
{
  mTolerance = parameterAsInt( parameters, QStringLiteral( "TOLERANCE" ), context );

  return true;
}

auto QgsGeometryCheckMultipartAlgorithm::createFeaturePool( QgsVectorLayer *layer, bool selectedOnly ) const -> QgsFeaturePool *
{

  return new QgsVectorDataProviderFeaturePool( layer, selectedOnly );
}

static auto outputFields( ) -> QgsFields
{
  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "gc_layerid" ), QVariant::String ) );
  fields.append( QgsField( QStringLiteral( "gc_layername" ), QVariant::String ) );
  fields.append( QgsField( QStringLiteral( "gc_featid" ), QVariant::Int ) );
  fields.append( QgsField( QStringLiteral( "gc_partidx" ), QVariant::Int ) );
  fields.append( QgsField( QStringLiteral( "gc_ringidx" ), QVariant::Int ) );
  fields.append( QgsField( QStringLiteral( "gc_vertidx" ), QVariant::Int ) );
  fields.append( QgsField( QStringLiteral( "gc_errorx" ), QVariant::Double ) );
  fields.append( QgsField( QStringLiteral( "gc_errory" ), QVariant::Double ) );
  fields.append( QgsField( QStringLiteral( "gc_error" ), QVariant::String ) );
  return fields;
}


auto QgsGeometryCheckMultipartAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) -> QVariantMap
{
  QString dest_output;
  QString dest_errors;
  std::unique_ptr< QgsProcessingFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  mInputLayer.reset( source->materialize( QgsFeatureRequest() ) );

  if ( !mInputLayer.get() )
    throw QgsProcessingException( QObject::tr( "Could not load source layer for INPUT" ) );

  QgsFields fields = outputFields();

  std::unique_ptr< QgsFeatureSink > sink_output( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest_output, fields, mInputLayer->wkbType(), mInputLayer->sourceCrs() ) );
  if ( !sink_output )
  {
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );
  }
  std::unique_ptr< QgsFeatureSink > sink_errors( parameterAsSink( parameters, QStringLiteral( "ERRORS" ), context, dest_errors, fields, Qgis::WkbType::Point, mInputLayer->sourceCrs() ) );
  if ( !sink_errors )
  {
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "ERRORS" ) ) );
  }

  int resolutionMethod{-1};
  if ( mIsInPlace )
  {
    resolutionMethod = parameterAsEnum( parameters, QStringLiteral( "RESOLUTIONMETHOD" ), context );
  }

  QgsProcessingMultiStepFeedback multiStepFeedback( mIsInPlace ? 4 : 3, feedback );

  QgsProject *project = mInputLayer->project() ? mInputLayer->project() : QgsProject::instance();
  std::unique_ptr<QgsGeometryCheckContext> checkContext = std::make_unique<QgsGeometryCheckContext>( mTolerance, mInputLayer->sourceCrs(), project->transformContext(), project );

  // Test detection
  QList<QgsGeometryCheckError *> checkErrors;
  QStringList messages;

  const QgsGeometryMultipartCheck check( checkContext.get(), QVariantMap() );

  multiStepFeedback.setCurrentStep( 1 );
  feedback->setProgressText( QObject::tr( "Preparing features…" ) );
  QMap<QString, QgsFeaturePool *> featurePools;
  featurePools.insert( mInputLayer->id(), createFeaturePool( mInputLayer.get() ) );

  multiStepFeedback.setCurrentStep( 2 );
  feedback->setProgressText( QObject::tr( "Collecting errors…" ) );
  check.collectErrors( featurePools, checkErrors, messages, feedback );

  multiStepFeedback.setCurrentStep( 3 );
  feedback->setProgressText( QObject::tr( "Exporting errors…" ) );
  double step{checkErrors.size() > 0 ? 100.0 / checkErrors.size() : 1};
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
          << mInputLayer->name()
          << error->featureId()
          << error->vidx().part
          << error->vidx().ring
          << error->vidx().vertex
          << error->location().x()
          << error->location().y()
          << error->value().toString();
    f.setAttributes( attrs );

    if ( mIsInPlace )
    {
      QgsGeometryCheck::Changes changes;
      QMap<QString, int> mergeIndice;
      check.fixError( featurePools, error, resolutionMethod, mergeIndice, changes );
    }
    else
    {
      f.setGeometry( error->geometry() );
      if ( !sink_output->addFeature( f, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink_output.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
    }

    f.setGeometry( QgsGeometry::fromPoint( QgsPoint( error->location().x(), error->location().y() ) ) );
    if ( !sink_errors->addFeature( f, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink_errors.get(), parameters, QStringLiteral( "ERRORS" ) ) );

    i++;
    feedback->setProgress( 100.0 * step * static_cast<double>( i ) );
  }


  multiStepFeedback.setCurrentStep( 4 );
  feedback->setProgressText( QObject::tr( "Exporting (fixed) layer…" ) );

  if ( mIsInPlace )
  {
    const QgsFeaturePool *featurePool = featurePools[ mInputLayer->id() ];
    QgsFeatureIds featureIds{featurePool->allFeatureIds()};
    QgsFeatureIterator featIt{mInputLayer->getFeatures( featureIds )};

    step = featureIds.size() > 0 ? 100.0 / featureIds.size() : 0;
    feedback->setProgress( 100.0 * step );

    QgsFeature feat;
    while ( featIt.nextFeature( feat ) )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }

      if ( !sink_output->addFeature( feat, QgsFeatureSink::FastInsert ) )
      {
        throw QgsProcessingException( writeFeatureError( sink_output.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
      }
    }
  }
  else
  {
    QgsFeatureIterator featIt{mInputLayer->getFeatures( )};
    QgsFeature feat;
    while ( featIt.nextFeature( feat ) )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }

      if ( !sink_output->addFeature( feat, QgsFeatureSink::FastInsert ) )
      {
        throw QgsProcessingException( writeFeatureError( sink_output.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
      }
    }
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest_output );
  outputs.insert( QStringLiteral( "ERRORS" ), dest_errors );

  return outputs;
}

bool QgsGeometryCheckMultipartAlgorithm::supportInPlaceEdit( const QgsMapLayer *layer ) const
{
  if ( const QgsVectorLayer *vl = qobject_cast< const QgsVectorLayer * >( layer ) )
  {
    return vl->geometryType() == Qgis::GeometryType::Line;
  }
  return false;
}

///@endcond
