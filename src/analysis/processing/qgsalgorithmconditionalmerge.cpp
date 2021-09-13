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
  addParameter( new QgsProcessingParameterMapLayer( QStringLiteral( "DEFAULT_INPUT" ), QObject::tr( "Input to transfer if exists" ) ) );
  addParameter( new QgsProcessingParameterMapLayer( QStringLiteral( "FALLBACK_INPUT" ), QObject::tr( "Input to transfer else" ) ) );

  addOutput( new QgsProcessingOutputMapLayer( QStringLiteral( "OUTPUT" ), QObject::tr( "Layer" ) ) );
}

bool QgsConditionalMergeAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mDefaultInput = parameterAsLayer( parameters, QStringLiteral( "DEFAULT_INPUT" ), context );
  mFallbackInput = parameterAsLayer( parameters, QStringLiteral( "FALLBACK_INPUT" ), context );

  return true;
}


QVariantMap QgsConditionalMergeAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  Q_UNUSED( parameters );
  Q_UNUSED( context );

  QString layerId;
  if ( mDefaultInput )
  {
    layerId = mDefaultInput->id();
  }
  else if ( mFallbackInput )
  {
    layerId = mFallbackInput->id();
  }
  else
  {
    throw QgsProcessingException( QStringLiteral( "No valid input" ) );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), layerId );
  return outputs;
}


///@endcond PRIVATE
