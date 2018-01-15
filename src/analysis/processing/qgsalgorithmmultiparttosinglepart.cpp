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

///@cond PRIVATE

QString QgsMultipartToSinglepartAlgorithm::name() const
{
  return QStringLiteral( "multiparttosingleparts" );
}

QString QgsMultipartToSinglepartAlgorithm::displayName() const
{
  return QObject::tr( "Multipart to singleparts" );
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

QgsProcessingAlgorithm::Flags QgsMultipartToSinglepartAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | QgsProcessingAlgorithm::FlagCanRunInBackground;
}

void QgsMultipartToSinglepartAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Single parts" ) ) );
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

QVariantMap QgsMultipartToSinglepartAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    return QVariantMap();

  QgsWkbTypes::Type sinkType = QgsWkbTypes::singleType( source->wkbType() );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, source->fields(),
                                          sinkType, source->sourceCrs() ) );
  if ( !sink )
    return QVariantMap();

  long count = source->featureCount();

  QgsFeature f;
  QgsFeatureIterator it = source->getFeatures();

  double step = count > 0 ? 100.0 / count : 1;
  int current = 0;
  while ( it.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    QgsFeature out = f;
    if ( out.hasGeometry() )
    {
      QgsGeometry inputGeometry = f.geometry();
      if ( inputGeometry.isMultipart() )
      {
        Q_FOREACH ( const QgsGeometry &g, inputGeometry.asGeometryCollection() )
        {
          out.setGeometry( g );
          sink->addFeature( out, QgsFeatureSink::FastInsert );
        }
      }
      else
      {
        sink->addFeature( out, QgsFeatureSink::FastInsert );
      }
    }
    else
    {
      // feature with null geometry
      sink->addFeature( out, QgsFeatureSink::FastInsert );
    }

    feedback->setProgress( current * step );
    current++;
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}


///@endcond


