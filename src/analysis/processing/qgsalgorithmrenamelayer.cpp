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
  return u"renamelayer"_s;
}

Qgis::ProcessingAlgorithmFlags QgsRenameLayerAlgorithm::flags() const
{
  return Qgis::ProcessingAlgorithmFlag::HideFromToolbox;
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
  return u"modelertools"_s;
}

QString QgsRenameLayerAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm renames a layer." );
}

QString QgsRenameLayerAlgorithm::shortDescription() const
{
  return QObject::tr( "Renames a layer." );
}

QgsRenameLayerAlgorithm *QgsRenameLayerAlgorithm::createInstance() const
{
  return new QgsRenameLayerAlgorithm();
}

void QgsRenameLayerAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMapLayer( u"INPUT"_s, QObject::tr( "Layer" ) ) );
  addParameter( new QgsProcessingParameterString( u"NAME"_s, QObject::tr( "New name" ) ) );
  addOutput( new QgsProcessingOutputMapLayer( u"OUTPUT"_s, QObject::tr( "Layer" ) ) );
}

QVariantMap QgsRenameLayerAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsMapLayer *layer = parameterAsLayer( parameters, u"INPUT"_s, context );
  const QString name = parameterAsString( parameters, u"NAME"_s, context );

  if ( !layer )
    throw QgsProcessingException( QObject::tr( "Invalid input layer" ) );

  if ( name.isEmpty() )
    throw QgsProcessingException( QObject::tr( "Invalid (empty) layer name" ) );

  const bool parameterWasLayerName = parameters.value( u"INPUT"_s ).toString() == layer->name();

  layer->setName( name );
  QVariantMap results;
  if ( parameterWasLayerName )
    results.insert( u"OUTPUT"_s, name );
  else
    results.insert( u"OUTPUT"_s, parameters.value( u"INPUT"_s ) );

  return results;
}

///@endcond
