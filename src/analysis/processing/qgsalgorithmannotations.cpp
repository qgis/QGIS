/***************************************************************************
                         qgsalgorithmannotations.cpp
                         ------------------------------
    begin                : September 2021
    copyright            : (C) 2021 by Nyall Dawson
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

#include "qgsalgorithmannotations.h"
#include "qgsannotationlayer.h"

///@cond PRIVATE

QString QgsTransferAnnotationsFromMainAlgorithm::name() const
{
  return QStringLiteral( "transferannotationsfrommain" );
}

QString QgsTransferAnnotationsFromMainAlgorithm::displayName() const
{
  return QObject::tr( "Transfer annotations from main layer" );
}

QStringList QgsTransferAnnotationsFromMainAlgorithm::tags() const
{
  return QObject::tr( "annotations,drawing,cosmetic,objects" ).split( ',' );
}

QString QgsTransferAnnotationsFromMainAlgorithm::group() const
{
  return QObject::tr( "Cartography" );
}

QString QgsTransferAnnotationsFromMainAlgorithm::groupId() const
{
  return QStringLiteral( "cartography" );
}

QgsProcessingAlgorithm::Flags QgsTransferAnnotationsFromMainAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | QgsProcessingAlgorithm::FlagNoThreading | QgsProcessingAlgorithm::FlagRequiresProject;
}


QString QgsTransferAnnotationsFromMainAlgorithm::shortHelpString() const
{
  return QObject::tr( "Transfer all annotations from the main annotation layer in a project to a new annotation layer." );
}

QgsTransferAnnotationsFromMainAlgorithm *QgsTransferAnnotationsFromMainAlgorithm::createInstance() const
{
  return new QgsTransferAnnotationsFromMainAlgorithm();
}

void QgsTransferAnnotationsFromMainAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterString( QStringLiteral( "LAYER_NAME" ), QObject::tr( "New layer name" ), QObject::tr( "Annotations" ) ) );

  addOutput( new QgsProcessingOutputMapLayer( QStringLiteral( "OUTPUT" ), QObject::tr( "New annotation layer" ) ) );
}

QVariantMap QgsTransferAnnotationsFromMainAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  if ( !context.project() )
    throw QgsProcessingException( QObject::tr( "No project available." ) );

  QgsAnnotationLayer *main = context.project()->mainAnnotationLayer();
  if ( !main )
    throw QgsProcessingException( QObject::tr( "Could not load main annotation layer for project." ) );

  std::unique_ptr< QgsAnnotationLayer > newLayer( main->clone() );
  newLayer->setName( parameterAsString( parameters, QStringLiteral( "LAYER_NAME" ), context ) );
  main->clear();

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), newLayer->id() );

  context.project()->addMapLayer( newLayer.release() );

  return outputs;
}

///@endcond
