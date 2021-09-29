/***************************************************************************
                         qgsalgorithmbranchmerge.cpp
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

#include "qgsalgorithmbranchmerge.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE



QString QgsBranchMergeAlgorithm::name() const
{
  return QStringLiteral( "branchmerger" );
}

QString QgsBranchMergeAlgorithm::displayName() const
{
  return QObject::tr( "Branch merger" );
}

QStringList QgsBranchMergeAlgorithm::tags() const
{
  return QObject::tr( "branch,test,merge" ).split( ',' );
}

QString QgsBranchMergeAlgorithm::group() const
{
  return QObject::tr( "Modeler tools" );
}

QString QgsBranchMergeAlgorithm::groupId() const
{
  return QStringLiteral( "modelertools" );
}

QString QgsBranchMergeAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm merges 2 branches by transferring one of 2 input layers to the output. "
                      "It checks if the layer of the first branch is valid before transferring. "
                      "If not, it does the same process with the second layer. "
                      "If it is not valid either, an error is raised."
                    );
}

QString QgsBranchMergeAlgorithm::shortDescription() const
{
  return QObject::tr( "Merges 2 branches into a model by selecting one of 2 input layers and transfers it to the output." );
}

QgsBranchMergeAlgorithm *QgsBranchMergeAlgorithm::createInstance() const
{
  return new QgsBranchMergeAlgorithm();
}

QgsProcessingAlgorithm::Flags QgsBranchMergeAlgorithm::flags() const
{
  return FlagHideFromToolbox;
}

void QgsBranchMergeAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMapLayer( QStringLiteral( "DEFAULT_INPUT" ), QObject::tr( "Layer to transfer if exists" ) ) );
  addParameter( new QgsProcessingParameterMapLayer( QStringLiteral( "FALLBACK_INPUT" ), QObject::tr( "Layer to transfer else" ) ) );

  addOutput( new QgsProcessingOutputMapLayer( QStringLiteral( "OUTPUT" ), QObject::tr( "Layer" ) ) );
}

bool QgsBranchMergeAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mDefaultInput = parameterAsLayer( parameters, QStringLiteral( "DEFAULT_INPUT" ), context );
  mFallbackInput = parameterAsLayer( parameters, QStringLiteral( "FALLBACK_INPUT" ), context );

  return true;
}


QVariantMap QgsBranchMergeAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( parameters );
  Q_UNUSED( context );
  Q_UNUSED( feedback );

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
