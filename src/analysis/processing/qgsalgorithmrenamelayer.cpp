/***************************************************************************
                         qgsalgorithmrenamelayer.cpp
                         ---------------------
    begin                : November 2017
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

#include "qgsalgorithmrenamelayer.h"

///@cond PRIVATE

QString QgsRenameLayerAlgorithm::name() const
{
  return QStringLiteral( "renamelayer" );
}

QgsProcessingAlgorithm::Flags QgsRenameLayerAlgorithm::flags() const
{
  return FlagHideFromToolbox | FlagCanRunInBackground;
}

QString QgsRenameLayerAlgorithm::displayName() const
{
  return QObject::tr( "Rename layer" );
}

QStringList QgsRenameLayerAlgorithm::tags() const
{
  return QObject::tr( "change,layer,name,title" ).split( ',' );
}

QString QgsRenameLayerAlgorithm::group() const
{
  return QObject::tr( "Modeler tools" );
}

QString QgsRenameLayerAlgorithm::groupId() const
{
  return QStringLiteral( "modelertools" );
}

QString QgsRenameLayerAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm renames a layer." );
}

QgsRenameLayerAlgorithm *QgsRenameLayerAlgorithm::createInstance() const
{
  return new QgsRenameLayerAlgorithm();
}

void QgsRenameLayerAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMapLayer( QStringLiteral( "INPUT" ), QObject::tr( "Layer" ) ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "NAME" ), QObject::tr( "New name" ) ) );
  addOutput( new QgsProcessingOutputMapLayer( QStringLiteral( "OUTPUT" ), QObject::tr( "Layer" ) ) );
}

QVariantMap QgsRenameLayerAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsMapLayer *layer = parameterAsLayer( parameters, QStringLiteral( "INPUT" ), context );
  QString name = parameterAsString( parameters, QStringLiteral( "NAME" ), context );

  if ( !layer )
    throw QgsProcessingException( QObject::tr( "Invalid input layer" ) );

  if ( name.isEmpty() )
    throw QgsProcessingException( QObject::tr( "Invalid (empty) layer name" ) );

  bool parameterWasLayerName = parameters.value( QStringLiteral( "INPUT" ) ).toString() == layer->name();

  layer->setName( name );
  QVariantMap results;
  if ( parameterWasLayerName )
    results.insert( QStringLiteral( "OUTPUT" ), name );
  else
    results.insert( QStringLiteral( "OUTPUT" ), parameters.value( QStringLiteral( "INPUT" ) ) );

  return results;
}

///@endcond
