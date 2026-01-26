/***************************************************************************
                         qgsalgorithmtruncatetable.cpp
                         ---------------------
    begin                : December 2019
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

#include "qgsalgorithmtruncatetable.h"

#include "qgsvectorlayer.h"

///@cond PRIVATE

QString QgsTruncateTableAlgorithm::name() const
{
  return u"truncatetable"_s;
}

QString QgsTruncateTableAlgorithm::displayName() const
{
  return QObject::tr( "Truncate table" );
}

QStringList QgsTruncateTableAlgorithm::tags() const
{
  return QObject::tr( "empty,delete,layer,clear,features" ).split( ',' );
}

QString QgsTruncateTableAlgorithm::group() const
{
  return QObject::tr( "Vector general" );
}

QString QgsTruncateTableAlgorithm::groupId() const
{
  return u"vectorgeneral"_s;
}

QString QgsTruncateTableAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm truncates a layer, by deleting all features from within the layer." )
         + u"\n\n"_s
         + QObject::tr( "Warning — this algorithm modifies the layer in place, and deleted features cannot be restored!" );
}

QString QgsTruncateTableAlgorithm::shortDescription() const
{
  return QObject::tr( "Truncates a layer, by deleting all features from within the layer." );
}

Qgis::ProcessingAlgorithmFlags QgsTruncateTableAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | Qgis::ProcessingAlgorithmFlag::NoThreading;
}

QgsTruncateTableAlgorithm *QgsTruncateTableAlgorithm::createInstance() const
{
  return new QgsTruncateTableAlgorithm();
}

void QgsTruncateTableAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterVectorLayer( u"INPUT"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) ) );
  addOutput( new QgsProcessingOutputVectorLayer( u"OUTPUT"_s, QObject::tr( "Truncated layer" ) ) );
}

QVariantMap QgsTruncateTableAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsVectorLayer *layer = parameterAsVectorLayer( parameters, u"INPUT"_s, context );

  if ( !layer )
    throw QgsProcessingException( QObject::tr( "Invalid input layer" ) );

  if ( !layer->dataProvider()->truncate() )
  {
    throw QgsProcessingException( QObject::tr( "Could not truncate table." ) );
  }

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, layer->id() );
  return outputs;
}

///@endcond
