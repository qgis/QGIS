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
  return u"transferannotationsfrommain"_s;
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
  return u"cartography"_s;
}

Qgis::ProcessingAlgorithmFlags QgsTransferAnnotationsFromMainAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::NoThreading | Qgis::ProcessingAlgorithmFlag::RequiresProject;
}

QString QgsTransferAnnotationsFromMainAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm transfers all annotations from the main annotation layer in a project to a new annotation layer." );
}

QString QgsTransferAnnotationsFromMainAlgorithm::shortDescription() const
{
  return QObject::tr( "Transfers all annotations from the main annotation layer in a project to a new annotation layer." );
}

QgsTransferAnnotationsFromMainAlgorithm *QgsTransferAnnotationsFromMainAlgorithm::createInstance() const
{
  return new QgsTransferAnnotationsFromMainAlgorithm();
}

void QgsTransferAnnotationsFromMainAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterString( u"LAYER_NAME"_s, QObject::tr( "New layer name" ), QObject::tr( "Annotations" ) ) );

  addOutput( new QgsProcessingOutputMapLayer( u"OUTPUT"_s, QObject::tr( "New annotation layer" ) ) );
}

QVariantMap QgsTransferAnnotationsFromMainAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  if ( !context.project() )
    throw QgsProcessingException( QObject::tr( "No project available." ) );

  QgsAnnotationLayer *main = context.project()->mainAnnotationLayer();
  if ( !main )
    throw QgsProcessingException( QObject::tr( "Could not load main annotation layer for project." ) );

  std::unique_ptr<QgsAnnotationLayer> newLayer( main->clone() );
  newLayer->setName( parameterAsString( parameters, u"LAYER_NAME"_s, context ) );
  main->clear();

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, newLayer->id() );

  context.project()->addMapLayer( newLayer.release() );

  return outputs;
}

///@endcond
