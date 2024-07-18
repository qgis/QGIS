/***************************************************************************
                         qgsalgorithmspatialindex.cpp
                         ---------------------
    begin                : November 2019
    copyright            : (C) 2019 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmspatialindex.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"

///@cond PRIVATE

QString QgsSpatialIndexAlgorithm::name() const
{
  return QStringLiteral( "createspatialindex" );
}

QString QgsSpatialIndexAlgorithm::displayName() const
{
  return QObject::tr( "Create spatial index" );
}

QStringList QgsSpatialIndexAlgorithm::tags() const
{
  return QObject::tr( "table,spatial,geometry,index,create,vector" ).split( ',' );
}

QString QgsSpatialIndexAlgorithm::group() const
{
  return QObject::tr( "Vector general" );
}

QString QgsSpatialIndexAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeneral" );
}

Qgis::ProcessingAlgorithmFlags QgsSpatialIndexAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::NoThreading;
}


QString QgsSpatialIndexAlgorithm::shortHelpString() const
{
  return QObject::tr( "Creates an index to speed up access to the features "
                      "in a layer based on their spatial location. Support "
                      "for spatial index creation is dependent on the layer's "
                      "data provider." );
}

QgsSpatialIndexAlgorithm *QgsSpatialIndexAlgorithm::createInstance() const
{
  return new QgsSpatialIndexAlgorithm();
}

void QgsSpatialIndexAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterVectorLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );

  addOutput( new QgsProcessingOutputVectorLayer( QStringLiteral( "OUTPUT" ), QObject::tr( "Indexed layer" ) ) );
}

QVariantMap QgsSpatialIndexAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsVectorLayer *layer = parameterAsVectorLayer( parameters, QStringLiteral( "INPUT" ), context );

  if ( !layer )
    throw QgsProcessingException( QObject::tr( "Could not load source layer for %1." ).arg( QLatin1String( "INPUT" ) ) );

  QgsVectorDataProvider *provider = layer->dataProvider();

  if ( provider->capabilities() & Qgis::VectorProviderCapability::CreateSpatialIndex )
  {
    if ( !provider->createSpatialIndex() )
    {
      feedback->pushInfo( QObject::tr( "Could not create spatial index" ) );
    }
  }
  else
  {
    feedback->pushInfo( QObject::tr( "Layer's data provider does not support spatial indexes" ) );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), layer->id() );
  return outputs;
}

///@endcond
