/***************************************************************************
                         qgsalgorithmattributeraster.cpp
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

#include "qgsalgorithmattributeindex.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"

///@cond PRIVATE

QString QgsAttributeIndexAlgorithm::name() const
{
  return QStringLiteral( "createattributeindex" );
}

QString QgsAttributeIndexAlgorithm::displayName() const
{
  return QObject::tr( "Create attribute index" );
}

QStringList QgsAttributeIndexAlgorithm::tags() const
{
  return QObject::tr( "table,attribute,index,create,vector" ).split( ',' );
}

QString QgsAttributeIndexAlgorithm::group() const
{
  return QObject::tr( "Vector general" );
}

QString QgsAttributeIndexAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeneral" );
}

QgsProcessingAlgorithm::Flags QgsAttributeIndexAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | QgsProcessingAlgorithm::FlagNoThreading;
}


QString QgsAttributeIndexAlgorithm::shortHelpString() const
{
  return QObject::tr( "Creates an index to speed up queries made against "
                      "a field in a table. Support for index creation is "
                      "dependent on the layer's data provider and the field type." );
}

QgsAttributeIndexAlgorithm *QgsAttributeIndexAlgorithm::createInstance() const
{
  return new QgsAttributeIndexAlgorithm();
}

void QgsAttributeIndexAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterVectorLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "FIELD" ), QObject::tr( "Attribute to index" ), QVariant(), QStringLiteral( "INPUT" ) ) );

  addOutput( new QgsProcessingOutputVectorLayer( QStringLiteral( "OUTPUT" ), QObject::tr( "Indexed layer" ) ) );
}

QVariantMap QgsAttributeIndexAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsVectorLayer *layer = parameterAsVectorLayer( parameters, QStringLiteral( "INPUT" ), context );

  if ( !layer )
    throw QgsProcessingException( QObject::tr( "Could not load source layer for %1." ).arg( QLatin1String( "INPUT" ) ) );

  const QString field = parameterAsString( parameters, QStringLiteral( "FIELD" ), context );

  QgsVectorDataProvider *provider = layer->dataProvider();

  const int fieldIndex = layer->fields().lookupField( field );
  if ( fieldIndex < 0 || layer->fields().fieldOrigin( fieldIndex ) != QgsFields::OriginProvider )
  {
    feedback->pushInfo( QObject::tr( "Can not create attribute index on %1" ).arg( field ) );
  }
  else
  {
    const int providerIndex = layer->fields().fieldOriginIndex( fieldIndex );
    if ( provider->capabilities() & QgsVectorDataProvider::CreateAttributeIndex )
    {
      if ( !provider->createAttributeIndex( providerIndex ) )
      {
        feedback->pushInfo( QObject::tr( "Could not create attribute index" ) );
      }
    }
    else
    {
      feedback->pushInfo( QObject::tr( "Layer's data provider does not support creating attribute indexes" ) );
    }
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), layer->id() );
  return outputs;
}

///@endcond
