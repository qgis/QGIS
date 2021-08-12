/***************************************************************************
                         qgsalgorithmmultiparttosinglepart.cpp
                         ---------------------
    begin                : April 2017
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

#include "qgsalgorithmmultiparttosinglepart.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE

QString QgsMultipartToSinglepartAlgorithm::name() const
{
  return QStringLiteral( "multiparttosingleparts" );
}

QString QgsMultipartToSinglepartAlgorithm::displayName() const
{
  return QObject::tr( "Multipart to singleparts" );
}

QString QgsMultipartToSinglepartAlgorithm::outputName() const
{
  return QObject::tr( "Single parts" );
}

QgsWkbTypes::Type QgsMultipartToSinglepartAlgorithm::outputWkbType( QgsWkbTypes::Type inputWkbType ) const
{
  return QgsWkbTypes::singleType( inputWkbType );
}

QStringList QgsMultipartToSinglepartAlgorithm::tags() const
{
  return QObject::tr( "multi,single,multiple,split,dump" ).split( ',' );
}

QString QgsMultipartToSinglepartAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsMultipartToSinglepartAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsMultipartToSinglepartAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes a vector layer with multipart geometries and generates a new one in which all geometries contain "
                      "a single part. Features with multipart geometries are divided in as many different features as parts the geometry "
                      "contain, and the same attributes are used for each of them." );
}

QgsMultipartToSinglepartAlgorithm *QgsMultipartToSinglepartAlgorithm::createInstance() const
{
  return new QgsMultipartToSinglepartAlgorithm();
}


QgsProcessingFeatureSource::Flag QgsMultipartToSinglepartAlgorithm::sourceFlags() const
{
  // skip geometry checks - this algorithm can be used to repair geometries
  return QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks;
}

QgsFeatureSink::SinkFlags QgsMultipartToSinglepartAlgorithm::sinkFlags() const
{
  return QgsFeatureSink::RegeneratePrimaryKey;
}

QgsFeatureList QgsMultipartToSinglepartAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback * )
{
  if ( !feature.hasGeometry() )
    return QgsFeatureList() << feature;

  const QgsGeometry inputGeometry = feature.geometry();
  QgsFeatureList outputs;
  if ( inputGeometry.isMultipart() )
  {
    const QVector<QgsGeometry> parts = inputGeometry.asGeometryCollection();
    for ( const QgsGeometry &g : parts )
    {
      QgsFeature out;
      out.setAttributes( feature.attributes() );
      out.setGeometry( g );
      outputs.append( out );
    }
  }
  else
  {
    outputs.append( feature );
  }
  return outputs;
}


///@endcond


