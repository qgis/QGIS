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

#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE

QString QgsAttributeIndexAlgorithm::name() const
{
  return u"createattributeindex"_s;
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
  return u"vectorgeneral"_s;
}

Qgis::ProcessingAlgorithmFlags QgsAttributeIndexAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::NoThreading;
}


QString QgsAttributeIndexAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates an index to speed up queries made against "
                      "a field in a table. Support for index creation is "
                      "dependent on the layer's data provider and the field type." );
}

QString QgsAttributeIndexAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates an index to speed up queries made against a field in a table." );
}

QgsAttributeIndexAlgorithm *QgsAttributeIndexAlgorithm::createInstance() const
{
  return new QgsAttributeIndexAlgorithm();
}

void QgsAttributeIndexAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterVectorLayer( u"INPUT"_s, QObject::tr( "Input layer" ), QList<int> { static_cast<int>( Qgis::ProcessingSourceType::Vector ) } ) );
  addParameter( new QgsProcessingParameterField( u"FIELD"_s, QObject::tr( "Attribute to index" ), QVariant(), u"INPUT"_s ) );

  addOutput( new QgsProcessingOutputVectorLayer( u"OUTPUT"_s, QObject::tr( "Indexed layer" ) ) );
}

QVariantMap QgsAttributeIndexAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsVectorLayer *layer = parameterAsVectorLayer( parameters, u"INPUT"_s, context );

  if ( !layer )
    throw QgsProcessingException( QObject::tr( "Could not load source layer for %1." ).arg( "INPUT"_L1 ) );

  const QString field = parameterAsString( parameters, u"FIELD"_s, context );

  QgsVectorDataProvider *provider = layer->dataProvider();

  const int fieldIndex = layer->fields().lookupField( field );
  if ( fieldIndex < 0 || layer->fields().fieldOrigin( fieldIndex ) != Qgis::FieldOrigin::Provider )
  {
    feedback->pushInfo( QObject::tr( "Can not create attribute index on %1" ).arg( field ) );
  }
  else
  {
    const int providerIndex = layer->fields().fieldOriginIndex( fieldIndex );
    if ( provider->capabilities() & Qgis::VectorProviderCapability::CreateAttributeIndex )
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
  outputs.insert( u"OUTPUT"_s, layer->id() );
  return outputs;
}

///@endcond
