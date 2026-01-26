/***************************************************************************
                         qgsalgorithmsaveselectedfeatures.cpp
                         ---------------------
    begin                : April 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgsalgorithmsaveselectedfeatures.h"

#include "qgsvectorlayer.h"

///@cond PRIVATE

void QgsSaveSelectedFeatures::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterVectorLayer( u"INPUT"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) ) );
  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Selected features" ) ) );
}

QString QgsSaveSelectedFeatures::name() const
{
  return u"saveselectedfeatures"_s;
}

QString QgsSaveSelectedFeatures::displayName() const
{
  return QObject::tr( "Extract selected features" );
}

QStringList QgsSaveSelectedFeatures::tags() const
{
  return QObject::tr( "selection,save,by" ).split( ',' );
}

QString QgsSaveSelectedFeatures::group() const
{
  return QObject::tr( "Vector general" );
}

QString QgsSaveSelectedFeatures::groupId() const
{
  return u"vectorgeneral"_s;
}

QString QgsSaveSelectedFeatures::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a new layer with all the selected features in a given vector layer.\n\n"
                      "If the selected layer has no selected features, the newly created layer will be empty." );
}

QString QgsSaveSelectedFeatures::shortDescription() const
{
  return QObject::tr( "Creates a layer with all the selected features in a given vector layer." );
}

QgsSaveSelectedFeatures *QgsSaveSelectedFeatures::createInstance() const
{
  return new QgsSaveSelectedFeatures();
}

bool QgsSaveSelectedFeatures::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsVectorLayer *selectLayer = parameterAsVectorLayer( parameters, u"INPUT"_s, context );
  if ( !selectLayer )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  mSelection = selectLayer->selectedFeatureIds();
  return true;
}

QVariantMap QgsSaveSelectedFeatures::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsVectorLayer *selectLayer = parameterAsVectorLayer( parameters, u"INPUT"_s, context );
  if ( !selectLayer )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, selectLayer->fields(), selectLayer->wkbType(), selectLayer->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );


  const int count = mSelection.count();
  int current = 0;
  const double step = count > 0 ? 100.0 / count : 1;

  QgsFeatureIterator it = selectLayer->getFeatures( QgsFeatureRequest().setFilterFids( mSelection ) );
  QgsFeature feat;
  while ( it.nextFeature( feat ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    if ( !sink->addFeature( feat, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );

    feedback->setProgress( current++ * step );
  }

  sink->finalize();

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, dest );
  return outputs;
}

///@endcond
