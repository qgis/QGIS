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

///@cond PRIVATE

QgsProcessingAlgorithm::Flags QgsSaveSelectedFeatures::flags() const
{
  return QgsProcessingAlgorithm::flags() | QgsProcessingAlgorithm::FlagCanRunInBackground;
}

void QgsSaveSelectedFeatures::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterVectorLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Selected features" ), QgsProcessing::TypeVectorPoint ) );
}

QString QgsSaveSelectedFeatures::name() const
{
  return QStringLiteral( "saveselectedfeatures" );
}

QString QgsSaveSelectedFeatures::displayName() const
{
  return QObject::tr( "Save Selected Features" );
}

QStringList QgsSaveSelectedFeatures::tags() const
{
  return QObject::tr( "selection,save" ).split( ',' );
}

QString QgsSaveSelectedFeatures::group() const
{
  return QObject::tr( "Vector general" );
}

QString QgsSaveSelectedFeatures::groupId() const
{
  return QStringLiteral( "vectorgeneral" );
}

QString QgsSaveSelectedFeatures::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a new layer with all the selected features in a given vector layer.\n\n"
                      "If the selected layer has no selected features, the newly created layer will be empty." );
}

QgsSaveSelectedFeatures *QgsSaveSelectedFeatures::createInstance() const
{
  return new QgsSaveSelectedFeatures();
}

QVariantMap QgsSaveSelectedFeatures::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsVectorLayer *selectLayer = parameterAsVectorLayer( parameters, QStringLiteral( "INPUT" ), context );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, selectLayer->fields(), selectLayer->wkbType(), selectLayer->sourceCrs() ) );
  if ( !sink )
    return QVariantMap();


  int count = selectLayer->selectedFeatureCount();
  int current = 0;
  double step = count > 0 ? 100.0 / count : 1;

  QgsFeatureIterator it = selectLayer->getSelectedFeatures();;
  QgsFeature feat;
  while ( it.nextFeature( feat ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    sink->addFeature( feat, QgsFeatureSink::FastInsert );

    feedback->setProgress( current++ * step );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

///@endcond



