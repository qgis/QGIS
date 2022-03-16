/***************************************************************************
                         qgsalgorithmflattenrelationships.h
                         ---------------------
    begin                : August 2020
    copyright            : (C) 2020 by Nyall Dawson
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

#include "qgsalgorithmflattenrelationships.h"
#include "qgsrelationmanager.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerfeatureiterator.h"

///@cond PRIVATE

QString QgsFlattenRelationshipsAlgorithm::name() const
{
  return QStringLiteral( "flattenrelationships" );
}

QString QgsFlattenRelationshipsAlgorithm::displayName() const
{
  return QObject::tr( "Flatten relationship" );
}

QStringList QgsFlattenRelationshipsAlgorithm::tags() const
{
  return QObject::tr( "join,export,single,table" ).split( ',' );
}

QString QgsFlattenRelationshipsAlgorithm::group() const
{
  return QObject::tr( "Vector general" );
}

QString QgsFlattenRelationshipsAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeneral" );
}

QString QgsFlattenRelationshipsAlgorithm::shortDescription() const
{
  return QObject::tr( "Flatten a relationship for a vector layer." );
}

QString QgsFlattenRelationshipsAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm flattens a relationship for a vector layer, exporting a single layer "
                      "containing one master feature per related feature. This master feature contains all "
                      "the attributes for the related features." );
}

QgsProcessingAlgorithm::Flags QgsFlattenRelationshipsAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | FlagRequiresProject;
}

void QgsFlattenRelationshipsAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterVectorLayer( QStringLiteral( "INPUT" ),
                QObject::tr( "Input layer" ), QList< int>() << QgsProcessing::TypeVector ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Flattened layer" ), QgsProcessing::TypeVectorAnyGeometry ) );
}

QgsFlattenRelationshipsAlgorithm *QgsFlattenRelationshipsAlgorithm::createInstance() const
{
  return new QgsFlattenRelationshipsAlgorithm();
}

bool QgsFlattenRelationshipsAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsProject *project = context.project();
  if ( !project )
    throw QgsProcessingException( QObject::tr( "No project available for relationships" ) );

  QgsVectorLayer *layer = parameterAsVectorLayer( parameters, QStringLiteral( "INPUT" ), context );
  if ( !layer )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  const QList<QgsRelation> relations = project->relationManager()->referencedRelations( layer );
  if ( relations.size() > 1 )
    throw QgsProcessingException( QObject::tr( "Found %n relation(s). This algorithm currently supports only a single relation.", nullptr, relations.size() ) );
  else if ( relations.empty() )
    throw QgsProcessingException( QObject::tr( "No relations found." ) );

  mRelation = relations.at( 0 );

  QgsVectorLayer *referencingLayer = mRelation.referencingLayer();
  if ( !referencingLayer )
    throw QgsProcessingException( QObject::tr( "Could not resolved referenced layer." ) );

  mReferencingSource = std::make_unique< QgsVectorLayerFeatureSource >( referencingLayer );
  mReferencingFields = referencingLayer->fields();

  return true;
}

QVariantMap QgsFlattenRelationshipsAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsProcessingFeatureSource > input( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !input )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  const QgsFields outFields = QgsProcessingUtils::combineFields( input->fields(), mReferencingFields );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, outFields,
                                          input->wkbType(), input->sourceCrs(), QgsFeatureSink::RegeneratePrimaryKey ) );
  if ( parameters.value( QStringLiteral( "OUTPUT" ) ).isValid() && !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  // Create output vector layer with additional attributes
  const double step = input->featureCount() > 0 ? 100.0 / input->featureCount() : 1;
  QgsFeatureIterator features = input->getFeatures( QgsFeatureRequest(), QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks );
  long long i = 0;
  QgsFeature feat;
  while ( features.nextFeature( feat ) )
  {
    i++;
    if ( feedback->isCanceled() )
    {
      break;
    }

    feedback->setProgress( i * step );

    QgsFeatureRequest referencingRequest = mRelation.getRelatedFeaturesRequest( feat );
    referencingRequest.setFlags( referencingRequest.flags() | QgsFeatureRequest::NoGeometry );
    QgsFeatureIterator childIt = mReferencingSource->getFeatures( referencingRequest );
    QgsFeature relatedFeature;
    while ( childIt.nextFeature( relatedFeature ) )
    {
      QgsAttributes attrs = feat.attributes();
      attrs.append( relatedFeature.attributes() );
      QgsFeature outFeat = feat;
      outFeat.setAttributes( attrs );
      if ( !sink->addFeature( outFeat, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
    }
  }

  QVariantMap outputs;
  if ( sink )
    outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}


///@endcond
