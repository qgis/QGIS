/***************************************************************************
                         qgsalgorithmconditionalmerge.cpp
                         ---------------------
    begin                : September 2021
    copyright            : (C) 2021 by Antoine Facchini
    email                : antoine dot facchini @oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmconditionalmerge.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE

QString QgsConditionalMergeAlgorithm::name() const
{
  return QStringLiteral( "conditionmerge" );
}

QString QgsConditionalMergeAlgorithm::displayName() const
{
  return QObject::tr( "Conditional merge" );
}

QStringList QgsConditionalMergeAlgorithm::tags() const
{
  return QObject::tr( "if,logic,test,merge" ).split( ',' );
}

QString QgsConditionalMergeAlgorithm::group() const
{
  return QObject::tr( "Modeler tools" );
}

QString QgsConditionalMergeAlgorithm::groupId() const
{
  return QStringLiteral( "modelertools" );
}

QString QgsConditionalMergeAlgorithm::shortHelpString() const
{
  return QObject::tr( "TODO" );
}

QgsConditionalMergeAlgorithm *QgsConditionalMergeAlgorithm::createInstance() const
{
  return new QgsConditionalMergeAlgorithm();
}

QgsProcessingAlgorithm::Flags QgsConditionalMergeAlgorithm::flags() const
{
  return FlagHideFromToolbox;
}

void QgsConditionalMergeAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterExpression( QStringLiteral( "EXPRESSION" ), QObject::tr( "Routing expression" ),
                QStringLiteral( "" ), QStringLiteral( "INPUT" ) ) );
  addParameter( new QgsProcessingParameterVectorLayer( QStringLiteral( "LAYER_IF" ), QObject::tr( "Layer to transfer if condition" ), QList< int >(), QVariant(), true ) );
  addParameter( new QgsProcessingParameterVectorLayer( QStringLiteral( "LAYER_ELSE" ), QObject::tr( "Layer to transfer else" ), QList< int >(), QVariant(), true ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Layer" ) ) );
}

bool QgsConditionalMergeAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  mExpression = QgsExpression( parameterAsString( parameters, QStringLiteral( "EXPRESSION" ), context ) );
  if ( mExpression.hasParserError() )
  {
    feedback->reportError( mExpression.parserErrorString() );
    return false;
  }

  mExpressionContext = createExpressionContext( parameters, context );
  mExpression.prepare( &mExpressionContext );

  mLayerIf = parameterAsVectorLayer( parameters, QStringLiteral( "LAYER_IF" ), context );
  mLayerElse = parameterAsVectorLayer( parameters, QStringLiteral( "LAYER_ELSE" ), context );

  return true;
}


QVariantMap QgsConditionalMergeAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsVectorLayer *selectedLayer = nullptr;

  const bool res = mExpression.evaluate( &mExpressionContext ).toBool();
  if ( mLayerIf && res )
  {
    selectedLayer = mLayerIf;
  }
  else if ( mLayerElse )
  {
    selectedLayer = mLayerElse;
  }
  else
  {
    throw QgsProcessingException( QStringLiteral( "No valid layer" ) );
  }

  QString sinkId;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, sinkId, selectedLayer->fields(),
                                          selectedLayer->wkbType(), selectedLayer->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  QgsFeatureIterator featureIt = selectedLayer->getFeatures();
  sink->addFeatures( featureIt, QgsFeatureSink::FastInsert );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), sinkId );
  return outputs;
}


///@endcond PRIVATE
